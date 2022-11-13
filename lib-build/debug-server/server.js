// server.js
var config         = require('./server-config');
var winston        = require('winston');
winston.loggers.add('main',config.winston);
var log = winston.loggers.get('main');
var express        = require('express');
var path           = require('path');
var fs             = require('fs');
var mongoose       = require('mongoose');
var expressSession = require('express-session');
var mongoStore     = require('connect-mongo')(expressSession);
var tcrud          = require('./node_lib/lib/node-ff-tcrud');

function startMongoose(callback) {
	log.info('Starting '+config.application.name);
	log.info('Connecting to: ' + config.mongo.url);
	mongoose.connection.on('error', callback);
	mongoose.connection.on('connected', callback);
	mongoose.connect(config.mongo.url,config.mongo.options);
}

function startMongooseModels(server,callback) {
	var modelsCount = 0;
	var modelsPath = __dirname + '/node_lib/model';
	fs.readdirSync(modelsPath).forEach(function (file) {
		if (~file.indexOf('.js')) {
			log.debug('Loading mongoose model: '+file);
			require(modelsPath + '/' + file);
			modelsCount++;
		}
	});
	log.info('Loaded mongoose models: '+modelsCount);
	
	var crudRoot = tcrud.build.config.createTCrudRoot();
	
	
	var crudAdmin = tcrud.build.config.createTCrud(crudRoot,'admin');
	crudAdmin.troles.push('admin');
	//crudAdmin.tenable = false;
	
	var crudAdminModels = tcrud.build.backend.mongoose.buildTCrudModels(mongoose,crudAdmin);
	log.info('crud admin models created: '+crudAdminModels.length);
	
	//var crudAdminNodeBaseCmd = crudAdmin 
	
	server.set('ff_root_tcrud_view',crudRoot);
	callback();
}

// Build Server
function createServerSync() {
	var server = express();
	server.disable('x-powered-by');
	server.set('config', config);
	server.set('trust proxy', config.server.httpTrustProxy);
	server.set('views', [path.join(__dirname, 'www_views'),path.join(__dirname, 'node_lib/lib/www_views')]);
	server.set('view engine', 'ejs');
	server.use('/static',express.static(path.join(__dirname,'www_static'),config.options.static));
	server.use('/static/module/bootstrap',express.static(path.join(__dirname,'node_modules/bootstrap/dist'),config.options.static));
	server.use('/static/module/flot',express.static(path.join(__dirname,'node_modules/flot'),config.options.static));
	
	// Add Server session support
//	server.use(expressSession({
//		secret: config.server.sessionSecret,
//		store: new mongoStore({
//			mongooseConnection: mongoose.connection,
//			collection: 'xsystem_session',
//			ttl: server.sessionTTL, 
//		})
//	}));
	return server;
}

function configServer(server,callback) {
	// Add Server config and routing
	var router = require('./node_lib/server-build');
	router.initConfig(server,config);
	
	// Store router list for server/routes page
	tcrud.factory.express.buildServerRoutes(server);
	
	// Auto build site assets
	var ServerInit = require('./server-init');
	ServerInit.build(server);
	
	// Test
	//var test = require('./node_lib/device/xdevice-encryption');
	//log.info('testA='+test.xxteaDecrypt('D57408F9D7F10917AFE9AA92C8051450','9A2EE3A293486E9FE73D77EFC8087D31'));
	//log.info('testA='+test.xxteaDecrypt('9A2EE3A293486E9F','D57408F9D7F10917AFE9AA92C8051450'));
	//log.info('testB=rf_fail=1');
	//log.info('testC='+test.xxteaDecrypt('944DF85BAEE43DC0D7914910D2B69547','9A2EE3A293486E9F'));

	callback();
}

function startServer(server) { 
	server.listen(config.server.httpPort);
	log.info('Started '+config.application.name+' on port: ' + config.server.httpPort);
}

startMongoose(function(err) {
	if (err) { throw err };
	server = createServerSync();
	startMongooseModels(server,function(err) {
		if (err) { throw err };
		configServer(server,function(err) {
			if (err) { throw err };
			startServer(server)
		});
	});
});

module.exports = {
	version: 'development'
};