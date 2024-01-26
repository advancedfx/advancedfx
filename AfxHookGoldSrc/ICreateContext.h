#pragma once

#include <gl/gl.h>

class ICreateContext abstract {
public:
    virtual HGLRC CreateContext(HDC hDC) = 0;
};
