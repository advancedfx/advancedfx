async function my_websocket(address) {
	
	let ws = await mirv.connect_async(address);

	await ws.out.feed('HELLO WORLD!');
	
	while(true) {
		let result = await ws.in.next();
		if(result === null) {
			ws.out.drop();
			ws.in.drop();
			break;
		} else {
			mirv.message('MESSAGE\n');
			switch(typeof result) {
				case 'string':
					mirv.message(result+'\n');
					if(result.toLowerCase() === 'quit') {
						await ws.out.close();
					}
					break;
				case 'object':
					mirv.message('Buffer: '+typeof result.consume()+'\n');
					break;
			}
		}
	}
}
/*
my_websocket('ws://localhost:31337/mirv').then(()=>{
		mirv.message('DONE\n');
}).catch((e)=>{
		mirv.warning(e.toString()+'\n');
});*/

/*
mirv.onGameEvent = function(e) {
	mirv.message(e.name+"("+e.id+") \""+e.data+"\"\n");
}
*/

mirv.onClientFrameStageNotify = undefined; /*function(e) {
	mirv.message("onClientFrameStageNotify: "+e.curStage+" / "+e.isBefore+"\n");
}*/


mirv.onCViewRenderSetupView = function(e) {
	//mirv.message("onCViewRenderSetupView: "+JSON.stringify(e)+"\n");
	return { x:0.01, y:0, z:0, fov: 120.1 }
}
