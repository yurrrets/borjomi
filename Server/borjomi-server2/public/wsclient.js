import { APIError } from "../src/common/error";

// From other common files
const ErrorCodes = {
    ConnectionClosed: -1,
    Ok: 0,
    GeneralError: 1,
    HandshakeRequired: 2,
    NotLoggedIn: 3,
    InvalidFormat: 4,
    FunctionNotFound: 5,
    InvalidParams: 6,
    NoPermission: 7
}

const MessageStatus = {
    New: 1,            // New message
    Sent: 2,           // Message was sent to executor
    Accepted: 3,       // Accepted by executor
    Processing: 4,     // Executing in process
    DoneOk: 5,         // Completed successfully
    DoneError: 6,      // Message processed, but error happened (most probably not completed)
    TimedOut: 7,       // Message hasn't been accepted or processed in time
}


function EventEmitter() {

    this.callbacks = {}

    this.on = function (event, cb) {
        if(!this.callbacks[event]) this.callbacks[event] = [];
        this.callbacks[event].push(cb)
    }

    this.off = function (event, cb) {
        let cbs = this.callbacks[event]
        if(!cbs) return
        for (let i = cbs.length-1; i >= 0; --i) {
            if (cbs[i] === cb) {
                cbs.splice(i, 1)
            }
        }
    }

    this.emit = function(event, ...data) {
        let cbs = this.callbacks[event]
        if(cbs){
            cbs.forEach(cb => cb(...data))
        }
    }
}

function AppWebSocket(wsUri) {
    if (typeof MozWebSocket == 'function')
        WebSocket = MozWebSocket;
    this.ws = new WebSocket( wsUri );
    this.eventer = new EventEmitter()
    this.ws.onopen = (evt) => { this.eventer.emit('open', evt) }
    this.ws.onclose = (evt) => { this.eventer.emit('close', evt) }
    this.ws.onmessage = (evt) => { this.eventer.emit('message', evt) }
    this.ws.onerror = (evt) => { this.eventer.emit('error', evt) }
    this.on = (event, cb) => { this.eventer.on(event, cb) }
    this.off = (event, cb) => { this.eventer.off(event, cb) }
    this.send = (...args) => { this.ws.send(...args) }
    this.close = (...args) => { this.ws.close(...args) }

    // register logs
    this.on('open', (evt) => { info("CONNECTED"); });
    this.on('close', (evt) => { info("DISCONNECTED"); });
    this.on('message', (evt) => {  debug( "Received:\n"+evt.data ) })
    this.on('error', (evt) => {
        if (evt.data) error('ERROR: ' + evt.data);
        else error('ERROR. Check Server is running.');
    });

    // event handling
    this.on('message', (evt) => {
        let obj = JSON.parse(evt.data)
        if (!obj) return
        if (obj.message) this.eventer.emit('serverMessage', obj.message)
        else if (obj.event) this.eventer.emit('serverEvent', obj)
        else this.eventer.emit('serverJsonObj', obj)
        this.eventer.emit('serverAnyJsonObj', obj)
    })
}


var websocket = null;

function initWebSocket(wsUri) {
    try {
        if ( websocket && websocket.ws.readyState == 1 ) {
            websocket.close();
        }
        info("Connecting to", wsUri)
        websocket = new AppWebSocket( wsUri );
        websocket.reqID = 1
        return true
    } catch (exception) {
        error('ERROR: ' + exception);
        return false
    }
}

function stopWebSocket() {
    if (websocket) {
        websocket.close();
    }
}

function sendDataRaw(sendStr){
    if ( websocket != null ) {
        debug("Sending:\n"+sendStr) 
        websocket.send( sendStr );
    } else {
        error("Socket is not connected");
    }
}

function sendDataJson(data){
    if ( websocket != null ) {
        var sendStr = customJSONStringify(data)
        debug("Sending:\n"+sendStr) 
        websocket.send( sendStr );
    } else {
        error("Socket is not connected");
    }
}

async function wsCallFunction(data, timeout = 10000) {
    return new Promise(function(resolve, reject) {
        let reqID = websocket.reqID
        websocket.reqID++

        let onServerJsonObj = function(ans) {
            if (ans.reqID != reqID) return
            websocket.off('serverJsonObj', onServerJsonObj)
            websocket.off('error', onError)
            resolve(ans)
        }
        let onError = function(evt) {
            websocket.off('serverJsonObj', onServerJsonObj)
            websocket.off('error', onError)
            reject(evt)
        }
        
        websocket.on('serverJsonObj', onServerJsonObj)
        websocket.on('error', onError)
        setTimeout(onError, timeout, new Error("Timeout occurred"))

        data.reqID = reqID
        sendDataJson(data)
    })
}



// server function calls

async function handshake() {
    return await wsCallFunction({
        'function': 'Handshake',
        'version': 1
    });
}

async function login(username, password) {
    return await wsCallFunction({
        'function': "login",
        'username': username,
        'password': password
    })
}

async function logout() {
    return await wsCallFunction({
        'function': "logout"
    })
}

async function createNewMessage(type, executor, validForMs, data) {
    return await wsCallFunction({}, {
        'function': "newMessage",
        'type': type,
        'executor': executor,
        'validForMs': validForMs,
        'params': data
    })
}

async function getMessageStatus(msgId) {
    return await wsCallFunction({
        'function': "getMessageAnswer",
        'messageId': msgId
    })
}

async function updateMessageStatus(msgId, status) {
    return await wsCallFunction({
        'function': "updateMessageStatus",
        'messageId': msgId,
        'status': status
    })
}

async function setMessageAnswer(msgId, errCode, errText, result) {
    return await wsCallFunction({
        'function': "setMessageAnswer",
        'messageId': msgId,
        'errorCode': errCode,
        'errorText': errText,
        'result': result
    })
}


// Transforms call to remote function to send message.
// Rules:
// input.function -> message type
// input.function removed
// then: input -> mesage params
// msg answer: 
// 1) MessageStatus.DoneOk: messageAnswer.answer.result -> return 
// 2) else: reject with messageAnswer.answer.errorCode, messageAnswer.answer.errorText
async function sendMsgLikeWsCallFunction(executor, data, timeout = 10000) {
    return new Promise(function(resolve, reject) {
        const msgType = data.function
        data.function = undefined
        createNewMessage(msgType, executor, timeout, data).then(function(newMsg) {
            let msgID = newMsg.messageId || -1
        
            let onServerEvent = function(evt) {
                if (evt.event != "messageAnswer" || evt.messageId != msgID) return
                websocket.off('serverEvent', onServerEvent)
                websocket.off('error', onError)
                if (evt.messageStatus == MessageStatus.DoneOk) {
                    resolve(mergeDeep({ errCode: 0 }, evt.answer.result))
                }
                else {
                    reject(new APIError(`Message id ${msgID} failed: ${evt.answer.errorText}`, evt.answer.errorCode))
                }
            }
            let onError = function(evt) {
                websocket.off('serverEvent', onServerEvent)
                websocket.off('error', onError)
                reject(evt)
            }
            
            websocket.on('serverEvent', onServerEvent)
            websocket.on('error', onError)
            setTimeout(onError, timeout, new Error("Timeout occurred"))
        }, function(err) {
            reject(err)
        })
    })
}