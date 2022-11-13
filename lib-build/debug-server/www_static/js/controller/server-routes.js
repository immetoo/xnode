
pageRouteInit.push(function ($routeProvider, $locationProvider) {
	$routeProvider.when('/ui/server/routes', {
		redirectTo: '/ui/server/routes/tech',
	});	
	$routeProvider.when('/ui/server/routes/tech', {
		templateUrl: '/ui/thtml/server/routes?group1=all,rss,json&group2=csv,xml,ui,angular',
		controller: XPageServerTechRoutes,
	});
	$routeProvider.when('/ui/server/routes/model', {
		templateUrl: '/ui/thtml/server/routes?group1=xnode,xnode_base,xnode-base-command&group2=xnode-data,xnode-data-value,xsystem-state,all',
		controller: XPageServerModelRoutes,
	});
});

function XPageServerTechRoutes($scope, $http) {
	$http.get('/api/json/server/routes?groups=json,xml,rss,csv,ui,angular').success(function(data, status, headers, config) {
		$scope.serverRoutes = data.data;
	});
}

function XPageServerModelRoutes($scope, $http) {
	$http.get('/api/json/server/routes?groups=xnode,xnode-base,xnode-base-command,xnode-data,xnode-data-value,system-state').success(function(data, status, headers, config) {
		$scope.serverRoutes = data.data;
	});
}
