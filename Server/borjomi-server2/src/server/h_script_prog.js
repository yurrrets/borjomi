import { optionalParam, requireParam } from './wsserver'
const { WSServer, WSContext } = require('./wsserver')
const message = require('../common/message')
const { ErrorCodes, APIError } = require("../common/error")
const scriptProg1 = require('./script_prog1')


const ScriptProgStatus = {
    NOT_RUN : "not_run",
    RUNNING : "running",
    STOPPING : "stopping"
}

let currentStatus = ScriptProgStatus.NOT_RUN
let currentScriptProg = undefined

async function _runScriptProgram()
{
    let ret = false
    try {
        await scriptProg1.start()
        ret = true
    }
    finally {
        try {
            await scriptProg1.finish()
        }
        catch (e) {
            ret = false
        }
        finally {
            currentStatus = ScriptProgStatus.NOT_RUN
            currentScriptProg = undefined
        }
    }
    return ret
}

function _startScriptProgram() {
    currentStatus = ScriptProgStatus.RUNNING
    currentScriptProg = _runScriptProgram
}

async function _stopScriptProgram() {
    currentStatus = ScriptProgStatus.STOPPING
    await scriptProg1.stop()
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function status(inObj, context) {
    return { status : currentStatus }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function run(inObj, context) {
    if (currentStatus != ScriptProgStatus.NOT_RUN)
        throw new APIError(`Currnent script program status is not "${ScriptProgStatus.NOT_RUN}". It is "${currentStatus}"`, ErrorCodes.GeneralError)
    _startScriptProgram()
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function stop(inObj, context) {
    if (currentStatus != ScriptProgStatus.RUNNING)
        throw new APIError(`Currnent script program status is not "${ScriptProgStatus.RUNNING}". It is "${currentStatus}"`, ErrorCodes.GeneralError)
    await _stopScriptProgram()
}


/** 
 * 
 * @param {WSServer} wsServer
 * */
function init(wsServer) {
    wsServer.addFunction("script.status", status)
    wsServer.addFunction("script.run", run)
    wsServer.addFunction("script.stop", stop)
}



export {
    init
}