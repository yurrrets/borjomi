import { optionalParam, requireParam } from './wsserver'

const config = require('./config')
const { ErrorCodes, APIError } = require("../common/error")
const { WSServer, WSContext } = require('./wsserver')
const message = require('../common/message')

const SerialPort = require('serialport')
const Readline = SerialPort.parsers.Readline

const SerialPortDelimiter = '\n'


/** @type SerialPort */
let serialport = null
const parser = new Readline({ delimiter : SerialPortDelimiter })


let channelBusy = false

/**
 * Calls arduino board over serial port and returns raw answer from arduino
 * which is represented by single line
 * @param {String} cmd raw command 
 */
function callArduino(cmd) {
    if (serialport == null) {
        throw new Error("SerialPort is not open")
    }
    if (channelBusy) {
        throw new Error("Another request is running to Arduino. You should wait for it finishes")
    }
    channelBusy = true

    return new Promise(function(resolve, reject) {
        function on_parser_data(data) {
            parser.off('data', on_parser_data)
            channelBusy = false
            resolve(data)
        }
        parser.on('data', on_parser_data)
        if (!cmd.endsWith(SerialPortDelimiter)) {
            cmd = cmd + SerialPortDelimiter
        }
        serialport.write(cmd)
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
    if (channelBusy) {
        throw new Error("Another request is running to Arduino. You should wait for it finishes")
    }
    channelBusy = true

    return new Promise(function(resolve, reject) {
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
            channelBusy = false
            resolve(ret)
        }, timeoutMs)
    });
}

function formatArduinoError(rawErrorStr) {
    switch (rawErrorStr.trim()) {
        case 'ERROR 0' : return rawErrorStr + " - no error"
        case 'ERROR 1' : return rawErrorStr + " - unknown command"
        case 'ERROR 2' : return rawErrorStr + " - command timeout (node did not respond)"
        case 'ERROR 3' : return rawErrorStr + " - node with given address doesn't exist"
        case 'ERROR 4' : return rawErrorStr + " - invalid <num> parameter provided, sensor/switch with given num doesn't exist"
        default: return rawErrorStr + " - unknown error or unexpected answer"
    }
}



function initPort() {
    return new Promise(function(resolve, reject) {
        serialport = new SerialPort(config.arduino.port, (error) => {
            if (error) reject(error)
            else resolve()
        })
        serialport.pipe(parser)
    })
}

function closePort() {
    if (!serialport) {
        return
    }
    serialport.close()
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function ping(inObj, context) {
    const address = optionalParam(inObj, "address", "integer")
    if (address) {
        const ans = await callArduino(`AT+PING?${address}`)
        const parts = ans.match(/\+PONG=(\d+)/)
        if (!parts || parts[1] != address) {
            throw APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
        return true
    }
    // multi ping variant
    const ansarr = await callArduinoTimeout("AT+PING?*")
    return ansarr.map(function(ans) {
        const parts = ans.match(/\+PONG=(\d+)/)
        if (!parts) {
            throw APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
        }
        return parseInt(parts[1])
    })
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
        throw APIError(formatArduinoError(ans), ErrorCodes.FunctionCallFailed)
    }
    const ver = parseInt(parts[2])
    return {
        major : ver >>> 16,
        minor : ver & 0xFFFF
    }
}

/** 
 * 
 * @param {WSServer} wsServer
 * */
function init(wsServer) {
    // wsServer.addFunction("")
}



export { init, initPort, closePort, callArduino }