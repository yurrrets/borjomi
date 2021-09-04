const log4js = require("log4js");
const logger = log4js.getLogger()

const WebSocket = require('ws')
const config = require('./config')
const db = require('./db')
const { ErrorCodes, APIError } = require("../common/error")
const { WSServer, requireParam } = require('./wsserver')
const login = require('./h_login')
const h_message = require('./h_message')
const m_server = require('./m_server')

let client = null
if (config.client.enabled) {
    client = require('./client')
}

let arduino = null
let h_arduino = null
let h_script_prog = null
if (config.arduino.enabled) {
    arduino = require('./arduino')
    h_arduino = require('./h_arduino')
    h_script_prog = require('./h_script_prog')
}

var wsService;


class WSService {
    constructor(){
        this.wss = new WSServer()
        this.wss.addFunction('ping', async function () {}) // ping superfunction
        this.wss.addFunction('login', login.login)
        this.wss.addFunction('logout', login.logout)
        this.wss.addFunction('getLoggedUserID', login.getLoggedUserID)
        this.wss.addFunction('getChildAccounts', login.getChildAccounts)
        this.wss.addFunction('hasChildAccount', login.hasChildAccount)
        
        this.wss.addFunction('newMessage', h_message.newMessage)
        this.wss.addFunction('getMessageAnswer', h_message.getMessageAnswer)
        this.wss.addFunction('updateMessageStatus', h_message.updateMessageStatus)
        this.wss.addFunction('setMessageAnswer', h_message.setMessageAnswer)
        if (h_arduino) {
            h_arduino.init(this.wss)
            h_script_prog.init(this.wss)
        }
    }

    async startWSService() {
        logger.info('starting service');
        this.wss.init({ 'port': 3001 })
        login.loginServer(this.wss.serverContext)
        m_server.init(this.wss)

        if (client) {
            logger.info('initing client')
            client.init(this.wss)
        }
        if (arduino) {
            logger.info('initing arduino')
            await arduino.initPort()
        }
    }
}

async function startWithDB(){
    log4js.configure(config.logger)

    try {
        logger.info('initing database')
        await db.init()
    } catch (e) {
        logger.error('init database failed', e)
    }
    
    try {
        wsService = new WSService()
        await wsService.startWSService();
    } catch (e) {
        logger.error('startWSService failed', e)
    }
}

function stopWithDB(){
    // wsService.close();
    db.finish()
}

// testStart()

module.exports = {wsService,startWithDB,stopWithDB}