/*
  Prerequisites:

    1. Install node.js and npm ( I used node-v10.15.3-x64.msi )
    2. npm install

  See also,

    http://einaros.github.com/ws/

  To run,
    (npm update if you haven't in a long time)
    node server.js

  Hints:

  - Text entered (with enter) is sent to client as exec.
  - You might want to whitelist / blacklist events from being transmitted if you need to reduce the data transmitted.
*/

"use strict"; // http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/

var readline = require('readline')
  , events = require('events')
  , util = require('util')
  , WebSocketServer = require('ws').Server
  , http = require('http')
  , bigInt = require("big-integer");

////////////////////////////////////////////////////////////////////////////////

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

function BufferReader(buffer)
{
	this.buffer = buffer
	this.index = 0;
}

BufferReader.prototype.readBigUInt64LE = function readBigUInt64LE(base) {
	
	var lo = this.readUInt32LE()
	var hi = this.readUInt32LE();
	
	return bigInt(lo).or(bigInt(hi).shiftLeft(32));
};

BufferReader.prototype.readUInt32LE = function readInt32LE() {
	var result = this.buffer.readUInt32LE(this.index);
	this.index += 4;
	
	return result;
};

BufferReader.prototype.readInt32LE = function readInt32LE() {
	var result = this.buffer.readInt32LE(this.index);
	this.index += 4;
	
	return result;
};

BufferReader.prototype.readInt16LE = function readInt16LE() {
	var result = this.buffer.readInt16LE(this.index);
	this.index += 2;
	
	return result;
};

BufferReader.prototype.readInt8 = function readInt8() {
	var result = this.buffer.readInt8(this.index);
	this.index += 1;
	
	return result;
};

BufferReader.prototype.readUInt8 = function readUInt8() {
	var result = this.buffer.readUInt8(this.index);
	this.index += 1;
	
	return result;
};

BufferReader.prototype.readBoolean = function readBoolean() {
	return 0 != this.readUInt8();
};

BufferReader.prototype.readFloatLE = function readFloatLE() {
	var result = this.buffer.readFloatLE(this.index);
	this.index += 4;
	
	return result;
};

BufferReader.prototype.readCString = function readCString()
{
	var delim = findDelim(this.buffer, this.index);
	if(this.index <= delim)
	{
		var result = this.buffer.toString('utf8', this.index, delim);
		this.index = delim + 1;
		
		return result;
	}
	
	throw new "BufferReader.prototype.readCString"; 
}

BufferReader.prototype.eof = function eof()
{
	return this.index >= this.buffer.length;
}

// GameEventUnserializer ///////////////////////////////////////////////////////

function GameEventDescription(bufferReader)
{
	this.eventId = bufferReader.readInt32LE();
	this.eventName = bufferReader.readCString();
	this.keys = [];
	this.enrichments = null;
	
	while(bufferReader.readBoolean())
	{
		var keyName = bufferReader.readCString();
		var keyType = bufferReader.readInt32LE();
		
		this.keys.push({
			name: keyName,
			type: keyType
		});
	}
}

GameEventDescription.prototype.unserialize = function unserialize(bufferReader)
{
	var clientTime = bufferReader.readFloatLE();
	
	var result = {
		name: this.eventName,
		clientTime: clientTime,
		keys: {}
	};
	
	for(var i=0; i < this.keys.length; ++i)
	{
		var key = this.keys[i];
		
		var keyName = key.name;
		
		var keyValue;
		
		switch(key.type)
		{
		case 1:
			keyValue = bufferReader.readCString();
			break;
		case 2:
			keyValue = bufferReader.readFloatLE();
			break;
		case 3:
			keyValue = bufferReader.readInt32LE();
			break;
		case 4:
			keyValue = bufferReader.readInt16LE();
			break;
		case 5:
			keyValue = bufferReader.readInt8();
			break;
		case 6:
			keyValue = bufferReader.readBoolean();
			break;
		case 7:
			keyValue = bufferReader.readBigUInt64LE();
			break;
		default:
			throw new "GameEventDescription.prototype.unserialize";
		}
		
		if(this.enrichments && this.enrichments[keyName])
		{
			keyValue = this.enrichments[keyName].unserialize(bufferReader, keyValue);
		}
		
		result.keys[key.name] = keyValue;
	}
	
	return result;
}

