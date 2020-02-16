'use strict';

const { ErrorCodes, APIError } = require("../common/error")
const WebSocket = require('ws')
import { AppWebSocket, API_VERSION } from "../common/appws"
const SERVER_VERSION = require("../../package.json").version
const { Broker } = require("./broker")



function _validateType(val,type){

    if (type !== undefined){
        if (type==='integer'){
            if (!Number.isInteger(val))
                return false
        }else{
            if (typeof val !== type)
                return false
        }
    }

    return true
}

function requireParam(inObj, name, type=undefined) {

    if (!(name in inObj))
        throw new APIError(`'${name}' parameter required`,
            ErrorCodes.InvalidParams)
    var ret = inObj[name]

    if (!_validateType(ret,type))
        throw new APIError(`'${name}' parameter required and should be ${type}`,
            ErrorCodes.InvalidParams)

    return inObj[name]
}

function optionalParam(inObj, name, type=undefined) {

    if (!(name in inObj))
        return null
    var ret = inObj[name]

    if (!_validateType(ret,type))
        throw new APIError(`Optional parameter '${name}' should be ${type} [if provided]`,
            ErrorCodes.InvalidParams)

    return inObj[name]
}

class WSContext {
    /**
     * 
     * @param {AppWebSocket} appWs 
     * @param {WSServer} wsServer 
     */
    constructor(appWs, wsServer) {
        this.broker = wsServer._broker
        this.ws = appWs
        this.handshakeParams = null
        this.loginInfo = null
    }
}

class ServerContext {
    constructor(wsServer) {
        this.broker = wsServer._broker
        this.handshakeParams = null
        this.loginInfo = null
    }
}

class WSServer {
    constructor() {
        this.functionMap = new Map()
        this.messageMap = new Map() // messages processed by server
        this._contextMap = new Map()
        this._onClientClose = null
        this._broker = new Broker()
        this._broker.newMessageHandler = this.onBrokerNewMessage

        this.serverContext = new ServerContext(this)
        this.serverContext.handshakeParams = { version: 1 }
    }
    setOnClientClose(func){ this._onClientClose = func }
    // WARNING calling ws.command await can cause dead lock.
    // func should expect object { inObj, ws, context }
    /**
     * 
     * @param {string} name function name
     * @param {{ inObj, context:WSContext }} func 
     */
    addFunction(name, func) {
        if (this.functionMap.has(name))
            throw Error(`Function with name "${name}" is already registered`)
        this.functionMap.set(name, func)
    }

    /**
     * 
     * @param {string} tp message type
     * @param {{ inObj }} func 
     */
    addMessage(tp, func) {
        if (this.messageMap.has(tp))
            throw Error(`Message with type "${name}" is already registered`)
        this.messageMap.set(tp, func)
    }

    handshake(inObj, ws) {
        if (this._contextMap.get(ws).handshakeParams) {
            throw new APIError("Handshake can be called only once")
        }

        var version = requireParam(inObj, 'version', 'number')
        if (version != 1)
            throw new APIError("Versions are not compatible")

        this._contextMap.get(ws).handshakeParams = inObj

        return { "version": API_VERSION, "ServerVersion": SERVER_VERSION }
    }

    async processFunction(inObj, ws) {

        if (inObj.function === 'Handshake') {
            return this.handshake(inObj, ws)
        }
        if (!this._contextMap.get(ws).handshakeParams)
            throw new APIError("Handshake function should be called first", ErrorCodes.HandshakeRequired)
        if (!this.functionMap.has(inObj.function))
            throw new APIError("Function not found.", ErrorCodes.FunctionNotFound)

        var func = this.functionMap.get(inObj.function)
        var context = this._contextMap.get(ws)
        var ret = await func(inObj, context)
        return ret
    }

    init(options) {
        this.wss = new WebSocket.Server(options)
        this.wss.on("connection", (ws,req) => {
            console.log("client connected");
            var appWs = new AppWebSocket(ws, true)
            this._contextMap.set(appWs, new WSContext(appWs, this))
            // const ip = req.headers['x-forwarded-for'].split(/\s*,\s*/)[0];
            const ip = req.connection.remoteAddress
            appWs.remoteAddress=ip
            appWs.setFunctionHandler((inObj, appWs) => this.processFunction(inObj, appWs))
            appWs.setOnClose( (appWs)=> this.onClientClose(appWs) )
        })
    }

    onClientClose(ws){
        // cleanup ot avoid memory leak
        this._contextMap.delete(ws)
        if (this._onClientClose)
            this._onClientClose(ws)
    }

    close() {
        console.log("closing server");
        if (this.wss)
            this.wss.close()
        this.wss = undefined
    }
}

export { WSServer, WSContext, ServerContext, requireParam, optionalParam }