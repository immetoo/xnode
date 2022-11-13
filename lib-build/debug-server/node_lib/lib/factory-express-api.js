var logger       = require('winston').loggers.get('main');
var clone        = require('clone');
var fs           = require('fs');
var ejs          = require('ejs');
var xmlmapping   = require('xml-mapping');
var rawbody      = require('raw-body');
var typeis       = require('type-is');
var mcrud        = require('crud-mongoose');
var ncrud        = require('node-crud');
var mongoose     = require('mongoose');
var buildView    = require('./build-view');

exports.bodyParserXml = function() {
	return function(req, res, next) {
		if (req._body) return next();
		req.body = req.body || {};
		
		if (!typeis(req, 'xml')) return next();
		
		// flag as parsed
		req._body = true;
		
		// parse
		rawbody(req, {
			limit: options.limit || '100kb',
			length: req.headers['content-length'],
			encoding: 'utf8'
		}, function (err, buf) {
			if (err) return next(err);
			
			if (0 == buf.length) {
				return next(error(400, 'invalid xml, empty body'));
			}
			try {
				req.body = xmlmapping.dump(buf/*, options.reviver*/);
			} catch (err){
				err.body = buf;
				err.status = 400;
				return next(err);
			}
			next();
		})
	}
}

var renderTemplateDataRead = function(tview, exportType, contentType, model, projection) {
	return function(data, query, cb) {
		var res = this.response;
		res.set('Content-Type', contentType);
		
		var cursor, fields;
		if (query.hasOwnProperty('fields')) {
			fields = query.fields;
			delete query.fields;
		}
		delete query.export;
		cursor = model.findOne(query);
		cursor.select(mcrud.tools.select(projection, fields));
		cursor.lean().exec(function(err,doc) {
			if (err) {
				cb(err || 'error in stream');
				return;
			}
			if (doc == null) {
				cb('no data'); // TODO: error is still in json...
				return;
			}
			//var templateReadHeader = fs.readFileSync('node_code/lib/www_views/node-ff-tcrud/'+exportType+'/read-header.ejs', 'utf8');
			var templateReadRecord = fs.readFileSync('node_code/lib/www_views/node-ff-tcrud/'+exportType+'/read-record.ejs', 'utf8');
			//var templateReadFooter = fs.readFileSync('node_code/lib/www_views/node-ff-tcrud/'+exportType+'/read-footer.ejs', 'utf8');
			res.write(ejs.render(templateReadRecord,{
				tview: tview,
				record: doc,
			}));
			res.end();
			
			//cb(); // FIXME
		});
	}
}

var renderTemplateDataList = function(tview, exportType, contentType) {
	return function(res, cursor, cb) {
		res.set('Content-Type', contentType);
		
		var templateHeader = fs.readFileSync('node_code/lib/www_views/node-ff-tcrud/'+exportType+'/list-header.ejs', 'utf8');
		var templateRecord = fs.readFileSync('node_code/lib/www_views/node-ff-tcrud/'+exportType+'/list-record.ejs', 'utf8');
		var templateFooter = fs.readFileSync('node_code/lib/www_views/node-ff-tcrud/'+exportType+'/list-footer.ejs', 'utf8');
		
		res.write(ejs.render(templateHeader,{
			tview: tview,
		}));
		cursor.stream()
			.on('data', function(doc) {
				res.write(ejs.render(templateRecord,{
					tview: tview,
					record: doc,
				}));
			})
			.on('error', function(e) {
				cb(e || 'error in stream');
			})
			.on('close', function() {
				res.write(ejs.render(templateFooter,{
					tview: tview,
				}));
				res.end();
				cb();
			});
	};
}

