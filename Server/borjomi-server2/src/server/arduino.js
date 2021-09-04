const log4js = require("log4js");
const logger = log4js.getLogger('arduino')

const config = require('./config')
const { ErrorCodes, APIError } = require("../common/error")

const SerialPort = require('serialport')
const Readline = SerialPort.parsers.Readline

const SerialPortDelimiter = '\r\n'


/** @type SerialPort */
let serialport = null
const parser = new Readline({ delimiter : SerialPortDelimiter })


let channelBusy = {
    promise: Promise.resolve()
}

/**
 * Calls arduino board over serial port and returns raw answer from arduino
 * which is represented by single line
 * @param {String} cmd raw command 
 */
function callArduino(cmd, timeoutMs = 5000) {
    if (serialport == null) {
        throw new Error("SerialPort is not open")
    }
    let prevChannelPromise = channelBusy.promise
    let resolver = {}
    resolver.promise = new Promise(function(resolve, reject){
        resolver.resolve = resolve
    })
    channelBusy = resolver

    return new Promise(function(resolve, reject) {
        prevChannelPromise
        .then(reopenPort)
        .then( () => {
            let tmrId = setTimeout(function() {
                parser.off('data', on_parser_data)
                reject(new Error('Timeout occurred'))
                closePort()
                resolver.resolve()
            }, timeoutMs)
            function on_parser_data(data) {
                logger.trace("readed:", data.trim())
                parser.off('data', on_parser_data)
                clearTimeout(tmrId)
                resolve(data)
                resolver.resolve()
            }
            parser.on('data', on_parser_data)
            if (!cmd.endsWith(SerialPortDelimiter)) {
                cmd = cmd + SerialPortDelimiter
            }
            logger.trace("callArduino writing:", cmd.trim())
            serialport.write(cmd)
        })
        .catch(reject)
    });
}

/**
 * Calls arduino board over serial port and returns raw answer from arduino.
 * Answer is gathering during whole timeout period and contains 0+ lines
 * @param {String} cmd raw command 
 * @returns {Stringp[]} array of strings; from 0 and more rows
 */
function callArduinoTimeout(cmd, timeoutMs) {
    if (serialport == null) {
        throw new Error("SerialPort is not open")
    }
    if (!timeoutMs) {
        throw new Error("Timeout param must be set")
    }
    
    let prevChannelPromise = channelBusy.promise
    let resolver = {}
    resolver.promise = new Promise(function(resolve, reject){
        resolver.resolve = resolve
    })
    channelBusy = resolver

    return new Promise(function(resolve, reject) {
        prevChannelPromise
        .then(reopenPort)
        .then( () => {
            var ret = []
            function on_parser_data(data) {
                logger.trace("readed:", data.trim())
                ret.push(data)
            }
            parser.on('data', on_parser_data)
            if (!cmd.endsWith(SerialPortDelimiter)) {
                cmd = cmd + SerialPortDelimiter
            }
            logger.trace("callArduinoTimeout writing:", cmd.trim())
            serialport.write(cmd)
            setTimeout(function() {
                logger.trace("callArduinoTimeout finished")
                parser.off('data', on_parser_data)
                resolve(ret)
                resolver.resolve()
            }, timeoutMs)
        })
        .catch(reject)
    });
}

function formatArduinoError(rawErrorStr) {
    rawErrorStr = rawErrorStr.trim()
    switch (rawErrorStr) {
        case 'ERROR 0' : return rawErrorStr + " - no errors"
        case 'ERROR 1' : return rawErrorStr + " - empty command; just ignore it"
        case 'ERROR 2' : return rawErrorStr + " - unknown command"
        case 'ERROR 3' : return rawErrorStr + " - reaction on command is not implemented on master node"
        case 'ERROR 4' : return rawErrorStr + " - command timeout (slave node did not respond)"
        case 'ERROR 5' : return rawErrorStr + " - got some error from slave node during transferring message"
        case 'ERROR 6' : return rawErrorStr + " - slave node return unexpected msg no or error happen during executing the command"
        default: return rawErrorStr + " - unknown error or unexpected answer"
    }
}



