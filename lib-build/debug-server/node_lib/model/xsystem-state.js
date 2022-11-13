var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
	name: {
		type: String,
		trim: true,
		index: { unique: true },
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	type: {
		type: String,
		trim: true,
		index: { unique: false },
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	value: {
		type: String,
		trim: true,
		tfield: {
			xtype: 'textarea',
			tvalidate: { io: 'string' },
		},
	},
	description: {
		type: String,
		trim: true,
		tfield: {
			ttype: 'textarea',
			tvalidate: { io: 'string' },
		},
	},
	changed_date: {
		type: Date,
		default: Date.now,
		tfield: {
			tlist: { tenable: false },
		},
	},
	created_date: {
		type: Date,
		default: Date.now,
		tfield: {
			tlist: { tenable: false },
		},
	}
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findLastChangedLimit5: function (callback) {
		logger.debug(modelBackend+'.findLastChangedLimit5');
		this.find({}).sort('-changed_date').limit(5).exec(callback);
	},
	findOneByName: function (name, callback) {
		logger.debug(modelBackend+'.findByName name='+name);
		this.findOne({name:name}).exec(callback);
	},
	ensureExcists: function (name, type, defaultValue, description, callback) {
		this.findOneByName(name, function(err, xprop) {
			if (err) {
				callback(err);
				return;
			}
			if (xprop == null) {
				logger.debug(modelBackend+'.getByName create name='+name+' defaultValue='+defaultValue);
				var model = mongoose.model(modelName);
				xprop = new model();
				xprop.name = name;
				xprop.type = type;
				xprop.value = defaultValue;
				xprop.description = description;
				xprop.save(function(err,xprop) {
					if (callback) {
						callback(err, xprop);
					}
				});
			} else {
				logger.debug(modelBackend+'.getByName fetched name='+name);
				if (callback) {
					callback(null, xprop);
				}
			}
		});
	},
	setByName: function (name, value, callback) {
		this.findOneByName(name, function(err, xprop) {
			if (err) { throw err }
			logger.debug(modelBackend+'.setByName name='+name+' valueNew='+value+' valueOld='+xprop.value);
			xprop.value = value;
			xprop.save(function(err) {
				callback(err, xprop);
			});
		});
	},
	incByName: function (name, callback) {
		this.findOneByName(name, function(err, xprop) {
			if (err) { throw err }
			xprop.value++;
			logger.debug(modelBackend+'.incByName name='+name+' value='+xprop.value);
			xprop.save(function(err) {
				callback(err, xprop);
			});
		});
	},
	incHexByName: function (name, callback) {
		this.findOneByName(name, function(err, xprop) {
			// DIY inc per 8 chars as JS goes to ~14chars before zering the rest. 
			var v = xprop.value;
			var r = '';
			var inc = true;
			for (var i=0;i<v.length/8;i++) {
				var sv = v.substring(v.length-(i*8)-8,v.length-(i*8));
				var svNum = parseInt(sv, 16);
				if (inc) {
					svNum++;
					inc = false;
				}
				if (svNum == (1+0xFFFFFFFF)) {
					svNum = 0;
					inc = true;
				}
				sv = svNum.toString(16).toUpperCase();
				while (i<((v.length/8)-1) && sv.length < 8) {
					sv = '0' + sv;
				}
				r = sv + r;
			}
			xprop.value = r; ///(parseInt(xprop.value, 16)+1).toString(16).toUpperCase();
			logger.debug(modelBackend+'.incHexByName name='+name+' value='+xprop.value);
			xprop.save(function(err) {
				callback(err, xprop);
			});
		});
	}
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
