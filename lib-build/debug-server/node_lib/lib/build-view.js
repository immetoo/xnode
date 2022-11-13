var logger       = require('winston').loggers.get('main');
var clone        = require('clone');
var tcrud        = require('./tcrud');
var tcrudView    = require('./tcrud-view');


exports.createBaseApiUri = function(tview, tech, action) {
	var uriView = tview.tslug;
	var uriTech = tview.tmeta.texport[tech].tslug;
	var uriAction = '';
	if (action) {
		uriAction = '/' + tview[action].texport[tech].tslug;
	}
	
	//logger.info('createBaseApiUri uriTech: '+uriTech+' uriView: '+uriView+' uriAction: '+uriAction);
	
	if (tview.tmeta.tserver.tpopfix) {
		return uriTech + '/' + uriView + uriAction;
	} else {
		return uriView + '/' + uriTech + uriAction;
	}
};

function allowSlugSlash(tcrud) {
	var result = true;
	if (tcrud.tparent) {
		result = allowSlugSlash(tcrud.tparent);
		if (!result) {
			return result;
		}
	}
	if (tcrud.tcount.tslug === '') {
		return 'tcount';
	}
	if (tcrud.tlist.tslug === '') {
		return 'tlist';
	}
	if (tcrud.tcreate.tslug === '') {
		return 'tcreate';
	}
	if (tcrud.tread.tslug === '') {
		return 'tread';
	}
	if (tcrud.tedit.tslug === '') {
		return 'tedit';
	}
	if (tcrud.tdelete.tslug === '') {
		return 'tdelete';
	}
//	if (tcrud.tverify.tslug === '') {
		//return false;
//	}
	
	return null;
}

function createTCrudViewSlug(tcrud) {
	var uriViewSlash = '/';
	var slug = uriViewSlash + tcrud.tslug;
	if (!tcrud.tenable || !tcrud.tparent) {
		slug = '';
	}
	if (tcrud.tparent) {
		return createTCrudViewSlug(tcrud.tparent)+slug;
	}
	return slug;
}

var fetchEmptyRoles = function() {
	return function (callback) {
		callback(null,[]);
	};
}

exports.createTCrudView = function(tcrud,xrollesCallback) {
	if (!tcrud) {
		throw new Error('no tcrud');
	}
	if (!xrollesCallback) {
		xrollesCallback = fetchEmptyRoles;
	}
	var tview = tcrudView.newInstance(tcrud);
	tview.tslug = createTCrudViewSlug(tcrud).substring(1);
	/*
	var uriViewSlashAllow = allowSlugSlash(tcrud);
	if (uriViewSlashAllow !== null) {
		uriViewSlash = '_';
		console.log('detected crud actions with empty tslug; deploying in list mode slash; '+uriViewSlash+' in: '+uriViewSlashAllow+' of: '+tcrud.xid);
		tview.tslug = buildSlug(uriViewSlash,tcrud).substring(1);
	} else {
		tview.tslug = buildSlug(uriViewSlash,tcrud);
	}
	*/
	return tview; 
}
