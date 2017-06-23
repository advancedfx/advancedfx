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
	/// Summary for DemoToolsWiz3
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	ref class DemoToolsWiz3 : public System::Windows::Forms::Form
	{
	public:
		DemoToolsWiz3(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

		bool bCheckedMarks( void )
		{
			return this->checkBoxMarks->Checked;
		}

		System::String ^ GetFile(int index)
		{
			return this->listDemos->Items[index]->Name;
		}

		int GetFileCount() {
			return this->listDemos->Items->Count;
		}

		void RemoveFile(int index) {
			this->listDemos->Items->RemoveAt(index);
		}

		System::String ^ GetOutDir()
		{
			return this->textOutDir->Text;
		}

		void SetOutDir(String ^ value) {
			this->textOutDir->Text = value;
		}

	private:
		void doUpdateFinish( void )
		{
			this->buttonNext->Enabled = (
				0 < listDemos->Items->Count
				&& IO::Directory::Exists( this->textOutDir->Text )
			);
		}

		void AddFile(String ^ fileName) {
			String ^ fullPath = IO::Path::GetFullPath(fileName);
			String ^ shortName = IO::Path::GetFileName(fileName);
			if(listDemos->Items->IndexOfKey(fullPath) < 0) {
				// not in the list yet
				ListViewItem ^ li = gcnew ListViewItem();
				li->Text = shortName;
				li->Name = fullPath;
				li->SubItems->Add(fullPath);

				listDemos->Items->Add(li);
			}
		}


	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DemoToolsWiz3()
		{
			if (components)
			{
				delete components;
			}
		}

	protected: 





	private: System::Windows::Forms::Button^  buttonCancel;
	private: System::Windows::Forms::Button^  buttonNext;
	private: System::Windows::Forms::Button^  buttonPrev;
	private: System::Windows::Forms::Label^  labelFileSelect;
	private: System::Windows::Forms::GroupBox^  groupBoxIn;




	private: System::Windows::Forms::GroupBox^  groupBoxOut;
	private: System::Windows::Forms::CheckBox^  checkBoxMarks;
	private: System::Windows::Forms::TextBox^  textOutDir;





	private: System::Windows::Forms::Button^  buttonOut;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog;

	private: System::Windows::Forms::ListView^  listDemos;

	private: System::Windows::Forms::ColumnHeader^  colName;
	private: System::Windows::Forms::ColumnHeader^  colPath;
	private: System::Windows::Forms::Button^  buttonAdd;
	private: System::Windows::Forms::Button^  buttonRemove;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::FolderBrowserDialog^  folderDialog;



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
			this->buttonCancel = (gcnew System::Windows::Forms::Button());
			this->buttonNext = (gcnew System::Windows::Forms::Button());
			this->buttonPrev = (gcnew System::Windows::Forms::Button());
			this->labelFileSelect = (gcnew System::Windows::Forms::Label());
			this->groupBoxIn = (gcnew System::Windows::Forms::GroupBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->buttonRemove = (gcnew System::Windows::Forms::Button());
			this->buttonAdd = (gcnew System::Windows::Forms::Button());
			this->listDemos = (gcnew System::Windows::Forms::ListView());
			this->colName = (gcnew System::Windows::Forms::ColumnHeader());
			this->colPath = (gcnew System::Windows::Forms::ColumnHeader());
			this->groupBoxOut = (gcnew System::Windows::Forms::GroupBox());
			this->checkBoxMarks = (gcnew System::Windows::Forms::CheckBox());
			this->textOutDir = (gcnew System::Windows::Forms::TextBox());
			this->buttonOut = (gcnew System::Windows::Forms::Button());
			this->openFileDialog = (gcnew System::Windows::Forms::OpenFileDialog());
			this->folderDialog = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->groupBoxIn->SuspendLayout();
			this->groupBoxOut->SuspendLayout();
			this->SuspendLayout();
			// 
			// buttonCancel
			// 
			this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Abort;
			this->buttonCancel->Location = System::Drawing::Point(385, 302);
			this->buttonCancel->Name = L"buttonCancel";
			this->buttonCancel->Size = System::Drawing::Size(75, 23);
			this->buttonCancel->TabIndex = 5;
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
			this->buttonNext->TabIndex = 0;
			this->buttonNext->Text = L"Finish";
			this->buttonNext->UseVisualStyleBackColor = true;
			// 
			// buttonPrev
			// 
			this->buttonPrev->DialogResult = System::Windows::Forms::DialogResult::No;
			this->buttonPrev->Location = System::Drawing::Point(197, 302);
			this->buttonPrev->Name = L"buttonPrev";
			this->buttonPrev->Size = System::Drawing::Size(75, 23);
			this->buttonPrev->TabIndex = 1;
			this->buttonPrev->Text = L"< Back";
			this->buttonPrev->UseVisualStyleBackColor = true;
			// 
			// labelFileSelect
			// 
			this->labelFileSelect->AutoSize = true;
			this->labelFileSelect->Location = System::Drawing::Point(12, 9);
			this->labelFileSelect->Name = L"labelFileSelect";
			this->labelFileSelect->Size = System::Drawing::Size(71, 13);
			this->labelFileSelect->TabIndex = 2;
			this->labelFileSelect->Text = L"File selection:";
			// 
			// groupBoxIn
			// 
			this->groupBoxIn->Controls->Add(this->label1);
			this->groupBoxIn->Controls->Add(this->buttonRemove);
			this->groupBoxIn->Controls->Add(this->buttonAdd);
			this->groupBoxIn->Controls->Add(this->listDemos);
			this->groupBoxIn->Location = System::Drawing::Point(12, 25);
			this->groupBoxIn->Name = L"groupBoxIn";
			this->groupBoxIn->Size = System::Drawing::Size(450, 169);
			this->groupBoxIn->TabIndex = 3;
			this->groupBoxIn->TabStop = false;
			this->groupBoxIn->Text = L"In: demo files";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(258, 24);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(152, 13);
			this->label1->TabIndex = 3;
			this->label1->Text = L"Tip: Drag and drop into the list.";
			// 
			// buttonRemove
			// 
			this->buttonRemove->Location = System::Drawing::Point(118, 19);
			this->buttonRemove->Name = L"buttonRemove";
			this->buttonRemove->Size = System::Drawing::Size(75, 23);
			this->buttonRemove->TabIndex = 2;
			this->buttonRemove->Text = L"Remove";
			this->buttonRemove->UseVisualStyleBackColor = true;
			this->buttonRemove->Click += gcnew System::EventHandler(this, &DemoToolsWiz3::buttonRemove_Click);
			// 
			// buttonAdd
			// 
			this->buttonAdd->Location = System::Drawing::Point(6, 19);
			this->buttonAdd->Name = L"buttonAdd";
			this->buttonAdd->Size = System::Drawing::Size(75, 23);
			this->buttonAdd->TabIndex = 1;
			this->buttonAdd->Text = L"Add";
			this->buttonAdd->UseVisualStyleBackColor = true;
			this->buttonAdd->Click += gcnew System::EventHandler(this, &DemoToolsWiz3::buttonAdd_Click);
			// 
			// listDemos
			// 
			this->listDemos->AllowDrop = true;
			this->listDemos->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->colName, this->colPath});
			this->listDemos->HeaderStyle = System::Windows::Forms::ColumnHeaderStyle::Nonclickable;
			this->listDemos->Location = System::Drawing::Point(6, 48);
			this->listDemos->Name = L"listDemos";
			this->listDemos->Size = System::Drawing::Size(438, 115);
			this->listDemos->TabIndex = 0;
			this->listDemos->UseCompatibleStateImageBehavior = false;
			this->listDemos->View = System::Windows::Forms::View::Details;
			this->listDemos->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &DemoToolsWiz3::listDemos_DragDrop);
			this->listDemos->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &DemoToolsWiz3::listDemos_DragEnter);
			// 
			// colName
			// 
			this->colName->Text = L"Name";
			this->colName->Width = 200;
			// 
			// colPath
			// 
			this->colPath->Text = L"Path";
			this->colPath->Width = 400;
			// 
			// groupBoxOut
			// 
			this->groupBoxOut->Controls->Add(this->checkBoxMarks);
			this->groupBoxOut->Controls->Add(this->textOutDir);
			this->groupBoxOut->Controls->Add(this->buttonOut);
			this->groupBoxOut->Location = System::Drawing::Point(12, 200);
			this->groupBoxOut->Name = L"groupBoxOut";
			this->groupBoxOut->Size = System::Drawing::Size(450, 78);
			this->groupBoxOut->TabIndex = 4;
			this->groupBoxOut->TabStop = false;
			this->groupBoxOut->Text = L"Out: folder to save to";
			// 
			// checkBoxMarks
			// 
			this->checkBoxMarks->AutoSize = true;
			this->checkBoxMarks->Checked = true;
			this->checkBoxMarks->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBoxMarks->Location = System::Drawing::Point(12, 52);
			this->checkBoxMarks->Name = L"checkBoxMarks";
			this->checkBoxMarks->Size = System::Drawing::Size(135, 17);
			this->checkBoxMarks->TabIndex = 2;
			this->checkBoxMarks->Text = L"add HLAE water marks";
			this->checkBoxMarks->UseVisualStyleBackColor = true;
			// 
			// textOutDir
			// 
			this->textOutDir->Location = System::Drawing::Point(12, 23);
			this->textOutDir->Name = L"textOutDir";
			this->textOutDir->Size = System::Drawing::Size(333, 20);
			this->textOutDir->TabIndex = 1;
			this->textOutDir->TextChanged += gcnew System::EventHandler(this, &DemoToolsWiz3::textOutDir_TextChanged);
			// 
			// buttonOut
			// 
			this->buttonOut->Location = System::Drawing::Point(351, 21);
			this->buttonOut->Name = L"buttonOut";
			this->buttonOut->Size = System::Drawing::Size(75, 23);
			this->buttonOut->TabIndex = 0;
			this->buttonOut->Text = L"Browse";
			this->buttonOut->UseVisualStyleBackColor = true;
			this->buttonOut->Click += gcnew System::EventHandler(this, &DemoToolsWiz3::buttonOut_Click);
			// 
			// openFileDialog
			// 
			this->openFileDialog->DefaultExt = L"dem";
			this->openFileDialog->FileName = L"*.dem";
			this->openFileDialog->Filter = L"Half-Life Demo (*.dem)|*.dem";
			this->openFileDialog->Multiselect = true;
			// 
			// folderDialog
			// 
			this->folderDialog->Description = L"Select (different) destination folder.";
			// 
			// DemoToolsWiz3
			// 
			this->ClientSize = System::Drawing::Size(474, 335);
			this->Controls->Add(this->groupBoxOut);
			this->Controls->Add(this->groupBoxIn);
			this->Controls->Add(this->labelFileSelect);
			this->Controls->Add(this->buttonPrev);
			this->Controls->Add(this->buttonNext);
			this->Controls->Add(this->buttonCancel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"DemoToolsWiz3";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"DemoTools Wizard - File Selection";
			this->groupBoxIn->ResumeLayout(false);
			this->groupBoxIn->PerformLayout();
			this->groupBoxOut->ResumeLayout(false);
			this->groupBoxOut->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private:
		//
		// Events:
		//

	System::Void buttonOut_Click(System::Object^  sender, System::EventArgs^  e){
		if(IO::Directory::Exists(textOutDir->Text)) 
			folderDialog->SelectedPath = textOutDir->Text;

		if (::DialogResult::OK == folderDialog->ShowDialog(this))
		{
			this->textOutDir->Text = folderDialog->SelectedPath;
		}

		doUpdateFinish();
	}

	System::Void buttonRemove_Click(System::Object^  sender, System::EventArgs^  e) {
		listDemos->BeginUpdate();
		while(0 < listDemos->SelectedIndices->Count)
			listDemos->Items->RemoveAt(listDemos->SelectedIndices[0]);
		listDemos->EndUpdate();

		doUpdateFinish();
	}

	System::Void buttonAdd_Click(System::Object^  sender, System::EventArgs^  e) {
		if(::DialogResult::OK == this->openFileDialog->ShowDialog(this)) {
			listDemos->BeginUpdate();
			for each(String ^ fileName in this->openFileDialog->FileNames) {
				AddFile(fileName);
			}
			listDemos->EndUpdate();
		}

		doUpdateFinish();
	}

	System::Void listDemos_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {
		if(e->Data->GetDataPresent(DataFormats::FileDrop)) {
			e->Effect = DragDropEffects::Link;
		}
	}
	System::Void listDemos_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {
		if(e->Data->GetDataPresent(DataFormats::FileDrop)) {
			try {
				array<String ^> ^ files =  dynamic_cast<array<String ^> ^>(e->Data->GetData(DataFormats::FileDrop));
				if(files) {
					for each(String ^ file in files)
						AddFile(file);
				}
			}
			catch(...) {
			}
		}
		doUpdateFinish();
	}

	System::Void textOutDir_TextChanged(System::Object^  sender, System::EventArgs^  e) {
		doUpdateFinish();
	}
}; // class

} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {

