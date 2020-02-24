const { WSServer, WSContext } = require('./wsserver')
const db = require('./db')
const message = require('../common/message')
import { APIError, ErrorCodes } from '../common/error';
import { getLoggedUserID } from './login'
import { AppWebSocket } from '../common/appws'


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
        else {
            // executor is remote client
            try {
                // check if client is now connected
                for (const kvp of wsServer._contextMap) {
                    /** @type { AppWebSocket } */
                    const appWs = kvp[0]
                    /** @type { WSContext } */
                    const wsContext = kvp[1]
                    const userID = getLoggedUserID(wsContext)
                    if (msg.executor !== userID) {
                        continue
                    }

                    // client found
                    appWs.sendObject(mergeDeep({ function: 'message' }, msg))
                }
            }
            catch(e) {
                // could be a connection error - so just ignore it
                console.log(`Error sending message ${msg.id} to client: `, e.message)
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