var logger       = require('winston').loggers.get('main');
var clone        = require('clone');

/*
var createRouter = function(server,path) {
	var router = express.Router();
	router.path = path;
	server.use(router.path,router);
	return router;
}
*/

var walkRouteTree = function(result,stack,parent_path) {
	if (parent_path == null) {
		parent_path = '';
	}
	stack.forEach(function(middleware) {
		if (middleware.route){
			var path = parent_path + middleware.route.path;
			for (var httpMethod in middleware.route.methods) {
				var data = {
					"uriPath": path,
					"httpMethod": httpMethod 
				};
				result.push(data);	
			}
		} else if (middleware.name === 'router') {
			var pp = parent_path + middleware.handle.path;
			walkRouteTree(result,middleware.handle.stack,pp);
		} else {
			//log.info('route err: '+JSON.stringify(middleware));
		}
	});
	return result;
}

exports.renderServerUptime = function(options) {
	if (!options) {
		options: {};
	}
	var timeBoot = new Date().getTime();
	return function (req, res, next) {
		var timeNow = new Date().getTime();
		var result = {
			time_boot: timeBoot,
			time_now: timeNow,
			uptime: timeNow-timeBoot
		}
		res.json({
			data: result
		});
	};
};

exports.renderServerRoutes = function(server) {
	if (!server) {
		throw new Error('no server');
	}
	return function (req, res, next) {
		var routeList = server.get('ff_tcrud_route_list');
		if (!routeList) {
			console.log('no routeList');
			routeList = [];
		}
		var reqGroups = req.query.groups;
		var groupList = [];
		if (reqGroups) {
			groupList = reqGroups.split(',');
		}
		//console.log('groups: '+reqGroups);
		var result = {
			all: [],
		};
		for (var i = 0; i < groupList.length; i++) {
			var groupName = groupList[i].split('/').join('_');
			result[groupName] = [];
		}
		groupList.sort(function(a, b) {
			return a.length < b.length; // longest first as we break on first hit
		});
		
		for (i = 0; i < routeList.length; i++) {
			var route = routeList[i];
			var added = false;
			for (var ii = 0; ii < groupList.length; ii++) {
				if (route.uriPath.indexOf(groupList[ii]) > 0) {
					var groupName = groupList[ii].split('/').join('_');
					result[groupName].push(route);
					added = true;
					break;
				}
			}
			if (!added) {
				result.all.push(route);	
			}
		}
		res.json({
			data: result
		});
	};
};

exports.buildServerRoutes = function(server) {		// todo add return function
	if (!server) {
		throw new Error('no server');
	}
	var result = walkRouteTree([],server._router.stack);
	result.sort(function(a, b) {
		return a.uriPath.localeCompare(b.uriPath);
	});
	server.set('ff_tcrud_route_list',result);
};

exports.sendRedirect = function (location) {
	if (!location) {
		throw new Error('no location');
	}
	return function(req, res) {
		res.redirect(location);
	};
};

exports.renderTemplatePath = function (viewPath) {
	if (!viewPath) {
		viewPath = '';
	}
	return function (req, res) {
		res.locals.query = req.query;
		//console.log('template query keys: '+Object.keys(req.query));
		var qi = req.url.indexOf('?');
		if (qi === -1) {
			qi = req.url.length;
		}
		res.render(viewPath + req.url.substring(req.route.path.length-1, qi));
	};
};
	
exports.buildCrudApi = require('./factory-express-api');
