const assert = require('assert');
import { Broker } from '../src/server/broker'
import { promisify } from 'util';


const tmout = promisify(setTimeout)

class Debug {
    log(...args) {
        // console.log(...args)
    }
}
const debug = new Debug()

describe('broker', function() {

    describe('broker send mes', function() {
        it('test 1', async function() {
            const b = new Broker()
            let storage = []
            let hCounter = 0
            b.newMessageHandler = async () => {
                debug.log('processing new messages start...')
                hCounter++
                while (storage.length) {
                    debug.log("processing:", storage.shift())
                }
                await tmout(50)
                debug.log('processing new messages finish')
            }
            storage.push("msg 1")
            debug.log("appeared:", storage[storage.length-1])
            b.notifyNewMessage()
            
            await tmout(10)
            storage.push("msg 2")
            debug.log("appeared:", storage[storage.length-1])
            b.notifyNewMessage()
            
            await tmout(20)
            storage.push("msg 3")
            debug.log("appeared:", storage[storage.length-1])
            b.notifyNewMessage()

            await tmout(100)
            assert.equal(hCounter, 2)
        });
    });
})