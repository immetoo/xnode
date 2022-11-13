
pageRouteInit.push(function ($routeProvider, $locationProvider) {
	$routeProvider.when('server/dash', {
		templateUrl: '/ui/thtml/server/dash',
		controller: XPageServerDash
	});
});

function XPageServerDash($scope, $http) {
	//$http.get('/api/posts').success(function(data, status, headers, config) {
	//      $scope.posts = data.posts;
	//});
}

