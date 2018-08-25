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

	/// <summary>
	/// Summary for DemoToolsWiz1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	ref class DemoToolsWiz1 : public System::Windows::Forms::Form
	{
	public:
		DemoToolsWiz1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

		bool bCheckedCleanup( void )
		{
			return this->checkBoxCleanup->Checked;
		}
		bool bCheckedFix( void )
		{
			return this->checkBoxFix->Checked;
		}
		bool bCheckedStuck( void )
		{
			return this->checkBoxStuck->Checked;
		}

		bool bCheckedDemoHeader( void )
		{
			return this->checkBoxDemoHeader->Checked;
		}

	private:
		void doUpdateNext ( void )
		{
			this->buttonNext->Enabled =
				this->checkBoxCleanup->Checked
				|| this->checkBoxFix->Checked
				|| this->checkBoxStuck->Checked
				|| this->checkBoxDemoHeader->Checked;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DemoToolsWiz1()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::CheckBox^  checkBoxCleanup; 
	private: System::Windows::Forms::CheckBox^  checkBoxFix;
	private: System::Windows::Forms::CheckBox^  checkBoxStuck;	
	private: System::Windows::Forms::Label^  labelCleanUp;	
	private: System::Windows::Forms::Label^  labelDemoFix;
	private: System::Windows::Forms::Label^  labelStuckFix;
	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::Button^  buttonNext;
	private: System::Windows::Forms::Button^  buttonPrev;

	private: System::Windows::Forms::CheckBox^  checkBoxDemoHeader;


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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(DemoToolsWiz1::typeid));
			this->checkBoxCleanup = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxFix = (gcnew System::Windows::Forms::CheckBox());
			this->checkBoxStuck = (gcnew System::Windows::Forms::CheckBox());
			this->labelCleanUp = (gcnew System::Windows::Forms::Label());
			this->labelDemoFix = (gcnew System::Windows::Forms::Label());
			this->labelStuckFix = (gcnew System::Windows::Forms::Label());
			this->buttonCancel = (gcnew System::Windows::Forms::Button());
			this->buttonNext = (gcnew System::Windows::Forms::Button());
			this->buttonPrev = (gcnew System::Windows::Forms::Button());
			this->checkBoxDemoHeader = (gcnew System::Windows::Forms::CheckBox());
			this->SuspendLayout();
			// 
			// checkBoxCleanup
			// 
			this->checkBoxCleanup->AutoSize = true;
			this->checkBoxCleanup->Location = System::Drawing::Point(21, 19);
			this->checkBoxCleanup->Name = L"checkBoxCleanup";
			this->checkBoxCleanup->Size = System::Drawing::Size(98, 17);
			this->checkBoxCleanup->TabIndex = 0;
			this->checkBoxCleanup->Text = L"DemoCleanUp:";
			this->checkBoxCleanup->UseVisualStyleBackColor = true;
			this->checkBoxCleanup->CheckedChanged += gcnew System::EventHandler(this, &DemoToolsWiz1::checkBoxXXX_CheckedChanged);
			// 
			// checkBoxFix
			// 
			this->checkBoxFix->AutoSize = true;
			this->checkBoxFix->Location = System::Drawing::Point(21, 72);
			this->checkBoxFix->Name = L"checkBoxFix";
			this->checkBoxFix->Size = System::Drawing::Size(70, 17);
			this->checkBoxFix->TabIndex = 3;
			this->checkBoxFix->Text = L"DemoFix:";
			this->checkBoxFix->UseVisualStyleBackColor = true;
			this->checkBoxFix->CheckedChanged += gcnew System::EventHandler(this, &DemoToolsWiz1::checkBoxXXX_CheckedChanged);
			// 
			// checkBoxStuck
			// 
			this->checkBoxStuck->AutoSize = true;
			this->checkBoxStuck->Location = System::Drawing::Point(21, 166);
			this->checkBoxStuck->Name = L"checkBoxStuck";
			this->checkBoxStuck->Size = System::Drawing::Size(138, 17);
			this->checkBoxStuck->TabIndex = 5;
			this->checkBoxStuck->Text = L"dem_forcehltv stuck fix:";
			this->checkBoxStuck->UseVisualStyleBackColor = true;
			this->checkBoxStuck->CheckedChanged += gcnew System::EventHandler(this, &DemoToolsWiz1::checkBoxXXX_CheckedChanged);
			// 
			// labelCleanUp
			// 
			this->labelCleanUp->AutoSize = true;
			this->labelCleanUp->Location = System::Drawing::Point(18, 39);
			this->labelCleanUp->Name = L"labelCleanUp";
			this->labelCleanUp->Size = System::Drawing::Size(267, 13);
			this->labelCleanUp->TabIndex = 1;
			this->labelCleanUp->Text = L"Allows you to remove scoreboard and commandmenus.";
			// 
			// labelDemoFix
			// 
			this->labelDemoFix->AutoSize = true;
			this->labelDemoFix->Location = System::Drawing::Point(18, 96);
			this->labelDemoFix->Name = L"labelDemoFix";
			this->labelDemoFix->Size = System::Drawing::Size(433, 52);
			this->labelDemoFix->TabIndex = 4;
			this->labelDemoFix->Text = resources->GetString(L"labelDemoFix.Text");
			// 
			// labelStuckFix
			// 
			this->labelStuckFix->AutoSize = true;
			this->labelStuckFix->Location = System::Drawing::Point(18, 189);
			this->labelStuckFix->Name = L"labelStuckFix";
			this->labelStuckFix->Size = System::Drawing::Size(421, 39);
			this->labelStuckFix->TabIndex = 6;
			this->labelStuckFix->Text = resources->GetString(L"labelStuckFix.Text");
			// 
			// buttonCancel
			// 
			this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Abort;
			this->buttonCancel->Location = System::Drawing::Point(385, 302);
			this->buttonCancel->Name = L"buttonCancel";
			this->buttonCancel->Size = System::Drawing::Size(75, 23);
			this->buttonCancel->TabIndex = 8;
			this->buttonCancel->Text = L"Cancel";
			this->buttonCancel->UseVisualStyleBackColor = true;
			// 
			// buttonNext
			// 
			this->buttonNext->DialogResult = System::Windows::Forms::DialogResult::Yes;
			this->buttonNext->Enabled = false;
			this->buttonNext->Location = System::Drawing::Point(278, 302);
			this->buttonNext->Name = L"buttonNext";
			this->buttonNext->Size = System::Drawing::Size(75, 23);
			this->buttonNext->TabIndex = 2;
			this->buttonNext->Text = L"Next >";
			this->buttonNext->UseVisualStyleBackColor = true;
			// 
			// buttonPrev
			// 
			this->buttonPrev->DialogResult = System::Windows::Forms::DialogResult::No;
			this->buttonPrev->Enabled = false;
			this->buttonPrev->Location = System::Drawing::Point(197, 302);
			this->buttonPrev->Name = L"buttonPrev";
			this->buttonPrev->Size = System::Drawing::Size(75, 23);
			this->buttonPrev->TabIndex = 7;
			this->buttonPrev->Text = L"< Back";
			this->buttonPrev->UseVisualStyleBackColor = true;
			// 
			// checkBoxDemoHeader
			// 
			this->checkBoxDemoHeader->AutoSize = true;
			this->checkBoxDemoHeader->Location = System::Drawing::Point(21, 248);
			this->checkBoxDemoHeader->Name = L"checkBoxDemoHeader";
			this->checkBoxDemoHeader->Size = System::Drawing::Size(206, 17);
			this->checkBoxDemoHeader->TabIndex = 9;
			this->checkBoxDemoHeader->Text = L"Try to convert demo version (47 -> 48)";
			this->checkBoxDemoHeader->UseVisualStyleBackColor = true;
			this->checkBoxDemoHeader->CheckedChanged += gcnew System::EventHandler(this, &DemoToolsWiz1::checkBoxXXX_CheckedChanged);
			// 
			// DemoToolsWiz1
			// 
			this->ClientSize = System::Drawing::Size(474, 335);
			this->Controls->Add(this->checkBoxDemoHeader);
			this->Controls->Add(this->buttonPrev);
			this->Controls->Add(this->buttonNext);
			this->Controls->Add(this->buttonCancel);
			this->Controls->Add(this->labelCleanUp);
			this->Controls->Add(this->labelDemoFix);
			this->Controls->Add(this->labelStuckFix);
			this->Controls->Add(this->checkBoxStuck);
			this->Controls->Add(this->checkBoxFix);
			this->Controls->Add(this->checkBoxCleanup);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"DemoToolsWiz1";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"DemoTools Wizard - Task selection";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	private:
		System::Void checkBoxXXX_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		{ doUpdateNext(); }
	};

} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {
