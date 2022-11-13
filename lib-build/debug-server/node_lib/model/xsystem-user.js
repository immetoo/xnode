var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
	username: {
		type: String,
		trim: true,
		index: { unique: true },
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	password: {
		type: String
	},
	active: {
		type: Boolean,
		default: true
	},
	changed_date: {
		type: Date,
		default: Date.now
	},
	created_date: {
		type: Date,
		default: Date.now
	}
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findLastChangedLimit5: function (callback) {
		logger.debug(modelBackend+'.findLastChangedLimit5');
		this.find({}).sort('-changed_date').limit(5).exec(callback);
	},
	findOneByUsername: function (username, callback) {
		logger.debug(modelBackend+'.findByName username='+username);
		this.findOne({username:username}).exec(callback);
	}
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
