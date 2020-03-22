const config = require('./config')
const { ErrorCodes, APIError } = require("../common/error")
const WebSocket = require('ws')
import { customJSONStringify, customJSONparse, mergeDeep } from '../common/utils'
const { WSServer, WSContext } = require('./wsserver')
const message = require('../common/message')

var websocket = null;


const debug = console.log
const info = console.log
const error = console.log


function sendDataRaw(sendStr){
    if ( websocket != null )
    {
        debug("Sending:\n"+sendStr) 
        websocket.send( sendStr );
    }else{
        error("Socket is not connected");
    }
}

function sendDataJson(data){
    if ( websocket != null )
    {
        var sendStr = customJSONStringify(data)
        debug("Sending:\n"+sendStr) 
        websocket.send( sendStr );
    }else{
        error("Socket is not connected");
    }
}



let channelBusy = false

function sendAndReadAnswerJson(data) {
    if (websocket == null) {
        throw new Error("Socket is not connected")
    }
    if (channelBusy) {
        throw new Error("Another data is sending over websocket. You should wait for it")
    }
    channelBusy = true

    return new Promise(function(resolve, reject) {
        let prevOnMessage = websocket.onmessage
        websocket.onmessage = function(evt) {
            try {
                prevOnMessage(evt)
                let ret = customJSONparse(evt.data)
                if (ret && ret.errCode) {
                    throw new Error(`Server returned error. errCode: ${ret.errCode}, errText: ${ret.errText}`)
                }
                resolve(ret)
            }
            finally {
                websocket.onmessage = prevOnMessage
                channelBusy = false
            }
        }
        sendDataJson(data)
    });
}




async function handshake(evt) {
    return await sendAndReadAnswerJson({'function':'Handshake','version':1,'reqID':1});
}

async function login() {
    return await sendAndReadAnswerJson({
        'function': "login",
        'username': config.client.login,
        'password': config.client.password
    })
}

function initWebSocket(wsServer) {
    try {
        if ( websocket && websocket.readyState == 1 )
            websocket.close();
        websocket = new WebSocket( config.client.serverUri );
        info("Connecting to", config.client.serverUri)
        websocket.onopen = function (evt) {
            info("SelfClient CONNECTED");
            setTimeout(async function() {
                await handshake()
                await login()
            }, 0)
        };
        websocket.onclose = function (evt) {
            info("SelfClient DISCONNECTED");
        };
        websocket.onmessage = function (evt) {
            // console.log( "Message received :", evt.data );
            debug( "Received:\n"+evt.data );
            onSocketMessage(evt, wsServer);
        };
        websocket.onerror = function (evt) {
            if (evt.data)
                error('ERROR: ' + evt.data);
            else
                error('ERROR. Check MaxInspect running and WebSocketAPI enabled.');
            // onCycleError(evt);
        };
    } catch (exception) {
        error('ERROR: ' + exception);
    }
}

function stopWebSocket() {
    if (websocket)
        websocket.close();
}

function onSocketMessage(evt, wsServer) {
    var obj = null
    try {
        obj = JSON.parse(evt.data)
    }
    catch {
        // ignore
    }
    if (!obj) return
    if (obj.message) {
        setTimeout(onClientMessage, 0, obj.message, wsServer)
    }
}

/** 
 * @param {message.Message} msg
 * @param {WSServer} wsServer
 * */
async function onClientMessage(msg, wsServer) {
    try {
        /** @type {message.Message} */
        const mappedMsg = mergeDeep({}, msg)
        mappedMsg.requestor = 0
        mappedMsg.executor = 0

        if (!wsServer.messageMap.has(mappedMsg.type)) {
            throw new APIError("Message handler not found.", ErrorCodes.FunctionNotFound)
        }
        
        let func = wsServer.messageMap.get(mappedMsg.type)
        let context = wsServer.serverContext
        let ret = await func(mappedMsg, context)
        
        await sendAndReadAnswerJson(mergeDeep({}, ret, {
            'function': "setMessageAnswer",
            'messageId': msg.id})
        )
    }
    catch(e) {
        await sendAndReadAnswerJson({
            'function': "setMessageAnswer",
            'messageId': msg.id,
            'errorCode': typeof(e.code) == "number" ? e.code : ErrorCodes.GeneralError,
            'errorText': e.message
        })
    }
}


/** 
 * 
 * @param {WSServer} wsServer
 * */
function init(wsServer) {
    initWebSocket(wsServer)
}

export { init }