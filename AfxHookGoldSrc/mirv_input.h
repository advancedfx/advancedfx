#pragma once

class MirvInput* MirvInput_Get();

bool MirvInput_Override(float deltaT, float& Tx, float& Ty, float& Tz, float& Rx, float& Ry, float& Rz, float& Fov);
void MirvInput_SupplyMouseFrameEnd();
