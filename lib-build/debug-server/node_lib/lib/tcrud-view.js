var clone        = require('clone');
var tsurgeon     = require('tree-surgeon');

var template = {
	tid:     null,
	tname:   null,
	tplural: null,
	tslug:   null,
	tcode:   null,
	tmodel:  null,
	tcount: {
		ttext:   null,
		tenable: null,
		texport: null,
	},
	tlist: {
		tfields: [],
		ttext:   null,
		tlinks:  null,
		tenable: null,
		texport: null,
	},
	tread: {
		tfields: [],
		ttext:   null,
		tenable: null,
		texport: null,
	},
	tedit: {
		tfields: [],
		ttext:   null,
		tenable: null,
		texport: null,
	},
	tcreate: {
		tfields: [],
		ttext:   null,
		tenable: null,
		texport: null,
	},
	tdelete: {
		ttext:   null,
		tenable: null,
		texport: null,
	},
	tverify: {
		ttext:   null,
		tenable: null,
		texport: null,
	},
	tmeta: {
		tfields:   null,
		tvalidate: null,
		tserver:   null,
		texport:   null,
	},
};

function copyByTemplate(prefix,objectDest,objectSrc,copyTemplate) {
	var keys = Object.keys(copyTemplate);
	var result = [];
	for (var i = 0; i < keys.length; i++) {
		var key = keys[i];
		var value = objectSrc[key];
		if (!value) {
			//console.log(prefix+'.'+key+' src object has no value.');
			continue; // no src value
		}
		var templateValue = copyTemplate[key];
		if (templateValue === null) {
			if (objectDest[key] !== null) {
				//console.log(prefix+'.'+key+' has own value: '+objectDest[key]);
				continue;
			} else {
				//console.log(prefix+'.'+key+' copy value: '+value);
				objectDest[key] = clone(value);	
			}
		} else {
			//console.log(prefix+'.'+key+' going deeper');
			copyByTemplate(prefix+'.'+key,objectDest[key],value,templateValue)
		}
	}
}

function addFilteredFields(tview,type,tcrud) {
	result = [];
	var keys = Object.keys(tcrud.tmeta.tfields);
	var result = [];
	for (var i = 0; i < keys.length; i++) {
		var key = keys[i];
		var tfield = tcrud.tmeta.tfields[key];
		if (tfield[type]) {
			if (!tfield[type].tenable) {
				//console.log('------------- type: ' + type + ' key: ' + key + ' res: ' + tfield[type].tenable)
				continue;
			}
		}
		result.push(key); // default is true..
	}
	tview[type].tfields = result;
}

function forceLookupKeySimple() {
	var low  = 100000;
	var high = 999999;
	return Math.floor(Math.random() * (high - low + 1) + low).toString(16).toUpperCase();
}

function forceLookupTFields(tview) {
	var keys = Object.keys(tview.tmeta.tfields);
	for (var i = 0; i < keys.length; i++) {
		var key = keys[i];
		var keyNew = 'FTL_' +forceLookupKeySimple() + '_' + key.substring(key.length/3,key.length/3*2); // no....its; Force template lookup 
		var tfield = tview.tmeta.tfields[key];
		tview.tmeta.tfields[key] = undefined;
		tview.tmeta.tfields[keyNew] = tfield;

		var ckeys = Object.keys(tview);
		for (var ii = 0; ii < ckeys.length; ii++) {
			var ckey = ckeys[ii];
			if (ckey === 'tmeta') {
				continue;
			}
			var obj = tview[ckey];
			if (obj && obj.tfields) {
				var tfieldsNew = [];
				for (var iii = 0; iii < obj.tfields.length; iii++) {
					var tkey = obj.tfields[iii];
					if (tkey === key) {
						tfieldsNew.push(keyNew);
					} else {
						tfieldsNew.push(tkey);
					}
				}
				obj.tfields = tfieldsNew;
			}
		}
	}
}

function formatValuePart(value,replaceChar,splitChar,partFn) {
	var result = '';
	var parts = value.split(splitChar);
	for (var i = 0; i < parts.length; i++) {
		var part = parts[i];
		if (part.length > 1) {
			result += partFn(part);
		} else {
			result += part;
		}
		if (i < parts.length - 1) {
			result += replaceChar;
		}
	}
	return result;
}

function formatValue(value,removeChars,replaceChar,partFn) {
	for (var i = 0; i < removeChars.length; i++) {
		value = formatValuePart(value,replaceChar,removeChars[i],partFn);
	}
	return value;
}

exports.newInstance = function (tcrud) {
	var tview = clone(template);
	var tid = tcrud.tid;
	tview.tid = tid;
	
	copyByTemplate('tview',tview,tcrud,template);
	
	//console.info('TCrudView.tslug: '+tview.tslug);
	
	if (!tview.tslug) {
		tview.tslug = formatValue(tid,['.',',','/','=','&','?',' '],'', function (part) {
			return part; // todo use fully correct uri removeal and escaping.
		})
		//console.info('TCrudView.tslug auto: '+tview.tslug);
	}
	if (!tview.tname) {
		tview.tname = formatValue(tid,['_','-','.',','],' ', function (part) {
			return part.substring(0,1).toUpperCase()+part.substring(1).toLowerCase();
		});
	}
	if (!tview.tcode) {
		tview.tcode = formatValue(tid,[' ','_','-','.',','],'',function (part) {
			return part.toLowerCase();
		});
	}
	
	if (!tview.tplural) {
		tview.tplural = tview.tname + 's'; 
	}
	
	addFilteredFields(tview,'tlist',tcrud);
	addFilteredFields(tview,'tread',tcrud);
	addFilteredFields(tview,'tedit',tcrud);
	addFilteredFields(tview,'tcreate',tcrud);
	
	forceLookupTFields(tview);
	
	return tview;
}

