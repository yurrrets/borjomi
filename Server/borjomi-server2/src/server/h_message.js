import { requireParam, optionalParam } from './wsserver'
import { APIError, ErrorCodes } from '../common/error'
const db = require('./db')
const message = require('../common/message')
import { ensureLogin, hasChildAccount } from './login'
import { WSContext } from './wsserver'


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
    const validFor = optionalParam(inObj, "validFor", "object")
    // if (!msg.validUntil && validFor !== null) {
    //     if (!(validFor > 0)) {
    //         throw APIError(`If parameter 'validFor' provided it should be a positive number`)
    //     }
    //     msg.validUntil = new Date(msg.creationDate)
    //     msg.validUntil.setDate(msg.validUntil.getDate() + validFor) // in days
    // }

    // check if user has permission to send message
    if (!hasChildAccount({ id: executor }, context)) {
        throw APIError(`Can't send message to executor ${executor}. Not a child account`, ErrorCodes.InvalidParams)
    }

    // check for validUntil date
    // if (msg.validUntil && !(msg.validUntil > msg.creationDate)) {
    //     throw APIError(`Parameter validUntil should be greater than creationDate`, ErrorCodes.InvalidParams)
    // }

    const msg = await db.createMessage(type, requestor, executor, params, validFor)
    context.broker.notifyNewMessage()
    return msg.id
}

async function getMessageAnswer(inObj, context) {
    const loginInfo = ensureLogin(context)
    const msgId = requireParam(inObj, "messageId", "string")

    // check if msgId is correct
    const msg = await db.getMessageByID()
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
    return await db.readMessageAnswer(msgId)
}

export { newMessage, getMessageAnswer }