function UseridEnrichment()
{
	this.enrichments = [
		'useridWithSteamId'
		, 'useridWithEyePosition'
		, 'useridWithEyeAngles'
	];
}

UseridEnrichment.prototype.unserialize = function unserialize(bufferReader, keyValue)
{
	var xuid = bufferReader.readBigUInt64LE().toString();
	var eyeOrigin = [bufferReader.readFloatLE(), bufferReader.readFloatLE(), bufferReader.readFloatLE()];
	var eyeAngles = [bufferReader.readFloatLE(), bufferReader.readFloatLE(), bufferReader.readFloatLE()];
	
	return {
		value: keyValue,
		xuid: xuid,
		eyeOrigin: eyeOrigin,
		eyeAngles: eyeAngles,
	};
}

function EntitynumEnrichment()
{
	this.enrichments = [
		'entnumWithOrigin'
		, 'entnumWithAngles'
	];
}

EntitynumEnrichment.prototype.unserialize = function unserialize(bufferReader, keyValue)
{
	var origin = [bufferReader.readFloatLE(), bufferReader.readFloatLE(), bufferReader.readFloatLE()];
	var angles = [bufferReader.readFloatLE(), bufferReader.readFloatLE(), bufferReader.readFloatLE()];
	
	return {
		value: keyValue,
		origin: origin,
		angles: angles,
	};
}

function GameEventUnserializer(enrichments)
{
	this.enrichments = enrichments; 
	this.knownEvents = {}; // id -> description	
}

GameEventUnserializer.prototype.unserialize = function unserialize(bufferReader)
{
	var eventId = bufferReader.readInt32LE();
	var gameEvent;
	
	if(0 == eventId)
	{
		gameEvent = new GameEventDescription(bufferReader);
		this.knownEvents[gameEvent.eventId] = gameEvent;
		
		if(this.enrichments[gameEvent.eventName]) gameEvent.enrichments = this.enrichments[gameEvent.eventName];
	}
	else gameEvent = this.knownEvents[eventId];
	
	if(undefined === gameEvent) throw new "GameEventUnserializer.prototype.unserialize";
	
	return gameEvent.unserialize(bufferReader);
}

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
    ws.send(new Uint8Array(Buffer.from('exec\0'+data.trim()+'\0','utf8')),{binary: true});
  }
});

var useridEnrichment = new UseridEnrichment();
var entitynumEnrichment = new EntitynumEnrichment();

