
pageRouteInit.push(function ($routeProvider, $locationProvider) {
	$routeProvider.when('/ui', {
		templateUrl: '/ui/thtml/server/index',
		controller: XPageServerIndex
	});
});

function XPageServerIndex($scope, $http) {
	//$http.get('/api/posts').success(function(data, status, headers, config) {
	//      $scope.posts = data.posts;
	//});
}
