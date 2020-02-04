var assert = require('assert');
const db = require('../src/server/db')

let loginData = {}

describe('db', function() {

    before(async function () {
        await db.init()
    });

    describe('#login', function() {
        it('check test login with username/password', async function() {
            const r = await db.login('test', 'test')
            assert.notEqual(r.id, 0);
        });
    });

    after(async function() {
        console.log("db.finish")
        await db.finish()
    });

});
