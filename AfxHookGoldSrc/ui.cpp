/*
File        : ui.cpp
Project     : Mirv Demo Tool
Authors     : Gavin Bramhill
Description : preparations for ui interfaces
*/



#include "ui.h"
/*#include "Gui.h"

#include "util/Color.h"
#include "component/Frame.h"
#include "component/MenuBar.h" 
#include "component/Slider.h"
#include "component/Window.h"
#include "component/Panel.h"
#include "component/Dialog.h"
#include "component/Button.h"

#ifdef _DEBUG
#pragma comment(lib, "UId.lib")
#else
#pragma comment(lib, "UI.lib")
#endif
*/

// using namespace ui;

//
// THIS IS JUST A PLACEHOLDER, HAVEN'T YET IMPLEMENTED THE FONT TEXTURE LOADING OR DRAWING.
//
/*
class BitmapFont : public ui::Font
{
public:
	BitmapFont() {};
	~BitmapFont() {};
	const std::string getFontName() const { return "BitmapFont"; }
	std::size_t getSize() const { return 10; }
	ui::util::Dimension getStringBoundingBox(const std::string &text) const
	{
		int width = static_cast<int>(text.length() * getSize());
		return ui::util::Dimension(width,static_cast<int>(getSize()));
	}
	void drawString(int x, int y, const std::string &text) {};
private:
	static const int textureWidth = 256;
	static const int textureHeight = 256;
	GLuint textureId;
	GLuint base;
};
*/

//
// THIS IS COMPLETE.
//
/*
class BitmapFontFactory : public ui::AbstractFontFactory
{
public:
	ui::Font * createFont(const std::string &font, std::size_t size) { return bitmapFont; }
	BitmapFontFactory() { bitmapFont = new BitmapFont(); }
	~BitmapFontFactory() { delete bitmapFont; }
private:
	BitmapFont *bitmapFont;
};
*/
//
// JUST A VERY SIMPLE TEST DIALOG WINDOW.
//
/*
class TestDialog : public ui::Dialog
{
public:
	TestDialog(Window *owner);
	~TestDialog();
private:
	Panel *contentPane;
	Slider *hSlider;
	Slider *vSlider;
	Button *button;
};

TestDialog::TestDialog(Window *owner) :	Dialog(owner,"Slider test")
{
	contentPane = new Panel();
	hSlider = new Slider(0,100);
	vSlider = new Slider(Component::VERTICAL);
	button = new Button("Hello!");

	contentPane->add(hSlider);
	contentPane->add(vSlider);
	contentPane->add(button);

	setContentPane(contentPane);
	pack();
}

TestDialog::~TestDialog()
{
	delete vSlider;
	delete hSlider;
	delete button;
	delete contentPane;
}
*/

//#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>

//
// ACTUAL GUI STUFF, INTERFACES WITH WHATEVER GUI WE DECIDE TO USE
//

class MDTUI : public UI
{
private:
	//Gui *m_pGui;
	float m_flLastTime;
	POINT mouse;

private:
	//Frame *m_pFrame;
	//TestDialog *m_pTestDialog;

public:
	MDTUI();
	~MDTUI();

	int Initialise();
	int Render(int width, int height);
	int Update(float time);
	int UpdateMouse(int x, int y) { mouse.x = x; mouse.y = y; /*m_pGui->importMouseMotion(x, y);*/ return 0; }

	int KeyDown(int key, int modifier) { /*m_pGui->importKeyPressed(key, modifier);*/ return 0; }
	int KeyUp(int key, int modifier) { /*m_pGui->importKeyReleased(key, modifier);*/ return 0; }
	int MouseDown(int button) { /*m_pGui->importMousePressed(button);*/	return 0; }
	int MouseUp(int button) { /*m_pGui->importMouseReleased(button);*/ return 0; }
};

MDTUI::MDTUI()
{
	/*m_pGui = new Gui();
	m_pGui->setFontFactory(new BitmapFontFactory());
	m_pFrame = new Frame(0, 0, 800, 600);
	m_pFrame->setBackground(new util::Color(0.0f, 0.0f, 0.0f, 0.0f));
	m_pFrame->setBorderPainted(false);
	m_pGui->addFrame(m_pFrame);
	m_pFrame->show();
	m_pTestDialog = new TestDialog(m_pFrame);
	m_pTestDialog->show();
	m_pTestDialog->setLocation(10, 10);*/

	m_flLastTime = 0.0f;
}

MDTUI::~MDTUI()
{
	/*delete m_pFrame;
	delete m_pGui;
	delete m_pTestDialog;*/
}

int MDTUI::Initialise()
{
	return 0;
}

int MDTUI::Update(float time)
{
	float deltaTime = time - m_flLastTime;

	// m_pGui->importUpdate(deltaTime);

	m_flLastTime = time;

	// m_pGui->importMouseMotion(mouse.x, mouse.y);

	return 0;
}

void glSetEnabled(GLenum cap, GLboolean mode)
{
	(mode ? glEnable(cap) : glDisable(cap));
}

int MDTUI::Render(int width, int height)
{
	GLint mode, src, dst;
	GLfloat colours[4];
	glGetIntegerv(GL_MATRIX_MODE, &mode);
	glGetIntegerv(GL_BLEND_SRC, &src);
	glGetIntegerv(GL_BLEND_DST, &dst);
	glGetFloatv(GL_CURRENT_COLOR, colours);

	GLboolean alpha = glIsEnabled(GL_ALPHA_TEST);
	GLboolean lighting = glIsEnabled(GL_LIGHTING);
	GLboolean texture = glIsEnabled(GL_TEXTURE_2D);
	GLboolean blend = glIsEnabled(GL_BLEND);
	GLboolean cull = glIsEnabled(GL_CULL_FACE);
	GLboolean depth = glIsEnabled(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawBuffer(GL_BACK);

	// Change projection matrix to orth
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, width, height, 0);

	// Back to model for rendering
	glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// m_pGui->paint();

	glDisable(GL_DEPTH_TEST);

	// This is the mouse cursor.
	glBegin(GL_QUADS);
		glVertex3f(static_cast<GLfloat>(mouse.x), static_cast<GLfloat>(mouse.y), 0);		// Top Left
		glVertex3f(static_cast<GLfloat>(mouse.x + 5), static_cast<GLfloat>(mouse.y), 0);		// Top Right
		glVertex3f(static_cast<GLfloat>(mouse.x + 5),static_cast<GLfloat>(mouse.y + 5), 0);	// Bottom Right
		glVertex3f(static_cast<GLfloat>(mouse.x), static_cast<GLfloat>(mouse.y + 5), 0);	// Bottom Left
	glEnd();

	glColor4fv(colours);

	glSetEnabled(GL_ALPHA_TEST, alpha);
	glSetEnabled(GL_LIGHTING, lighting);
	glSetEnabled(GL_TEXTURE_2D, texture);
	glSetEnabled(GL_CULL_FACE, cull);
	glSetEnabled(GL_DEPTH_TEST, depth);
	glSetEnabled(GL_BLEND, blend);

	//glPopMatrix();

	// Reset the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Back to whatever we were before
	glMatrixMode(mode);
	glBlendFunc(src, dst);

	return 0;
}

// Singleton.
MDTUI mdt_ui;
UI *gui = &mdt_ui;
