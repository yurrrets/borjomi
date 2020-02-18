const mysql = require('mysql2/promise')
const config = require('./config')
var pool = null
const crypto = require('crypto')
const message = require('../common/message')
import { APIError, ErrorCodes } from '../common/error';


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
 * Input parameters correspond to the ones of Message class.
 * The exception is validTimeout, which is a Date object that indicates a relative time to live for Message.
 * So message.validUntil = Date.now() + validTimeout
 * @param {Date} validTimeout
 * @returns {message.Message} new Message object with correct values
 */
async function createMessage(type, requestor, executor, params, validTimeout) {
    try {
        var connection = await pool.getConnection();
        const dt = new Date()
        const validUntil = validTimeout ? new Date(dt.getTime() + validTimeout.getTime()) : null
        const [rst] = await connection.query(
            `INSERT INTO message (status, type, creation_date, last_modified, requestor_id, executor_id, valid_until, params)
             VALUES (?, ?, ?, ?, ?, ?, ?, ?)`,
            [message.MessageStatus.New, type, dt, dt, requestor, executor, validUntil, JSON.stringify(params)]
        )
        let msg = new message.Message()
        msg.id = rst.insertId
        msg.status = message.MessageStatus.New
        msg.type = type
        msg.creationDate = dt
        msg.lastModified = dt
        msg.requestor = requestor
        msg.executor = executor
        msg.validUntil = validUntil
        msg.params = params
        return msg
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
        await connection.query(`DELETE FROM message_answer WHERE message_id = ?`, [id]) // delete anwser if it exists
        const [rst] = await connection.query(`DELETE FROM message WHERE id = ?`, [id])
        return rst.affectedRows > 0
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

/**
 * Update status of given message in db.
 * @param {int} id messageId
 * @param {message.MessageStatus} status new status
 * @returns {Date} if success, return message's lastModified date; otherwise returns null
 */
async function updateMessageStatus(id, status) {
    try {
        var connection = await pool.getConnection();
        const dt = new Date()
        const [rst] = await connection.query(
            `UPDATE message
             SET status = ?, last_modified = ?
             WHERE id = ?`,
            [status, dt, id]
        )
        return rst.affectedRows > 0 ? dt : null
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

/**
 * Get Message object by given id, or returns null if no message with given id found
 * @param {int} id 
 */
async function getMessageByID(id) {
    try {
        var connection = await pool.getConnection();
        const [rows] = await connection.query(
            `SELECT id, status, type, creation_date, last_modified, requestor_id, executor_id, valid_until, params
             FROM message
             WHERE id = ?`,
            [id]
        )
        if (rows.length == 0) {
            return null
        }
        const row = rows[0]
        let msg = new message.Message()
        msg.id = row.id
        msg.status = row.status
        msg.type = row.type
        msg.creationDate = row.creation_date
        msg.lastModified = row.last_modified
        msg.requestor = row.requestor_id
        msg.executor = row.executor_id
        msg.validUntil = row.valid_until
        if (row.params)
            msg.params = JSON.parse(row.params)
        return msg
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

async function getMessageIDsByStatus(status) {
    try {
        var connection = await pool.getConnection();
        const [rows] = await connection.query(
            `SELECT id
             FROM message
             WHERE status = ?
             ORDER BY last_modified`,
            [status]
        )
        return rows.map(x => x)
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

async function registerMessageAnswer(msgId, errCode, errMessage, notes) {
    if (typeof(errCode) != "number") {
        errCode = ErrorCodes.GeneralError
    }
    try {
        var connection = await pool.getConnection();
        // ensure that there is no answer
        const [rows] = await connection.query(
            `SELECT COUNT(id) as cnt
             FROM message_answer
             WHERE message_id = ?`,
            [msgId]
        )
        if (rows[0].cnt > 0) {
            throw new APIError(`Error answer with message_id ${msgId} is already registered`, ErrorCodes.InvalidParams)
        }
        // update message status and ensure all is ok
        const lastModDate = await this.updateMessageStatus(msgId, errCode === ErrorCodes.Ok ? message.MessageStatus.DoneOk : message.MessageStatus.DoneError)
        if (!lastModDate) {
            throw new APIError(`Can't update status of message with id ${msgId}. Maybe invalid message_id?`, ErrorCodes.InvalidParams)
        }
        // add new answer
        const [rst] = await connection.query(
            `INSERT INTO message_answer (message_id, error_code, error_text, notes)
             VALUES (?, ?, ?, ?)`,
            [msgId, errCode, errMessage, notes]
        )
        return rst.insertId
    }
    finally {
        if (connection) {
            connection.release()
        }
    }
}

async function readMessageAnswer(msgId) {
    try {
        var connection = await pool.getConnection();
        const [rows] = await connection.query(
            `SELECT message_id, error_code, error_text, notes
             FROM message_answer
             WHERE message_id = ?`,
            [msgId]
        )
        if (rows.length === 0) {
            return null
        }
        const row = rows[0]
        let msga = new message.MessageAnswer()
        msga.messageId = row.message_id
        msga.errorCode = row.error_code
        msga.errorText = row.error_text
        msga.notes = row.notes
        return msga
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
    createMessage, removeMessage, updateMessageStatus, getMessageByID, getMessageIDsByStatus,
    registerMessageAnswer, readMessageAnswer
}