const WebSocket = require('ws')
import { AppWebSocket } from "../common/appws"
const SERVER_VERSION = require("../../package.json").version


class APIError extends Error {
    constructor(message, code = 0) {
        super(message)
        this.name = "APIError"
        this.code = code
    }
}

class WSServer {
    constructor() {
        this.functionMap = new Map()
        this._handshakeParams = new Map()
        this._onClientClose = null
    }
    setOnClientClose(func){ this._onClientClose = func }
    // WARNING calling ws.command await can cause dead lock.
    // func should expect object { inObj, ws, handshakeParams }
    /**
     * 
     * @param {string} name function name
     * @param {{ inObj, ws:SMXWebSocket, handshakeParams }} func 
     */
    addFunction(name, func) {
        if (this.functionMap.has(name))
            throw Error(`Function with name "${name}" is already registered`)
        this.functionMap.set(name, func)
    }

    handshake(inObj, ws) {
        if (this._handshakeParams.has(ws))
            throw new APIError("Handshake can be called only once")

        var version = requireParam(inObj, 'version', 'number')
        if (version != 1)
            throw new APIError("Versions not compatible")

        this._handshakeParams.set(ws, inObj)

        return { "version": API_VERSION, "ServerVersion": SERVER_VERSION }
    }

    async processFunction(inObj, ws) {

        if (inObj.function === 'Handshake') {
            return this.handshake(inObj, ws)
        }
        if (!this._handshakeParams.has(ws))
            throw new APIError("Handshake function should be called first", ErrorCodes.HandshakeRequired)
        if (!this.functionMap.has(inObj.function))
            throw new APIError("Function not found.", ErrorCodes.FunctionNotFound)

        var func = this.functionMap.get(inObj.function)
        var handshakeParams = this._handshakeParams.get(ws)
        var ret = await func({ inObj, ws, handshakeParams })
        return ret
    }

    initWS(options) {
        this.wss = new WebSocket.Server(options)
        this.wss.on("connection", (ws,req) => {
            console.log("client connected");
            var smxWs = new AppWebSocket(ws, true)
            // const ip = req.headers['x-forwarded-for'].split(/\s*,\s*/)[0];
            const ip = req.connection.remoteAddress
            smxWs.remoteAddress=ip
            smxWs.setFunctionHandler((inObj, ws) => this.processFunction(inObj, ws))
            smxWs.setOnClose( (ws)=> this.onClientClose(ws) )
        })
    }

    onClientClose(ws){
        // cleanup ot avoid memory leak
        this._handshakeParams.delete(ws)
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

export { WSServer, APIError }