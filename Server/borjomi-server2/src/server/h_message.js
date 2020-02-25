import { requireParam, optionalParam } from './wsserver'
import { APIError, ErrorCodes } from '../common/error'
const db = require('./db')
const message = require('../common/message')
import { ensureLogin, hasChildAccount } from './login'
import { WSContext } from './wsserver'
import { mergeDeep } from '../common/utils'


/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function newMessage(inObj, context) {
    const loginInfo = ensureLogin(context)

    const type = requireParam(inObj, "type", "string")
    const requestor = loginInfo.userID
    const executor = requireParam(inObj, "executor", "integer")
    // const validUntil = optionalParam(inObj, "validUntil", "object")
    const params = optionalParam(inObj, "params", "object")
    const validForMs = optionalParam(inObj, "validForMs", "integer")
    // if (!msg.validUntil && validFor !== null) {
    //     if (!(validFor > 0)) {
    //         throw APIError(`If parameter 'validFor' provided it should be a positive number`)
    //     }
    //     msg.validUntil = new Date(msg.creationDate)
    //     msg.validUntil.setDate(msg.validUntil.getDate() + validFor) // in days
    // }

    // check if user has permission to send message
    if (executor !== 0 && !(await hasChildAccount({ id: executor }, context))) {
        throw new APIError(`Can't send message to executor ${executor}. Not a child account`, ErrorCodes.InvalidParams)
    }

    // check for validUntil date
    // if (msg.validUntil && !(msg.validUntil > msg.creationDate)) {
    //     throw APIError(`Parameter validUntil should be greater than creationDate`, ErrorCodes.InvalidParams)
    // }

    const msg = await db.createMessage(type, requestor, executor, params, validForMs)
    context.broker.notifyNewMessage()
    return { "messageId": msg.id }
}

async function getMessageAnswer(inObj, context) {
    const loginInfo = ensureLogin(context)
    const msgId = requireParam(inObj, "messageId", "integer")

    // check if msgId is correct
    const msg = await db.getMessageByID(msgId)
    if (!msg) {
        throw new APIError(`Message with id ${msgId} is not found`, ErrorCodes.InvalidParams)
    }

    // check permission
    if (loginInfo.userID != msg.requestor &&
        loginInfo.userID != msg.executor &&
        !loginInfo.isServer) {
            throw new APIError("Access denied", ErrorCodes.NoPermission)
        }
    
    // getting answer
    return {
        messageId: msg.id,
        messageStatus: msg.status,
        answer: await db.readMessageAnswer(msgId)
    }
}

export { newMessage, getMessageAnswer }