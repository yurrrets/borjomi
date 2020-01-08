var express = require('express');
var router = express.Router();

var mysql = require('mysql');
var session = require('express-session');
var bodyParser = require('body-parser');
var path = require('path');

var db = require('../model/db');

router.get('/', function(req, res, next) {
    if (!req.session.loggedin) {
        res.redirect('/login');
        res.end();
        return;
    }
    db.connection.query(`SELECT a2.username FROM accounts_rel al
                         JOIN accounts a1 ON a1.id = al.parent_account_id
                         JOIN accounts a2 ON a2.id = al.child_account_id
                         WHERE a1.username = ?`,
                         [req.session.username],
                         function(error, results, fields) {
                             res.render('account-list', { accounts: results.map(val => val.username) });
                             res.end();
                         });
});

module.exports = router;