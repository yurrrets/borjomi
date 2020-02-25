const { ErrorCodes, APIError } = require("./error")
import {customJSONparse,customJSONStringify} from '../common/utils'

var API_VERSION = 1

/**
 * Wrapper for App WebSocket
 */
class AppWebSocket {
    /**
     * @param {WebSocket} connection
     * @param {boolean} verbose sets print all messages
     */
    constructor(connection, handleInvalidJson = false, verbose = false) {
        this.connection = connection

        if (connection) {
            this._bindOnConnection(this.connection)
        }
        this._nextRequestID=0
        this.commandPromise = new Map()
        this._verbose = verbose
        this._functionHandler = null
        this._onClose = null
        this._formatError = null
        this._handleInvalidJson = handleInvalidJson
        this._handshakeResp = null
    }

    handshakeResponce(){ return this._handshakeResp }

    setFunctionHandler(func) { this._functionHandler = func }

    setOnClose(func){ this._onClose = func }

    _processJSONMessage(inObj) {

        if ("function" in inObj) {
            // command message process
            if (this._functionHandler) {
                this._functionHandler(inObj, this).then(
                    (result) => {
                        if (result === undefined)
                            result = {}
                        if (typeof(result)!=="object" || Array.isArray(result)){
                            console.log("Function return not object. In req:"+JSON.stringify(inObj))
                            result = {errCode: ErrorCodes.GeneralError,errText:"Server tryes to return not object"}
                        }
                        if (!('errCode' in result))
                            result.errCode = ErrorCodes.Ok
                        
                        this.sendResp(inObj,result)
                    }).catch(
                        (err) => {
                            console.log(err.stack)
                            let errObjSend;
                            if (err instanceof APIError)
                                errObjSend = {
                                    errCode: err.code,
                                    errText: err.message
                                }
                            else
                                errObjSend = {
                                    errCode: ErrorCodes.GeneralError,
                                    errText: err.message
                                }
                            try{
                                this.sendResp(inObj,errObjSend)
                            }catch(err){
                            }
                            
                        }
                    )
            } else {             
                this.sendResp(inObj,{
                    'errCode': ErrorCodes.GeneralError,
                    'errText': 'Function handler not specified.'
                })
            }
        } else if ("errCode" in inObj) {
            var reqID=inObj["reqID"]
            if (reqID===undefined){
                console.error("no reqID in responce")
                this.close()
                return
            }
            var prom=this.commandPromise.get(reqID)
            if (prom===undefined){
                console.error("No promise for reqID")
                this.close()
                return
            }
            this.commandPromise.delete(reqID)
            // responce message process
            if (inObj.errCode !== 0) {
                prom.reject(new APIError(inObj.errText, inObj.errCode))
            } else {
                prom.resolve(inObj)
            }
        } else if ("eventType" in inObj){
            // temporary log events for debug
            console.log(inObj);
        }
    }
    /**
     * Process incoming messages from socket.
     * Convert it to json object.
     * @param {WebSocket.Data} message
     */
    _messageReceived(message) {
        if (typeof (message) === 'string') {
            if (this._verbose)
                console.log("Received: '" + message + "'");
            try {
                let inObj = customJSONparse(message)
                this._processJSONMessage(inObj)
            } catch (err) {
                if (this._handleInvalidJson) {
                    this.sendObject({
                        errCode : ErrorCodes.InvalidFormat,
                        errText : "Input data is not valid json."
                    })
                }
            }

        }
        if (Buffer.isBuffer(message)) {
            var headerSize = message.readUInt32BE()
            var currPos = 4
            let inObj = message.slice(currPos, currPos + headerSize).toString()
            currPos += headerSize
            var blobs = []
            while (currPos < message.length) {
                let blobSize = message.readUInt32BE(currPos)
                currPos += 4
                blobs.push(message.slice(currPos, currPos + blobSize))
                currPos += blobSize
            }
            try {
                inObj = customJSONparse(inObj,blobs)
                this._processJSONMessage(inObj)
            } catch (err) {
                console.log(err.stack)
                if (this._handleInvalidJson) {
                    this.sendResp(inObj,{
                        errCode: ErrorCodes.InvalidFormat,
                        errText: "Binary header is not valid json."
                    })
                }
            }
        }
    }

    _bindOnConnection(connection) {

        // TODO: correct error handling
        connection.on('error', function (error) {
            console.log("Connection Error: " + error.toString());
        });
        connection.on('close', () => {
            for ( let prom of  this.commandPromise.values() )
                prom.reject(new APIError("Connection closed",ErrorCodes.ConnectionClosed))
            this.commandPromise.clear()
            if (this._onClose)
                this._onClose(this)
            console.log('Connection Closed');
        });

        connection.on('message', (message) => {
            this._messageReceived(message)
        });
    }

    /**
     * Perfomrs connection to MaxInspect. Calls Handshake
     * @param {string} path server path "localhost:1234"
     */
    async connect(connection) {
        this.connection = connection
        await new Promise((resolve, reject) => {
            this.connection.on('open', function () {
                resolve()
            });
            this.connection.on('error', function (error) {
                reject(error)
            });
        })

        this._bindOnConnection(this.connection)

        this._handshakeResp = await this.command("Handshake", { "version": 1 })
    }

    close() {
        if (this.connection)
            this.connection.close()
    }

    /**
     * Runs MaxInspect command. Returs result object or error.
     * @param {string} func 
     * @param {object} params object as parameter { "ResultID":10 }
     */
    command(func, params={}) {
        if (!this.connection) {
            throw new Error("Not connected")
        }

        if (params === undefined)
            params = {}
        params["function"] = func
        var reqID=this._nextRequestID
        this._nextRequestID+=1
        params["reqID"]=reqID
        return new Promise((resolve, reject) => {
            
            this.commandPromise.set(reqID, { "reject": reject, "resolve": resolve } )
            this.sendObject(params)
        });
    }

    sendObject(object){
        this.connection.send(customJSONStringify(object))
    }
    sendResp(req,object){
        if (req===undefined)
            throw Error("Invalid args of sendResp")
        // return request id if it was passed
        if (req.reqID !== undefined){
            object.reqID=req.reqID
        }            
        this.sendObject(object)
    }

}

export {AppWebSocket, API_VERSION}