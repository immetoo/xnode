var logger       = require('winston').loggers.get('main');
var clone        = require('clone');
var tcrud        = require('./tcrud');
var tcrudField   = require('./tcrud-field');


exports.createTCrudRoot = function() {
	return tcrud.newInstance('root');
}

exports.createTCrud = function(parent, id) {
	var result = tcrud.newInstance(id);
	parent.tchilds.push(result);
	result.tparent = parent;
	return result;
}