exports.injectExportFormat = function(apiBase) {
	return function ( req, res, next ) {
		if (req.url.indexOf(apiBase) >= 0) {
			if (req.url.indexOf('xml') > 0) {
				req.query['export'] = 'xml';
			} else if (req.url.indexOf('csv') > 0) {
				req.query['export'] = 'csv';
			} else if (req.url.indexOf('rss') > 0) {
				req.query['export'] = 'rss';
			}
			if (req.url.indexOf('json') == 0) {
				console.log('inject export type: '+req.query['export']);
			}
		}
		next();
	};
}

var _    = require('underscore');

var filterQueryList = function (model) {
	return mcrud.parseQuery()
		.defaults({ limit: '10' })
		.overrides({})
		.maxes({ limit: 1000 });
		//.removes()
}

var filterDataUpdate = function (model) {
	return mcrud.parseData()
		overrides({});
		//.removes('auth')
		//.overrides({ updated: Date.now })
		//.defaults({ 'info.gender': 'M' }))
}

function allowCrudType(tview,tcrudType) {
	return tview[tcrudType].tenable; // todo add roles
}

//var rssXNodeMap = function(link,feed,data) {
//	feed.addNewItem(data.net_id, link+'/ui/XNode/read/'+data._id, data.changed_data, data.net_id+' id', {});
//}

var sendRedirect = function (location) {
	if (!location) {
		throw new Error('no location');
	}
	return function(req, res) {
		res.redirect(location);
	};
};

var renderCrudView = function (tview) {
	return function (req, res, next) {
		var result = {
			tview: tview,
		}
		res.json({
			data: result
		});
	};
}

var renderCrudController = function (tview, thtmlPrefix, tapiPrefix) {
	return function (req, res, next) {
		res.set('Content-Type', 'text/javascript');
		res.render('node-ff-tcrud/angular/controller',{
			tview: tview,
			thtmlPrefix: thtmlPrefix,
			tapiPrefix: tapiPrefix,
		});
	};
}

var renderCrudTemplate = function (tview,paction) {
	return function (req, res, next) {
		res.render('node-ff-tcrud/angular/'+paction+'/'+req.params.action,{
			tview: tview,
		});
	};
}

