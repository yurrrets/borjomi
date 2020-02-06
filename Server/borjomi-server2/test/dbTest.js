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
            await db.removeToken(r.token)
            assert.notEqual(r.id, 0);
        });
    });

    describe('#login_token', function() {
        it('check test login with token', async function() {
            const r1 = await db.login('test', 'test')
            const r2 = await db.loginToken(r1.token)
            await db.removeToken(r1.token)
            
            assert.notEqual(r1.id, 0);
            assert.equal(r1.id, r2.id)
            assert.equal(r1.token, r2.token)
            assert.equal(r1.username, r2.username)
        });
    });

    describe('#child_accounts', function() {
        it('tries to get child accounts for "test" user', async function() {
            const r = await db.login('test', 'test')
            await db.removeToken(r.token)
            let child_accs = await db.getChildAccounts(r.id)
            // console.log(child_accs)
            assert.notEqual(child_accs.length, 0);
        });
    });

    after(async function() {
        await db.finish()
    });

});