function initPort() {
    serialport = new SerialPort(config.arduino.port, config.arduino.openOptions)
    serialport.on('error', console.log)
    serialport.pipe(parser)
    return reopenPort()
}

function reopenPort() {
    return new Promise(function(resolve, reject) {
        if (serialport.isOpen) resolve()
        serialport.open( (error) => {
            if (error) {
                reject(error)
            }
            else {
                const on_parser_data = (data) => { } // ignore any data
                parser.on('data', on_parser_data)
                setTimeout(() => {
                    parser.off('data', on_parser_data)
                    resolve()
                }, 4000)
            }
        })
    })
}

function closePort() {
    if (!serialport) {
        return
    }
    if (serialport.isOpen) {
        serialport.close()
    }
}



async function ping(address) {
    const ans = await callArduino(`AT+PING?${address}`)
    const parts = ans.match(/\+PONG=(\d+)/)
    if (!parts || parts[1] != address) {
        throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
    }
    return address
}

async function pingAll() {
    // multi ping variant
    const ansarr = await callArduinoTimeout("AT+PING?*", 5000)
    return ansarr.map(function(ans) {
        const parts = ans.match(/\+PONG=(\d+)/)
        if (!parts) {
            throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
        return parseInt(parts[1])
    })
}

async function version(address) {
    const ans = await callArduino(`AT+VERSION?${address}`)
    const parts = ans.match(/\+VERSION=(\d+),(\d+)/)
    if (!parts || parts[1] != address) {
        throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
    }
    const ver = parseInt(parts[2])
    return {
        major : (ver >>> 24) & 0xFF,
        minor : (ver >>> 16) & 0xFF,
        rev : ver & 0xFFFF
    }
}

async function capabilities(address) {
    const ans = await callArduino(`AT+CAPABILITIES?${address}`)
    const parts = ans.match(/\+CAPABILITIES=(\d+),(.*)/)
    if (!parts || parts[1] != address) {
        throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
    }
    const allcaps = parts[2]
    const allmatch = [...allcaps.matchAll(/(\w*):(\d+);/g)]
    let caps = []
    for (const m of allmatch) {
        caps.push([ m[1], parseInt(m[2]) ])
    }
    return caps
}

async function getsetCmd(cmd, address, num, state) {
    if (state === undefined) {
        // get branch
        const ans = await callArduino(`AT+${cmd}?${address},${num}`)
        const regex = new RegExp(/\+/.source + cmd + /=(\d+),(\d+),(\d+)/.source)
        const parts = ans.match(regex)
        if (!parts || parts[1] != address || parts[2] != num) {
            throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
        return parseInt(parts[3])
    }
    else {
        const setstate = state ? 1 : 0
        const ans = await callArduino(`AT+${cmd}=${address},${num},${setstate}`)
        if (ans != "OK") {
            console.log(`Real ans: _${ans}_`)
            throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
    }
}

async function getCmd(cmd, address, num) {
    const ans = await callArduino(`AT+${cmd}?${address},${num}`)
    const regex = new RegExp(/\+/.source + cmd + /=(\d+),(\d+),(.*)/.source)
    const parts = ans.match(regex)
    if (!parts || parts[1] != address || parts[2] != num) {
        throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
    }
    return parts[3]
}

async function soil(address, num) {
    return parseInt(await getCmd("SOIL", address, num))
}

async function pressure(address, num) {
    return parseFloat(await getCmd("PRESSURE", address, num))
}

async function current(address, num) {
    return parseFloat(await getCmd("CURRENT", address, num))
}

async function voltage(address, num) {
    return parseFloat(await getCmd("VOLTAGE", address, num))
}

async function water(address, num, state) {
    return await getsetCmd("WATER", address, num, state)
}

async function dcadapter(address, num, state) {
    return await getsetCmd("DCADAPTER", address, num, state)
}

async function pump(address, num, state) {
    return await getsetCmd("PUMP", address, num, state)
}



export {
    initPort, closePort, // init-finish
    callArduino, callArduinoTimeout, // infrastructure
    ping, pingAll, version, capabilities, // general
    soil, pressure, current, voltage, // sensors
    water, dcadapter, pump // management
}