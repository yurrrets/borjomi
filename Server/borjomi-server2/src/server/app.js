const WebSocket = require('ws')
const db = require('./db')
const { ErrorCodes, APIError } = require("../common/error")
const { WSServer, requireParam } = require('./wsserver')
const login = require('./login')
const h_message = require('./h_message')
const m_server = require('./m_server')

var wsService;


class WSService {
    constructor(){
        this.wss = new WSServer()
        this.wss.addFunction('login', login.login)
        this.wss.addFunction('newMessage', h_message.newMessage)
        this.wss.addFunction('getMessageAnswer', h_message.getMessageAnswer)
    }

    async startWSService() {
        console.log('starting service');
        this.wss.init({ 'port': 3001 })
        login.loginServer(this.wss.serverContext)
        m_server.init(this.wss)
    }
}

async function startWithDB(){
    await db.init()
    // wsService.startWSService();
    wsService = new WSService()
    await wsService.startWSService();
}

function stopWithDB(){
    // wsService.close();
    db.finish()
}

// testStart()

module.exports = {wsService,startWithDB,stopWithDB}