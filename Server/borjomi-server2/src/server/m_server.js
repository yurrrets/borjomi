const { WSServer } = require('./wsserver')
const db = require('./db')
const message = require('../common/message')
import { APIError, ErrorCodes } from '../common/error';


/**
 * 
 * @param {WSServer} wsServer 
 */
async function onBrokerNewMessage(wsServer) {
    // get new messages
    const newMsgs = await db.getMessageIDsByStatus(message.MessageStatus.New)
    for (const msgId of newMsgs) {
        const msg = await db.getMessageByID(msgId)
        if (msg.executor === 0) {
            // executor is server
            try {
                if (!wsServer.messageMap.has(msg.type)) {
                    throw new APIError("Message handler not found.", ErrorCodes.FunctionNotFound)
                }
                
                await db.updateMessageStatus(msg.id, message.MessageStatus.Sent)
                var func = wsServer.messageMap.get(msg.type)
                var context = wsServer.serverContext
                await func(msg, context)
            }
            catch(e) {
                await db.registerMessageAnswer(msgId, typeof(e.code) == "number" ? e.code : ErrorCodes.GeneralError, e.message)
            }
        }
    }
}

async function ping(msg, context) {
    await db.registerMessageAnswer(msg.id, ErrorCodes.Ok)
}

/**
 * 
 * @param {WSServer} wsServer 
 */
function init(wsServer) {
    wsServer._broker.newMessageHandler = async function () { await onBrokerNewMessage(wsServer) }
    wsServer.addMessage('ping', ping)
}



export { init }