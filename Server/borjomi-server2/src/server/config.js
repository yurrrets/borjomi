import { mergeDeep } from '../common/utils'


let config = {
    db: {
        host     : "localhost",
        user     : 'borjomi',
        password : 'borjomi_pass',
        database : 'borjomi',
        waitForConnections : true,
        connectionLimit    : 10,
        queueLimit         : 0
    },
    client: {
        enabled    : false,
        serverUri  : "ws://server:port",
        login      : "login",
        password   : "pass",
        connectionTimeout  : 5000,  // in ms
        reconnectTimeout   : 60000, // in ms
        serverPingEvery    : 10000, // in ms
    },
    arduino: {
        enabled    : false,
        port       : "/dev/ttyUSB0",
        openOptions: {
            autoOpen : false,
            baudRate : 9600,
            xon      : false,
            xoff     : false,
            xany     : false,
            parity   : 'none',
            dataBits : 8,
            stopBits : 1,
            rtscts   : true,
            dtr      : false
        }
    }
}


try {
    let localCfg = require('../../config.json')
    config = mergeDeep({}, config, localCfg)
}
catch(err) {
    console.log(`load config.json failed: ${err}`)
}

module.exports = config