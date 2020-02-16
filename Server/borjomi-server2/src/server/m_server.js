const { WSServer } = require('./wsserver')
const db = require('./db')
const message = require('../common/message')


/**
 * 
 * @param {WSServer} wsServer 
 */
async function onBrokerNewMessage(wsServer) {
    console.log("onBrokerNewMessage; wsServer._contextMap.size =", this._contextMap.size)

    // get new messages
    const newMsgs = await db.getMessageIDsByStatus(message.MessageStatus.New)
    for (msgId of newMsgs) {
        const msg = await db.getMessageByID(msgId)
        if (msg.executor === 0) {
            // executor is server
            try {
                if (!wsServer.messageMap.has(msg.type)) {
                    throw new APIError("Function not found.", ErrorCodes.FunctionNotFound)
                }
                
                // mark it as sent
                var func = wsServer.messageMap.get(msg.type)
                var context = wsServer.serverContext
                await func(inObj, context)
            }
            catch {
                // to do
            }
        }
    }
}

async function ping(context) {
    ~
}

/**
 * 
 * @param {WSServer} wsServer 
 */
function init(wsServer) {
    wsServer._broker.onBrokerNewMessage = async function () { await onBrokerNewMessage(wsServer) }
    wsServer.addMessage('ping', ping)
}