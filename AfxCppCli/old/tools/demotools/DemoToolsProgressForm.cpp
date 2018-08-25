#include "StdAfx.h"

#include "DemoToolsProgressForm.h"

using namespace AfxCppCli::old::tools;

System::Void DemoToolsProgressForm::backgroundWorker_DoWork(System::Object^  sender, System::ComponentModel::DoWorkEventArgs^  e) {
	String ^ outDir = (String ^)e->Argument;

	String ^ file;
	while(!backgroundWorker->CancellationPending
		&& nullptr != (file = (String ^)this->Invoke(m_DoGetCurrentFile))
	) {
		bool bOk = false;
		
		try {
			bOk = m_Fix->Run(
			file,
			String::Concat(outDir, "\\", IO::Path::GetFileName(file))
			);
		}
		catch(...) {
			bOk = false;
		}
		array<Object^>^ arg = {bOk};
		this->Invoke(m_DoCompleteFile, arg);

		backgroundWorker->ReportProgress(100);
	}
}
