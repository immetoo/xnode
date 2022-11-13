
pageRouteInit.push(function ($routeProvider, $locationProvider) {
	$routeProvider.when('/ui/server/about', {
		templateUrl: '/ui/thtml/server/about',
		controller: XPageServerAbout
	});
});

function XPageServerAbout($scope, $http) {
	//$http.get('/api/posts').success(function(data, status, headers, config) {
	//      $scope.posts = data.posts;
	//});
}
