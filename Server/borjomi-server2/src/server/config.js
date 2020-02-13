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