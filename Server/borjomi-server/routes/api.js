var express = require('express');
var router = express.Router();
var errors = require('../model/errors')
var crypto = require('crypto')

var db = require('../model/db')

router.get('/login', function(req, res, next) {
    var username = req.query.username;
	var password = req.query.password;
	if (username && password) {
		db.connection.query('SELECT * FROM accounts WHERE username = ? AND password = ?', [username, password], function(error, results, fields) {
			if (results.length > 0) {
                r = errors.Ok;
                r.key = crypto.randomBytes(32).toString('hex');
                res.json(r);
			} else {
				res.json(errors.NotAuth);
			}
		});
	} else {
		res.json(errors.NotAuth);
	}
});

module.exports = router;
