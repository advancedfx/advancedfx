#pragma once

#include "OverwriteDialog.h"

#include "DemoToolsWiz3.h"
#include "demotools.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace AfxCppCli {
namespace old {
namespace tools {

	/// <summary>
	/// Summary for DemoToolsProgressForm
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class DemoToolsProgressForm : public System::Windows::Forms::Form
	{
	public:
		DemoToolsProgressForm(DemoToolsWiz3 ^ wiz3, CHlaeDemoFix ^ fix)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//

			m_DoGetCurrentFile = gcnew DoGetCurrentFileDelegate(this, &DemoToolsProgressForm::DoGetCurrentFile);
			m_DoCompleteFile = gcnew DoCompleteFileDelegate(this, &DemoToolsProgressForm::DoCompleteFile);

			m_Completed = false;
			m_Failed = false;
			m_FileIndex = 0;
			m_Fix = fix;
			m_Wiz3 = wiz3;
			m_CountTotal = m_Wiz3->GetFileCount();
			m_CountCur = 0;

		}

		void BackgroundWorker_OnDemoFixProgess(System::Object ^ sender, int percentage) {
			backgroundWorker->ReportProgress(percentage);
		}

		void GO(System::Windows::Forms::IWin32Window ^ owner) {
			OnDemoFixProgressDelegate ^ del = gcnew OnDemoFixProgressDelegate(this, &DemoToolsProgressForm::BackgroundWorker_OnDemoFixProgess);

			m_AlwaysYes = false;
			m_AlwaysNo = false;
			m_Completed = false;
			m_Failed = false;
			
			m_Fix->OnDemoFixProgress += del;

			this->CreateHandle();
			this->backgroundWorker->RunWorkerAsync(m_Wiz3->GetOutDir());
			this->ShowDialog(owner);

			m_Fix->OnDemoFixProgress -= del;

		}

