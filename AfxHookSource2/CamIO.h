#pragma once
#include "../shared/CamIO.h"

class S2CamIO {
public:
    CamImport* GetCamImport();
    void SetCamImport(CamImport* pCamImport);

    CamExport* GetCamExport();
    void SetCamExport(CamExport* pCamExport);

	void Console_CamIO(advancedfx::ICommandArgs * args);

	void ShutDown();

private:
    CamImport* m_CamImport = nullptr;
    CamExport* m_CamExport = nullptr;
};

double MirvCamIO_GetTimeFn(void);

extern S2CamIO g_S2CamIO;