// ( see https://wiki.alliedmods.net/Counter-Strike:_Global_Offensive_Events )
var enrichments = {
	'player_death' : {
		'userid': useridEnrichment,
		'attacker': useridEnrichment,
		'assister': useridEnrichment,
	},
	'other_death' : {
		'attacker': useridEnrichment,
	},
	'player_hurt' : {
		'userid': useridEnrichment,
		'attacker': useridEnrichment,
	},
	'item_purchase' : {
		'userid': useridEnrichment,
	},
	'bomb_beginplant' : {
		'userid': useridEnrichment,
	},
	'bomb_abortplant' : {
		'userid': useridEnrichment,
	},
	'bomb_planted' : {
		'userid': useridEnrichment,
	},
	'bomb_defused' : {
		'userid': useridEnrichment,
	},
	'bomb_exploded' : {
		'userid': useridEnrichment,
	},
	'bomb_pickup' : {
		'userid': useridEnrichment,
	},
	'bomb_dropped' : {
		'userid': useridEnrichment,
		'entindex': entitynumEnrichment,
	},
	'defuser_dropped': {
		'entityid': entitynumEnrichment,
	},
	'defuser_pickup' : {
		'entityid': entitynumEnrichment,
		'userid': useridEnrichment,
	},
	'bomb_begindefuse' : {
		'userid': useridEnrichment,
	},
	'bomb_abortdefuse' : {
		'userid': useridEnrichment,
	},
	'hostage_follows' : {
		'userid': useridEnrichment,
		'hostage': entitynumEnrichment,
	},
	'hostage_hurt' : {
		'userid': useridEnrichment,
		'hostage': entitynumEnrichment,
	},
	'hostage_killed' : {
		'userid': useridEnrichment,
		'hostage': entitynumEnrichment,
	},	
	'hostage_rescued' : {
		'userid': useridEnrichment,
		'hostage': entitynumEnrichment,
	},	
	'hostage_stops_following' : {
		'userid': useridEnrichment,
		'hostage': entitynumEnrichment,
	},
	'hostage_call_for_help': {
		'hostage': entitynumEnrichment,
	},
	'vip_escaped' : {
		'userid': useridEnrichment,
	},		
	'player_radio' : {
		'userid': useridEnrichment,
	},
	'bomb_beep' : {
		'entindex': entitynumEnrichment,
	},
	'weapon_fire' : {
		'userid': useridEnrichment,
	},	
	'weapon_fire_on_empty' : {
		'userid': useridEnrichment,
	},	
	'grenade_thrown' : {
		'userid': useridEnrichment,
	},	
	'weapon_outofammo' : {
		'userid': useridEnrichment,
	},	
	'weapon_reload' : {
		'userid': useridEnrichment,
	},	
	'weapon_zoom' : {
		'userid': useridEnrichment,
	},	
	'silencer_detach' : {
		'userid': useridEnrichment,
	},	
	'inspect_weapon' : {
		'userid': useridEnrichment,
	},	
	'weapon_zoom_rifle' : {
		'userid': useridEnrichment,
	},	
	'player_spawned' : {
		'userid': useridEnrichment,
	},	
	'item_pickup' : {
		'userid': useridEnrichment,
	},	
	'item_pickup_failed' : {
		'userid': useridEnrichment,
	},	
	'item_remove' : {
		'userid': useridEnrichment,
	},	
	'ammo_pickup' : {
		'userid': useridEnrichment,
		'index': entitynumEnrichment,
	},	
	'item_equip' : {
		'userid': useridEnrichment,
	},
	'enter_buyzone' : {
		'userid': useridEnrichment,
	},
	'exit_buyzone' : {
		'userid': useridEnrichment,
	},
	'enter_bombzone' : {
		'userid': useridEnrichment,
	},
	'exit_bombzone' : {
		'userid': useridEnrichment,
	},
	'enter_rescue_zone' : {
		'userid': useridEnrichment,
	},
	'exit_rescue_zone' : {
		'userid': useridEnrichment,
	},
	'silencer_off' : {
		'userid': useridEnrichment,
	},
	'silencer_on' : {
		'userid': useridEnrichment,
	},
	'buymenu_open' : {
		'userid': useridEnrichment,
	},
	'buymenu_close' : {
		'userid': useridEnrichment,
	},
	'round_end' : {
		'winner': useridEnrichment,
	},
	'grenade_bounce' : {
		'userid': useridEnrichment,
	},
	'hegrenade_detonate' : {
		'userid': useridEnrichment,
	},
	'flashbang_detonate' : {
		'userid': useridEnrichment,
	},
	'smokegrenade_detonate' : {
		'userid': useridEnrichment,
	},
	'smokegrenade_expired' : {
		'userid': useridEnrichment,
	},
	'molotov_detonate' : {
		'userid': useridEnrichment,
	},
	'decoy_detonate' : {
		'userid': useridEnrichment,
	},
	'decoy_started' : {
		'userid': useridEnrichment,
	},
	'tagrenade_detonate' : {
		'userid': useridEnrichment,
	},
	'decoy_firing' : {
		'userid': useridEnrichment,
	},
	'bullet_impact' : {
		'userid': useridEnrichment,
	},
	'player_footstep' : {
		'userid': useridEnrichment,
	},
	'player_jump' : {
		'userid': useridEnrichment,
	},
	'player_blind' : {
		'userid': useridEnrichment,
		'entityid': entitynumEnrichment,
	},
	'player_falldamage' : {
		'userid': useridEnrichment,
	},
	'door_moving' : {
		'entityid': entitynumEnrichment,
		'userid': useridEnrichment,
	},
	'spec_target_updated' : {
		'userid': useridEnrichment,
	},
	'player_avenged_teammate' : {
		'avenger_id': useridEnrichment,
		'avenged_player_id': useridEnrichment,
	},
	'round_mvp' : {
		'userid': useridEnrichment,
	},
	'player_decal' : {
		'userid': useridEnrichment,
	},
	
	// ... left out the gg / gungame shit, feel free to add it ...
	
	'player_reset_vote' : {
		'userid': useridEnrichment,
	},
	'start_vote' : {
		'userid': useridEnrichment,
	},
	'player_given_c4' : {
		'userid': useridEnrichment,
	},
	'player_become_ghost' : {
		'userid': useridEnrichment,
	},

	// ... left out the tr shit, feel free to add it ...
		
	'jointeam_failed' : {
		'userid': useridEnrichment,
	},
	'teamchange_pending' : {
		'userid': useridEnrichment,
	},
	'ammo_refill' : {
		'userid': useridEnrichment,
	},
	
	// ... left out the dangerzone shit, feel free to add it ...
	
	// others:
	
	'weaponhud_selection' : {
		'userid': useridEnrichment,
	},
};

