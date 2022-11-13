var XDeviceEncryption = require('./xdevice-encryption');
var Mongoose          = require('mongoose');
var XSystemState      = Mongoose.model( 'xsystem-state' );
var XNode             = Mongoose.model( 'xnode' );
var XNodeBase         = Mongoose.model( 'xnode-base' );
var XNodeBaseCommand  = Mongoose.model( 'xnode-base-command' );
var XNodeData         = Mongoose.model( 'xnode-data' );
var XNodeDataValue    = Mongoose.model( 'xnode-data-value' );
var Async             = require('async');
var Winston           = require('winston');
var Log               = Winston.loggers.get('main');

var createXnodeBase = function(created_ip,callbackResult) {
	Async.series({
		cnt_base_net_id: function(callback) {
			XSystemState.incHexByName('_a_seq_base_net_id', function(err, xprop) {
				callback(err,xprop.value);
			});
		},
		cnt_base_net_key: function(callback){
			XSystemState.incHexByName('_a_seq_base_net_key', function(err, xprop) {
				callback(err,xprop.value);
			});
		},
		cnt_base_net_mac: function(callback){
			XSystemState.incHexByName('_a_seq_base_net_mac', function(err, xprop) {
				callback(err,xprop.value);
			});
		},
		cnt_base_rf_key: function(callback){
			XSystemState.incHexByName('_a_seq_base_rf_key',  function(err, xprop) {
				callback(err,xprop.value);
			});
		}
	},
	function(err, result) {
		if (err) {
			callbackResult(err);
			return;
		}
		var xnodeBase = new XNodeBase();
		xnodeBase.created_ip = created_ip;
		xnodeBase.net_id  = result.cnt_base_net_id;
		xnodeBase.net_key = result.cnt_base_net_key;
		xnodeBase.net_mac = result.cnt_base_net_mac;
		xnodeBase.rf_key  = result.cnt_base_rf_key;
		xnodeBase.init_index = 0;
		xnodeBase.save(function(err,xnodeBase) {
			Log.debug("XDeviceConnectorA.createXnodeBase _id="+xnodeBase._id+" net_id="+xnodeBase.net_id);
			callbackResult(err,xnodeBase);
		});
	});
}

var handleInit = function(req,res,next) {
	Log.debug('XDeviceConnectorA.handleInit');
	var net_id = req.body.ni; 
	if (net_id == 0) {
		createXnodeBase(req.ip,function(err,xnodeBase) {
			if (err) { return next(err); }
			Log.info('XDeviceConnectorA.handleInit response: Xinet_id '+xnodeBase.net_id);
			res.send('Xinet_id '+xnodeBase.net_id+'\n');
		});
	} else {
		XNodeBase.findOneByNetId(net_id,function(err,xnodeBase) {
			if (err) { return next(err); }
			if (xnodeBase == null) {
				Log.warn('XDeviceConnectorA.handleInit illegal net_id='+req.body.ni);
				res.send('X');
				return;
			}
			if (xnodeBase.init_index > 4) {
				Log.error('initXnode stateError, device was already init-ited.');
				res.send('XXreboot');
				return;
			}
			xnodeBase.init_index++;
			xnodeBase.save(function(err) {
				var result;
				if (xnodeBase.init_index == 1) {
					result = 'Xinet_key '+xnodeBase.net_key;
				} else if (xnodeBase.init_index == 2) {
					result = 'Xinet_mac '+xnodeBase.net_mac;
				} else if (xnodeBase.init_index == 3) {
					result = 'Xirf_key '+xnodeBase.rf_key;
				} else if (xnodeBase.init_index == 4) {
					result = 'Xireboot';
					XNodeBaseCommand.insert(net_id,'help',req.ip,next); // trigger node data on next ping :)
				} else {
					result = 'X'; // code error 
				}
				Log.info('XDeviceConnectorA.initXnode response net_id='+xnodeBase.net_id+' step='+xnodeBase.init_index+' reply='+result);
				res.send(result);
			});
		});
	}
};

var replyCommand = function(req,res,next) {
	XNodeBaseCommand.findOpenByNetId(req.body.ni, function ( err, data ) {
		if (err) { return next(err); }
		if (data.length > 0) {
			Log.debug('XDeviceConnectorA.replyCommand send cmd: '+data[0].command);
			data[0].send_date = new Date();
			data[0].save(function(err) {
				if (err) { return next(err); }
				res.send('XX'+data[0].command);
			});
		} else {
			res.send('X');
		}
	});
}

var logData = function(xnode,data_raw,data_text,req) {
	var xdata = new XNodeData();
	xdata.remote_ip = req.ip;
	xdata.node_id = xnode.node_id;
	xdata.net_id = req.body.ni;
	xdata.data_raw = data_raw;
	xdata.data_text = data_text;
	xdata.save(function(err,xdata) {
		if (err) { return next(err); }
		Log.debug("XDeviceConnectorA.logData _id="+xdata._id+" net_id="+xdata.net_id+" node_id="+xdata.node_id+" data_text.length="+xdata.data_text.length);
	});
}

var logDataValue = function(xnode,data_text,req, next) {
	if (data_text.indexOf('help') > 0) {
		return; // done't save help cmds as values.
	}
	if (data_text.indexOf('#ERR') > 0) {
		return; // done't save errors.
	}
	var data_lines = data_text.split('\n');
	for(var i = 0; i < data_lines.length; i ++) {
		var data_line = data_lines[i];
		var data_rows = data_line.split('=');
		var data_name = data_rows[0];
		var data_value = data_rows[1];
		if (data_name.length == 0) {
			continue;
		}
		xval = new XNodeDataValue();
		xval.net_id = xnode.net_id;
		xval.node_id = xnode.node_id;
		xval.name = data_name;
		xval.value = data_value;
		xval.save(function(err,xval) {
			if (err) { return next(err); }
			Log.debug("XDeviceConnectorA.logDataValue _id="+xval._id+" net_id="+xval.net_id+" node_id="+xval.node_id+" name="+xval.name+" value="+xval.value);
		});
	}
}