var buildCrud = function (server,tcrud) {
	
	if (!tcrud.tenable || !tcrud.tmodel) {
		logger.info('disabled server: '+tcrud.tid);
		return;
	}
	
	var tview = buildView.createTCrudView(tcrud /*, fetchCrudRoles()*/);
	var model = mongoose.model( tcrud.tmodel );
		
	// Export tdebug
	var uriDebug = tcrud.tmeta.tserver.tslug + '/' +buildView.createBaseApiUri(tview,'json') + '/_debug/';
	var tcrudClone = clone(tcrud);
	tcrudClone.tparent = null; // rm loop: tparent.tchilds[] = this
	server.get(uriDebug + 'tview',      renderCrudView(tview));
	server.get(uriDebug + 'tcrud',      renderCrudView(tcrudClone));
	
	if (_.contains(tview.tmeta.tserver.texports,'json')) { 
		
		if (allowCrudType(tview,'tlist')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'json','tlist')).Read()
				.pipe(filterQueryList(model))
				.pipe(mcrud.findAll(model).stream());
		}
		if (allowCrudType(tview,'tcreate')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'json','tcreate')).Create()
				.pipe(mcrud.createNew(model));
		}
		if (allowCrudType(tview,'tread')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'json','tread') + '/:_id').Read()
				.pipe(mcrud.findOne(model));
		}
		if (allowCrudType(tview,'tedit')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'json','tedit') + '/:_id').Update()
				.pipe(filterDataUpdate(model))
				.pipe(mcrud.updateOne (model));
		}
		if (allowCrudType(tview,'tdelete')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'json','tdelete') +'/:_id').Delete()
				.pipe(mcrud.removeOne(model));
		}
		if (allowCrudType(tview,'tcount')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'json','tcount')).Read()
				.pipe(filterQueryList(model))
				.pipe(mcrud.findAll(model).stream());
		}
		if (allowCrudType(tview,'tverify')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'json','tverify') + '/:_id').Update()
				.pipe(mcrud.findOne(model));
		}
	}
	
	if (_.contains(tview.tmeta.tserver.texports,'xml')) {
		if (allowCrudType(tview,'tlist')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'xml','tlist')).Read()
				.pipe(filterQueryList(model))
				.pipe(mcrud.findAll(model).stream().exports({ xml: renderTemplateDataList(tview,'xml','text/xml') }));
		}
		if (allowCrudType(tview,'tcreate')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'xml','tcreate')).Create()
				.pipe(mcrud.createNew(model));
		}
		if (allowCrudType(tview,'tread')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'xml','tread') +'/:_id').Read()
				.pipe(renderTemplateDataRead(tview,'xml','text/xml',model));
		}
		if (allowCrudType(tview,'tedit')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'xml','tedit')+'/:_id').Update()
				.pipe(filterDataUpdate(model))
				.pipe(mcrud.updateOne (model));
		}
		if (allowCrudType(tview,'tdelete')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'xml','tdelete')+'/:_id').Delete()
				.pipe(mcrud.removeOne (model));
		}
	}
	
	if (_.contains(tview.tmeta.tserver.texports,'csv')) {
		if (allowCrudType(tview,'tlist')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'csv','tlist')).Read()
				.pipe(filterQueryList(model))
				.pipe(mcrud.findAll(model).stream().exports({ csv: renderTemplateDataList(tview,'csv','text/csv') }));
		}
		if (allowCrudType(tview,'tread')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'csv','tread') +'/:_id').Read()
				.pipe(renderTemplateDataRead(tview,'csv','text/csv',model));
		}
	}
	
	if (_.contains(tview.tmeta.tserver.texports,'rss')) {
		if (allowCrudType(tview,'tlist')) {
			ncrud.entity(buildView.createBaseApiUri(tview,'rss','tlist')).Read()
				.pipe(mcrud.parseQuery()
					.defaults({ limit: '10' })
					.maxes({ limit: 50 })
				)
				.pipe(mcrud.findAll(model)
					.stream()
					.exports({ rss: renderTemplateDataList(tview,'rss','text/xml') })
				);
		}
		//.exports({ rss: exports.exportRss('todo',tview.xid,rssXNodeMap) })
	}
	if (_.contains(tview.tmeta.tserver.texports,'angular')) {
		
		var uriPrefix = tcrud.tmeta.tserver.tslug + '/' + buildView.createBaseApiUri(tview,'angular');
		var thtmlPrefix = uriPrefix + '/' + tview.tmeta.texport.angular.thtml;
		var tapiPrefix = tcrud.tmeta.tserver.tslug + '/' +buildView.createBaseApiUri(tview,'json');
		
		//server.get('/ui/include/crud/:name/:action',     XViewInclude.renderTemplateCrud);
		//server.get('/ui/include/js/controller/:name',    XViewInclude.renderCrudController);
		
		server.get(uriPrefix + '/thtml/crud/:action',  renderCrudTemplate(tview,'thtml/crud/'));
		server.get(uriPrefix + '/controller.js',      renderCrudController(tview,thtmlPrefix,tapiPrefix));
		//server.get(uriPrefix + '/service.js',       renderCrudService(tview))
	}
}

function walkViews(server,tcrud) {
	//logger.debug('walk views: '+tcrud.tid);
	if (tcrud.tparent) {
		buildCrud(server,tcrud);
	}
	for (var i = 0; i < tcrud.tchilds.length; i++) {
		walkViews(server,tcrud.tchilds[i]);
	}
}

function buildCrudApi(server) {		// todo add return function
	
	troot = server.get('ff_root_tcrud_view');
	walkViews(server,troot);
	
	server.use(exports.injectExportFormat(troot.tmeta.tserver.tslug));
	server.use(exports.bodyParserXml());
	ncrud.launch(server,{
	      base: troot.tmeta.tserver.tslug,
	      cors: false,
	      timeoute: 10e3
	    });
}

module.exports = buildCrudApi;
