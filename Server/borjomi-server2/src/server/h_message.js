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

    let msg = new message.Message()
    msg.status = message.MessageStatus.New
    msg.type = requireParam(inObj, "type", "string")
    msg.creationDate = new Date()
    msg.lastModified = msg.creationDate
    msg.requestor = loginInfo.userID
    msg.executor = requireParam(inObj, "executor", "integer")
    msg.validUntil = optionalParam(inObj, "validUntil", "object")
    msg.params = optionalParam(inObj, "params", "object")

    const validFor = optionalParam(inObj, "validFor", "number")
    if (!msg.validUntil && validFor !== null) {
        if (!(validFor > 0)) {
            throw APIError(`If parameter 'validFor' provided it should be a positive number`)
        }
        msg.validUntil = new Date(msg.creationDate)
        msg.validUntil.setDate(msg.validUntil.getDate() + validFor) // in days
    }

    // check if user has permission to send message
    if (!hasChildAccount({ id: msg.executor }, context)) {
        throw APIError(`Can't send message to executor ${msg.executor}. Not a child account`, ErrorCodes.InvalidParams)
    }

    // check for validUntil date
    if (msg.validUntil && !(msg.validUntil > msg.creationDate)) {
        throw APIError(`Parameter validUntil should be greater than creationDate`, ErrorCodes.InvalidParams)
    }

    const ret = await db.createMessage(msg)
    context.broker.notifyNewMessage()
    return ret
}