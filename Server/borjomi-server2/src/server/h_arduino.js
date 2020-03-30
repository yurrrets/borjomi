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
            resolve(data)
        }
        parser.on('data', on_parser_data)
        if (!cmd.endsWith(SerialPortDelimiter)) {
            cmd = cmd + SerialPortDelimiter
        }
        serialport.write(cmd)
    });
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

async function version() {

}

/** 
 * 
 * @param {WSServer} wsServer
 * */
function init(wsServer) {
    // wsServer.addFunction("")
}



export { init, initPort, closePort, callArduino }