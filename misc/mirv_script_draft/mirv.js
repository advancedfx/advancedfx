var wsAddress = wsAddress || 'ws://localhost:31337/mirv';
var wsEnable = wsEnable || true;

// Usage:
//
// 1. Start e.g. the server.js in mirv_ws_server.
// 2. Enter in game console (adjust as needed): mirv_script_exec "wsAddress = 'ws://localhost:31337/mirv'; wsEnable = true"
// 3. Enter in game console (adjust as needed): mirv_script_load "C:\source\advancedfx\misc\mirv_script_draft\mirv.js"
//
// Afterwards you can use:
// mirv_script_exec "wsEnable = false"
// mirv_script_exec "wsEnable = true"

////////////////////////////////////////////////////////////////////////////////

{
	let setView = null;
	let tickCount = 0;
	var wsConnection = null;

	function WsConnection(options) {
		let self = this;
		this.closed = false;
		this.exception = null;
		this.onException = options.onException || null;
		this.wsOut = null;
		this.wsOutResolve = null;
		this.wsOutReject = null;
		this.wsOutNext = new Promise((resolve,reject)=>{
			self.wsOutResolve = resolve;
			self.wsOutReject = reject;
		});
		this.wsInBuffer = [];

		async function readNext(wsIn) {
			try {
				while(true) {
					let message = await wsIn.next();
					if(message === null) {
						wsIn.drop();
						self.wsOut.drop();
						this.closed = true;
						break;
					}
					switch(typeof message) {
						case 'string':
							self.wsInBuffer.push(message);
							break;
						case 'object':
							self.wsInBuffer.push(message.consume());
							break;
						default:
							throw "WsConnection: Unknown incomming message type.";
					}
				}
			}
			catch(e) {
				self.setException(e);
				try {
					wsIn.drop();
				} catch {}
				try {
					self.wsOut.drop();
				} catch {}
			}
		}

		mirv.connect_async(options.address).catch((e)=>{
			self.setException(e);
			self.wsOutReject(e);
		}).then((ws)=>{
			self.wsOut = ws.out;
			readNext(ws.in);
			self.wsOutResolve();
		});
	}

	WsConnection.prototype.setException = function(e) {
		if(this.exception === null) {
			this.exception = e;
			if(this.onException) this.onException(e);
		}
	}

	WsConnection.prototype.getException = function() {
		return this.exception;
	}

	WsConnection.prototype.hasException = function() {
		return null !== this.exception;
	}


	WsConnection.prototype.isConnected = function() {
		return null === this.exception && null !== this.wsOut && !this.isClosed;
	}

	WsConnection.prototype.isClosed = function() {
		return this.isClosed;
	}

	WsConnection.prototype.close = function() {
		var self = this;
		this.wsOutNext.then(()=>self.wsOut.close().catch((e)=>self.setException(e)));
	}

	WsConnection.prototype.flush = function() {
		var self = this;
		this.wsOutNext.then(()=>self.wsOut.flush().catch((e)=>self.setException(e)));
	}

	WsConnection.prototype.feed = function(data) {
		var self = this;
		this.wsOutNext.then(()=>self.wsOut.feed(data).catch((e)=>self.setException(e)));
	}

	WsConnection.prototype.send = function(data) {
		var self = this;
		this.wsOutNext.then(()=>self.wsOut.send(data).catch((e)=>self.setException(e)));
	}

	WsConnection.prototype.next = function() {
		if(0 < this.wsInBuffer.length) return this.wsInBuffer.pop();
		return null;
	}

	WsConnection.prototype.hasNext = function() {
		return 0 < this.wsInBuffer.length;
	}

	mirv.onGameEvent = function(e) {
		// mirv.message("onGameEvent: "+e.name+"("+e.id+") \""+e.data+"\"\n");
		if(null !== wsConnection) {
			try {
				wsConnection.send(
					JSON.stringify(
						{
							"type": "onGameEvent",
							"id": e.id,
							"name": e.name,
							"data": JSON.parse(e.data)
						}
					)
				)
			}
			catch(e) {
				mirv.warning("onGameEvent: Error while sending message:"+e.toString()+"\n");
			}
		}
	}

	mirv.onClientFrameStageNotify = function(e) {
		//mirv.message("onClientFrameStageNotify: "+e.curStage+" / "+e.isBefore+"\n");

		if(e.curStage == 0 // FRAME_START - called on host_frame (1 per tick).
			&& e.isBefore) {

			// Restore websockett connection:

			if(null === wsConnection || wsConnection.hasException()) {
				if(null !== wsConnection) {
					mirv.warning("onClientFrameStageNotify: wsConnection failed: "+wsConnection.getException().toString()+"\n");
					wsConnection.close();
					wsConnection = null;
					// setView = null; // our connection crashed ... consider this.
				}

				// Every 64 ticks we try to restore the connection:
				if(wsEnable && 0 == tickCount % 64) {
					mirv.message("onClientFrameStageNotify: making new wsConnection: "+wsAddress+"\n");
					wsConnection = new WsConnection({
						address: wsAddress
					});
				}
			}
			
			if(null !== wsConnection && !wsEnable) {
				wsConnection.close();
				wsConnection = null;
			}

			// We use this to request an extra processing of jobs from HLAE (currently by default it only proccesses jobs upon after FRAME_RENDER_END == 6)
			mirv.run_jobs();
			mirv.run_jobs_async();

			if(null !== wsConnection) {

				// Flush any messages that are lingering:
				wsConnection.flush();

				// Handle messages that came in meanwhile:
				for(let message = wsConnection.next(); message !== null; message = wsConnection.next()) {
					try {
						switch(typeof message) {
						case "string":
							{
								mirv.message(message.toString()+"\n")
								message = JSON.parse(message);
								switch(message.type) {
								case "exec":
									mirv.exec(message.data);
									break;
								case "setview":
									setView = message.data;
									break;
								case "quit":
									wsEnable = false;
									wsConnection.close();
									mirv.exec("quit");
									break;
								default:
									mirv.warning("onClientFrameStageNotify: Unknown incoming message.type:"+message.type+"\n");
									break;
								}
							}
							break;
						default:
							mirv.warning("onClientFrameStageNotify: Warning: Unhandled incoming message of type: "+typeof message);
							break;
						}
					}
					catch(e) {
						mirv.warning("onClientFrameStageNotify: Error while handling incoming message:"+e.toString()+"\n");
					}
				}
			}

			tickCount += 1;
		}

		if(e.curStage == 5 // FRAME_RENDER_START - this is not called when demo is paused (can be multiple per tick).
			&& e.isBefore) {
		}

		if(e.curStage == 5 // FRAME_RENDER_END - this is not called when demo is paused (can be multiple per tick).
			&& e.isBefore) {
				if(null !== wsConnection) wsConnection.flush();
		}	
	}

	mirv.onCViewRenderSetupView = function(e) {
		//mirv.message("onCViewRenderSetupView: "+JSON.stringify(e)+"\n");
		//return { x:300, y:0.0, z:80, fov: 90 }
		//return;
		if(null !== wsConnection) {
			try {
				wsConnection.send(
					JSON.stringify(
						{
							"type": "onCViewRenderSetupView",
							"data": e
						}
					)
				)

				// we could flush and then wait for a reply here to set a view instantly, but don't understimate network round-trip time!
			}
			catch(e) {
				mirv.warning("onGameEvent: Error while sending message:"+e.toString()+"\n");
			}
		}
		if(null !== setView) {
			return setView;
		}
	}
}
