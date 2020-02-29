import { requireParam, optionalParam } from './wsserver'
import { APIError, ErrorCodes } from '../common/error'
const db = require('./db')
const message = require('../common/message')
import { ensureLogin, hasChildAccount } from './login'
import { WSContext } from './wsserver'
import { mergeDeep, customJSONStringify } from '../common/utils'


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

async function updateMessageStatus(inObj, context) {
    const loginInfo = ensureLogin(context)
    const msgId = requireParam(inObj, "messageId", "integer")
    const status = requireParam(inObj, "status", "integer")

    // check if msgId is correct
    const msg = await db.getMessageByID(msgId)
    if (!msg) {
        throw new APIError(`Message with id ${msgId} is not found`, ErrorCodes.InvalidParams)
    }

    // check permission
    const isExec = loginInfo.userID === msg.executor
    const isServ = loginInfo.isServer
    if (!isExec && !isServ) {
        throw new APIError("Access denied", ErrorCodes.NoPermission)
    }

    // check status flow
    let validMap = new Map()
    const s = message.MessageStatus
    validMap.set(s.New, [
        { s: s.Sent, v: isServ },
        { s: s.TimedOut, v: isServ }
    ])
    validMap.set(s.Sent, [
        { s: s.New, v: isServ },
        { s: s.Accepted, v: isExec },
        { s: s.Processing, v: isExec },
        { s: s.DoneOk, v: isExec },
        { s: s.DoneError, v: isExec }
    ])
    validMap.set(s.Accepted, [
        { s: s.Processing, v: isExec },
        { s: s.DoneOk, v: isExec },
        { s: s.DoneError, v: isExec }
    ])
    validMap.set(s.Processing, [
        { s: s.DoneOk, v: isExec },
        { s: s.DoneError, v: isExec }
    ])
    
    // first - check if there is new state among acceptable
    let arr = validMap.get(msg.status) || []
    let newState = arr.find(el => el.s === status)
    if (!newState) {
        throw new APIError(`Message id ${msg.id}: can't change status from ${msg.status} to ${status}`, ErrorCodes.InvalidParams)
    }
    // then check for permission
    if (!newState.v) {
        throw new APIError(`Access denied. Can't change status for message ${msg.id}`, ErrorCodes.NoPermission)
    }

    const res = await db.updateMessageStatus(msg.id, status)
    if (!res) {
        throw new APIError(`Change status for message ${msg.id} failed`, ErrorCodes.GeneralError)
    }

    return {}
}

async function setMessageAnswer(inObj, context) {
    const msgId = requireParam(inObj, "messageId", "integer")
    const errorCode = requireParam(inObj, "errorCode", "integer")
    const errorText = optionalParam(inObj, "errorText", "string")
    const answer = optionalParam(inObj, "answer", "object")

    // first update status
    await updateMessageStatus({
        messageId: msgId,
        status: (errorCode === 0 ? message.MessageStatus.DoneOk : message.MessageStatus.DoneError)
    }, context)

    // everything good - add an answer
    await db.registerMessageAnswer(msgId, errorCode, errorText, customJSONStringify(answer))
}

export { newMessage, getMessageAnswer, updateMessageStatus, setMessageAnswer }