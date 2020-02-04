var assert = require('assert');
const login = require('../src/server/login')
import { WSContext } from '../src/server/wsserver'
const db = require('../src/server/db')

describe('login', function() {

    before(async function () {
        await db.init()
    });

    describe('login', function() {
        it('check teste login with username/password', async function() {
            let context = new WSContext()
            const {token} = await login.login({username: 'test', password: 'test'}, context)
            assert.ok(token && (token.length >= 32), 'token expected');
        });
    });

    after(async function() {
        await db.finish()
    });

});
