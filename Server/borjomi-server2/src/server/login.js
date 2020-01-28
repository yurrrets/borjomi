'use strict';
import {requireParam} from './wsserver'


async function login({inObj, context}) {
    if (context.loginInfo)
        throw new APIError("Already loged in")
    if (inObj.password!=undefined){
        const username=requireParam(inObj,'username')
        var {id,name,permission_mask,token} = await db.doLogin(username,inObj.password)
    }
    else if (inObj.token!=undefined)
        var {id,name,permission_mask,token} = await db.doTokenLogin(inObj.token)
    else
        throw new APIError("password or token required")

    if (id===0)
        throw new APIError("Login failed")
    var connection_id = await db.addLoginLog("LOGIN",id,context.ws.remoteAddress,inObj.GUID,context.handshakeParams.ClientVersion)
    context.loginInfo={ userID: id, permission_mask, token, connection_id, GUID:inObj.GUID }

    console.log(`UserID ${id} Logged in`)
    return {token}
}

async function logout({inObj, context}) {
    const loginInfo = context.loginInfo
    if (!loginInfo) {
        return
    }
    await db.addLogout(loginInfo.connection_id)
    await db.removeToken(loginInfo.token)
    context.loginInfo = null
}

export { login, logout }