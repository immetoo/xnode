var logger       = require('winston').loggers.get('main');
var mongoose     = require('mongoose');
var tmongoose    = require('../lib/node-ff-tcrud').build.backend.mongoose;
var modelName    = __filename.split('/').pop().split('.')[0];
var modelBackend = modelName.split('-').join('_');

var modelMeta = {
	net_id: {
		type: String,
		index: { unique: false },
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	command: {
		type: String,
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	insert_ip: {
		type: String,
		tfield: {
			tvalidate: { io: 'string' },
		},
	},
	insert_date: {
		type: Date,
		default: Date.now,
		tfield: {
			tlist: { tenable: false },
		},
	},
	send_date: {
		type: Date
	},
	done_date: {
		type: Date
	}
};

var modelSchema = new mongoose.Schema(modelMeta);

modelSchema.statics = tmongoose.buildStaticsModelValidated(modelMeta,modelSchema, {
	findByNetIdLimit10: function (net_id, callback) {
		logger.debug(modelBackend+'.findByNetId net_id='+net_id);
		this.find({net_id:net_id}).sort('-insert_date').limit(10).exec(callback);
	},
	findOpenByNetId: function (net_id, callback) {
		logger.debug(modelBackend+'.findOpenByNetId net_id='+net_id);
		this.find({net_id:net_id}).where('send_date',null).sort('insert_date').limit(1).exec(callback);
	},
	findOpenCommandLimit5: function (callback) {
		logger.debug(modelBackend+'.findOpenCommands');
		var dateLastHour = new Date(new Date().getTime() - 30*60*1000); // soft limit on long open errors.
		this.find({insert_date: { $gt: dateLastHour}}).where('send_date',null).sort('insert_date').limit(5).exec(callback);
	},
	findSendCommandLimit5: function (callback) {
		logger.debug(modelBackend+'.findSendCommands');
		var dateLastHour = new Date(new Date().getTime() - 60*60*1000);
		this.find({send_date: { $gt: dateLastHour}}).sort('-insert_date').limit(5).exec(callback);
	},
	insert: function (net_id,command,insert_ip, next) {
		var xcmd = new this();
		xcmd.net_id  = net_id;
		xcmd.command = command;
		xcmd.insert_ip = insert_ip;
		xcmd.save(function(err,xcmd) {
			if (err) { return next(err); }
			logger.debug(modelBackend+'.insert _id='+xcmd._id+' net_id='+xcmd.net_id);
		});
	}
});

module.exports = mongoose.model(modelName, modelSchema, modelBackend);
