var mysql = require('mysql');

var connection = mysql.createConnection({
	host     : 'localhost',
	user     : 'borjomi',
	password : 'borjomi_pass',
	database : 'borjomi'
});

exports.connection = connection