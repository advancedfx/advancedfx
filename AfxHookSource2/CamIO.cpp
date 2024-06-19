#include "CamIO.h"
#include "../shared/CamIO.h"
#include "../shared/MirvCamIO.h"

S2CamIO g_S2CamIO;

CamImport* S2CamIO::GetCamImport() {
    return m_CamImport;
}

void S2CamIO::SetCamImport(CamImport* pCamImport) {
    if (m_CamImport) {
        delete m_CamImport;
    }
    m_CamImport = pCamImport;
}

CamExport* S2CamIO::GetCamExport() {
    return m_CamExport;
}

void S2CamIO::SetCamExport(CamExport* pCamExport) {
    if (m_CamExport) {
        delete m_CamExport;
    }
    m_CamExport = pCamExport;
}

void S2CamIO::Console_CamIO(advancedfx::ICommandArgs * args)
{
	MirvCamIO_ConsoleCommand(args, m_CamImport, m_CamExport, MirvCamIO_GetTimeFn);
}

void S2CamIO::ShutDown() {
    if (m_CamImport) {
        delete m_CamImport;
        m_CamImport = nullptr;
    }
    if (m_CamExport) {
        delete m_CamExport;
        m_CamExport = nullptr;
    }
}
