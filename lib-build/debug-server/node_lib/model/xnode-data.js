var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
	//node : {type : Schema.ObjectId, ref : 'XNode'}
	node_id: {
		type: Number,
		index: { unique: false },
		tfield: {
			tvalidate: { io: 'number' },
		},
	},
	data_raw: {
		type: String,
		tfield: {
			ttype: 'textarea',
			tvalidate: { io: 'string' },
			tlist: { tenable: false },
		}
	},
	data_text: {
		type: String,
		tfield: {
			tvalidate: { io: 'number' },
			ttype: 'textarea',
		}
	},
	remote_ip: {
		type: String,
		tfield: {
			tname: 'Remote IP',
			tvalidate: { io: 'ip_address' },
			tlist: { tenable: false },
		}
	},
	insert_date: {
		type: Date,   default: Date.now,
	}
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findLastLimit5: function (callback) {
		logger.debug(modelBackend+'.findLastLimit5');
		this.find({}).sort('-insert_date').limit(5).exec(callback);
	}
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