wss.on('connection', function(newWs) {
	if(ws)
	{
		ws.close();
		ws = newWs;
	}
	
	ws = newWs;
    
	wsConsole.print('/mirv	 connected');
	
	var gameEventUnserializer = new GameEventUnserializer(enrichments);
	
    ws.on('message', function(data) {
        if (data instanceof Buffer)
		{
			var bufferReader = new BufferReader(Buffer.from(data));
			
			try
			{
				while(!bufferReader.eof())
				{
					var cmd = bufferReader.readCString();
					wsConsole.print(cmd);
					
					switch(cmd)
					{
					case 'hello':
						{
							var version = bufferReader.readUInt32LE();
							wsConsole.print('version = '+version);
							if(2 != version) throw "Error: version mismatch";
							
							ws.send(new Uint8Array(Buffer.from(
								'transBegin\0'
							,'utf8')), {binary: true});
							
							ws.send(new Uint8Array(Buffer.from(
								'exec\0mirv_pgl events enrich clientTime 1\0','utf8'
							)), {binary: true});
							
							for(var eventName in enrichments)
							{
								for(var keyName in enrichments[eventName])
								{
									var arrEnrich = enrichments[eventName][keyName].enrichments;
									
									for(var i=0; i < arrEnrich.length; ++i)
									{
										ws.send(new Uint8Array(Buffer.from(
											'exec\0mirv_pgl events enrich eventProperty "'+arrEnrich[i]+'" "'+eventName+'" "'+keyName+'"\0'
										,'utf8')), {binary: true});
									}
								}
							}
							
							ws.send(new Uint8Array(Buffer.from(
								'exec\0mirv_pgl events enabled 1\0'
							,'utf8')), {binary: true});
							
							ws.send(new Uint8Array(Buffer.from(
								'exec\0mirv_pgl events useCache 1\0'
							,'utf8')), {binary: true});

							ws.send(new Uint8Array(Buffer.from(
								'transEnd\0'
							,'utf8')), {binary: true});
						}
						break;
					case 'dataStart':
						break;
					case 'dataStop':
						break;
					case 'levelInit':
						{
							var map = bufferReader.readCString();
							wsConsole.print('map = '+map);
						}
						break;
					case 'levelShutdown':
						break;
					case 'cam':
						{
							var time = bufferReader.readFloatLE();
							wsConsole.print('time = '+time);
							var xPosition = bufferReader.readFloatLE();
							wsConsole.print('xPosition = '+xPosition);
							var yPosition = bufferReader.readFloatLE();
							wsConsole.print('yPosition = '+yPosition);
							var zPosition = bufferReader.readFloatLE();
							wsConsole.print('zPosition = '+zPosition);
							var xRotation = bufferReader.readFloatLE();
							wsConsole.print('xRotation = '+xRotation);
							var yRotation = bufferReader.readFloatLE();
							wsConsole.print('yRotation = '+yRotation);
							var zRotation = bufferReader.readFloatLE();
							wsConsole.print('zRotation = '+zRotation);
							var fov = bufferReader.readFloatLE();
							wsConsole.print('fov = '+fov);
						}
						break;
					case 'gameEvent':
						{
							var gameEvent = gameEventUnserializer.unserialize(bufferReader);
							wsConsole.print(JSON.stringify(gameEvent));
						}
						break;
					default:
						throw "Error: unknown message";
					}
				}
			}
			catch(err)
			{
				wsConsole.print('Error: '+err.toString()+' at '+bufferReader.index+'.');
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
wsConsole.print('Listening on port 31337, path /mirv ...');
