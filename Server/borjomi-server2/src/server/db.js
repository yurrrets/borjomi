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
        ret.token = crypto.randomBytes(32).toString('hex');

        // insert token
        // TODO
        return ret
    }
    finally {
        if (connection) {
            connection.release()
            // connection.end()
            pool.end()
        }
    }
}

export { init, finish, login }