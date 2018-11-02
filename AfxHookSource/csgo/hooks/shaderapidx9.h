#pragma once

struct CShaderAPIDx8_TextureInfo {
	const char * TextureName;
	const char * TextureGroup;
};

CShaderAPIDx8_TextureInfo * CShaderAPIDx8_GetCreateTextureInfo();

bool Hook_CShaderAPIDx8();
