var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
	net_id: {
		type:    String,
		index: { unique: false }, 
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	node_id: {
		type:    Number,
		index: { unique: false },
		tfield: {
			tvalidate: { io: 'number' },
		},
	},
	insert_date: {
		type: Date,
		index: { unique: false },
		default: Date.now,
		tfield: {
			tlist: { tenable: false },
		}
	},
	info_date: {
		type: Date,
		tfield: {
			tlist: { tenable: false },
		}
	},
	
	name: {
		type: String,
	},
	description: {
		type:     String,
		tfield: {
			tlist: { tenable: false },
		}
	},
	gps_latitude: {
		type:     Number,
		tfield: {
			tname: 'Latitude',
			tlist: { tenable: false },
		}
	},
	gps_longitude: {
		type:    Number,
		tfield: {
			tname: 'Longitude',
			tlist: { tenable: false },
		}
	},
	
	rx_bytes: {
		type:    Number,
		default: 0,
		tfield: {
			tname: 'RX bytes'
		}
	},
	rx_requests: {
		type:    Number,
		default: 0,
		tfield: {
			tname: 'RX req#'
		}
	},
	rx_date: {
		type:    Date,
		index: { unique: false },
		default: Date.now,
		tfield: {
			tname: 'RX Date'
		}
	}
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findByNetId: function (net_id, callback) {
		logger.debug(modelBackend+'.findByNetId net_id='+net_id+"");
		this.find({net_id:net_id}).sort('insert_date').exec(callback);
	},
	findByNetIdAndNodeId: function (net_id,node_id, callback) {
		logger.debug(modelBackend+'.findByNetIdAndNodeId net_id='+net_id+' node_id='+node_id);
		this.findOne({net_id:net_id,node_id:node_id}).sort('insert_date').exec(callback);
	}
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
