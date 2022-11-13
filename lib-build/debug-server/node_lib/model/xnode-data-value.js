var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
		/*
	uuid: {
		type: String,
		index: { unique: true },
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	*/
	node_id: {
		type: Number,
		index: { unique: false },
		tfield: {
			tvalidate: { io: 'number' },
		},
	},
	insert_date: {
		type: Date,
		index: { unique: false },
		default: Date.now,
	},
	name: {
		type: String,
		index: { unique: false },
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	value: {
		type: String,
	}
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findLastAddedLimit5: function (callback) {
		logger.debug(modelBackend+'.findLastAddedLimit5');
		this.find({}).sort('-insert_date').limit(5).exec(callback);
	},
	findByNetIdAndNodeId: function (net_id,node_id, callback) {
		logger.debug(modelBackend+'.findByNetIdAndNodeId net_id='+net_id+' node_id='+node_id);
		this
		.aggregate({ $match: { net_id:net_id ,node_id:node_id }})
		.group({ _id: "$name", value: { $min : "$value" } })
		.sort("_id")
		.exec(callback);
		//this.find({net_id:net_id,node_id:node_id}).sort('insert_date').exec(callback);
	}
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
