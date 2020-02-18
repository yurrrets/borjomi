var assert = require('assert');
const login = require('../src/server/login')
import { WSContext } from '../src/server/wsserver'
import { APIError } from '../src/common/error';
import { createContext } from 'vm';
const db = require('../src/server/db')

function createWsContext() {
    // fake server
    let wsserver = { _broker: null }
    return new WSContext(null, wsserver)
}

describe('login', function() {

    before(async function () {
        await db.init()
    });

    describe('login-logout', function() {
        it('check test login with username/password and token', async function() {
            let context = createWsContext()
            const {token} = await login.login({username: 'test', password: 'test'}, context)
            assert.ok(token && (token.length >= 32), 'token expected')

            let context2 = createWsContext()
            const {token: token2} = await login.login({token}, context2)
            assert.equal(token, token2)

            await login.logout({}, context)
            await login.logout({}, context2)
        });
    });

    describe('ensure login', function() {
        it('check when not logged in', async function() {
            assert.throws(function() { login.ensureLogin(createWsContext()) }, APIError)
        });
        it('check after logged in', async function() {
            let context = createWsContext()
            const {token} = await login.login({username: 'test', password: 'test'}, context)
            login.ensureLogin(context)
            await login.logout({}, context)
        });
    });

    describe('get child accounts', function() {
        it('check login.getChildAccounts func', async function() {
            let context = createWsContext()
            const {token} = await login.login({username: 'test', password: 'test'}, context)
            const child_accs = await login.getChildAccounts({}, context);
            assert.notEqual(child_accs.length, 0)
            for (const val of child_accs) {
                assert.equal(await login.hasChildAccount({ id: val.id }, context), true)
            }
            assert.equal(await login.hasChildAccount({ id: -1 }, context), false)
            assert.rejects(async function () { await login.hasChildAccount({ }, context) }, APIError)
            await login.logout({}, context)
        });
    });

    after(async function() {
        await db.finish()
    });

});
