'use strict';

const mysql = require('mysql2/promise')
const config = require('./config')
const pool = mysql.createPool(config.db)


async function login(username, password) {
    try
    {
        var connection = await pool.getConnection();
        const [rows, fields] = await connection.query(
            'SELECT * FROM accounts WHERE username = ? AND password = ?',
            [username, password])
        if (!rows.length) {
            return { errCode: 1 }
        }
        return { errCode: 0 }
    }
    catch (error) {
        console.log(error)
        return { errCode: 2 }
    }
    finally {
        if (connection) {
            connection.release()
            // connection.end()
            pool.end()
        }
    }
}

module.exports.login = login