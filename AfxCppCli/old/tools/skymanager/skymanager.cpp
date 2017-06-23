#include "stdafx.h"

#include "skymanager.h"


using namespace System::IO;

using namespace AfxCppCli::old::tools;

bool skymanager::PointInRect(Drawing::Point % point, int invSx, int invSy, CubeSide % cs) {
	PointF csp;

	csp.X = invSx ? point.X / float(invSx) : 0;
	csp.Y = invSy ? point.Y / float(invSy) : 0;

	return
		cs.tpLeft.X <= csp.X
		&& cs.tpLeft.Y <= csp.Y
		&& csp.X <= cs.tpRight.X
		&& csp.Y <= cs.btRight.Y
	;
}


void skymanager::CoordToViewPoint(PointF % coordPoint, int sx, int sy, Point % viewPoint) {
	viewPoint.X = (int)(coordPoint.X *sx);
	viewPoint.Y = (int)(coordPoint.Y *sy);
}


void skymanager::GetCubeCoordsFlat(CubeSides % outCoords) {
	outCoords.back.tpLeft.X  = 0;
	outCoords.back.tpLeft.Y  = 1.0f/3;
	outCoords.back.tpRight.X = 1.0f/4;
	outCoords.back.tpRight.Y = 1.0f/3;
	outCoords.back.btRight.X = 1.0f/4;
	outCoords.back.btRight.Y = 2.0f/3;
	outCoords.back.btLeft.X  = 0;
	outCoords.back.btLeft.Y  = 2.0f/3;	

	outCoords.right.tpLeft.X  = 1.0f/4;
	outCoords.right.tpLeft.Y  = 1.0f/3;
	outCoords.right.tpRight.X = 2.0f/4;
	outCoords.right.tpRight.Y = 1.0f/3;
	outCoords.right.btRight.X = 2.0f/4;
	outCoords.right.btRight.Y = 2.0f/3;
	outCoords.right.btLeft.X  = 1.0f/4;
	outCoords.right.btLeft.Y  = 2.0f/3;	

	outCoords.front.tpLeft.X  = 2.0f/4;
	outCoords.front.tpLeft.Y  = 1.0f/3;
	outCoords.front.tpRight.X = 3.0f/4;
	outCoords.front.tpRight.Y = 1.0f/3;
	outCoords.front.btRight.X = 3.0f/4;
	outCoords.front.btRight.Y = 2.0f/3;
	outCoords.front.btLeft.X  = 2.0f/4;
	outCoords.front.btLeft.Y  = 2.0f/3;	

	outCoords.left.tpLeft.X  = 3.0f/4;
	outCoords.left.tpLeft.Y  = 1.0f/3;
	outCoords.left.tpRight.X = 1;
	outCoords.left.tpRight.Y = 1.0f/3;
	outCoords.left.btRight.X = 1;
	outCoords.left.btRight.Y = 2.0f/3;
	outCoords.left.btLeft.X  = 3.0f/4;
	outCoords.left.btLeft.Y  = 2.0f/3;	

	outCoords.up.tpLeft.X  = 1.0f/4;
	outCoords.up.tpLeft.Y  = 0;
	outCoords.up.tpRight.X = 2.0f/4;
	outCoords.up.tpRight.Y = 0;
	outCoords.up.btRight.X = 2.0f/4;
	outCoords.up.btRight.Y = 1.0f/3;
	outCoords.up.btLeft.X  = 1.0f/4;
	outCoords.up.btLeft.Y  = 1.0f/3;	

	outCoords.down.tpLeft.X  = 1.0f/4;
	outCoords.down.tpLeft.Y  = 2.0f/3;
	outCoords.down.tpRight.X = 2.0f/4;
	outCoords.down.tpRight.Y = 2.0f/3;
	outCoords.down.btRight.X = 2.0f/4;
	outCoords.down.btRight.Y = 1;
	outCoords.down.btLeft.X  = 1.0f/4;
	outCoords.down.btLeft.Y  = 1;	
}


