#include "stdafx.h"

#include "bvhimport.h"

#include <stdio.h>
#include <windows.h>


/// <remarks> If pz is 0, the function returns </remarks>
char * CrLfZ2LfZ(char * opz) {
	char *pz = opz;

	if(pz) {
		while(char c = *pz) {
			if(
				'\r' == c
				&& '\n' == *(pz+1)
				&& '\0' == *(pz+2)
			) {
				// \r\n\0
				*pz = '\n';
				*(pz+1) = '\0';
				break;
			}

			pz++;
		}
	}

	return opz;
}

inline int myround(double x)
{
	return x < 0 ? (int)(x -0.5) : (int)(x +0.5);
}


// BvhImport //////////////////////////////////////////////////////////////////

BvhImport::BvhImport()
{
	m_Active = false;
}

void BvhImport::CloseMotionFile()
{
	if(m_Active)
	{
		m_Active = false;
		fclose(m_File);
	}
}

int BvhImport::DecodeBvhChannel(char * pszRemainder, char * & aoutNewRemainder)
{
	aoutNewRemainder = pszRemainder;
	if(!pszRemainder)
		return -1;

	int iret = -1;

	while(0 != *pszRemainder && (' ' == *pszRemainder || '\t' == *pszRemainder) )
	{
		pszRemainder++;
	}

	if(pszRemainder == strstr(pszRemainder,"Xposition")) iret = BC_Xposition;
	if(pszRemainder == strstr(pszRemainder,"Yposition")) iret = BC_Yposition;
	if(pszRemainder == strstr(pszRemainder,"Zposition")) iret = BC_Zposition;
	if(pszRemainder == strstr(pszRemainder,"Zrotation")) iret = BC_Zrotation;
	if(pszRemainder == strstr(pszRemainder,"Xrotation")) iret = BC_Xrotation;
	if(pszRemainder == strstr(pszRemainder,"Yrotation")) iret = BC_Yrotation;

	if(0 <= iret)
		aoutNewRemainder = pszRemainder + 9;

	return iret;
}

bool BvhImport::CopyToCampath(double timeOfs, double fov, CamPath & camPath)
{
	camPath.Clear();

	if(!m_Active)
		return true;

	// we start at the first frame
	if(fseek(m_File,m_MotionFPos,SEEK_SET))
	{
		// read error
		CloseMotionFile();
		return false;
	}

	for(m_LastFrame = 0; m_LastFrame < m_Frames; ++m_LastFrame)
	{
		char * pc;

		// read current frame:
		pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
		if(!pc)
		{
			// read error
			CloseMotionFile();
			return false;
		}

		// decode frame:
		int ichan;
		char tc = 0;
		char * pc2;
		double fff;

		pc = ms_readbuff;
		for(ichan = 0; ichan < 6; ichan++)
		{
			pc2 = strchr(pc,' ');
			if(!pc2)
				pc2 = strchr(pc,'\t');
			if(!pc2)
				pc2 = pc + strlen(pc);

			tc = *pc2;
			*pc2 = 0;

			fff = 0;
			fff = atof(pc);

			m_Cache[channelcode[ichan]] = fff;

			*pc2 = tc;
			pc = pc2;

			while(0 != *pc && (' ' == *pc || '\t' == *pc)) pc++;
		}

		double Ty = (-m_Cache[0]);
		double Tz = (+m_Cache[1]);
		double Tx = (-m_Cache[2]);
		double Rz = (-m_Cache[3]);
		double Rx = (-m_Cache[4]);
		double Ry = (+m_Cache[5]);

		camPath.Add(timeOfs +m_LastFrame * m_FrameTime, CamPathValue(Tx, Ty, Tz, Rx, Ry, Rz, fov));
	}

	return true;
}

