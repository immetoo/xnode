var clone        = require('clone');

var template = {
	tid:     null,
	tname:   null,
	tplural: null,
	tslug:   null,
	tmodel:  null,
	tenable: true,
	tparam:  null,
	troles:  [],
	tparent: null,
	tchilds: [],
	tlist: {
		tenable: true,
		ttext:   'Listing of ..',
		troles:  [],
		tserver: {
			tpipe: {
				query: {
					defaults: { limit: '10' },
					maxes: { limit: 50 },
				},
			},
		},
		texport: {
			json: {
				tslug: 'list',
			},
			xml: {
				tslug: 'list',
			},
			rss: {
				tslug: 'list',
			},
			csv: {
				tslug: 'list',
			},
			angular: {
				tslug: 'list',
				thtml: 'crud/list',
				tcontroller: {
					prefix: 'tcrudAuto',
					postfix: 'ListCntr',
					argu: '$scope, $http, $location, $routeParams',
				},
				tlinks: {
					'DataTODO':'/ui/XNodeData/list/XNode/{{row.net_id}}/{{row.net_id}}',
				},
			},
		},
	},
	tcreate: {
		tenable: true,
		ttext:   null,
		troles:  [],
		texport: {
			json: {
				tslug: 'create',
			},
			xml: {
				tslug: 'create',
			},
			angular: {
				tslug: 'create',
				thtml: 'crud/create',
				tcontroller: {
					prefix: 'tcrudAuto',
					postfix: 'CreateCntr',
					argu: '$scope, $http, $location, $routeParams',
				},
			},
		},
	},
	tedit: {
		tenable: true,
		ttext:   null,
		troles:  [],
		texport: {
			json: {
				tslug: 'edit',
			},
			xml: {
				tslug: 'edit',
			},
			angular: {
				tslug: 'edit',
				thtml: 'crud/edit',
				tcontroller: {
					prefix: 'tcrudAuto',
					postfix: 'EditCntr',
					argu: '$scope, $http, $location, $routeParams',
				},
			},
		},
	},
	tread: {
		tenable: true,
		ttext:   null,
		troles:  [],
		tslug:   '',
		texport: {
			json: {
				tslug: 'read',
			},
			xml: {
				tslug: 'read',
			},
			csv: {
				tslug: 'read',
			},
			angular: {
				tslug: 'read',
				thtml: 'crud/read',
				troute: {
					
				},
				tcontroller: {
					prefix: 'tcrudAuto',
					postfix: 'ReadCntr',
					argu: '$scope, $http, $location, $routeParams',
				},
			},
		},
	},
	tdelete: {
		tenable: true,
		ttext:   null,
		troles:  [],
		texport: {
			json: {
				tslug: 'delete',
			},
			xml: {
				tslug: 'delete',
			},
			angular: {
				tslug: 'delete',
				thtml: 'crud/delete',
				tcontroller: {
					prefix: 'tcrudAuto',
					postfix: 'DeleteCntr',
					argu: '$scope, $http, $location, $routeParams',
				},
			},
		},
	},
	tcount: {
		tenable: true,
		ttext:   'Count records',
		troles:  [],
		texport: {
			json: {
				tslug: 'list-count',
			},
			xml: {
				tslug: 'list-count',
			},
			csv: {
				tslug: 'list-count',
			},
		},
	},
	tverify: {
		tenable: true,
		ttext:   'Verify data',
		troles:  [],
		texport: {
			json: {
				tslug: 'verify',
			},
			xml: {
				tslug: 'verify',
			},
			csv: {
				tslug: 'verify',
			},
		},
	},
	ttemplate: {
		tid: '_id',
		tname: '_id',
		tdescription: '_id',
	},
	tmeta: {
		troles: [],
		tfields: {},
		tvalidate: {
			io:     { 'admin_role': 'object|properties[test]' },
		},
		tserver: {
			thost: 'http://localhost:8080/',
			tslug: '/api',
			tpopfix: true,
			texports: ['json','xml','csv','rss','angular'],
		},
		texport: {
			json: {
				tslug: 'json',
			},
			xml: {
				tslug: 'xml',
			},
			csv: {
				tslug: 'csv',
			},
			rss: {
				tslug: 'rss',
				itemTitle: '$data._id',
				itemLink: '$siteLink/ui/$modelUri/read/$[\'data._id\']',
				itemDate: '',
				itemDescription: '',
				//itemFields: '',
			},
			angular: {
				tslug: 'angular',
				tbase: '/ui',
				thtml: 'thtml',
			},
		},
	},
};

exports.newInstance = function (tid) {
	var tcrud = clone(template);
	tcrud.tid = tid;
	tcrud.tslug = tid; // view fills more defaults.
	return tcrud;
}

