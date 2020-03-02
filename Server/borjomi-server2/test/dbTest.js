var assert = require('assert');
const db = require('../src/server/db')
const message = require('../src/common/message')
import { ErrorCodes } from '../src/common/error'

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

    describe('#messages', function() {
        it('message crud', async function() {
            const msg = await db.createMessage("ping", 1, 2, { testParam: "testValue"}, new Date(0, 0, 1)) // 1 day
            assert.ok(msg && msg.id)

            let msg1 = await db.getMessageByID(msg.id)
            assert.ok(msg1)
            // assert.deepEqual(msg, msg1) - don't use this, because dates will be rounded because of datetime type (ms cutting) in DBms
            let lastModDate = await db.updateMessageStatus(msg1.id, message.MessageStatus.Sent)
            msg1.status = message.MessageStatus.Sent
            assert.ok(lastModDate)

            let msg2 = await db.getMessageByID(msg.id)
            assert.ok(msg2)
            assert.deepEqual(msg1, msg2)

            let flag = await db.removeMessage(msg.id)
            assert.ok(flag)
        });

        it('message answer', async function() {
            const msg1 = await db.createMessage("ring", 3, 4)
            assert.ok(msg1 && msg1.id)

            const flag1 = await db.registerMessageAnswer(msg1.id, null, "Some error", "my bbbbbiggggg notes")
            const msga1 = await db.readMessageAnswer(msg1.id)
            assert.equal(msga1.errorCode, ErrorCodes.GeneralError)
            assert.equal(msga1.errorText, "Some error")
            assert.equal(msga1.result, "my bbbbbiggggg notes")

            await db.removeMessage(msg1.id)

            const msg2 = await db.createMessage("sing", 5, 6)
            assert.ok(msg2 && msg2.id)

            const flag2 = await db.registerMessageAnswer(msg2.id, ErrorCodes.Ok)
            const msga2 = await db.readMessageAnswer(msg2.id)
            assert.equal(msga2.errorCode, ErrorCodes.Ok)

            await db.removeMessage(msg2.id)
        })
    });

    after(async function() {
        await db.finish()
    });

});
