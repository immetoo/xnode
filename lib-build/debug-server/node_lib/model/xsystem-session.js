var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
	_id: {
		type: String,
		tfield: {
			tname: 'SID',
			ttype: 'textarea'
		}
	},
	session: {
		type: String,
		tfield: {
			ttype: 'textarea'
		}
	},
	expires: {
		type: Date,
	},
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findLastChangedLimit5: function (callback) {
		logger.debug(modelBackend+'.findLastChangedLimit5');
		this.find({}).sort('-changed_date').limit(5).exec(callback);
	},
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