System::Drawing::Bitmap ^ skymanager::LoadTaragaFile(System::String ^ filePath)
{
	Bitmap ^ bmp = nullptr;
	BinaryReader ^ br = gcnew BinaryReader(gcnew FileStream(filePath, FileMode::Open));

	try {
		br->ReadBytes(2);
		unsigned char colorMapType = br->ReadByte();
		br->ReadBytes(9);
		int width  = (int)br->ReadUInt16();
		int height = (int)br->ReadUInt16();
		unsigned char bpp = br->ReadByte();
		br->ReadBytes(1);

		if(2 == colorMapType && 24 == bpp || 32 == bpp)
		{
			bmp = gcnew Bitmap(width, height, Imaging::PixelFormat::Format24bppRgb);

			for(int ih=0; ih<height; ih++)
				for(int iw=0; iw<width; iw++) {
					int color = (br->ReadByte()) +(br->ReadByte()<<8) +(br->ReadByte()<<16);
					
					if(32 == bpp) color = color +(br->ReadByte() << 24);

					bmp->SetPixel(iw, height-ih-1, Color::FromArgb(color));					
				}
		}
	}
	catch(...) {
		bmp = nullptr;
	}
	finally  {
		br->Close();
	}

	return bmp;
}

void skymanager::LoadImage() {
	 if(Windows::Forms::DialogResult::OK == openImageDialog->ShowDialog(this) )
	 {
		 Image ^img = nullptr;
		 try
		 {
			 System::String ^ fileName = openImageDialog->FileName;

			if(fileName->EndsWith(".tga"))
				img = LoadTaragaFile(fileName);

			 if(!img)
				 img = Image::FromFile(openImageDialog->FileName);
		 }
		 catch(...)
		 {
			 img = nullptr;
			 MessageBox::Show(
				 "Loading image file failed.",
				 "Error",
				 MessageBoxButtons::OK,
				 MessageBoxIcon::Error
			);
		 };

		 if(img)
		 {
			int iw = img->Size.Width;
			int ih = img->Size.Height;
			
			Bitmap ^bmp = gcnew Bitmap(iw, ih, Imaging::PixelFormat::Format24bppRgb);
			Graphics ^gfx = Graphics::FromImage(bmp);
			gfx->DrawImage(img, 0, 0, iw, ih);

			// make sure the file is unlocked:
			delete img;					

			// Down:
			if(this->radioSelDown->Checked)
			{
				imageDown = bmp;
			}
			// Left:
			else if(this->radioSelLeft->Checked)
			{
				imageLeft = bmp;
			}
			// Front:
			else if(this->radioSelFront->Checked)
			{
				imageFront = bmp;
			}
			// Back:
			else if(this->radioSelBack->Checked)
			{
				imageBack = bmp;
			}
			// Right:
			else if(this->radioSelRight->Checked)
			{
				imageRight = bmp;
			}
			// Up:
			else if(this->radioSelUp->Checked)
			{
				imageUp = bmp;
			}

			 picImage->Refresh();
			 picPreview->Refresh();

		 }
	 }
}


System::Void skymanager::picPreview_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	SkySelection(e->Location);
}


System::Void skymanager::picPreview_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
	SkySelection(e->Location);
	if(this->buttonLoad->Enabled)
		LoadImage();
}



