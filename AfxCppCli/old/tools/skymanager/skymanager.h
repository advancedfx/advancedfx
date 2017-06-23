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
	/// Summary for skymanager
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class skymanager : public System::Windows::Forms::Form
	{
	public:
		skymanager(System::String ^ gamePath)
		{
			this->m_GamePath = gamePath;

			CreateInitialImages();

			InitializeComponent();

			 picImage->Refresh();
			 picPreview->Refresh();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~skymanager()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		System::String ^ m_GamePath;

	private: System::Windows::Forms::CheckBox^  checkPreRight;
	private: System::Windows::Forms::CheckBox^  checkPreBack;
	private: System::Windows::Forms::CheckBox^  checkPreUp;
	private: System::Windows::Forms::CheckBox^  checkPreDown;
	private: System::Windows::Forms::CheckBox^  checkPreFront;
	private: System::Windows::Forms::CheckBox^  checkPreLeft;
	private: System::Windows::Forms::RadioButton^  radioSelSky;
	private: System::Windows::Forms::GroupBox^  groupMirror;

	private: System::Windows::Forms::Button^  buttonLoad;
	private: System::Windows::Forms::GroupBox^  groupImage;
	private: System::Windows::Forms::Button^  butMirHorz;
	private: System::Windows::Forms::Button^  butMirVert;
	private: System::Windows::Forms::GroupBox^  groupRotate;

	private: System::Windows::Forms::Button^  butRot;


	private:
		Bitmap ^ imageFront;
		Bitmap ^ imageLeft;
		Bitmap ^ imageRight;
		Bitmap ^ imageDown;
		Bitmap ^ imageBack;
		Bitmap ^ imageUp;

	private: System::Windows::Forms::OpenFileDialog^  openImageDialog;
	private: System::Windows::Forms::RadioButton^  radioPreFlat;
	private: System::Windows::Forms::RadioButton^  radioPrePer;
	private: System::Windows::Forms::Panel^  picImage;
	private: System::Windows::Forms::Panel^  picPreview;
	private: System::Windows::Forms::Label^  labelHint;



	private: System::Windows::Forms::FolderBrowserDialog^  folderBrowserDialog;

	private:
		ref struct CubeSide {
			PointF btLeft;
			PointF btRight;
			PointF tpRight;
			PointF tpLeft;
		};
		ref struct CubeSides {
			CubeSide back;
			CubeSide right;
			CubeSide front;
			CubeSide left;
			CubeSide up;
			CubeSide down;			
		};

		void CoordToViewPoint(PointF % coordPoint, int sx, int sy, Point % viewPoint);

		void GetCubeCoordsFlat(CubeSides % outCoords);

		void LoadImage();

		System::Drawing::Bitmap ^LoadTaragaFile(System::String ^ filePath);

		void PaintPicPreview(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e);

		bool PointInRect(Drawing::Point % point, int invSx, int invSy, CubeSide % cs);

		void SkySelection(Point point);

		void CreateInitialImages()
		{
			Graphics ^imgGfx;
			Drawing::Font^ drawFont = gcnew Drawing::Font( Drawing::FontFamily::GenericSerif,16 );
			SolidBrush^ drawBrush = gcnew SolidBrush( Color::Blue );
			PointF drawPoint = PointF(16.0f,16.0f);

			imageFront = gcnew Bitmap(128, 128, Imaging::PixelFormat::Format24bppRgb);
			imgGfx = Graphics::FromImage(imageFront);
			imgGfx->Clear(Drawing::Color::Aqua);
			imgGfx->DrawString("Front", drawFont, drawBrush, drawPoint);

			imageLeft = gcnew Bitmap(128, 128, Imaging::PixelFormat::Format24bppRgb);
			imgGfx = Graphics::FromImage(imageLeft);
			imgGfx->Clear(Drawing::Color::Aqua);
			imgGfx->DrawString("Left", drawFont, drawBrush, drawPoint);

			imageRight = gcnew Bitmap(128, 128, Imaging::PixelFormat::Format24bppRgb);
			imgGfx = Graphics::FromImage(imageRight);
			imgGfx->Clear(Drawing::Color::Aqua);
			imgGfx->DrawString("Right", drawFont, drawBrush, drawPoint);

			imageDown = gcnew Bitmap(128, 128, Imaging::PixelFormat::Format24bppRgb);
			imgGfx = Graphics::FromImage(imageDown);
			imgGfx->Clear(Drawing::Color::Aqua);
			imgGfx->DrawString("Down", drawFont, drawBrush, drawPoint);

			imageBack = gcnew Bitmap(128, 128, Imaging::PixelFormat::Format24bppRgb);
			imgGfx = Graphics::FromImage(imageBack);
			imgGfx->Clear(Drawing::Color::Aqua);
			imgGfx->DrawString("Back", drawFont, drawBrush, drawPoint);

			imageUp = gcnew Bitmap(128, 128, Imaging::PixelFormat::Format24bppRgb);
			imgGfx = Graphics::FromImage(imageUp);
			imgGfx->Clear(Drawing::Color::Aqua);
			imgGfx->DrawString("Up", drawFont, drawBrush, drawPoint);
		}

		Bitmap ^ GetSelectedImage()
		{
			// Down:
			if(this->radioSelDown->Checked)
			{
				return imageDown;
			}
			// Left:
			else if(this->radioSelLeft->Checked)
			{
				return imageLeft;
			}
			// Front:
			else if(this->radioSelFront->Checked)
			{
				return imageFront;
			}
			// Back:
			else if(this->radioSelBack->Checked)
			{
				return imageBack;
			}
			// Right:
			else if(this->radioSelRight->Checked)
			{
				return imageRight;
			}
			// Up:
			else if(this->radioSelUp->Checked)
			{
				return imageUp;
			};

			return nullptr;
		}

		// does not catch exceptions
		void WriteHlaeImage(Bitmap ^ bmpF, String ^ fileName)
		{
			bmpF->Save(fileName, Imaging::ImageFormat::Bmp);

		}


	private: System::Windows::Forms::GroupBox^  groupPreview;
	private: System::Windows::Forms::GroupBox^  groupSelect;
	private: System::Windows::Forms::RadioButton^  radioSelRight;
	private: System::Windows::Forms::RadioButton^  radioSelBack;
	private: System::Windows::Forms::RadioButton^  radioSelUp;
	private: System::Windows::Forms::RadioButton^  radioSelDown;
	private: System::Windows::Forms::RadioButton^  radioSelFront;
	private: System::Windows::Forms::RadioButton^  radioSelLeft;
	private: System::Windows::Forms::Button^  buttonExport;


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
			this->groupPreview = (gcnew System::Windows::Forms::GroupBox());
			this->radioPreFlat = (gcnew System::Windows::Forms::RadioButton());
			this->radioPrePer = (gcnew System::Windows::Forms::RadioButton());
			this->checkPreRight = (gcnew System::Windows::Forms::CheckBox());
			this->checkPreBack = (gcnew System::Windows::Forms::CheckBox());
			this->checkPreUp = (gcnew System::Windows::Forms::CheckBox());
			this->checkPreDown = (gcnew System::Windows::Forms::CheckBox());
			this->checkPreFront = (gcnew System::Windows::Forms::CheckBox());
			this->checkPreLeft = (gcnew System::Windows::Forms::CheckBox());
			this->groupSelect = (gcnew System::Windows::Forms::GroupBox());
			this->labelHint = (gcnew System::Windows::Forms::Label());
			this->radioSelSky = (gcnew System::Windows::Forms::RadioButton());
			this->radioSelRight = (gcnew System::Windows::Forms::RadioButton());
			this->radioSelBack = (gcnew System::Windows::Forms::RadioButton());
			this->radioSelUp = (gcnew System::Windows::Forms::RadioButton());
			this->radioSelDown = (gcnew System::Windows::Forms::RadioButton());
			this->radioSelFront = (gcnew System::Windows::Forms::RadioButton());
			this->radioSelLeft = (gcnew System::Windows::Forms::RadioButton());
			this->buttonExport = (gcnew System::Windows::Forms::Button());
			this->groupMirror = (gcnew System::Windows::Forms::GroupBox());
			this->butMirVert = (gcnew System::Windows::Forms::Button());
			this->butMirHorz = (gcnew System::Windows::Forms::Button());
			this->buttonLoad = (gcnew System::Windows::Forms::Button());
			this->groupImage = (gcnew System::Windows::Forms::GroupBox());
			this->picImage = (gcnew System::Windows::Forms::Panel());
			this->groupRotate = (gcnew System::Windows::Forms::GroupBox());
			this->butRot = (gcnew System::Windows::Forms::Button());
			this->openImageDialog = (gcnew System::Windows::Forms::OpenFileDialog());
			this->folderBrowserDialog = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->picPreview = (gcnew System::Windows::Forms::Panel());
			this->groupPreview->SuspendLayout();
			this->groupSelect->SuspendLayout();
			this->groupMirror->SuspendLayout();
			this->groupImage->SuspendLayout();
			this->groupRotate->SuspendLayout();
			this->SuspendLayout();
			// 
			// groupPreview
			// 
			this->groupPreview->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->groupPreview->Controls->Add(this->radioPreFlat);
			this->groupPreview->Controls->Add(this->radioPrePer);
			this->groupPreview->Controls->Add(this->checkPreRight);
			this->groupPreview->Controls->Add(this->checkPreBack);
			this->groupPreview->Controls->Add(this->checkPreUp);
			this->groupPreview->Controls->Add(this->checkPreDown);
			this->groupPreview->Controls->Add(this->checkPreFront);
			this->groupPreview->Controls->Add(this->checkPreLeft);
			this->groupPreview->Location = System::Drawing::Point(209, 1);
			this->groupPreview->Name = L"groupPreview";
			this->groupPreview->Size = System::Drawing::Size(281, 88);
			this->groupPreview->TabIndex = 3;
			this->groupPreview->TabStop = false;
			this->groupPreview->Text = L"Preview";
			// 
			// radioPreFlat
			// 
			this->radioPreFlat->Checked = true;
			this->radioPreFlat->Location = System::Drawing::Point(150, 65);
			this->radioPreFlat->Name = L"radioPreFlat";
			this->radioPreFlat->Size = System::Drawing::Size(100, 17);
			this->radioPreFlat->TabIndex = 7;
			this->radioPreFlat->TabStop = true;
			this->radioPreFlat->Text = L"flat";
			this->radioPreFlat->UseVisualStyleBackColor = true;
			this->radioPreFlat->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// radioPrePer
			// 
			this->radioPrePer->Location = System::Drawing::Point(15, 65);
			this->radioPrePer->Name = L"radioPrePer";
			this->radioPrePer->Size = System::Drawing::Size(100, 17);
			this->radioPrePer->TabIndex = 6;
			this->radioPrePer->Text = L"cube";
			this->radioPrePer->UseVisualStyleBackColor = true;
			this->radioPrePer->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// checkPreRight
			// 
			this->checkPreRight->Checked = true;
			this->checkPreRight->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkPreRight->Enabled = false;
			this->checkPreRight->Location = System::Drawing::Point(99, 19);
			this->checkPreRight->Name = L"checkPreRight";
			this->checkPreRight->Size = System::Drawing::Size(60, 17);
			this->checkPreRight->TabIndex = 1;
			this->checkPreRight->Text = L"Right";
			this->checkPreRight->UseVisualStyleBackColor = true;
			this->checkPreRight->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// checkPreBack
			// 
			this->checkPreBack->Checked = true;
			this->checkPreBack->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkPreBack->Enabled = false;
			this->checkPreBack->Location = System::Drawing::Point(15, 19);
			this->checkPreBack->Name = L"checkPreBack";
			this->checkPreBack->Size = System::Drawing::Size(60, 17);
			this->checkPreBack->TabIndex = 0;
			this->checkPreBack->Text = L"Back";
			this->checkPreBack->UseVisualStyleBackColor = true;
			this->checkPreBack->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// checkPreUp
			// 
			this->checkPreUp->Enabled = false;
			this->checkPreUp->Location = System::Drawing::Point(184, 19);
			this->checkPreUp->Name = L"checkPreUp";
			this->checkPreUp->Size = System::Drawing::Size(60, 17);
			this->checkPreUp->TabIndex = 4;
			this->checkPreUp->Text = L"Up";
			this->checkPreUp->UseVisualStyleBackColor = true;
			this->checkPreUp->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// checkPreDown
			// 
			this->checkPreDown->Checked = true;
			this->checkPreDown->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkPreDown->Enabled = false;
			this->checkPreDown->Location = System::Drawing::Point(184, 42);
			this->checkPreDown->Name = L"checkPreDown";
			this->checkPreDown->Size = System::Drawing::Size(60, 17);
			this->checkPreDown->TabIndex = 5;
			this->checkPreDown->Text = L"Down";
			this->checkPreDown->UseVisualStyleBackColor = true;
			this->checkPreDown->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// checkPreFront
			// 
			this->checkPreFront->Enabled = false;
			this->checkPreFront->Location = System::Drawing::Point(15, 42);
			this->checkPreFront->Name = L"checkPreFront";
			this->checkPreFront->Size = System::Drawing::Size(60, 17);
			this->checkPreFront->TabIndex = 2;
			this->checkPreFront->Text = L"Front";
			this->checkPreFront->UseVisualStyleBackColor = true;
			this->checkPreFront->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// checkPreLeft
			// 
			this->checkPreLeft->Enabled = false;
			this->checkPreLeft->Location = System::Drawing::Point(99, 42);
			this->checkPreLeft->Name = L"checkPreLeft";
			this->checkPreLeft->Size = System::Drawing::Size(60, 17);
			this->checkPreLeft->TabIndex = 3;
			this->checkPreLeft->Text = L"Left";
			this->checkPreLeft->UseVisualStyleBackColor = true;
			this->checkPreLeft->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Prev_CheckedChanged);
			// 
			// groupSelect
			// 
			this->groupSelect->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->groupSelect->Controls->Add(this->labelHint);
			this->groupSelect->Controls->Add(this->radioSelSky);
			this->groupSelect->Controls->Add(this->radioSelRight);
			this->groupSelect->Controls->Add(this->radioSelBack);
			this->groupSelect->Controls->Add(this->radioSelUp);
			this->groupSelect->Controls->Add(this->radioSelDown);
			this->groupSelect->Controls->Add(this->radioSelFront);
			this->groupSelect->Controls->Add(this->radioSelLeft);
			this->groupSelect->Location = System::Drawing::Point(209, 95);
			this->groupSelect->Name = L"groupSelect";
			this->groupSelect->Size = System::Drawing::Size(281, 106);
			this->groupSelect->TabIndex = 0;
			this->groupSelect->TabStop = false;
			this->groupSelect->Text = L"Image Selection";
			// 
			// labelHint
			// 
			this->labelHint->AutoSize = true;
			this->labelHint->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
			this->labelHint->Location = System::Drawing::Point(115, 83);
			this->labelHint->Name = L"labelHint";
			this->labelHint->Size = System::Drawing::Size(147, 13);
			this->labelHint->TabIndex = 7;
			this->labelHint->Text = L"Hint: (double) click the image.";
			// 
			// radioSelSky
			// 
			this->radioSelSky->Location = System::Drawing::Point(15, 79);
			this->radioSelSky->Name = L"radioSelSky";
			this->radioSelSky->Size = System::Drawing::Size(120, 17);
			this->radioSelSky->TabIndex = 6;
			this->radioSelSky->Text = L"Complete sky";
			this->radioSelSky->UseVisualStyleBackColor = true;
			this->radioSelSky->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Sel_CheckedChanged);
			// 
			// radioSelRight
			// 
			this->radioSelRight->Location = System::Drawing::Point(99, 23);
			this->radioSelRight->Name = L"radioSelRight";
			this->radioSelRight->Size = System::Drawing::Size(60, 17);
			this->radioSelRight->TabIndex = 1;
			this->radioSelRight->Text = L"Right";
			this->radioSelRight->UseVisualStyleBackColor = true;
			this->radioSelRight->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Sel_CheckedChanged);
			// 
			// radioSelBack
			// 
			this->radioSelBack->Checked = true;
			this->radioSelBack->Location = System::Drawing::Point(15, 23);
			this->radioSelBack->Name = L"radioSelBack";
			this->radioSelBack->Size = System::Drawing::Size(60, 17);
			this->radioSelBack->TabIndex = 0;
			this->radioSelBack->TabStop = true;
			this->radioSelBack->Text = L"Back";
			this->radioSelBack->UseVisualStyleBackColor = true;
			this->radioSelBack->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Sel_CheckedChanged);
			// 
			// radioSelUp
			// 
			this->radioSelUp->Location = System::Drawing::Point(184, 23);
			this->radioSelUp->Name = L"radioSelUp";
			this->radioSelUp->Size = System::Drawing::Size(60, 17);
			this->radioSelUp->TabIndex = 4;
			this->radioSelUp->Text = L"Up";
			this->radioSelUp->UseVisualStyleBackColor = true;
			this->radioSelUp->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Sel_CheckedChanged);
			// 
			// radioSelDown
			// 
			this->radioSelDown->Location = System::Drawing::Point(184, 51);
			this->radioSelDown->Name = L"radioSelDown";
			this->radioSelDown->Size = System::Drawing::Size(60, 17);
			this->radioSelDown->TabIndex = 5;
			this->radioSelDown->Text = L"Down";
			this->radioSelDown->UseVisualStyleBackColor = true;
			this->radioSelDown->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Sel_CheckedChanged);
			// 
			// radioSelFront
			// 
			this->radioSelFront->Location = System::Drawing::Point(15, 51);
			this->radioSelFront->Name = L"radioSelFront";
			this->radioSelFront->Size = System::Drawing::Size(60, 17);
			this->radioSelFront->TabIndex = 2;
			this->radioSelFront->Text = L"Front";
			this->radioSelFront->UseVisualStyleBackColor = true;
			this->radioSelFront->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Sel_CheckedChanged);
			// 
			// radioSelLeft
			// 
			this->radioSelLeft->Location = System::Drawing::Point(99, 51);
			this->radioSelLeft->Name = L"radioSelLeft";
			this->radioSelLeft->Size = System::Drawing::Size(60, 17);
			this->radioSelLeft->TabIndex = 3;
			this->radioSelLeft->Text = L"Left";
			this->radioSelLeft->UseVisualStyleBackColor = true;
			this->radioSelLeft->CheckedChanged += gcnew System::EventHandler(this, &skymanager::Sel_CheckedChanged);
			// 
			// buttonExport
			// 
			this->buttonExport->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->buttonExport->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->buttonExport->Location = System::Drawing::Point(3, 341);
			this->buttonExport->Name = L"buttonExport";
			this->buttonExport->Size = System::Drawing::Size(199, 28);
			this->buttonExport->TabIndex = 2;
			this->buttonExport->Text = L"Export to game folder";
			this->buttonExport->UseVisualStyleBackColor = true;
			this->buttonExport->Click += gcnew System::EventHandler(this, &skymanager::buttonExport_Click);
			// 
			// groupMirror
			// 
			this->groupMirror->Controls->Add(this->butMirVert);
			this->groupMirror->Controls->Add(this->butMirHorz);
			this->groupMirror->Location = System::Drawing::Point(112, 68);
			this->groupMirror->Name = L"groupMirror";
			this->groupMirror->Size = System::Drawing::Size(220, 50);
			this->groupMirror->TabIndex = 1;
			this->groupMirror->TabStop = false;
			this->groupMirror->Text = L"mirror";
			// 
			// butMirVert
			// 
			this->butMirVert->Location = System::Drawing::Point(112, 19);
			this->butMirVert->Name = L"butMirVert";
			this->butMirVert->Size = System::Drawing::Size(102, 20);
			this->butMirVert->TabIndex = 1;
			this->butMirVert->Text = L"vertical";
			this->butMirVert->UseVisualStyleBackColor = true;
			this->butMirVert->Click += gcnew System::EventHandler(this, &skymanager::ImageOpClick);
			// 
			// butMirHorz
			// 
			this->butMirHorz->Location = System::Drawing::Point(4, 19);
			this->butMirHorz->Name = L"butMirHorz";
			this->butMirHorz->Size = System::Drawing::Size(102, 20);
			this->butMirHorz->TabIndex = 0;
			this->butMirHorz->Text = L"horizontal";
			this->butMirHorz->UseVisualStyleBackColor = true;
			this->butMirHorz->Click += gcnew System::EventHandler(this, &skymanager::ImageOpClick);
			// 
			// buttonLoad
			// 
			this->buttonLoad->Location = System::Drawing::Point(112, 19);
			this->buttonLoad->Name = L"buttonLoad";
			this->buttonLoad->Size = System::Drawing::Size(369, 35);
			this->buttonLoad->TabIndex = 0;
			this->buttonLoad->Text = L"Load from File";
			this->buttonLoad->UseVisualStyleBackColor = true;
			this->buttonLoad->Click += gcnew System::EventHandler(this, &skymanager::buttonLoad_Click);
			// 
			// groupImage
			// 
			this->groupImage->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->groupImage->Controls->Add(this->picImage);
			this->groupImage->Controls->Add(this->buttonLoad);
			this->groupImage->Controls->Add(this->groupMirror);
			this->groupImage->Controls->Add(this->groupRotate);
			this->groupImage->Location = System::Drawing::Point(3, 207);
			this->groupImage->Name = L"groupImage";
			this->groupImage->Size = System::Drawing::Size(487, 125);
			this->groupImage->TabIndex = 1;
			this->groupImage->TabStop = false;
			this->groupImage->Text = L"Image Tools";
			// 
			// picImage
			// 
			this->picImage->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->picImage->Location = System::Drawing::Point(8, 18);
			this->picImage->Name = L"picImage";
			this->picImage->Size = System::Drawing::Size(100, 100);
			this->picImage->TabIndex = 3;
			this->picImage->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &skymanager::PaintPicImage);
			// 
			// groupRotate
			// 
			this->groupRotate->Controls->Add(this->butRot);
			this->groupRotate->Location = System::Drawing::Point(338, 68);
			this->groupRotate->Name = L"groupRotate";
			this->groupRotate->Size = System::Drawing::Size(143, 50);
			this->groupRotate->TabIndex = 2;
			this->groupRotate->TabStop = false;
			this->groupRotate->Text = L"rotate";
			// 
			// butRot
			// 
			this->butRot->Location = System::Drawing::Point(7, 19);
			this->butRot->Name = L"butRot";
			this->butRot->Size = System::Drawing::Size(126, 20);
			this->butRot->TabIndex = 0;
			this->butRot->Text = L"right";
			this->butRot->UseVisualStyleBackColor = true;
			this->butRot->Click += gcnew System::EventHandler(this, &skymanager::ImageOpClick);
			// 
			// openImageDialog
			// 
			this->openImageDialog->Filter = L"(*.bmp) Windows Bitmap|*.bmp|(*.jpg) Joint Photographic Experts Group|*.jpg|(*.tg" 
				L"a) Truevison TGA (uncompressed color 24/32bit)|*.tga|(*.*) All files|*.*";
			this->openImageDialog->Title = L"Select an image file ...";
			// 
			// folderBrowserDialog
			// 
			this->folderBrowserDialog->Description = L"Select the game folder where hl.exe is located";
			this->folderBrowserDialog->RootFolder = System::Environment::SpecialFolder::MyComputer;
			// 
			// picPreview
			// 
			this->picPreview->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
				| System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->picPreview->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->picPreview->Location = System::Drawing::Point(3, 1);
			this->picPreview->Name = L"picPreview";
			this->picPreview->Size = System::Drawing::Size(200, 200);
			this->picPreview->TabIndex = 4;
			this->picPreview->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &skymanager::PaintPicPreview);
			this->picPreview->MouseDoubleClick += gcnew System::Windows::Forms::MouseEventHandler(this, &skymanager::picPreview_MouseDoubleClick);
			this->picPreview->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &skymanager::picPreview_MouseClick);
			this->picPreview->ClientSizeChanged += gcnew System::EventHandler(this, &skymanager::picPreview_ClientSizeChanged);
			// 
			// skymanager
			// 
			this->AcceptButton = this->buttonExport;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(492, 373);
			this->Controls->Add(this->picPreview);
			this->Controls->Add(this->buttonExport);
			this->Controls->Add(this->groupImage);
			this->Controls->Add(this->groupSelect);
			this->Controls->Add(this->groupPreview);
			this->MinimumSize = System::Drawing::Size(500, 400);
			this->Name = L"skymanager";
			this->ShowIcon = false;
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Show;
			this->Text = L"HLAE Sky Manager";
			this->groupPreview->ResumeLayout(false);
			this->groupSelect->ResumeLayout(false);
			this->groupSelect->PerformLayout();
			this->groupMirror->ResumeLayout(false);
			this->groupImage->ResumeLayout(false);
			this->groupRotate->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion

