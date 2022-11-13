var favicon           = require('static-favicon');
var morgan            = require('morgan')
var cookieParser      = require('cookie-parser')
var bodyParser        = require('body-parser');
var express           = require('express');
var tcrud             = require('./lib/node-ff-tcrud');
var xdeviceA          = require('./device/xdevice-connector-a');

var renderIndex = function(server) {
	return function (req, res, next) {
		res.render('index', {
			pageTitle:       server.get('config').application.index.pageTitle,
			pageKeywords:    server.get('config').application.index.pageKeywords,
			pageCssFiles:    server.get('ff_assets_css'),
			pageJsFiles:     server.get('ff_assets_js'),
		});
	}
}

exports.initConfig = function(server,config) {
	server.use(favicon());
	server.use(morgan(config.options.morgan.logLevel));
	server.use(cookieParser(config.options.cookieParser.secretKey));
	server.use(bodyParser.json());
	server.use(bodyParser.urlencoded());
	
	server.post('/_a',                           xdeviceA.deviceControl);
	
	tcrud.factory.express.buildCrudApi(server);
	server.get('/api/json/server/uptime',        tcrud.factory.express.renderServerUptime());
	server.get('/api/json/server/routes',        tcrud.factory.express.renderServerRoutes(server));
	
	server.get('/',                              tcrud.factory.express.sendRedirect('/ui'));
	server.get('/ui',                            renderIndex(server));
	server.get('/ui/thtml/*',                    tcrud.factory.express.renderTemplatePath('thtml/'));
	server.get('/ui/*',                          renderIndex(server)); // must be last; for HTML5 history
}

