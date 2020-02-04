var assert = require('assert');
const login = require('../src/server/login')
import { WSContext } from '../src/server/wsserver'
import { APIError } from '../src/common/error';
const db = require('../src/server/db')

describe('login', function() {

    before(async function () {
        await db.init()
    });

    describe('login-logout', function() {
        it('check test login with username/password and token', async function() {
            let context = new WSContext()
            const {token} = await login.login({username: 'test', password: 'test'}, context)
            assert.ok(token && (token.length >= 32), 'token expected')

            let context2 = new WSContext()
            const {token: token2} = await login.login({token}, context2)
            assert.equal(token, token2)

            await login.logout({}, context)
            await login.logout({}, context2)
        });
    });

    describe('ensure login', function() {
        it('check when not logged in', async function() {
            assert.rejects(login.ensureLogin, APIError)
        });
        it('check after logged in', async function() {
            let context = new WSContext()
            const {token} = await login.login({username: 'test', password: 'test'}, context)
            login.ensureLogin({}, context)
            await login.logout({}, context)
        });
    });

    after(async function() {
        await db.finish()
    });

});