void skymanager::PaintPicPreview(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
	int w = picPreview->ClientSize.Width - 1;
	int h = picPreview->ClientSize.Height - 1;

	Point ptBkLeft, ptBkRight,ptBkDown;
	Point ptRtLeft, ptRtRight,ptRtDown;
	Point ptFtLeft, ptFtRight,ptFtDown;
	Point ptLfLeft, ptLfRight,ptLfDown;
	Point ptUpLeft, ptUpRight,ptUpDown;
	Point ptDnLeft, ptDnRight,ptDnDown;

	bool bDAll = false;

	if(this->radioPrePer->Checked)
	{
		Point ptBoxUpLeft   = Point(0         , h >> 2);
		Point ptBoxUpTop    = Point(w >> 1    , 0);
		Point ptBoxUpRight  = Point(w         , h >> 2);
		Point ptBoxUpBottom = Point(w >> 1    , (h>>1));

		Point ptBoxDnLeft   = Point(0         , (h*3) >> 2);
		Point ptBoxDnTop    = Point(w >> 1    , (h>>1));
		Point ptBoxDnRight  = Point(w         , (h*3) >> 2);
		Point ptBoxDnBottom = Point(w >> 1    , h);

		ptBkLeft  = ptBoxUpLeft;
		ptBkRight = ptBoxUpTop;
		ptBkDown  = ptBoxDnLeft;

		ptRtLeft  = ptBoxUpTop;
		ptRtRight = ptBoxUpRight;
		ptRtDown  = ptBoxDnTop;

		ptFtLeft  = ptBoxUpRight;
		ptFtRight = ptBoxUpBottom;
		ptFtDown  = ptBoxDnRight;

		ptLfLeft  = ptBoxUpBottom;
		ptLfRight = ptBoxUpLeft;
		ptLfDown  = ptBoxDnBottom;

		ptUpLeft  = ptBoxUpLeft;
		ptUpRight = ptBoxUpBottom;
		ptUpDown  = ptBoxUpTop;

		ptDnLeft  = ptBoxDnTop;
		ptDnRight = ptBoxDnRight;
		ptDnDown  = ptBoxDnLeft;

	} else {
		CubeSides cc;
		GetCubeCoordsFlat(cc);

		bDAll = true;

		CoordToViewPoint(cc.back.tpLeft, w, h, ptBkLeft);
		CoordToViewPoint(cc.back.tpRight, w, h, ptBkRight);
		CoordToViewPoint(cc.back.btLeft, w, h, ptBkDown);

		CoordToViewPoint(cc.right.tpLeft, w, h, ptRtLeft);
		CoordToViewPoint(cc.right.tpRight, w, h, ptRtRight);
		CoordToViewPoint(cc.right.btLeft, w, h, ptRtDown);

		CoordToViewPoint(cc.front.tpLeft, w, h, ptFtLeft);
		CoordToViewPoint(cc.front.tpRight, w, h, ptFtRight);
		CoordToViewPoint(cc.front.btLeft, w, h, ptFtDown);

		CoordToViewPoint(cc.left.tpLeft, w, h, ptLfLeft);
		CoordToViewPoint(cc.left.tpRight, w, h, ptLfRight);
		CoordToViewPoint(cc.left.btLeft, w, h, ptLfDown);

		CoordToViewPoint(cc.up.tpLeft, w, h, ptUpLeft);
		CoordToViewPoint(cc.up.tpRight, w, h, ptUpRight);
		CoordToViewPoint(cc.up.btLeft, w, h, ptUpDown);

		CoordToViewPoint(cc.down.tpLeft, w, h, ptDnLeft);
		CoordToViewPoint(cc.down.tpRight, w, h, ptDnRight);
		CoordToViewPoint(cc.down.btLeft, w, h, ptDnDown);
	}

	Drawing::Pen ^spen = gcnew Drawing::Pen(System::Drawing::Color::Red);

	// Draw Images:

	// Draw Down:
	if(bDAll || this->checkPreDown->Checked)
	{
	array<Point>^ pta = {ptDnLeft, ptDnRight, ptDnDown };
	e->Graphics->DrawImage(imageDown, pta);
	}
	// Draw Back:
	if(bDAll || this->checkPreBack->Checked)
	{
	array<Point>^ pta = {ptBkLeft, ptBkRight, ptBkDown };
	e->Graphics->DrawImage(imageBack, pta);
	}
	// Draw Right:
	if(bDAll || this->checkPreRight->Checked)
	{
	array<Point>^ pta = {ptRtLeft, ptRtRight, ptRtDown };
	e->Graphics->DrawImage(imageRight, pta);
	}
	// Draw Left:
	if(bDAll || this->checkPreLeft->Checked)
	{
	array<Point>^ pta = {ptLfLeft, ptLfRight, ptLfDown };
	e->Graphics->DrawImage(imageLeft, pta);
	}
	// Draw Front:
	if(bDAll || this->checkPreFront->Checked)
	{
	array<Point>^ pta = {ptFtLeft, ptFtRight, ptFtDown };
	e->Graphics->DrawImage(imageFront, pta);
	}
	// Draw Up:
	if(bDAll || this->checkPreUp->Checked)
	{
	array<Point>^ pta = {ptUpLeft, ptUpRight, ptUpDown };
	e->Graphics->DrawImage(imageUp, pta);
	}

	// Draw Selection:

	bool bSelAll = this->radioSelSky->Checked;

	// Draw Down:
	if(bSelAll || this->radioSelDown->Checked)
	{
	Point ptT = Point(ptDnRight.X +ptDnDown.X - ptDnLeft.X, ptDnRight.Y +ptDnDown.Y - ptDnLeft.Y);
	array<Point>^ pta = {ptDnLeft, ptDnRight, ptT, ptDnDown };
	e->Graphics->DrawPolygon(spen, pta);
	}
	// Draw Left:
	if(bSelAll || this->radioSelLeft->Checked)
	{
	Point ptT = Point(ptLfRight.X +ptLfDown.X - ptLfLeft.X, ptLfRight.Y +ptLfDown.Y - ptLfLeft.Y);
	array<Point>^ pta = {ptLfLeft, ptLfRight, ptT, ptLfDown };
	e->Graphics->DrawPolygon(spen, pta);
	}
	// Draw Front:
	if(bSelAll || this->radioSelFront->Checked)
	{
	Point ptT = Point(ptFtRight.X +ptFtDown.X - ptFtLeft.X, ptFtRight.Y +ptFtDown.Y - ptFtLeft.Y);
	array<Point>^ pta = {ptFtLeft, ptFtRight, ptT, ptFtDown };
	e->Graphics->DrawPolygon(spen, pta);
	}
	// Draw Back:
	if(bSelAll || this->radioSelBack->Checked)
	{
	Point ptT = Point(ptBkRight.X +ptBkDown.X - ptBkLeft.X, ptBkRight.Y +ptBkDown.Y - ptBkLeft.Y);
	array<Point>^ pta = {ptBkLeft, ptBkRight, ptT, ptBkDown };
	e->Graphics->DrawPolygon(spen, pta);
	}
	// Draw Right:
	if(bSelAll || this->radioSelRight->Checked)
	{
	Point ptT = Point(ptRtRight.X +ptRtDown.X - ptRtLeft.X, ptRtRight.Y +ptRtDown.Y - ptRtLeft.Y);
	array<Point>^ pta = {ptRtLeft, ptRtRight, ptT, ptRtDown };
	e->Graphics->DrawPolygon(spen, pta);
	}
	// Draw Up:
	if(bSelAll || this->radioSelUp->Checked)
	{
	Point ptT = Point(ptUpRight.X +ptUpDown.X - ptUpLeft.X, ptUpRight.Y +ptUpDown.Y - ptUpLeft.Y);
	array<Point>^ pta = {ptUpLeft, ptUpRight, ptT, ptUpDown };
	e->Graphics->DrawPolygon(spen, pta);
	};

}

void skymanager::SkySelection(Point point) {

	int w = picPreview->ClientSize.Width - 1;
	int h = picPreview->ClientSize.Height - 1;
	Point m = point;

	if(this->radioPreFlat->Checked) {
		CubeSides cs;
		GetCubeCoordsFlat(cs);

		if(PointInRect(m, w, h, cs.back)) this->radioSelBack->Checked = true;
		else if(PointInRect(m, w, h, cs.right)) this->radioSelRight->Checked = true;
		else if(PointInRect(m, w, h, cs.front)) this->radioSelFront->Checked = true;
		else if(PointInRect(m, w, h, cs.left)) this->radioSelLeft->Checked = true;
		else if(PointInRect(m, w, h, cs.up)) this->radioSelUp->Checked = true;
		else if(PointInRect(m, w, h, cs.down)) this->radioSelDown->Checked = true;
		else this->radioSelSky->Checked = true;
	}
}

