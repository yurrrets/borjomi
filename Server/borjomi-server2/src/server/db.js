import { MessageStatus } from '../common/message';

'use strict';

const mysql = require('mysql2/promise')
const config = require('./config')
var pool = null
const crypto = require('crypto')
const message = require('../common/message')


async function init() {
    pool = mysql.createPool(config.db)
}

async function finish() {
    pool.end()
    pool = null
}

async function login(username, password) {
    try
    {
        var ret = { id: 0, username: "", token: "" }
        var connection = await pool.getConnection();
        const [rows, fields] = await connection.query(
            'SELECT id, username FROM account WHERE username = ? AND password = ?',
            [username, password])
        if (!rows.length) {
            return ret // no login/password pair
        }
        ret.id = rows[0].id
        ret.username = rows[0].username
        ret.token = crypto.randomBytes(32).toString('hex')
        
        // insert token
        let expire_date = new Date(Date.now())
        expire_date.setDate(expire_date.getDate() + 5) // add 5 days
        await connection.query(
            'INSERT INTO account_token (token, account_id, expire_date) VALUES (?, ?, ?)',
            [ret.token, ret.id, expire_date]
        )

        return ret
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

async function loginToken(token) {
    try
    {
        var ret = { id: 0, username: "", token: "" }
        var connection = await pool.getConnection();
        const [rows, fields] = await connection.query(
            `SELECT a.id, a.username, t.token, t.expire_date
             FROM account_token t 
             JOIN account a ON t.account_id = a.id
             WHERE t.token = ?`,
            [token])
        if (!rows.length) {
            return ret // no such token found
        }

        // check expire date
        if (Date.now() > rows[0].expire_date) {
            return ret
        }

        ret.id = rows[0].id
        ret.username = rows[0].username
        ret.token = rows[0].token

        // update expire_date
        let expire_date = new Date(Date.now())
        expire_date.setDate(expire_date.getDate() + 5) // add 5 days
        await connection.query(
            'UPDATE account_token SET expire_date = ? WHERE token = ?',
            [expire_date, ret.token]
        )

        return ret
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

async function removeToken(token) {
    try
    {
        var connection = await pool.getConnection();
        await connection.query(
            'DELETE FROM account_token WHERE token = ?',
            [token]
        )
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

async function getChildAccounts(userID) {
    try
    {
        var connection = await pool.getConnection();
        const [rows, fields] = await connection.query(
            `SELECT a.id, a.username
             FROM account_rel r
             JOIN account a ON r.child_account_id = a.id
             WHERE r.parent_account_id = ?`,
            [userID]
        )
        return rows.map(x => { return {id: x.id, username: x.username} })
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}


/**
 * Inserts to DB new message.
 * Message parameter id is ignored. Input message object is not modified
 * @param {message.Message} message
 * @returns id of newly inserted message
 */
async function createMessage(message) {
    try {
        var connection = await pool.getConnection();
        const [rst] = await connection.query(
            `INSERT INTO message (status, type, creation_date, last_modified, requestor_id, executor_id, valid_until, params)
             VALUES (?, ?, ?, ?, ?, ?, ?, ?)`,
            [message.status, message.type, message.creationDate, message.lastModified, message.requestor, message.executor,
             message.validUntil, JSON.stringify(message.params)]
        )
        return rst.insertId
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

/**
 * Removes message with given id
 * @param {int} id 
 * @returns true if message was removed, false it message was not found
 */
async function removeMessage(id) {
    try {
        var connection = await pool.getConnection();
        const [rst] = await connection.query(`DELETE FROM message WHERE id = ?`, [id])
        return rst.affectedRows > 0
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

export {
    init, finish,
    login, loginToken, removeToken, getChildAccounts,
    createMessage, removeMessage
}