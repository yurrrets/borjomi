import { optionalParam, requireParam } from './wsserver'
const { WSServer, WSContext } = require('./wsserver')
const message = require('../common/message')
const ar = require('./arduino')

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function ping(inObj, context) {
    const address = optionalParam(inObj, "address", "integer")
    let ret = undefined
    if (address) {
        ret = [ ar.ping(address) ]
    }
    else {
        ret = ar.pingAll()
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
    return ar.version(address)
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function capabilities(inObj, context) {
    const address = requireParam(inObj, "address", "integer")
    return { 
        caps: ar.capabilities(address)
    }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function getsetCmd(inObj, context, ar_func) {
    const address = requireParam(inObj, "address", "integer")
    const num = requireParam(inObj, "num", "integer")
    const state = optionalParam(inObj, "state", "integer")
    const ret = await ar_func(address, num, state)

    if (state === undefined) {
        return {
            state: ret
        }
    }
    else {
        return { }
    }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function getCmd(inObj, context, ar_func) {
    const address = requireParam(inObj, "address", "integer")
    const num = requireParam(inObj, "num", "integer")
    return { value: await ar_func(address, num) }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function water(inObj, context) {
    return await getsetCmd(inObj, context, ar.water)
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function soil(inObj, context) {
    return await getIntVal(inObj, context, ar.soil)
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function pressure(inObj, context) {
    return await getFloatVal(inObj, context, ar.pressure)
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function current(inObj, context) {
    return await getFloatVal(inObj, context, ar.current)
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function voltage(inObj, context) {
    return await getFloatVal(inObj, context, ar.voltage)
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function dcadapter(inObj, context) {
    return await getsetCmd(inObj, context, ar.dcadapter)
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function pump(inObj, context) {
    return await getsetCmd(inObj, context, ar.pump)
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
    init,
    ping, version
}