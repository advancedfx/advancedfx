/*
File        : ui.h
Project     : Mirv Demo Tool
Authors     : Gavin Bramhill
Description : preparations for ui interfaces
*/

#ifndef UI_H
#define UI_H

class UI
{
public:
	virtual int Initialise() = 0;
	virtual int Render(int width, int height) = 0;
	virtual int Update(float time) = 0;
	virtual int UpdateMouse(int x, int y) = 0;

	virtual int KeyDown(int key, int modifier) = 0;
	virtual int KeyUp(int key, int modifier) = 0;
	virtual int MouseDown(int button) = 0;
	virtual int MouseUp(int button) = 0;
};

enum MouseButtons
{
	MOUSE_BUTTON1, //left
	MOUSE_BUTTON2, // right
	MOUSE_BUTTON3, // 3rd button
	MOUSE_SCROLL_UP, // scroll wheel up
	MOUSE_SCROLL_DOWN // scroll wheel down
};

#endif