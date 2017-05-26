/*
  Prerequisites:

    1. Install node.js and npm
    2. npm install ws

  See also,

    http://einaros.github.com/ws/

  To run,

    node server.js

  Hint:

    Text entered (with enter) is sent to client as exec.
*/

"use strict"; // http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/

var readline = require('readline')
  , events = require('events')
  , util = require('util')
  , WebSocketServer = require('ws').Server
  , http = require('http');

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
    ws.send(new Uint8Array(Buffer.from('exec\0'+data.trim()+'\0','utf8')),{binary: true});
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
        if (data instanceof Buffer)
		{
			var buffer = Buffer.from(data);
			var idx = 0;
			while(idx < buffer.length)
			{
				function findDelim(buffer,idx)
				{
					var delim = -1;
					for(var i = idx; i < buffer.length; ++i)
					{
						if(0 == buffer[i])
						{
							delim = i;
							break;
						}
					}
					
					return delim;
				}
				
				var delim = findDelim(buffer,idx);
				
				try
				{
					if(idx <= delim)
					{
						var cmd = buffer.toString('utf8',idx,delim);
						idx = delim + 1;
						wsConsole.print(cmd);
						switch(cmd)
						{
						case 'hello':
							if(4 <= buffer.length -idx)
							{
								var version = buffer.readUInt32LE(idx);
								wsConsole.print('version = '+version);
								idx += 4;
								if(0 == version)
									continue;
							}
							break;
						case 'levelInit':
							{
								var delim = findDelim(buffer, idx);
								if(idx <= delim)
								{
									var map = buffer.toString('utf8',idx,delim);
									wsConsole.print('map = '+map);
									idx = delim + 1;
									continue;
								}
							}
							break;
						case 'levelShutdown':
							continue;
							break;
						case 'cam':
							if(8*8 <= buffer.length -idx)
							{
								var time = buffer.readDoubleLE(idx);
								wsConsole.print('time = '+time);
								idx += 8;
								var xPosition = buffer.readDoubleLE(idx);
								wsConsole.print('xPosition = '+xPosition);
								idx += 8;
								var yPosition = buffer.readDoubleLE(idx);
								wsConsole.print('yPosition = '+yPosition);
								idx += 8;
								var zPosition = buffer.readDoubleLE(idx);
								wsConsole.print('zPosition = '+zPosition);
								idx += 8;
								var xRotation = buffer.readDoubleLE(idx);
								wsConsole.print('xRotation = '+xRotation);
								idx += 8;
								var yRotation = buffer.readDoubleLE(idx);
								wsConsole.print('yRotation = '+yRotation);
								idx += 8;
								var zRotation = buffer.readDoubleLE(idx);
								wsConsole.print('zRotation = '+zRotation);
								idx += 8;
								var fov = buffer.readDoubleLE(idx);
								wsConsole.print('fov = '+fov);
								idx += 8;
								continue;
							}
							break;
						}
					}
				}
				catch(err)
				{
					wsConsole.print('Error: '+err+'.');
				}
			
				wsConsole.print('Error: Invalid data received at index '+idx+'.');
				break;
			}
		}
    });
    ws.on('close', function() {
      wsConsole.print('Connection closed!');
    });
    ws.on('error', function(e) {
    });
});
server.listen(31337);
wsConsole.print('Listening on port 31337...');
