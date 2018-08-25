#pragma once

#include <windows.h>
#include <gl\gl.h>

class Zooming
{
private:
	float m_flZoom;
	bool m_bActive;

public:
	void Start() { m_bActive = true; }
	void Stop() { m_bActive = false; }

	bool isZooming() { return (m_bActive || m_flZoom != 0); }
	void adjustViewportParams(GLint &x, GLint &y, GLsizei &width, GLsizei &height);
	void adjustFrustumParams(GLdouble &left, GLdouble &right, GLdouble &bottom, GLdouble &top);
	void handleZoom();
};

extern Zooming g_Zooming;

