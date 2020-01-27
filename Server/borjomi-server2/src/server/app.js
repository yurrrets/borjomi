const WebSocket = require('ws')
const db = require('./db')
const { ErrorCodes, APIError } = require("../common/error")
const { WSServer, requireParam } = require('./wsserver')

var wsService;


class WSService {
    constructor(){
        this.wss = new WSServer()
        this.wss.addFunction('login', this.login)
    }

    async startWSService() {
        console.log('starting service');
        this.wss.initWS({ 'port': 3001 })
    }

    async login({inObj, ws, handshakeParams}) {
        console.log(inObj)
        const username = requireParam(inObj, 'username', 'string')
        const password = requireParam(inObj, 'password', 'string')
        console.log(username, ":", password)
        if (!username || !password) {
            throw new APIError("'username' and 'password' parameters are required")
        }
        return {
            errCode: 0,
            key: 'abcddtd'
        }
    }
}

async function startWithDB(){
    // await db.init()
    // wsService.startWSService();
    wsService = new WSService()
    wsService.startWSService();
}

function stopWithDB(){
    // wsService.close();
    // db.close()
}

// testStart()

module.exports = {wsService,startWithDB,stopWithDB}