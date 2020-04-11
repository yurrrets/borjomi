var assert = require('assert');
const login = require('../src/server/h_arduino')
import { APIError } from '../src/common/error';
import { WSContext } from '../src/server/wsserver'
import { Broker } from '../src/server/broker';
const util = require('util')
const arduino = require('../src/server/h_arduino')


function createWsContext() {
    // fake server
    let wsserver = { _broker: null }
    let context = new WSContext(null, wsserver)
    context.broker = new Broker
    return context
}


describe('arduino', function() {

    before(async function () {
        this.timeout(5000);
        await arduino.initPort()
        console.log("port opened")
    });

    describe('basic func', function() {
        this.timeout(5000);
        it('version', async function() {
            let str = await arduino.callArduino('AT+VERSION?8')
            console.log("COM port answer: " + str)
        });
    });

    describe('complex func', function() {
        this.timeout(12000);
        context = createWsContext()
        it('ping', async function() {
            // check for wrong node
            await assert.rejects(async function() { await arduino.ping({ address: 1 }, context) }, APIError)
            // ping node 8
            let ans = await arduino.ping({ address: 8 }, context)
            // console.log("Arduino ping node 8: " + util.inspect(ans))
            assert.ok(ans.pong.includes(8))
            // ping all nodes
            ans = await arduino.ping({}, context)
            // console.log("Arduino ping node *: " + util.inspect(ans))
            assert.ok(ans.pong.includes(8))
        });
        it('version', async function() {
            let ver = await arduino.version({ address: 8 })
            console.log(`Arduino node 8 version: ${ver.major}.${ver.minor}.${ver.rev}`)
            assert.ok(Number.isInteger(ver.major) && Number.isInteger(ver.minor) && Number.isInteger(ver.rev))
        });
    });

    after(async function () {
        this.timeout(5000);
        arduino.closePort()
        console.log("port closed")
    })

})