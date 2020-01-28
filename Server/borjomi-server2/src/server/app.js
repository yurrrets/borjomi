const WebSocket = require('ws')
const db = require('./db')
const { ErrorCodes, APIError } = require("../common/error")
const { WSServer, requireParam } = require('./wsserver')
const login = require('./login')

var wsService;


class WSService {
    constructor(){
        this.wss = new WSServer()
        this.wss.addFunction('login', login.login)
    }

    async startWSService() {
        console.log('starting service');
        this.wss.initWS({ 'port': 3001 })
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