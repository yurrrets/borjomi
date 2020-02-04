'use strict';

const mysql = require('mysql2/promise')
const config = require('./config')
var pool = null
const crypto = require('crypto')


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
            'SELECT id, username FROM accounts WHERE username = ? AND password = ?',
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
            'INSERT INTO account_tokens (token, accounts_id, expire_date) VALUES (?, ?, ?)',
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
             FROM account_tokens t 
             JOIN accounts a ON t.accounts_id = a.id
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
            'UPDATE account_tokens SET expire_date = ? WHERE token = ?',
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
            'DELETE FROM account_tokens WHERE token = ?',
            [token]
        )
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

export { init, finish, login, loginToken, removeToken }