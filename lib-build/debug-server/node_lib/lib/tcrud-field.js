var clone        = require('clone');

var template = {
	tid:         null,
	tname:       null,
	ttype:       null,
	tslug:       null,
	tvalidate: {
		io:     null,
	},
	tlist: {
		tenable: true,
		ttext:   null,
		troles:  [],
	},
	tread: {
		tenable: true,
		ttext:   null,
		troles:  [],
	},
	tedit: {
		tenable: true,
		ttext:   null,
		troles:  [],
	},
	tcreate: {
		tenable: true,
		ttext:   null,
		troles:  [],
	},
};

exports.autoFieldName = function (fieldKey,fieldName) {
	if (fieldKey === undefined) {
		throw new Error('no fieldKey');
	}
	if (fieldName && fieldName.length !== 0) {
		return fieldName;
	}
	var result = '';
	var names = fieldKey.split('_');
	for (var i in names) {
		var name = names[i];
		if (name.length > 1) {
			name = name.substring(0,1).toUpperCase() + name.substring(1);
		}
		result = result + name + ' ';
	}
	return result;
}

var autoFieldEnable = function(tfield,type) {
	if (type === undefined) {
		throw new Error('no type');
	}
	if (tfield[type] === undefined) {
		tfield[type] = {};
	}
	if (tfield[type].tenable !== undefined) {
		return;
	}
	var fieldKey = tfield.tid;
	var result = true;
	if ('tlist' === type) {
		var name = fieldKey.toLowerCase();
		if (fieldKey.indexOf('description') >= 0) {
			result = false;
		} else if (fieldKey.indexOf('comment') >= 0) {
			result = false;
		}
	}
	tfield[type].tenable = result;
}


exports.newInstance = function (id) {
	var tfield = clone(template);
	tfield.tid = id;
	return tfield;
}

exports.fillDefaults = function (tfield) {
	if (tfield === undefined) {
		throw new Error('no tfield');
	}
	var tid = tfield.tid;
	if (tid === undefined) {
		throw new Error('no tfield.tid');
	}
	if (tfield.tname === undefined) {
		tfield.tname = exports.autoFieldName(tid);
	}
	if (tfield.tname === undefined) {
		tfield.tname = exports.autoFieldName(tid);
	}
	if (tfield.tslug === undefined) {
		tfield.tslug = tid;
	}
	if (tfield.ttype === undefined) {
		tfield.ttype = 'text';
	}
	if (tfield.tlist === undefined) {
		tfield.tlist = {};
	}
	autoFieldEnable(tfield,'tlist');
	autoFieldEnable(tfield,'tread');
	autoFieldEnable(tfield,'tedit');
	autoFieldEnable(tfield,'tcreate');
}

