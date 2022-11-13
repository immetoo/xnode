'use strict';

var logger         = require('winston').loggers.get('main');
var mongoose       = require('mongoose');
var async          = require('async');
var fs             = require('fs');
var tcrud          = require('./node_lib/lib/node-ff-tcrud');
var assets         = require('node-ff-assets');
var xsystemState   = mongoose.model( 'xsystem-state' );

function walkTree(builderConfigJs,server,crud) {
	if (crud.tparent && crud.tenable && crud.tmodel) {
		var localPort = server.get('config').server.httpPort;
		var tview = tcrud.build.view.createTCrudView(crud/*, fetchCrudRoles()*/);
		var base = troot.tmeta.tserver.tslug + '/' + tcrud.build.view.createBaseApiUri(tview,'angular');
		var assetLink = '/static/js/lib/tcrud/'+tview.tslug+'/controller.js@@http://localhost:'+localPort+base+'/controller.js';
		//console.log('asssetLinkDyna; '+assetLink);
		builderConfigJs.linkSources.push(assetLink);
	}
	for (var i = 0; i < crud.tchilds.length; i++) {
		walkTree(builderConfigJs,server,crud.tchilds[i]);
	}
}

function pushDynamicLinkSourcesJs(builderConfigJs,server) {
	return function(callback) {
		// Extend js list with crud entries
		troot = server.get('ff_root_tcrud_view');
		walkTree(builderConfigJs,server,troot);
		callback();
	};
}

function buildAssets(server,callbackDone) {
	var singleResult = 'false' !== process.env.DEV_ASSETS_SINGLE_RESULT;
	var assetsConfig = require('./server-assets.json');
	assets.build({
		assets: {
			js: {
				configCreate: assets.factory.builder.configCreate.fromJSON(assetsConfig,'js'),
				configFill: function (config, callback) {
					async.series([
						assets.factory.lib.async.pushLinkSources(config,  '/static/js/',             'www_static/js/'),
						assets.factory.lib.async.pushLinkSources(config,  '/static/js/controller/',  'www_static/js/controller/'),
						pushDynamicLinkSourcesJs(config,server),
					],callback);
				},
			},
			css: {
				configCreate: assets.factory.builder.configCreate.fromJSON(assetsConfig,'css'),
				configFill: function (config, callback) {
					async.series([
						assets.factory.lib.async.pushLinkSources(config, '/static/css/',            'www_static/css/'),
					],callback);
					
				},
				//assemblerCreate: function (assemblerConfig, callback) {
				//	callback(null,new assets.AssetAssembler(assemblerConfig,assets.factory.assembler.constructor.readFileMinify()));
				//},
			},
		},
//		assemblerCreate: assets.factory.builder.assemblerCreate.readFileRegex(),
		assemblerFill: function (assembler, callback) {
			var serverResultKey = 'ff_assets_'+assembler.config.assetType;
			assembler.on ('log',    assets.factory.assembler.event.log.winston(logger,assembler.config.assetType));
			assembler.on ('result', assets.factory.assembler.event.result.objectSet(server,serverResultKey));
			assembler.config.linkTargetSingleResult = singleResult;
			callback();
		},
	},callbackDone);
}

var initServerStateValue = function(server,type,name,defaultValue,description) {
	xsystemState.ensureExcists(name,type,defaultValue,description,function(err,data) {
		if (err) { throw err }
		logger.info('state checked: '+data.name);
	});
}

exports.build = function(server) {
	buildAssets(server,function(err) {
		if (err) {
			logger.info('Server init failed: '+err);
			return;
		}
		logger.info('Server init done.');
		// init other stuff...
		
		initServerStateValue(server,'_a','_a_seq_base_net_id',  '1F2E3D4C5B6A','XNode Base net_id sequence');
		initServerStateValue(server,'_a','_a_seq_base_net_key' ,'9A2EE3A293486E9FE73D77EFC8087D2F','XNode Base net_key sequence')
		initServerStateValue(server,'_a','_a_seq_base_net_mac', '7469694D33B1','XNode Base net_mac sequence')
		initServerStateValue(server,'_a','_a_seq_base_rf_key',  '1A2B93586E9F170D72EFC8FE72D53A9F','XNode Base rf_key sequence');
		
		initServerStateValue(server,'xds','xds_boot_date',new Date().getTime(),'Server Last boot date');
		initServerStateValue(server,'xds','xds_boot_count','0','Server Boot Counter');
		
		setTimeout(function() {
			xsystemState.incByName('xds_boot_count',function(err,data) {});
			xsystemState.setByName('xds_boot_date',new Date().getTime(),function(err,data) {});
		}, 500);	
	});
}
