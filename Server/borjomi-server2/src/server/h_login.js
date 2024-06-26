'use strict';
import { WSContext, ServerContext, requireParam } from './wsserver'
import { APIError, ErrorCodes } from '../common/error';
const db = require('./db')


class LoginInfo {
    constructor(userID, token) {
        this.userID = userID
        this.token = token
    }
}

/**
 * 
 * @param {object} inObj 
 * @param {WSContext} context 
 */
async function login(inObj, context) {
    if (context.loginInfo)
        throw new APIError("Already logged in")
    if (inObj.password!=undefined){
        const username=requireParam(inObj,'username')
        var {id,name,token} = await db.login(username,inObj.password)
    }
    else if (inObj.token!=undefined)
        var {id,name,token} = await db.loginToken(inObj.token)
    else
        throw new APIError("password or token required")

    if (id===0)
        throw new APIError("Login failed")
    // var connection_id = 0 //await db.addLoginLog("LOGIN",id,context.ws.remoteAddress,context.handshakeParams.ClientVersion)
    context.loginInfo = new LoginInfo(id, token)

    context.broker.notifyNewMessage() // send messages to just logged in user (if any)
    // console.log(`UserID ${id} Logged in`)
    return {token}
}

async function logout(inObj, context) {
    const loginInfo = context.loginInfo
    if (!loginInfo) {
        return
    }
    // await db.addLogout(loginInfo.connection_id)
    await db.removeToken(loginInfo.token)
    context.loginInfo = null
}

/**
 * 
 * @param {ServerContext} context 
 */
function loginServer(context) {
    context.loginInfo = new LoginInfo(0, null)
    context.loginInfo.isServer = true
}

/**
 * 
 * @param {WSContext} context 
 */
function getLoggedUserID(context) {
    const loginInfo = context.loginInfo
    if (!loginInfo) {
        return { id: -1 }
    }
    if (loginInfo.isServer) {
        return { id: 0 }
    }
    return { id: loginInfo.userID }
}

/**
 * 
 * @param {WSContext} context 
 * @returns {LoginInfo}
 */
function ensureLogin(context) {
    const loginInfo = context.loginInfo
    if (!loginInfo || (!loginInfo.userID && !loginInfo.isServer)) {
        throw new APIError("Please log in first", ErrorCodes.NotLoggedIn)
    }
    return loginInfo
}

async function getChildAccounts(inObj, context) {
    ensureLogin(context)
    return { childAccounts: await db.getChildAccounts(context.loginInfo.userID) }
}

async function hasChildAccount(inObj, context) {
    // not optimized version - let it be
    if (!inObj || !inObj.id) {
        throw new APIError("id parameter required", ErrorCodes.InvalidParams)
    }
    const child_accs = (await getChildAccounts({}, context)).childAccounts
    for (const val of child_accs) {
        if (val.id == inObj.id) {
            return { result: true }
        }
    }
    return { result: false }
}

export {
     LoginInfo, login, logout, loginServer, getLoggedUserID, ensureLogin,
     getChildAccounts, hasChildAccount
}