bool BvhImport::GetCamPosition(double fTimeOfs, double outCamdata[6])
{
	char * pc;

	if(!m_Active || !outCamdata)
		return false; // not active

	// calc targetframe:
	int iCurFrame = myround(fTimeOfs / m_FrameTime);
	if(iCurFrame < 0 || iCurFrame >= m_Frames)
		return false; // out of range

	if(	iCurFrame == m_LastFrame)
	{
		// pEngfuncs->Con_DPrintf("Using cached cam motion frame: %i (%f)\n",iCurFrame,fTimeOfs);
		memcpy(outCamdata,m_Cache,sizeof(m_Cache));
		return true;
	}

	if(0 == iCurFrame)
	{
		// we start at the first frame
		if(fseek(m_File,m_MotionFPos,SEEK_SET))
		{
			// read error
			CloseMotionFile();
			return false;
		}
	} else {
		// seek to frame position:
		pc = 0;
		
		int iSeekFrames;
		if(0 <= m_LastFrame)
			iSeekFrames = iCurFrame - m_LastFrame -1;
		else
			iSeekFrames = iCurFrame;
		
		while(iSeekFrames)
		{
			if(0<iSeekFrames)
			{
				// seek forward
				pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
				if(!pc)
				{
					// read error
					CloseMotionFile();
					return false;
				}
				iSeekFrames--;
			}
			else
			{
				// seek backward
				char ac=0;
				bool bReadErr = false;
				int iReadBreaks = 2;

				while(!bReadErr && 0<iReadBreaks)
				{
					if(!bReadErr)
						bReadErr = 0 != fseek(m_File,-1,SEEK_CUR);

					if(!bReadErr)
						bReadErr = 1 != fread(&ac,sizeof(char),1,m_File);

					bReadErr = bReadErr || ftell(m_File)<m_MotionFPos;

					if('\n' == ac)
						iReadBreaks--;

					if(!bReadErr)
						bReadErr = 0 != fseek(m_File,-1,SEEK_CUR);
				}

				bReadErr = bReadErr || 0 != fseek(m_File,+1,SEEK_CUR);

				if(bReadErr)
				{
					// read error
					CloseMotionFile();
					return false;
				}

				iSeekFrames++;
			}
		}
	}

	// read current frame:
	pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
	if(!pc)
	{
		// read error
		CloseMotionFile();
		return false;
	}

	m_LastFrame = iCurFrame;

	// decode frame:
	int ichan;
	char tc = 0;
	char * pc2;
	double fff;

	pc = ms_readbuff;
	for(ichan = 0; ichan < 6; ichan++)
	{
		pc2 = strchr(pc,' ');
		if(!pc2)
			pc2 = strchr(pc,'\t');
		if(!pc2)
			pc2 = pc + strlen(pc);

		tc = *pc2;
		*pc2 = 0;

		fff = 0;
		fff = atof(pc);

		m_Cache[channelcode[ichan]] = fff;

		*pc2 = tc;
		pc = pc2;

		while(0 != *pc && (' ' == *pc || '\t' == *pc)) pc++;
	}

	// pEngfuncs->Con_DPrintf("Imported cam motion frame: %i (%f)\n",iCurFrame,fTimeOfs);
	memcpy(outCamdata,m_Cache,sizeof(m_Cache));
	return true;
}


bool BvhImport::IsActive()
{
	return m_Active;
}

bool BvhImport::LoadMotionFile(wchar_t const * fileName)
{
	char * pc;
	char * pc2;

	if(m_Active)
		CloseMotionFile();

	_wfopen_s(&m_File, fileName, L"rb");

	if(!m_File)
		return false;

	// check if this could be a valid BVH file:
	pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
	if(!pc || strcmp(ms_readbuff,"HIERARCHY\n"))
	{
		fclose(m_File);
		return false;
	}

	// skip till first channels entry:
	pc2 = 0;
	while(!pc2)
	{
		pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
		if(!pc)
		{
			fclose(m_File);
			return false;
		}

		pc2 = strstr(ms_readbuff,"CHANNELS 6 ");
	}

	// determine channel assignment:
	pc2 += strlen("CHANNELS 6 ");

	for(int i=0; i<6; i++) channelcode[i]=-1;

	for(int i=0; i <6; i++)
	{
		int icode = DecodeBvhChannel(pc2,pc2);

		if(0 <= icode)
			channelcode[i]=icode;
	}

	for(int i=0; i<6; i++)
	{
		if(channelcode[i]<0)
		{
			fclose(m_File);
			return false;
		}
	}

	// skip till MOTION entry:
	pc2 = 0;
	while(!pc2)
	{
		pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
		if(!pc)
		{
			fclose(m_File);
			return false;
		}

		pc2 = strstr(ms_readbuff,"MOTION\n");
	}

	// read frames:
	pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
	if(!pc || strcmp(ms_readbuff,"Frames:") <= 0)
	{
		fclose(m_File);
		return false;
	}
	pc += strlen("Frames:");
	m_Frames = atoi(pc);

	// read frame time:
	pc = CrLfZ2LfZ(fgets(ms_readbuff,sizeof(ms_readbuff)/sizeof(char),m_File));
	if(!pc || strcmp(ms_readbuff,"Frame Time:") <= 0)
	{
		fclose(m_File);
		return false;
	}
	pc += strlen("Frame Time:");
	m_FrameTime = atof(pc);
	if(m_FrameTime <= 0)
	{
		fclose(m_File);
		return false;
	}

	m_MotionFPos = ftell(m_File);
	m_LastFrame = -1;
	m_Active = true;

	return true;
}


char BvhImport::ms_readbuff[1024];

BvhImport::~BvhImport()
{
	CloseMotionFile();
}


