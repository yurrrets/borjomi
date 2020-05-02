import { optionalParam, requireParam } from './wsserver'

const config = require('./config')
const { ErrorCodes, APIError } = require("../common/error")
const { WSServer, WSContext } = require('./wsserver')
const message = require('../common/message')

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
        prevChannelPromise.then(reopenPort).then( () => {
            let tmrId = setTimeout(function() {
                parser.off('data', on_parser_data)
                reject(new Error('Timeout occurred'))
                closePort()
                resolver.resolve()
            }, timeoutMs)
            function on_parser_data(data) {
                parser.off('data', on_parser_data)
                clearTimeout(tmrId)
                resolve(data)
                resolver.resolve()
            }
            parser.on('data', on_parser_data)
            if (!cmd.endsWith(SerialPortDelimiter)) {
                cmd = cmd + SerialPortDelimiter
            }
            serialport.write(cmd)
        }, (error) => { reject(error) })
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
        prevChannelPromise.then(reopenPort).then( () => {
            var ret = []
            function on_parser_data(data) {
                ret.push(data)
            }
            parser.on('data', on_parser_data)
            if (!cmd.endsWith(SerialPortDelimiter)) {
                cmd = cmd + SerialPortDelimiter
            }
            serialport.write(cmd)
            setTimeout(function() {
                parser.off('data', on_parser_data)
                resolve(ret)
                resolver.resolve()
            }, timeoutMs)
        })
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
    serialport = new SerialPort(config.arduino.port, { autoOpen: false })
    serialport.on('error', console.log)
    serialport.pipe(parser)
    return reopenPort()
}

function reopenPort() {
    return new Promise(function(resolve, reject) {
        if (serialport.isOpen) resolve()
        serialport.open( (error) => {
            if (error) reject(error)
            else  resolve()
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

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function ping(inObj, context) {
    const address = optionalParam(inObj, "address", "integer")
    let ret = undefined
    if (address) {
        const ans = await callArduino(`AT+PING?${address}`)
        const parts = ans.match(/\+PONG=(\d+)/)
        if (!parts || parts[1] != address) {
            throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
        ret = [ address ]
    }
    else {
        // multi ping variant
        const ansarr = await callArduinoTimeout("AT+PING?*", 5000)
        ret = ansarr.map(function(ans) {
            const parts = ans.match(/\+PONG=(\d+)/)
            if (!parts) {
                throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
            }
            return parseInt(parts[1])
        })
    }
    return { pong : ret }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function version(inObj, context) {
    const address = requireParam(inObj, "address", "integer")
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

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function capabilities(inObj, context) {
    const address = requireParam(inObj, "address", "integer")
    const ans = await callArduino(`AT+CAPABILITIES?${address}`)
    const parts = ans.match(/\+CAPABILITIES=(\d+),(.*)/)
    if (!parts || parts[1] != address) {
        throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
    }
    const allcaps = parts[2]
    const allmatch = [...allcaps.matchAll(/(\w*):(\d+);/g)]
    let ret = { 
        caps: []
    }
    for (const m of allmatch) {
        ret.caps.push([ m[1], parseInt(m[2]) ])
    }
    return ret
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function getsetCmd(inObj, context, cmd) {
    const address = requireParam(inObj, "address", "integer")
    const num = requireParam(inObj, "num", "integer")
    const state = optionalParam(inObj, "state", "integer")

    if (state === undefined) {
        const ans = await callArduino(`AT+${cmd}?${address},${num}`)
        const regex = new RegExp(/\+/.source + cmd + /=(\d+),(\d+),(\d+)/.source)
        const parts = ans.match(regex)
        if (!parts || parts[1] != address || parts[2] != num) {
            throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
        const currstate = parseInt(parts[3])
        return {
            state: currstate
        }
    }
    else {
        const setstate = state
        const ans = await callArduino(`AT+${cmd}=${address},${num},${setstate}`)
        if (ans != "OK") {
            console.log(`Real ans: _${ans}_`)
            throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
        return { }
    }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function getCmd(inObj, context, cmd) {
    const address = requireParam(inObj, "address", "integer")
    const num = requireParam(inObj, "num", "integer")

    const ans = await callArduino(`AT+${cmd}?${address},${num}`)
    const regex = new RegExp(/\+/.source + cmd + /=(\d+),(\d+),(.*)/.source)
    const parts = ans.match(regex)
    if (!parts || parts[1] != address || parts[2] != num) {
        throw new APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
    }
    return parts[3]
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function getIntVal(inObj, context, cmd) {
    return { value: parseInt(await getCmd(inObj, context, cmd)) }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function getFloatVal(inObj, context, cmd) {
    return { value: parseFloat(await getCmd(inObj, context, cmd)) }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function water(inObj, context) {
    return await getsetCmd(inObj, context, "WATER")
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function soil(inObj, context) {
    return await getIntVal(inObj, context, "SOIL")
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function pressure(inObj, context) {
    return await getFloatVal(inObj, context, "PRESSURE")
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function current(inObj, context) {
    return await getFloatVal(inObj, context, "CURRENT")
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function voltage(inObj, context) {
    return await getFloatVal(inObj, context, "VOLTAGE")
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function dcadapter(inObj, context) {
    return await getsetCmd(inObj, context, "DCADAPTER")
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function pump(inObj, context) {
    return await getsetCmd(inObj, context, "PUMP")
}

/** 
 * 
 * @param {WSServer} wsServer
 * */
function init(wsServer) {
    wsServer.addFunction("arduino.ping", ping)
    wsServer.addFunction("arduino.version", version)
    wsServer.addFunction("arduino.capabilities", capabilities)
    wsServer.addFunction("arduino.water", water)
    wsServer.addFunction("arduino.soil", soil)
    wsServer.addFunction("arduino.pressure", pressure)
    wsServer.addFunction("arduino.current", current)
    wsServer.addFunction("arduino.voltage", voltage)
    wsServer.addFunction("arduino.dcadapter", dcadapter)
    wsServer.addFunction("arduino.pump", pump)
}



export {
    init, initPort, closePort, callArduino,
    ping, version
}