private: System::Void PaintPicImage(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		
			 if(this->radioSelSky->Checked)
			 {
				 return;
			 }

			 Image ^ img = this->GetSelectedImage();

			if(!img)
				 return;

			e->Graphics->DrawImage(img,0,0,picImage->ClientSize.Width,picImage->ClientSize.Height);
		 }

private: System::Void Prev_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 bool bE = this->radioPrePer->Checked;
				
			 this->checkPreBack->Enabled = bE;
			 this->checkPreFront->Enabled = bE;
			 this->checkPreLeft->Enabled = bE;
			 this->checkPreRight->Enabled = bE;
			 this->checkPreUp->Enabled = bE;
			 this->checkPreDown->Enabled = bE;

			 this->labelHint->Visible = !bE;

			 picPreview->Refresh();
		 }
private: System::Void Sel_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
			 this->buttonLoad->Enabled = !this->radioSelSky->Checked;
			 picImage->Refresh();
			 picPreview->Refresh();
		 }
private: System::Void ImageOpClick(System::Object^  sender, System::EventArgs^  e) {
			Bitmap ^img;

			 // Determine the operation target:
			if(this->radioSelSky->Checked)
			 {
				 // Operation on complete sky:
				if(sender == this->butMirHorz)				 
				{
					imageFront->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipX);
					imageBack->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipX);

					imageDown->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);
					imageUp->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);


					imageRight->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipX);
					imageLeft->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipX);

					img = imageRight;
					imageRight = imageLeft;
					imageLeft = img;
				}
				else if(sender == this->butMirVert)				 
				{
					imageFront->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);
					imageLeft->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);

					imageBack->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);
					imageRight->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);

					imageDown->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);
					imageUp->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);

					img = imageDown;
					imageDown = imageUp;
					imageUp = img;
				}
				else if(sender == this->butRot)
				{
					img = imageBack;
					imageBack = imageRight;
					imageRight = imageFront;
					imageFront = imageLeft;
					imageLeft = img;

					imageUp->RotateFlip(Drawing::RotateFlipType::Rotate90FlipNone);
					imageDown->RotateFlip(Drawing::RotateFlipType::Rotate270FlipNone);
				}

			 }
			 
			img = this->GetSelectedImage();

			 if(img)
			 {
				 // determine operation:

				 if(sender == this->butMirHorz)
					 img->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipY);
				 else if(sender == this->butMirVert)
					 img->RotateFlip(Drawing::RotateFlipType::RotateNoneFlipX);
				 else if(sender == this->butRot)
					 img->RotateFlip(Drawing::RotateFlipType::Rotate90FlipNone);
			 }

 			 picImage->Refresh();
			 picPreview->Refresh();
		 }
