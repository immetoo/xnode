'use strict';

var crudRouteInit = [];
var pageRouteInit = [];

angular.module('xdsUI', ['ngRoute','xdsUI.filters', 'xdsUI.services', 'xdsUI.directives']).
	config(['$routeProvider', '$locationProvider', function($routeProvider, $locationProvider) {

		// init page controllers
		for (var i = 0; i < pageRouteInit.length; i++) {
			pageRouteInit[i]($routeProvider, $locationProvider);
		}
		// init crud controllers
		for (var i = 0; i < crudRouteInit.length; i++) {
			crudRouteInit[i]($routeProvider, $locationProvider);
		}
		
		//$routeProvider.otherwise("/404", {
		//      templateUrl: "partials/404.html",
		//      controller: "PageCtrl"
		//});
		
		$routeProvider.otherwise({ redirectTo: '/ui' });
		// todo: add ie9 warning, base kills svg url property
		$locationProvider.html5Mode({enabled: true, requireBase: false});
	}]);

