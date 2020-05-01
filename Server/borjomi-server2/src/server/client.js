const config = require('./config')
const { ErrorCodes, APIError } = require("../common/error")
const WebSocket = require('ws')
import { customJSONStringify, customJSONparse, mergeDeep } from '../common/utils'
const { WSServer, WSContext } = require('./wsserver')
const message = require('../common/message')

var websocket = null;


// const debug = function () {} //console.log
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
        let timeoutTmrId = setTimeout(function() {
            reject(new APIError("Timeout. Answer hasn't been received during timeout period", ErrorCodes.TimeoutElapsed))
        }, config.client.connectionTimeout)
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
                clearTimeout(timeoutTmrId)
                websocket.onmessage = prevOnMessage
                channelBusy = false
            }
        }
        sendDataJson(data)
    });
}




async function handshake() {
    return await sendAndReadAnswerJson({
        'function': 'Handshake',
        'version': 1,
        'reqID': 1
    })
}

async function login() {
    return await sendAndReadAnswerJson({
        'function': "login",
        'username': config.client.login,
        'password': config.client.password
    })
}

async function answerMessage(msgId, msgAns) {
    await sendAndReadAnswerJson(mergeDeep({}, msgAns, {
        'function': "setMessageAnswer",
        'messageId': msgId
    }))
}

async function ping(targetId) {
    if (typeof targetId === 'undefined') {
        targetId = 0 // server
    }
    await sendAndReadAnswerJson(targetId ? {
        'function': "newMessage",
        'type': "ping",
        'executor': targetId,
        'validForMs': 1000*30 // 30 sec
    } : {
        'function' : "ping"  // short variant for server with less traffic usage
    })
}




function scheduleReconnect(wsServer) {
    info("Scheduling RECONNECT")
    setTimeout(initWebSocket, config.client.reconnectTimeout, wsServer)
}

var pingTimerId = null

function periodicPing(fStart) {
    if (fStart) {
        pingTimerId = setTimeout(async function() {
            try {
                await ping()
                if (pingTimerId) periodicPing(true)
            }
            catch {
                if (websocket) websocket.close()
            } 
        }, config.client.serverPingEvery)
    } else {
        if (!pingTimerId) return
        clearInterval(pingTimerId)
        pingTimerId = null
    }
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
            periodicPing(true)
        };
        websocket.onclose = function (evt) {
            info("SelfClient DISCONNECTED");
            scheduleReconnect(wsServer)
            periodicPing(false)
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
                error('ERROR. Some error happended. Data: ' + customJSONStringify(evt));
            // onCycleError(evt);
        };
    } catch (exception) {
        error('ERROR: ' + exception);
        scheduleReconnect(wsServer)
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

        if (wsServer.messageMap.has(mappedMsg.type)) {
            let func = wsServer.messageMap.get(mappedMsg.type)
            let context = wsServer.serverContext
            let ret = await func(mappedMsg, context)
            
            await answerMessage(msg.id, ret)
        }
        
        else if (wsServer.functionMap.has(mappedMsg.type)) {
            let func = wsServer.functionMap.get(mappedMsg.type)
            let context = wsServer.serverContext
            let ret = await func(mappedMsg.params, context)
            
            await answerMessage(msg.id, {
                errorCode: 0,
                result: ret
            })
        }

        else 
            throw new APIError("Message handler not found.", ErrorCodes.FunctionNotFound)
    }
    catch(e) {
        await answerMessage(msg.id, {
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