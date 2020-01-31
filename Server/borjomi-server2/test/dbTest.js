var assert = require('assert');
const db = require('../src/server/db')

let loginData = {}

describe('db', function() {
    describe('#login', function() {
        it('check test login with username/password', async function() {
            const r = await db.login('test', 'test')
            assert.equal(r.errCode, 0);
        });
    });
});
