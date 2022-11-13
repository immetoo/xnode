var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
	net_id: {
		type: String,
		index: { unique: true },
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	net_key: {
		type: String,
		tfield: {
			tvalidate: { io: 'string' },
			tlist: { tenable: false },
		},
	},
	net_mac: {
		type: String,
		tcrud: {
			tname: 'Net MAC',
			tlist: { tenable: false },
		}
	},
	rf_key: {
		type: String,
		tfield: {
			tname: 'RF Key',
			tlist: { tenable: false },
		}
	},
	init_index: {
		type: Number,
		tfield: {
			tlist: { tenable: false },
		}
	},
	
	ping_counter: {
		type: Number,
		default: 0,
		tfield: {
			tname: 'Ping #'
		}
	},
	ping_last_date: {
		type: Date,
		index: { unique: true, sparse: true }
	},
	ping_rtt: {
		type: Number,
		default: 0,
		tfield: {
			tname: 'Ping RTT'
		}
	},
	
	changed_date: {
		type: Date,
		default: Date.now,
		tfield: {
			tlist: { tenable: false },
		}
	},
	created_date: {
		type: Date,
		default: Date.now,
		tfield: {
			tlist: { tenable: false },
		}
	},
	created_ip: {
		type: String,
		tfield: {
			tvalidate: { io: 'string' },
		},
	}
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findOneByNetId: function (net_id, callback) {
		logger.debug(modelBackend+'.findByNetId net_id='+net_id);
		this.findOne({net_id:net_id}).exec(callback);
	},
	findLastPingLimit5: function (callback) {
		var dateLastHour = new Date(new Date().getTime() - 60*60*1000);
		logger.debug(modelBackend+'.findLastPinged lastDate: '+dateLastHour);
		this.find({}).where('ping_last_date').gt(dateLastHour).sort('-ping_last_date').limit(5).exec(callback);
	}
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