private:
	System::Void buttonLoad_Click(System::Object^  sender, System::EventArgs^  e) {
		LoadImage();
 }

private: System::Void buttonExport_Click(System::Object^  sender, System::EventArgs^  e) {
			try
			{
				String ^ str = m_GamePath;
				folderBrowserDialog->SelectedPath = IO::Path::GetDirectoryName(str);
				if(Windows::Forms::DialogResult::OK == folderBrowserDialog->ShowDialog(this))
				{
					str = folderBrowserDialog->SelectedPath ;

					WriteHlaeImage(imageBack , String::Concat(str,"\\mdtskybk.bmp"));
					WriteHlaeImage(imageDown , String::Concat(str,"\\mdtskydn.bmp"));
					WriteHlaeImage(imageFront, String::Concat(str,"\\mdtskyft.bmp"));
					WriteHlaeImage(imageLeft , String::Concat(str,"\\mdtskylf.bmp"));
					WriteHlaeImage(imageRight, String::Concat(str,"\\mdtskyrt.bmp"));
					WriteHlaeImage(imageUp   , String::Concat(str,"\\mdtskyup.bmp"));
				}
			}
			 catch(Exception ^e)
			 {

				 String ^ strEx = String::Concat("Exporting image files failed:\n", e);

				 MessageBox::Show(
					 strEx,
					 "Error",
					 MessageBoxButtons::OK,
					 MessageBoxIcon::Error
				);
			 };
		 }
private: System::Void picPreview_ClientSizeChanged(System::Object^  sender, System::EventArgs^  e) {
			 picPreview->Refresh();
		 }
private: System::Void picPreview_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);

private: System::Void picPreview_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
};


} // namespace tools {
} // namespace old {
} // namespace AfxCppCli {
