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
	/// Summary for DemoToolsWiz2
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	ref class DemoToolsWiz2 : public System::Windows::Forms::Form
	{
	public:
		DemoToolsWiz2(void)
		{
			InitializeComponent();

			AddMap("+commandmenu", "+c0mmandmenu");
			AddMap("+showscores", "+sh0wscores");
			AddMap("+score", "+sc0re");
			AddMap("togglescores", "t0gglescores");
		}

		int GetMapCnt()
		{
			return listMap->Items->Count;
		}

		System::String ^ GetMapDst(int index)
		{
			return listMap->Items[index]->SubItems[1]->Text;
		}

		System::String ^ GetMapSrc(int index)
		{
			return listMap->Items[index]->SubItems[0]->Text;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~DemoToolsWiz2()
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
	private: System::Windows::Forms::Label^  labelDemoCleanUp;
	private: System::Windows::Forms::Label^  labelCleanUpHelp;









	private: System::Windows::Forms::TextBox^  textBoxCmdDst;



	private: System::Windows::Forms::TextBox^  textBoxCmdSrc;
	private: System::Windows::Forms::Label^  labelCmdMap;





	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::ListView^  listMap;


	private: System::Windows::Forms::ColumnHeader^  colFrom;
	private: System::Windows::Forms::ColumnHeader^  colTo;
	private: System::Windows::Forms::Button^  buttonAdd;
	private: System::Windows::Forms::Button^  buttonRemove;
	private: System::Windows::Forms::Button^  buttonSwap;





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
			this->labelDemoCleanUp = (gcnew System::Windows::Forms::Label());
			this->labelCleanUpHelp = (gcnew System::Windows::Forms::Label());
			this->textBoxCmdDst = (gcnew System::Windows::Forms::TextBox());
			this->textBoxCmdSrc = (gcnew System::Windows::Forms::TextBox());
			this->labelCmdMap = (gcnew System::Windows::Forms::Label());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->buttonSwap = (gcnew System::Windows::Forms::Button());
			this->buttonAdd = (gcnew System::Windows::Forms::Button());
			this->buttonRemove = (gcnew System::Windows::Forms::Button());
			this->listMap = (gcnew System::Windows::Forms::ListView());
			this->colFrom = (gcnew System::Windows::Forms::ColumnHeader());
			this->colTo = (gcnew System::Windows::Forms::ColumnHeader());
			this->groupBox1->SuspendLayout();
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
			this->buttonNext->Location = System::Drawing::Point(278, 302);
			this->buttonNext->Name = L"buttonNext";
			this->buttonNext->Size = System::Drawing::Size(75, 23);
			this->buttonNext->TabIndex = 0;
			this->buttonNext->Text = L"Next >";
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
			// labelDemoCleanUp
			// 
			this->labelDemoCleanUp->AutoSize = true;
			this->labelDemoCleanUp->Location = System::Drawing::Point(12, 20);
			this->labelDemoCleanUp->Name = L"labelDemoCleanUp";
			this->labelDemoCleanUp->Size = System::Drawing::Size(79, 13);
			this->labelDemoCleanUp->TabIndex = 2;
			this->labelDemoCleanUp->Text = L"DemoCleanUp:";
			// 
			// labelCleanUpHelp
			// 
			this->labelCleanUpHelp->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->labelCleanUpHelp->Location = System::Drawing::Point(12, 42);
			this->labelCleanUpHelp->Name = L"labelCleanUpHelp";
			this->labelCleanUpHelp->Size = System::Drawing::Size(450, 46);
			this->labelCleanUpHelp->TabIndex = 3;
			this->labelCleanUpHelp->Text = L"By default the scoreboard and the commandmenu is removed.\r\nIf that is fine with y"
				L"ou just continue to the next page (Click Next).";
			// 
			// textBoxCmdDst
			// 
			this->textBoxCmdDst->Location = System::Drawing::Point(198, 154);
			this->textBoxCmdDst->Name = L"textBoxCmdDst";
			this->textBoxCmdDst->Size = System::Drawing::Size(140, 20);
			this->textBoxCmdDst->TabIndex = 8;
			this->textBoxCmdDst->Text = L"yet unused";
			// 
			// textBoxCmdSrc
			// 
			this->textBoxCmdSrc->Location = System::Drawing::Point(12, 154);
			this->textBoxCmdSrc->Name = L"textBoxCmdSrc";
			this->textBoxCmdSrc->Size = System::Drawing::Size(140, 20);
			this->textBoxCmdSrc->TabIndex = 6;
			// 
			// labelCmdMap
			// 
			this->labelCmdMap->AutoSize = true;
			this->labelCmdMap->Location = System::Drawing::Point(167, 157);
			this->labelCmdMap->Name = L"labelCmdMap";
			this->labelCmdMap->Size = System::Drawing::Size(16, 13);
			this->labelCmdMap->TabIndex = 7;
			this->labelCmdMap->Text = L"->";
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->buttonSwap);
			this->groupBox1->Controls->Add(this->buttonAdd);
			this->groupBox1->Controls->Add(this->buttonRemove);
			this->groupBox1->Controls->Add(this->listMap);
			this->groupBox1->Controls->Add(this->textBoxCmdDst);
			this->groupBox1->Controls->Add(this->textBoxCmdSrc);
			this->groupBox1->Controls->Add(this->labelCmdMap);
			this->groupBox1->Location = System::Drawing::Point(12, 91);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(450, 180);
			this->groupBox1->TabIndex = 4;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Command mappings";
			// 
			// buttonSwap
			// 
			this->buttonSwap->Location = System::Drawing::Point(344, 74);
			this->buttonSwap->Name = L"buttonSwap";
			this->buttonSwap->Size = System::Drawing::Size(100, 23);
			this->buttonSwap->TabIndex = 11;
			this->buttonSwap->Text = L"Swap";
			this->buttonSwap->UseVisualStyleBackColor = true;
			this->buttonSwap->Click += gcnew System::EventHandler(this, &DemoToolsWiz2::buttonSwap_Click);
			// 
			// buttonAdd
			// 
			this->buttonAdd->Location = System::Drawing::Point(344, 152);
			this->buttonAdd->Name = L"buttonAdd";
			this->buttonAdd->Size = System::Drawing::Size(100, 23);
			this->buttonAdd->TabIndex = 10;
			this->buttonAdd->Text = L"Add";
			this->buttonAdd->UseVisualStyleBackColor = true;
			this->buttonAdd->Click += gcnew System::EventHandler(this, &DemoToolsWiz2::buttonAdd_Click);
			// 
			// buttonRemove
			// 
			this->buttonRemove->Location = System::Drawing::Point(344, 19);
			this->buttonRemove->Name = L"buttonRemove";
			this->buttonRemove->Size = System::Drawing::Size(100, 23);
			this->buttonRemove->TabIndex = 9;
			this->buttonRemove->Text = L"Remove";
			this->buttonRemove->UseVisualStyleBackColor = true;
			this->buttonRemove->Click += gcnew System::EventHandler(this, &DemoToolsWiz2::buttonRemove_Click);
			// 
			// listMap
			// 
			this->listMap->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) { this->colFrom, this->colTo });
			this->listMap->FullRowSelect = true;
			this->listMap->GridLines = true;
			this->listMap->HeaderStyle = System::Windows::Forms::ColumnHeaderStyle::Nonclickable;
			this->listMap->HideSelection = false;
			this->listMap->Location = System::Drawing::Point(12, 19);
			this->listMap->Name = L"listMap";
			this->listMap->Size = System::Drawing::Size(326, 115);
			this->listMap->TabIndex = 6;
			this->listMap->UseCompatibleStateImageBehavior = false;
			this->listMap->View = System::Windows::Forms::View::Details;
			this->listMap->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &DemoToolsWiz2::listMap_KeyDown);
			// 
			// colFrom
			// 
			this->colFrom->Text = L"From";
			this->colFrom->Width = 150;
			// 
			// colTo
			// 
			this->colTo->Text = L"To";
			this->colTo->Width = 150;
			// 
			// DemoToolsWiz2
			// 
			this->ClientSize = System::Drawing::Size(474, 335);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->labelCleanUpHelp);
			this->Controls->Add(this->labelDemoCleanUp);
			this->Controls->Add(this->buttonPrev);
			this->Controls->Add(this->buttonNext);
			this->Controls->Add(this->buttonCancel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"DemoToolsWiz2";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"DemoTools Wizard - DemoCleanUp settings";
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


private:
		void AddMap(String ^ src, String ^ dst)
		{
			ListViewItem ^ li = nullptr;

			int idx = listMap->Items->IndexOfKey(src);

			if(idx < 0)
			{
				// not in the list yet
				li = gcnew ListViewItem();
				li->Text = src;
				li->Name = src;
				li->SubItems->Add(dst);

				listMap->Items->Add(li);
			}
			else
			{
				li = listMap->Items[idx];
				li->Name = src;
				li->SubItems[0]->Text = src;
				li->SubItems[1]->Text = dst;
			}
		}

		void Swap(ListViewItem ^ item)
		{
			String ^ src = item->SubItems[0]->Text;
			String ^ dst = item->SubItems[1]->Text;

			item->Name = dst;
			item->SubItems[0]->Text = dst;
			item->SubItems[1]->Text = src;
		}

private:
	System::Void buttonAdd_Click(System::Object^  sender, System::EventArgs^  e)
	{
		AddMap(this->textBoxCmdSrc->Text, this->textBoxCmdDst->Text);
	}
	
	System::Void buttonRemove_Click(System::Object^  sender, System::EventArgs^  e)
	{
		listMap->BeginUpdate();
		while(0 < listMap->SelectedIndices->Count)
			listMap->Items->RemoveAt(listMap->SelectedIndices[0]);
		listMap->EndUpdate();
	}
private:
	System::Void buttonSwap_Click(System::Object^  sender, System::EventArgs^  e)
	 {
		 listMap->BeginUpdate();
		 if(listMap->SelectedIndices->Count <= 0)
		 {
			for each(ListViewItem ^ li in listMap->Items)
				Swap(li);
		 }
		 else
		 {
			 for(int i=0; i < listMap->SelectedIndices->Count; i++)
				 Swap(listMap->Items[listMap->SelectedIndices[i]]);
		 }


		 listMap->EndUpdate();
	 }
private: System::Void listMap_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e)
		 {
			 if(e->Control && Keys::A == e->KeyCode)
			 {
				 bool selectedAll = listMap->Items->Count == listMap->SelectedIndices->Count;

				 listMap->BeginUpdate();
				 listMap->SelectedIndices->Clear();
				 if(!selectedAll)
				 {
					 for(int i=0; i<listMap->Items->Count; i++)
					 {
						 listMap->SelectedIndices->Add(i);
					 }
				 }
				 listMap->EndUpdate();
			 }
		 }
};

} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {

