'use strict';

var tcrud = {
	factory: {
		express:       require('./factory-express'),
	},
	build: {
		config:        require('./build-config'),
		view:          require('./build-view'),
		backend: {
			mongoose:  require('./build-backend-mongoose'),
		},
	}
};

module.exports = tcrud;

/*

	//app.use(function (req, res, next) {res.locals.c_var='test';next();});


var http = require('http');
var ejs = require('ejs');
var fs = require('fs');
http.createServer(function (req, res) {
res.writeHead(200, {'Content-Type': 'text/xml'});
fs.readFile('placemark.ejs', 'utf8', function (err, template) {

var content = ejs.render(template,{
    name:"test name",
    description:"this is the description",
    coordinates:"-122.0822035425683,37.42228990140251,0",
    filename: __dirname + '/placemark.ejs'
});

res.write(content);
res.end()
});
}).listen(8000);


    var scripts = document.getElementsByTagName("script");
    var currentScriptPath = scripts[scripts.length-1].src;
    console.log("currentScriptPath:"+currentScriptPath);
    var baseUrl = currentScriptPath.substring(0, currentScriptPath.lastIndexOf('/') + 1);
    console.log("baseUrl:"+baseUrl);
    var bu2 = document.querySelector("script[src$='routes.js']");
    currentScriptPath = bu2.src;
    console.log("bu2:"+bu2);
    console.log("src:"+bu2.src);
    baseUrl = currentScriptPath.substring(0, currentScriptPath.lastIndexOf('/') + 1);
    console.log("baseUrl:"+baseUrl);
*/