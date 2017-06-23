#pragma once

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-03-04 dominik.matrixstorm.com
//
// First changes:
// 2017-03-04 dominik.matrixstorm.com

extern bool g_csgo_Force_GetInterpolationAmount; // requires Hook_csgo_C_BaseEntity_GetInterpolationAmount.

bool Hook_csgo_C_BaseEntity_GetInterpolationAmount(void);