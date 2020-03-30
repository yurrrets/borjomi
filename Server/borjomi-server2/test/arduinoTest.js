var assert = require('assert');
const login = require('../src/server/h_arduino')
import { APIError } from '../src/common/error';
const arduino = require('../src/server/h_arduino')


describe('arduino', function() {

    before(async function () {
        this.timeout(5000);
        await arduino.initPort()
        console.log("port opened")
    });

    describe('basic func', function() {
        it('version', async function() {
            let str = await arduino.callArduino('AT+VERSION?8')
            console.log("COM port answer: " + str)
        });
    });

    after(async function () {
        this.timeout(5000);
        arduino.closePort()
        console.log("port closed")
    })

})