var logRxCount = function(xnode,data_text,next) {
	xnode.rx_date=new Date();
	xnode.rx_requests++;
	xnode.rx_bytes=data_text.length + xnode.rx_bytes;
	xnode.save(function(err,xnode) {
		if (err) { return next(err); }
		Log.debug("XDeviceConnectorA.logRxCount _id="+xnode._id+" rx_requests="+xnode.rx_requests);
	});
}

var updateMetaInfo = function(xnode,req, next) {
	if (xnode.info_date == null) {
		xnode.info_date = new Date(new Date().getTime() - 60*60*1000 - 1000); // triggers update
	}
	var dateLastHour = new Date(new Date().getTime() - 60*60*1000);
	if (dateLastHour.getTime() > xnode.info_date.getTime() ) {
		
		if (xnode.node_id == 1) {
			XNodeBaseCommand.insert(xnode.net_id,'sys_info',req.ip, next);
			XNodeBaseCommand.insert(xnode.net_id,'rf_info',req.ip, next);
			XNodeBaseCommand.insert(xnode.net_id,'net_info',req.ip, next);
			XNodeBaseCommand.insert(xnode.net_id,'net_info_eth0',req.ip, next);
		} else {
			Log.debug("todo");
			//XNodeBaseCommand.insert(xnode.net_id,'@2 sys_info',req.ip, next);
			//XNodeBaseCommand.insert(xnode.net_id,'@2 rf_info',req.ip, next);
		}
		
		xnode.info_date = new Date();
		xnode.save(function(err,xsat) {
			if (err) { return next(err); }
			Log.debug("XDeviceConnectorA.updateMetaInfo _id="+xnode._id);
		});
	}
}

var handleDataNode = function(xnode,req,res,next) {
	var net_id = req.body.ni;
	var node_id = req.body.nn;
	var data_raw = req.body.nd;
	
	Log.debug('XDeviceConnectorA.handleDataNode ni='+xnode.net_id);
	
//	log.debug('XDeviceConnectorA.handleData TODO FIX XXTEA');
//	var result = XDeviceEncryption.xxteaDecrypt(req.body.nd,xnode.hw_net_key);
//	log.debug('XDeviceConnectorA.handleData xxteaDecrypt='+result+' hw_net_key='+xnode.hw_net_key);
	
	var data_text = data_raw;
	
	logRxCount      (xnode,data_text,next);
	logData         (xnode,data_raw,data_text,req);
	logDataValue    (xnode,data_text, req, next);
	updateMetaInfo  (xnode,req, next);
	replyCommand    (req,res,next);
}

var handleData = function(req,res,next) {
	var net_id = req.body.ni;
	var node_id = req.body.nn;
	if (net_id == '000000000000') {
		Log.warn('XDeviceConnectorA.handleData zero net_id='+net_id);
		res.send('X');
		return;
	}
	if (node_id == 0) {
		Log.warn('XDeviceConnectorA.handleData zero node_id='+node_id);
		res.send('X');
		return;
	}
	Log.debug('XDeviceConnectorA.handleData ni='+net_id+' nn='+node_id);
	XNode.findByNetIdAndNodeId(net_id,node_id,function(err,xnode) {
		if (err) { return next(err); }
		if (xnode == null) {
			Log.debug('XDeviceConnectorA.handleData creating new XNode.');
			xnode = new XNode();
			xnode.net_id = net_id;
			xnode.node_id = node_id;
			xnode.save(function(err) {
				if (err) { return next(err); }
				handleDataNode(xnode,req,res,next);
			});
		} else {
			handleDataNode(xnode,req,res,next);
		}
	});
}

var handlePing = function(req,res,next) {
	var net_id = req.body.ni;
	if (net_id == '000000000000') {
		Log.debug('XDeviceConnectorA.handlePing zero net_id='+net_id);
		res.send('X');
		return;
	}
	Log.debug('XDeviceConnectorA.handlePing net_id='+net_id);
	XNodeBase.findOneByNetId(net_id,function(err,xnodeBase) {
		if (err) { return next(err); }
		if (xnodeBase == null) {
			Log.warn('XDeviceConnectorA.handlePing illegal net_id='+net_id);
			res.send('X');
			return;
		}
		xnodeBase.ping_counter++;
		xnodeBase.ping_last_date=new Date();
		xnodeBase.save(function(err) {
			if (err) { return next(err); }
			if (req.body.rc == 0) {
				res.send('X'); // no cmd on first ping.
			} else {
				replyCommand(req,res,next);
			}
		});
	});
}

exports.deviceControl = function(req, res, next) {
	var postType = req.body.pt;
	var url = req.originalUrl || req.url;
	var ip = req.ip;
	Log.debug('POST '+url+' ip='+ip+' pt='+postType+' rc='+req.body.rc+' ni='+req.body.ni);
	if (postType == 'i') {
		handleInit(req,res,next);
		return;
	}
	if (postType == 'p') {
		handlePing(req,res,next);
		return;
	}
	if (postType == 'd') {
		handleData(req,res,next);
		return;
	}
	Log.warn('XDeviceConnectorA.deviceControl unknown postType='+postType);
	res.send('X');
}