		bool GetResult(void) {
			return m_Completed && !m_Failed;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DemoToolsProgressForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::ComponentModel::Container ^components;

		delegate System::String ^ DoGetCurrentFileDelegate();
		delegate void DoCompleteFileDelegate(bool bSuccess);

		DoGetCurrentFileDelegate ^ m_DoGetCurrentFile;
		DoCompleteFileDelegate ^ m_DoCompleteFile;

		bool m_Completed;
		bool m_Failed;
		bool m_AlwaysYes;
		bool m_AlwaysNo;
		int m_FileIndex;
		CHlaeDemoFix ^ m_Fix;
		DemoToolsWiz3 ^ m_Wiz3;
		int m_CountTotal, m_CountCur;


	private: System::ComponentModel::BackgroundWorker^  backgroundWorker;
	private: System::Windows::Forms::ProgressBar^  progressBar1;

	private: System::Windows::Forms::ProgressBar^  progressBar2;
	private: System::Windows::Forms::Button^  buttonCancel;



		bool CheckSkip(String ^ file) {

			String ^ target =
				String::Concat(m_Wiz3->GetOutDir(), "\\", IO::Path::GetFileName(file));

			if(!IO::File::Exists(target))
				return false;

 			if(m_AlwaysNo) return true;
			if(m_AlwaysYes) return false;

			OverwriteDialogResult or = OverwriteDialog::ShowOverWriteDialog(this, target);

			m_AlwaysYes = OverwriteDialogResult::YesToAll == or;
			m_AlwaysNo = OverwriteDialogResult::NoToAll == or;

			return m_AlwaysNo || (!m_AlwaysYes &&  OverwriteDialogResult::Yes != or);
		}

		System::String ^ DoGetCurrentFile() {
			while(m_FileIndex < m_Wiz3->GetFileCount()) {
				String ^ file = m_Wiz3->GetFile(m_FileIndex);
				if(CheckSkip(file)) {
					m_FileIndex++;
					m_CountCur++;
				}
				else
					return file;
			}

			return nullptr;
		}

		void DoCompleteFile(bool bSuccess) {
			if(bSuccess)
				m_Wiz3->RemoveFile(m_FileIndex);
			else {
				m_FileIndex++;
				m_Failed = true;
			}

			m_CountCur++;
		}

		System::Void backgroundWorker_DoWork(System::Object^  sender, System::ComponentModel::DoWorkEventArgs^  e);

		System::Void backgroundWorker_ProgressChanged(System::Object^  sender, System::ComponentModel::ProgressChangedEventArgs^  e) {
			int fileCount = m_Wiz3->GetFileCount();
			if(m_CountCur < m_CountTotal) {
				int perc = e->ProgressPercentage;
				this->progressBar1->Value =
					100*(100*m_CountCur+perc) /  m_CountTotal;
				;
				this->progressBar2->Value = perc;
			}
			else {
				this->progressBar1->Value = 1000;
				this->progressBar2->Value = 100;
			}
		}

#pragma region Windows Form Designer generated code
private: System::Void InitializeComponent() {
			 this->backgroundWorker = (gcnew System::ComponentModel::BackgroundWorker());
			 this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			 this->buttonCancel = (gcnew System::Windows::Forms::Button());
			 this->progressBar2 = (gcnew System::Windows::Forms::ProgressBar());
			 this->SuspendLayout();
			 // 
			 // backgroundWorker
			 // 
			 this->backgroundWorker->WorkerReportsProgress = true;
			 this->backgroundWorker->WorkerSupportsCancellation = true;
			 this->backgroundWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &DemoToolsProgressForm::backgroundWorker_DoWork);
			 this->backgroundWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &DemoToolsProgressForm::backgroundWorker_RunWorkerCompleted);
			 this->backgroundWorker->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &DemoToolsProgressForm::backgroundWorker_ProgressChanged);
			 // 
			 // progressBar1
			 // 
			 this->progressBar1->Location = System::Drawing::Point(12, 12);
			 this->progressBar1->Maximum = 10000;
			 this->progressBar1->Name = L"progressBar1";
			 this->progressBar1->Size = System::Drawing::Size(468, 20);
			 this->progressBar1->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			 this->progressBar1->TabIndex = 0;
			 // 
			 // buttonCancel
			 // 
			 this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			 this->buttonCancel->Location = System::Drawing::Point(209, 58);
			 this->buttonCancel->Name = L"buttonCancel";
			 this->buttonCancel->Size = System::Drawing::Size(75, 23);
			 this->buttonCancel->TabIndex = 1;
			 this->buttonCancel->Text = L"Can&cel";
			 this->buttonCancel->UseVisualStyleBackColor = true;
			 this->buttonCancel->Click += gcnew System::EventHandler(this, &DemoToolsProgressForm::button1_Click);
			 // 
			 // progressBar2
			 // 
			 this->progressBar2->Location = System::Drawing::Point(12, 32);
			 this->progressBar2->Name = L"progressBar2";
			 this->progressBar2->Size = System::Drawing::Size(468, 10);
			 this->progressBar2->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			 this->progressBar2->TabIndex = 2;
			 // 
			 // DemoToolsProgressForm
			 // 
			 this->ClientSize = System::Drawing::Size(492, 90);
			 this->Controls->Add(this->progressBar2);
			 this->Controls->Add(this->buttonCancel);
			 this->Controls->Add(this->progressBar1);
			 this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			 this->MaximizeBox = false;
			 this->Name = L"DemoToolsProgressForm";
			 this->ShowIcon = false;
			 this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			 this->Text = L"DemoTools ...";
			 this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &DemoToolsProgressForm::DemoToolsProgressForm_FormClosing);
			 this->ResumeLayout(false);

		 }
#pragma endregion

private: System::Void backgroundWorker_RunWorkerCompleted(System::Object^  sender, System::ComponentModel::RunWorkerCompletedEventArgs^  e) {
			 m_Completed = true;
			 this->Close();
		 }
private: System::Void DemoToolsProgressForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
			 e->Cancel = !m_Completed;
		 }
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			 backgroundWorker->CancelAsync();
		 }
};

} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {


