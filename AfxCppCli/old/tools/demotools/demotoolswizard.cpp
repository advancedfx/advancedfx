#include "stdafx.h"

#include "demotoolswizard.h"

#include "DemoToolsWiz1.h"
#include "DemoToolsWiz2.h"
#include "DemoToolsWiz3.h"
#include "demotools.h"

#include "DemoToolsProgressForm.h"



using namespace System;
using namespace System::Windows::Forms;

using namespace AfxCppCli::old::tools;

enum class CheckOutDirResult {
	Invalid,
	Ok,
	PartOfFilePath
};

CheckOutDirResult CheckOutDir(DemoToolsWiz3 ^ wiz3) {
	String ^ outDir = wiz3->GetOutDir();

	if(!IO::Directory::Exists(outDir))
		return CheckOutDirResult::Invalid;

	for(int i=0; i < wiz3->GetFileCount(); i++) {
		String ^ fileDir = IO::Path::GetDirectoryName(wiz3->GetFile(i));

		if(IO::Directory::Equals(fileDir, outDir))
			return CheckOutDirResult::PartOfFilePath;
	}

	return CheckOutDirResult::Ok;
}

DemoToolsWizard::DemoToolsWizard()
{
	m_OutputPath = "";
}

bool DemoToolsWizard::ShowDialog(System::Windows::Forms::IWin32Window ^ parentWindow)
{
	DemoToolsWiz1 ^wiz1 = gcnew DemoToolsWiz1();
	DemoToolsWiz2 ^wiz2 = gcnew DemoToolsWiz2();
	DemoToolsWiz3 ^wiz3 = gcnew DemoToolsWiz3();

	{
		String ^ dir = m_OutputPath;
		if(IO::Directory::Exists(dir)) wiz3->SetOutDir(dir);
	}

	DialogResult dResult;
	int iShowDialog = 0;
	CheckOutDirResult cr;
	bool bDemoToolsOk = false;

	while (iShowDialog>=0 && iShowDialog < 3)
	{
		switch (iShowDialog)
		{
		case 0:
			dResult = wiz1->ShowDialog( parentWindow );
			if (DialogResult::Yes != dResult)
				iShowDialog = -1;
			else
				iShowDialog = wiz1->bCheckedCleanup() ? 1 : 2;
			break;
		case 1:
			dResult = wiz2->ShowDialog( parentWindow );
			switch (dResult)
			{
			case DialogResult::No:
				iShowDialog = 0;
				break;
			case DialogResult::Yes:
				iShowDialog = 2;
				break;
			default:
				iShowDialog = -1;
			};
			break;
		case 2:
			dResult = wiz3->ShowDialog( parentWindow );
			switch (dResult)
			{
			case DialogResult::No:
				iShowDialog =
					wiz1->bCheckedCleanup() ? 1 : 0;
				break;
			case DialogResult::Yes:
				cr = CheckOutDir(wiz3);
				switch(cr) {
				case CheckOutDirResult::Ok:
					iShowDialog = 3;
					break;
				case CheckOutDirResult::PartOfFilePath:
					Windows::Forms::MessageBox::Show(
						"Error: One or more files have the same folder you have selected as output folder!\nPlease select a different folder!",
						"Invalid output folder",
						MessageBoxButtons::OK,
						MessageBoxIcon::Exclamation
					);
					iShowDialog = 2;
					break;
				default:
					Windows::Forms::MessageBox::Show(
						"Selected output folder is invalid.\nPlease select a different folder.",
						"Invalid output folder",
						MessageBoxButtons::OK,
						MessageBoxIcon::Exclamation
					);
					iShowDialog = 2;
					break;
				};
				break;
			default:
				iShowDialog = -1;
			};
			break;
		default:
			iShowDialog = -1;
		};

		if (iShowDialog==3)
		{
			// Finish pressed.

			iShowDialog = -1;

			m_OutputPath = wiz3->GetOutDir();

			CHlaeDemoFix ^dtool = gcnew CHlaeDemoFix();

			dtool->EnableDirectoryFix( wiz1->bCheckedFix() );
			dtool->EnableHltvFix( wiz1->bCheckedStuck() );
			dtool->EnableWaterMarks( wiz3->bCheckedMarks() );

			if( wiz1->bCheckedDemoHeader() )
			{
				//dtool->bSet_GameDll = true;
				dtool->bSet_GameDll = false;
				//dtool->strSet_GameDll = "cstrike_beta";

				dtool->bSet_NetworkVersion = true;
				dtool->uiSet_NetWorkVersion = 48;

				dtool->bSet_ProtocolVersion = true;
				dtool->ui_ProtoVersion = 48;

				//dtool->bFuckOff = true;
				dtool->bFuckOff = false;
			} else {
				dtool->bSet_GameDll = false;
				dtool->bSet_NetworkVersion = false;
				dtool->bSet_ProtocolVersion = false;
				dtool->bFuckOff = false;
			}

			if (wiz1->bCheckedCleanup())
			{
				System::String ^astr;

				bool bEnabledSth = false;
				
				for (int i=0; i < wiz2->GetMapCnt(); i++)
				{
					astr = wiz2->GetMapSrc(i);
					if ( !System::String::IsNullOrEmpty( astr ) )
					{
						dtool->AddCommandMapping( astr, wiz2->GetMapDst(i) );
						bEnabledSth = true;
					}
				}
				
				dtool->EnableDemoCleanUp( bEnabledSth );
			}

			DemoToolsProgressForm ^dp = gcnew DemoToolsProgressForm(wiz3, dtool);

			dp->GO(parentWindow);

			if(!dp->GetResult())
			{
				iShowDialog = 2;
				MessageBox::Show(
					"Some files failed and remain in the box.",
					"Some failed",
					MessageBoxButtons::OK,
					MessageBoxIcon::Error
				);
			}
			else
				bDemoToolsOk = true;


			delete dp;
			delete dtool;
		}

	}

	delete wiz3;
	delete wiz2;
	delete wiz1;

	return bDemoToolsOk;
}
