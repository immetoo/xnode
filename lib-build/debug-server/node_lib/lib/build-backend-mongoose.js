var logger       = require('winston').loggers.get('main');
var clone        = require('clone');
var mongoose     = require('mongoose');
var validate     = require('validate.io');
var tcrudField   = require('./tcrud-field');
var buildConfig  = require('./build-config');

function autoFieldType(fieldMeta,fieldType) {
	if (!fieldMeta) {
		throw new Error('no fieldMeta');
	}
	if (fieldType && fieldType.length != 0) {
		return fieldType;
	}
	if (fieldMeta.type == Date) {
		return 'date';
	}
	return 'text';
}

exports.buildFields = function(modelMeta) {
	if (!modelMeta) {
		throw new Error('no modelMeta');
	}
	var tfields = {};
	var keys = Object.keys(modelMeta);
	for (i = 0; i < keys.length; i++) {
		var key = keys[i];
		var value = modelMeta[key];
		var tfield = null;
		if (key && value && value.tfield) {
			//console.log('tfield model cloned');
			tfield        = clone(value.tfield);
			tfield.tid    = key;
			tfield.tname  = tcrudField.autoFieldName(key,tfield.tname);
			tfield.type   = autoFieldType(value,tfield.ttype);
		} else if (key && value) {
			//console.log('tfield model auto created');
			tfield        = tcrudField.newInstance(key);
			tfield.tname  = tcrudField.autoFieldName(key);
			tfield.type   = autoFieldType(value);
		}
		if (tfield.tvalidate && tfield.tvalidate.io) {
			//console.log('tfield validate rule: '+tfield.tvalidate.io);
		}
		tcrudField.fillDefaults(tfield);
		tfields[tfield.tid] = tfield;
	}
	return tfields;
}

function ss(valueRule) {
	return function (value, response) {
		response(validate(valueRule,value));
	};
}

exports.createModelValidators = function (modelSchema,modelFields) {
	if (!modelSchema) {
		throw new Error('no modelSchema');
	}
	if (!modelFields) {
		throw new Error('no modelFields');
	}
	var keys = Object.keys(modelFields);
	for (var i = 0; i < keys.length; i++) {
		var key = keys[i];
		var tfield = modelFields[key];
		if (!tfield.tvalidate) {
			continue;
		}
		if (!tfield.tvalidate.io) {
			continue;
		}
		modelSchema.path(tfield.tid).validate(ss(tfield.tvalidate.io), '{PATH} validation failed: '+tfield.tvalidate.io);
	}
}

exports.buildStatics = function(modelFields,modelStatics) {
	if (!modelFields) {
		throw new Error('no modelFields');
	}
	if (!modelStatics) {
		modelStatics = {};
	}
	modelStatics['ff_tcrud_fields'] = modelFields;
	return modelStatics;
}

exports.buildTCrudModel = function(mongoose,tcrudParent,name) {
	var model = mongoose.model(name);
	var tcrud = buildConfig.createTCrud(tcrudParent,name);
	var tfields = model['ff_tcrud_fields'];
	if (tfields) {
		tcrud.tmeta.tfields = tfields;
	}
	tcrud.tmodel = name;
	//tcrud.tbackend = 'mongoose';
	return tcrud;
}

exports.buildTCrudModels = function(mongoose,tcrudParent) {
	var result = [];
	var modelNames = mongoose.connection.modelNames();
	for (var i = 0; i < modelNames.length; i++) {
		result.push(exports.buildTCrudModel(mongoose,tcrudParent,modelNames[i]))
	}
	return result;
}


// ----- wrappers

exports.buildStaticsModel = function(modelMeta,modelStatics) {
	return exports.buildStatics(exports.buildFields(modelMeta),modelStatics);
}

exports.buildStaticsModelValidated = function(modelMeta,modelSchema,modelStatics) {
	var modelFields = exports.buildFields(modelMeta);
	exports.createModelValidators(modelSchema,modelFields);
	return exports.buildStatics(modelFields,modelStatics);
}

