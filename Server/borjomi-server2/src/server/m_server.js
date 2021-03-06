const { WSServer, WSContext } = require('./wsserver')
const db = require('./db')
const message = require('../common/message')
import { APIError, ErrorCodes } from '../common/error';
import { getLoggedUserID } from './h_login'
import { AppWebSocket } from '../common/appws'
import { mergeDeep } from '../common/utils'
const h_message = require('./h_message')


const SentTimeoutMs = 1000*60  // 1 min

/**
 * 
 * @param {WSServer} wsServer 
 */
async function onBrokerNewMessage(wsServer) {
    // get new messages
    const newMsgs = await db.getMessageIDsByStatus(message.MessageStatus.New)
    for (const msgId of newMsgs) {
        const msg = await db.getMessageByID(msgId)
        // first check if message is timed out
        if (msg.validUntil && (msg.validUntil < new Date())) {
            await db.updateMessageStatus(msg.id, message.MessageStatus.TimedOut)
            continue
        }
        if (msg.executor === 0) {
            // executor is server
            try {
                if (!wsServer.messageMap.has(msg.type)) {
                    throw new APIError("Message handler not found.", ErrorCodes.FunctionNotFound)
                }
                
                await db.updateMessageStatus(msg.id, message.MessageStatus.Sent)
                let func = wsServer.messageMap.get(msg.type)
                let context = wsServer.serverContext
                let ret = await func(msg, context)
                await h_message.setMessageAnswer(ret, context)
            }
            catch(e) {
                await db.registerMessageAnswer(msgId, typeof(e.code) == "number" ? e.code : ErrorCodes.GeneralError, e.message)
            }
        }
        else {
            // executor is remote client
            try {
                // check if client is now connected
                for (const kvp of wsServer._contextMap) {
                    /** @type { AppWebSocket } */
                    const appWs = kvp[0]
                    /** @type { WSContext } */
                    const wsContext = kvp[1]
                    const userID = getLoggedUserID(wsContext).id
                    if (msg.executor !== userID) {
                        continue
                    }

                    // client found
                    await db.updateMessageStatus(msg.id, message.MessageStatus.Sent)
                    appWs.sendObject({ message: msg })
                    setTimeout(checkSentMessages, SentTimeoutMs, wsServer) // check in a minute - if status still is Sent, need re-send a message
                }
            }
            catch(e) {
                // could be a connection error - so just ignore it
                console.log(`Error sending message ${msg.id} to client: `, e.message)
            }
        }
    }
}

/**
 * 
 * @param {WSServer} wsServer 
 */
async function onBrokerMessageStatusChanged(wsServer, msgId) {
    const msg = await db.getMessageByID(msgId)
    // try to inform remote client about changed message status (if it's available)
    try {
        for (const kvp of wsServer._contextMap) {
            /** @type { AppWebSocket } */
            const appWs = kvp[0]
            /** @type { WSContext } */
            const wsContext = kvp[1]
            const userID = getLoggedUserID(wsContext).id
            if (msg.requestor !== userID) {
                continue
            }

            // client found
            appWs.sendObject({
                event: "messageStatusChanged",
                messageId: msg.id,
                messageStatus: msg.status
            })
        }
    }
    catch(e) {
        // could be a connection error - so just ignore it
        console.log(`Error sending event ${msg.id} to client: `, e.message)
    }
}

/**
 * 
 * @param {WSServer} wsServer 
 */
async function onBrokerMessageAnswer(wsServer, msgId) {
    const msg = await db.getMessageByID(msgId)
    // try to inform remote client about message answer (if it's available)
    try {
        for (const kvp of wsServer._contextMap) {
            /** @type { AppWebSocket } */
            const appWs = kvp[0]
            /** @type { WSContext } */
            const wsContext = kvp[1]
            const userID = getLoggedUserID(wsContext).id
            if (msg.requestor !== userID) {
                continue
            }

            // client found
            appWs.sendObject({
                event: "messageAnswer",
                messageId: msg.id,
                messageStatus: msg.status,
                answer: await db.readMessageAnswer(msgId)
            })
        }
    }
    catch(e) {
        // could be a connection error - so just ignore it
        console.log(`Error sending event ${msg.id} to client: `, e.message)
    }
}

/**
 * 
 * @param {WSServer} wsServer 
 */
async function checkSentMessages(wsServer) {
    // get new messages
    const sentMsgs = await db.getMessageIDsByStatus(message.MessageStatus.Sent)
    let flag = false
    for (const msgId of sentMsgs) {
        const msg = await db.getMessageByID(msgId)
        const dt = new Date(msg.lastModified.getTime() + SentTimeoutMs)
        if (dt < new Date()) {
            await db.updateMessageStatus(msg.id, message.MessageStatus.New)
            flag = true
        }
    }
    if (flag) {
        wsServer._broker.notifyNewMessage()
    }
}

async function ping(msg, context) {
    return { messageId: msg.id, errorCode: 0}
}

/**
 * 
 * @param {WSServer} wsServer 
 */
function init(wsServer) {
    wsServer._broker.newMessageHandler = async function () { await onBrokerNewMessage(wsServer) }
    wsServer._broker.messageStatusChanged = async function (msgId) { await onBrokerMessageStatusChanged(wsServer, msgId) }
    wsServer._broker.messageAnswerHandler = async function (msgId) { await onBrokerMessageAnswer(wsServer, msgId) }
    wsServer.addMessage('ping', ping)
}



export { init }