#include "stdafx.h"

#include "AfxShaders.h"

#include "hlaeFolder.h"
//#include "SourceInterfaces.h"

#include <string>

bool GetShaderDirectory(std::wstring & outShaderDirectory)
{
	outShaderDirectory.assign(GetHlaeFolderW());

	outShaderDirectory.append(L"resources\\shaders\\");

	return true;
}

void * LoadShaderFileInMemory(const wchar_t * fileName, size_t & outSize)
{
	outSize = 0;

	if(!fileName)
		return 0;

	std::wstring shaderDir;

	if(!GetShaderDirectory(shaderDir))
		return 0;

	shaderDir.append(fileName);

	FILE * file = 0;
	bool bOk = 0 == _wfopen_s(&file, shaderDir.c_str(), L"rb");
	void * so = 0;
	size_t size = 0;

	bOk = bOk 
		&& 0 != file
		&& 0 == fseek(file, 0, SEEK_END)
	;

	if(bOk)
	{
		size = ftell(file);

		so = (void *)malloc( size );
		bOk = 0 != so
			&& 0 == fseek(file, 0, SEEK_SET);
	}

	if(bOk)
	{
		bOk = size == fread(so, 1, size, file);
	}

	if(file) fclose(file);

	if(bOk) {
		outSize = size; 
		return so;
	}
	
	if(so) free(so);
	
	return 0;
}

int FindAcsIndex(FILE * file, size_t indexOfs, int lowIndex, int hiIndex, int combo)
{
	if(hiIndex < lowIndex)
		return -1;

	int testIndex = (lowIndex +hiIndex) >> 1;

	if(0 != fseek(file, indexOfs + testIndex * (sizeof(int) + sizeof(int)), SEEK_SET))
		return -1;

	int value;

	if(1 != fread(&value,sizeof(value),1,file))
		return -1;

	if(value == combo)
	{
		return testIndex;
	}

	return combo < value
		? FindAcsIndex(file, indexOfs, lowIndex, testIndex-1, combo)
		:  FindAcsIndex(file, indexOfs, testIndex+1, hiIndex, combo)
	;
}

/// <remarks>Assumes that the file is right after the position of the combo (i.e. use after FindAcsIndex).</remarks>
bool GetAcsComboOffsetAndSize(FILE * file, int index, int indexSize, size_t & outComboOffset, size_t & outComboSize)
{
	int comboOfs;

	if(1 != fread(&comboOfs,sizeof(comboOfs),1,file))
		return false;

	if(index+1 < indexSize)
	{
		if(0 != fseek(file, sizeof(int), SEEK_CUR))
			return false;

		int nextComboOfs;

		if(1 != fread(&nextComboOfs,sizeof(nextComboOfs),1,file))
			return false;

		outComboOffset = comboOfs;
		outComboSize = nextComboOfs - comboOfs;

		return true;
	}

	// Is last combo, so filesize determines the size:
	if(0 != fseek(file, 0, SEEK_END))
		return false;

	size_t nextComboOfs = ftell(file);

	outComboOffset = comboOfs;
	outComboSize = nextComboOfs - comboOfs;

	return true;
}

void * LoadFromAcsShaderFileInMemory(const wchar_t * fileName, int combo, size_t & outSize)
{
	outSize = 0;

	if(!fileName)
		return 0;

	std::wstring shaderDir;

	if(!GetShaderDirectory(shaderDir))
		return 0;

	shaderDir.append(fileName);

	FILE * file = 0;
	bool bOk = 0 == _wfopen_s(&file, shaderDir.c_str(), L"rb");
	void * so = 0;
	size_t size = 0;
	int version;
	int indexSize;
	size_t indexOfs;
	int index;
	size_t comboSize;
	size_t comboOfs;

	bOk = bOk 
		&& 0 != file
		&& 1 == fread(&version,sizeof(version),1,file)
		&& 0 == version
		&& 1 == fread(&indexSize,sizeof(indexSize),1,file)
		&& (indexOfs = ftell(file), true)
		&& -1 != (index = FindAcsIndex(file,indexOfs,0,indexSize-1,combo))
		&& GetAcsComboOffsetAndSize(file, index, indexSize, comboOfs, comboSize)
	;

	if(bOk)
	{
		//Tier0_Msg("Combo %i is at 0x%08x and has size 0x%08x!\n",combo,comboOfs,comboSize);

		so = (void *)malloc( comboSize );
		bOk = 0 != so
			&& 0 == fseek(file, comboOfs, SEEK_SET)
			&& comboSize == fread(so, 1, comboSize, file)
		;
	}

	if(file) fclose(file);

	if(bOk) {
		outSize = comboSize;
		return so;
	}
	
	if(so) free(so);
	
	return 0;
}
