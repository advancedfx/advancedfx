#pragma once

// Copyright (c) advancedfx.org
//
// Last changes:
// 2017-03-03 dominik.matrixstorm.com
//
// First changes:
// 2017-03-03 dominik.matrixstorm.com

extern bool g_csgo_dt_recv_force_players_m_bClientSideAnimation; // requires Hook_csgo_RecvProxy_Int32ToInt8.

bool Hook_csgo_RecvProxy_Int32ToInt8(void);

