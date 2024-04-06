/*
  Prerequisites:

    1. Install node.js and npm ( I used node-v10.15.3-x64.msi )
    2. npm install

  See also,

    http://einaros.github.com/ws/

  To run,
    (npm update if you haven't in a long time)
    node server.js
*/

"use strict"; // http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/

var readline = require('readline')
  , events = require('events')
  , util = require('util')
  , WebSocketServer = require('ws').Server
  , http = require('http')
  , bigInt = require("big-integer");

////////////////////////////////////////////////////////////////////////////////

function Console() {
  if (!(this instanceof Console)) return new Console();

  this.stdin = process.stdin;
  this.stdout = process.stdout;

  this.readlineInterface = readline.createInterface(this.stdin, this.stdout);

  var self = this;

  this.readlineInterface.on('line', function line(data) {
    self.emit('line', data);
  }).on('close', function close() {
    self.emit('close');
  });
}

util.inherits(Console, events.EventEmitter);

Console.prototype.print = function print(msg) {
   this.stdout.write(msg + '\n');
};

var ws = null;
var wsConsole = new Console();
var server = http.createServer();
var wss = new WebSocketServer({server: server, path: '/mirv'});

wsConsole.on('close', function close() {
  if (ws) ws.close();
  process.exit(0);
});

wsConsole.on('line', function line(data) {
  if (ws) {
	//ws.send(new Uint8Array(Buffer.from('exec\0'+data.trim()+'\0','utf8')),{binary: true});
	ws.send(data);
  }
});

wss.on('connection', function(newWs) {
	if(ws)
	{
		ws.close();
		ws = newWs;
	}
	
	ws = newWs;
    
	wsConsole.print('/mirv	 connected');
	
    ws.on('message', function(data) {
		if (typeof data == "string") {
			wsConsole.print(data);
		}
        if (data instanceof Buffer)
		{
			wsConsole.print(data.toString());
		}
    });
    ws.on('close', function(code,reason) {
      wsConsole.print('Connection closed: '+code.toString()+" / "+reason);
    });
    ws.on('error', function(e) {
    });
});
server.listen(31337);
wsConsole.print('Listening on port 31337, path /mirv ...');
