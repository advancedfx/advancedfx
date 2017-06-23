#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace AfxCppCli {
namespace old {
namespace tools {

enum class OverwriteDialogResult {
	Cancel,
	NoToAll,
	No,
	Yes,
	YesToAll
};

/// <summary>
/// Summary for OverwriteDialog
///
/// WARNING: If you change the name of this class, you will need to change the
///          'Resource File Name' property for the managed resource compiler tool
///          associated with all .resx files this class depends on.  Otherwise,
///          the designers will not be able to interact properly with localized
///          resources associated with this form.
/// </summary>
public ref class OverwriteDialog : public System::Windows::Forms::Form
{
public:
	OverwriteDialog(String ^ fileName)
	{
		InitializeComponent();
		//
		//TODO: Add the constructor code here
		//

		this->Icon =  System::Drawing::Icon::ExtractAssociatedIcon(System::Windows::Forms::Application::ExecutablePath);

		this->textFileName->Text = fileName;

		m_Result = OverwriteDialogResult::Cancel;
	}

	OverwriteDialogResult GetResult() {
		return m_Result;
	}

	static OverwriteDialogResult ShowOverWriteDialog(Windows::Forms::IWin32Window ^ owner, String ^ fileName)
	{
		OverwriteDialogResult result;
		OverwriteDialog ^ dlg = nullptr;

		try
		{
			dlg = gcnew OverwriteDialog(fileName);
			dlg->ShowDialog(owner);
			result = dlg->GetResult();
		}
		finally
		{
			if(nullptr != dlg) delete dlg;
		}

		return result;
	}


protected:
	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	~OverwriteDialog()
	{
		if (components)
		{
			delete components;
		}
	}

private:
	OverwriteDialogResult m_Result;

private: System::Windows::Forms::Label^  label1;
private: System::Windows::Forms::TextBox^  textFileName;
protected: 

private: System::Windows::Forms::Label^  label2;
private: System::Windows::Forms::Panel^  panel1;
private: System::Windows::Forms::Button^  buttonYes;
private: System::Windows::Forms::Button^  buttonYesAll;
private: System::Windows::Forms::Button^  buttonNo;
private: System::Windows::Forms::Button^  buttonNoAll;





private:
	/// <summary>
	/// Required designer variable.
	/// </summary>
	System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	void InitializeComponent(void)
	{
		this->label1 = (gcnew System::Windows::Forms::Label());
		this->textFileName = (gcnew System::Windows::Forms::TextBox());
		this->label2 = (gcnew System::Windows::Forms::Label());
		this->panel1 = (gcnew System::Windows::Forms::Panel());
		this->buttonYes = (gcnew System::Windows::Forms::Button());
		this->buttonYesAll = (gcnew System::Windows::Forms::Button());
		this->buttonNo = (gcnew System::Windows::Forms::Button());
		this->buttonNoAll = (gcnew System::Windows::Forms::Button());
		this->SuspendLayout();
		// 
		// label1
		// 
		this->label1->AutoSize = true;
		this->label1->Location = System::Drawing::Point(73, 9);
		this->label1->Name = L"label1";
		this->label1->Size = System::Drawing::Size(42, 13);
		this->label1->TabIndex = 5;
		this->label1->Text = L"The file";
		// 
		// textFileName
		// 
		this->textFileName->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		this->textFileName->Location = System::Drawing::Point(76, 25);
		this->textFileName->Name = L"textFileName";
		this->textFileName->ReadOnly = true;
		this->textFileName->Size = System::Drawing::Size(404, 20);
		this->textFileName->TabIndex = 6;
		// 
		// label2
		// 
		this->label2->AutoSize = true;
		this->label2->Location = System::Drawing::Point(73, 48);
		this->label2->Name = L"label2";
		this->label2->Size = System::Drawing::Size(206, 13);
		this->label2->TabIndex = 7;
		this->label2->Text = L"already exists, do you want to overwrite it\?";
		// 
		// panel1
		// 
		this->panel1->Location = System::Drawing::Point(23, 18);
		this->panel1->Name = L"panel1";
		this->panel1->Size = System::Drawing::Size(32, 32);
		this->panel1->TabIndex = 4;
		this->panel1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &OverwriteDialog::panel1_Paint);
		// 
		// buttonYes
		// 
		this->buttonYes->Location = System::Drawing::Point(14, 77);
		this->buttonYes->Name = L"buttonYes";
		this->buttonYes->Size = System::Drawing::Size(90, 30);
		this->buttonYes->TabIndex = 2;
		this->buttonYes->Text = L"&Yes";
		this->buttonYes->UseVisualStyleBackColor = true;
		this->buttonYes->Click += gcnew System::EventHandler(this, &OverwriteDialog::buttonYes_Click);
		// 
		// buttonYesAll
		// 
		this->buttonYesAll->Location = System::Drawing::Point(139, 77);
		this->buttonYesAll->Name = L"buttonYesAll";
		this->buttonYesAll->Size = System::Drawing::Size(90, 30);
		this->buttonYesAll->TabIndex = 3;
		this->buttonYesAll->Text = L"Ye&s to all";
		this->buttonYesAll->UseVisualStyleBackColor = true;
		this->buttonYesAll->Click += gcnew System::EventHandler(this, &OverwriteDialog::buttonYesAll_Click);
		// 
		// buttonNo
		// 
		this->buttonNo->Location = System::Drawing::Point(264, 77);
		this->buttonNo->Name = L"buttonNo";
		this->buttonNo->Size = System::Drawing::Size(90, 30);
		this->buttonNo->TabIndex = 1;
		this->buttonNo->Text = L"&No";
		this->buttonNo->UseVisualStyleBackColor = true;
		this->buttonNo->Click += gcnew System::EventHandler(this, &OverwriteDialog::buttonNo_Click);
		// 
		// buttonNoAll
		// 
		this->buttonNoAll->DialogResult = System::Windows::Forms::DialogResult::Cancel;
		this->buttonNoAll->Location = System::Drawing::Point(389, 77);
		this->buttonNoAll->Name = L"buttonNoAll";
		this->buttonNoAll->Size = System::Drawing::Size(90, 30);
		this->buttonNoAll->TabIndex = 0;
		this->buttonNoAll->Text = L"N&o to all";
		this->buttonNoAll->UseVisualStyleBackColor = true;
		this->buttonNoAll->Click += gcnew System::EventHandler(this, &OverwriteDialog::buttonNoAll_Click);
		// 
		// OverwriteDialog
		// 
		this->AcceptButton = this->buttonYes;
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->CancelButton = this->buttonNoAll;
		this->ClientSize = System::Drawing::Size(492, 116);
		this->Controls->Add(this->buttonNoAll);
		this->Controls->Add(this->buttonNo);
		this->Controls->Add(this->buttonYesAll);
		this->Controls->Add(this->buttonYes);
		this->Controls->Add(this->panel1);
		this->Controls->Add(this->label2);
		this->Controls->Add(this->textFileName);
		this->Controls->Add(this->label1);
		this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
		this->MaximizeBox = false;
		this->MinimizeBox = false;
		this->Name = L"OverwriteDialog";
		this->ShowInTaskbar = false;
		this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
		this->Text = L"File already exists";
		this->Shown += gcnew System::EventHandler(this, &OverwriteDialog::OverwriteDialog_Shown);
		this->ResumeLayout(false);
		this->PerformLayout();

	}
#pragma endregion

private:
	System::Void panel1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		e->Graphics->DrawIcon(Drawing::SystemIcons::Warning, 0, 0);
	}
private: System::Void buttonYes_Click(System::Object^  sender, System::EventArgs^  e) {
			 m_Result = OverwriteDialogResult::Yes;
			 this->Close();
		 }
private: System::Void buttonYesAll_Click(System::Object^  sender, System::EventArgs^  e) {
			 m_Result = OverwriteDialogResult::YesToAll;
			 this->Close();
		 }
private: System::Void buttonNo_Click(System::Object^  sender, System::EventArgs^  e) {
			 m_Result = OverwriteDialogResult::No;
			 this->Close();
		 }
private: System::Void buttonNoAll_Click(System::Object^  sender, System::EventArgs^  e) {
			 m_Result = OverwriteDialogResult::NoToAll;
			 this->Close();
		 }
private: System::Void OverwriteDialog_Shown(System::Object^  sender, System::EventArgs^  e) {
			 this->textFileName->Focus();
			 this->textFileName->Select(this->textFileName->Text->Length, 0);
			 this->textFileName->ScrollToCaret();
			 this->Focus();
		 }
};

} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {

