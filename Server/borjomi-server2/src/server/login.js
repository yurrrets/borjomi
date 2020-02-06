'use strict';
import {requireParam} from './wsserver'
import { APIError, ErrorCodes } from '../common/error';
const db = require('./db')


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
    var connection_id = 0 //await db.addLoginLog("LOGIN",id,context.ws.remoteAddress,context.handshakeParams.ClientVersion)
    context.loginInfo={ userID: id, token, connection_id }

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

function ensureLogin(context) {
    const loginInfo = context.loginInfo
    if (!loginInfo || !loginInfo.userID) {
        throw new APIError("Please log in first", ErrorCodes.NotLoggedIn)
    }
}

async function getChildAccounts(inObj, context) {
    ensureLogin(context)
    return await db.getChildAccounts(context.loginInfo.userID)
}

async function hasChildAccount(inObj, context) {
    // not optimized version - let it be
    if (!inObj || !inObj.id) {
        throw new APIError("id parameter required", ErrorCodes.InvalidParams)
    }
    const child_accs = await getChildAccounts({}, context)
    for (const val of child_accs) {
        if (val.id == inObj.id) {
            return true
        }
    }
    return false
}

export {
     login, logout, ensureLogin,
     getChildAccounts, hasChildAccount
}