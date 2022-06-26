#pragma once

#ifdef AFX_MIRV_PGL

/*

Changes from version 2.0.2 to version 2.0.3:
- The messages "transBegin" and "transEnd" have been added, so that the server can group messsage that must be processed together (in order to avoid side effects).
- Added "gameEvent" message, for decoding it we recommend to have a look at the sample code in misc/mirv_pgl_test/server.js,
  since it depends state shared between client and server in order to reduce the amount of data sent.
- Added mirv_pgl events command to control sending and set-up of "gameEvent" messages, by default none are sent.

Changes from version 2 to version 2(.0.2):

- "cam" message is sent the first time the current game frame is presented on-screen.
- Fixed "cam" message not sent when multiple streams are renderred.

Changes from version 1 to version 2:

- Added messages "dataStart" and "dataStop"
  All present other messages except "hello" will only be sent between those two.
  Please note:
  1) From issuing "mirv_pgl stop" to the actual "dataStop" message some time AND data may pass, due to threading and delays on the network layer or due to buffers!
  2) It is possible to receive multiple "dataStop" message without an previous "dataStart" message, since "dataStop" cancels all other messages.
  3) "dataStop" really cancels all messages, it can even cancel "hello", however if you made it do that, you already have done s.th. wrong!

- Added "mirv_pgl dataStart" and "mirv_pgl dataStop" commands.

- The initial state is only to send "hello" (no other data).

- Remember: After an connection loss, the initial is the same again, meaning sending of data has to be enabled again (i.e. by using a exec message from server side.

Tip1:
  You can use exec to exec mirv_pgl commands from server, there is no deadlock. This is i.e. useful to make the server enable / disable sending of data!
Tip2:
  1) With "mirv_cvar_hack host_sleep x" you can make the game sleep x milliseconds, this is great for throttling, since you can enforce a maximum FPS this way (any value you want).
  2) Not as useful but should be mentioned: With "mirv_cvar_hack fps_max 30" you can throttle the game down as low as 30 FPS.
  This is not exactly the throttling that you might have thought off, but this should be pretty okay, because that way you can be sure that data for each frame is sent (nothing dropped).


Changes from version 0 to version 1:

- "cam" uses all Float (instead of Double) now.

- "mirv_pgl url" sets / gets the url to use, "mirv_pgl start" doesn't take parameters anymore.


Usage:


It is a good idea to run with the FPS limited (either by vsync or by fps_max).
Otherwise the network / server will be flooded with "cam" messages or the send buffer will overflow eventually, since there is currently no throttling at all implemented so far.


Console commands:

mirv_pgl url [<url>] - Set the server's URL, example: mirv_pgl url "ws://localhost:31337/mirv"
mirv_pgl start - (Re-)Starts connectinion to server.
mirv_pgl stop - Stops connection to server.

It is safe to exec mirv_pgl stop from the server, but how will you reconnect then?


Example server:

An example server is located in misc/mirv_pgl_test/server.js


Connection loss:

If the connection is lost and not stopped, it is retried every 5 seconds.
The server-side state should be reset upon connection loss (as if mirv_pgl stop had been called).


Messages:
  The messages are exchanged as binary frames.

  Multiple messages can be in a single frame!!!!

  The message data is not aligned / padded!

  CString is a null-terminated string as in C-Language.


Messages sent to server:

"hello"
Purpose:
  Is sent upon (re)-connecting.
  If received with unexpected version, server should close the connection.
Format:
  CString cmd = "hello"
  UInt32 version = 3;

"dataStart"
Purpose:
  Informs the server that a new transmission of data is about to start.
Format:
	CString cmd = "dataStart"

"dataStop"
Purpose:
  Informs the server that the transmission of data has been cancelled.
  It is possible to receive multiple "dataStop" messages without any "dataStart".
  "dataStop" cancels all other messages (can even cancel "hello" if you use it wrong)!
  Format:
  CString cmd = "dataStop"

"levelInit"
Purpose:
  Is sent if in a level upon after "dataStart" or if a new level is loaded.
Format:
  CString cmd = "levelInit"
  CString levelName;

"levelShutdown"
Purpose:
  Is sent when level is shut down. (Can be sent multiple times.)
Format:
  CString cmd = "levelShutdown"

"cam"
Purpose:
  Is sent after "levelInit" after the frame has been presented on screen.
  The data content is from when the frame presented has been calculated by the engine.
  The fov is currently automatically converted according to Alien Swarm SDK (suitable for CS:GO).
  If multiple streams are renderred it is sent after the last stream has been presented.
Format:
  CString cmd = "cam";
  Float time;
  Float xPosition;
  Float yPosition;
  Float zPoisiton;
  Float xRotation;
  Float yRotation;
  Float zRotation;
  Float fov;


Messages received:

"exec"
Purpose:
  Schedules cmds for console execution.
  Use with CAUTION: Flooding the client with too many commands will crash the game. Also don't use mirv_pgl start / stop (will cause deadlock).
Format:
  CString cmd = "exec";
  CString cmds;

"transBegin"
Purpose:
  Begins a transaction.

"transEnd"
Purpose:
  Ends a transaction.



Ideas for the future:
- Implement throttling.
- Implement black image command with feedback when presented.
- Implement white image command with feedback when presented.
- Implement optional time-code (float) graphic overlay at top of screen, this would allow syncing the images and the camdata on remote PC perfectly (as long as turned on).

*/

#include <d3d9.h>
#include <vector>

namespace MirvPgl
{
	struct CamData
	{
		float Time = 0;
		float XPosition = 0;
		float YPosition = 0;
		float ZPosition = 0;
		float XRotation = 0;
		float YRotation = 0;
		float ZRotation = 0;
		float Fov = 90;

		CamData();
		CamData(float time, float xPosition, float yPosition, float zPosition, float xRotation, float yRotation, float zRotation, float fov);
	};

	// On Main thread:

	void Init();
	void Shutdown();

	void Url_set(char const * url);
	char const * Url_get(void);

	void Start();
	void Stop();

	void DataStart();
	void DataStop();

	bool IsDataActive();
	bool IsDrawingActive();

	void CheckStartedAndRestoreIfDown();
	void ExecuteQueuedCommands();
	void QueueThreadDataForDrawingThread(void);
	void QueueDrawing(CamData const & camData, int width, int height);

	void SupplyCamData(CamData const & camData);

	void SupplyLevelInit(char const * mapName);
	void SupplyLevelShutdown();

	bool OnViewOverride(float& Tx, float& Ty, float& Tz, float& Rx, float& Ry, float& Rz, float& Fov);

	// On Drawing thead:

	void D3D9_BeginDevice(IDirect3DDevice9 * device);
	void D3D9_EndDevice();
	void D3D9_Reset();

	void DrawingThread_UnleashData();
}

#endif
