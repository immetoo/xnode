module.exports = {
  winston: {
    console: {
      level: 'debug',
      silent: false,
      colorize: true,
      timestamp: null
    },
    file: {
      level: 'info',
      silent: false,
      colorize: false,
      json: false,
      //timestamp: null,
      //maxFiles: 100,
      //maxsize: 1024*1024*1024,
      filename: 'www_logs/server.log'
    }
  },
  mongo: {
    url:  process.env.MONGO_URL || 'mongodb://localhost:27017/xnode-debug-server',
    options : {
      //user: 'myUserName',
      //pass: 'myPassword',
      db: {
        fsync: false,
        journal: false,
        native_parser: true,
        forceServerObjectId: true
      },
      server: {
        poolSize: 4,
        socketOptions: {
          connectTimeoutMS: 500,
          keepAlive: 1,
          auto_reconnect: true
        }
      }
    }
  },
  server: {
    httpPort: process.env.HTTP_PORT || 8008,
    httpTrustProxy: true,
    sessionSecret: 'debugSecret',
    sessionTTL: 14 * 24 * 60 * 60, // = 14 days. Default
  },
  application: {
    name: 'Xnode Debug Server',
    index: {
      pageTitle: 'Xnode Debug Server',
      pageKeywords: 'xnode,sensor,data,weather,wireless',
    },
  },
  options: {
    static: {
      maxAge: 86400000
    },
    morgan: {
      logLevel: process.env.LOG_MORGAN || 'dev'
    },
    cookieParser: {
      secretKey: 'DeBugMe@11'
    },
    rss: {
      link: 'http://localhost:8008',
      author: 'Xnode',
      options: {
      }
    }
  }
};