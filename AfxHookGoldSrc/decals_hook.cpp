#include "stdafx.h"

#include <hlsdk.h>

#include "cmdregister.h"
#include "hl_addresses.h"

#include <shared/StringTools.h>

#include <Windows.h>
#include <deps/release/Detours/src/detours.h>

#include <list>
#include <string>

extern cl_enginefuncs_s *pEngfuncs;

//

struct decal_filter_entry_s {
	char * sz_Mask;
	int i_TargetDecalIndex;
};

struct g_decals_hook_s
{
	std::list<decal_filter_entry_s *> filters;
	bool b_debugprint = false;
	bool b_ListActive = false;
} g_decals_hook;

bool IsFilterEntryMatched( decal_filter_entry_s *pentry, const char * sz_Target )
{
	const char * sz_Mask = pentry->sz_Mask;

	return StringWildCard1Matched(sz_Mask, sz_Target);
}

REGISTER_DEBUGCMD_FUNC(test_filtermask)
{
	if(pEngfuncs->Cmd_Argc() != 1+2)
		return;

	decal_filter_entry_s entry;
	entry.sz_Mask = pEngfuncs->Cmd_Argv(1);

	bool bRes = IsFilterEntryMatched(&entry,pEngfuncs->Cmd_Argv(2));

	pEngfuncs->Con_Printf("%s\n", bRes ? "true" : "false" );
}

//
// decal filtering hook:

typedef texture_t * (* Draw_DecalMaterial_t)( int decaltexture );
Draw_DecalMaterial_t detoured_Draw_DecalMaterial = NULL;

// DecalTexture hook:
//   this function is called in a unknown sub function of R_DrawWorld that is
//   called before R_BlendLightmaps. the unknown functions draws out all
//   decals of the map as it seems or s.th. and uses this one to get
//   a decal's texture

texture_t * touring_Draw_DecalMaterial( int decaltexture )
{
	texture_t *tex;
	tex = detoured_Draw_DecalMaterial( decaltexture );

	if( tex )
	{
		const char * texname = tex->name;
		if( g_decals_hook.b_debugprint ) pEngfuncs->Con_Printf("Draw_DecalMaterial( %u ) = 0x%08x, name = %s\n",decaltexture,(DWORD)tex,texname);

		if(  g_decals_hook.b_ListActive )
		{

			std::list<decal_filter_entry_s *>::iterator iterend = g_decals_hook.filters.end();
			for (std::list<decal_filter_entry_s *>::iterator iter = g_decals_hook.filters.begin(); iter != iterend; iter++)
			{
				decal_filter_entry_s *entry = *iter;
				if( IsFilterEntryMatched( entry, texname) )
				{
					// matched, replace the texture( pointer) with the new one:
					tex = detoured_Draw_DecalMaterial( entry->i_TargetDecalIndex );
					break; // first filter in the list always wins
				}
			}

		}

	}
	else if( g_decals_hook.b_debugprint ) pEngfuncs->Con_Printf("Draw_DecalMaterial( %u ) = NULL\n",decaltexture);

	return tex;
}

bool InstallHook_Draw_DecalMaterial()
{
	static bool firstRun = true;
	static bool firstResult = true;
	if (!firstRun) return firstResult;
	firstRun = false;

	if (HL_ADDR_GET(Draw_DecalMaterial))
	{
		LONG error = NO_ERROR;

		detoured_Draw_DecalMaterial = (Draw_DecalMaterial_t)AFXADDR_GET(Draw_DecalMaterial);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)detoured_Draw_DecalMaterial, touring_Draw_DecalMaterial);
		error = DetourTransactionCommit();

		if (NO_ERROR != error)
		{
			firstResult = false;
			ErrorBox("Interception failed:\InstallHook_Draw_DecalMaterial");
		}
	}
	else
		firstResult = false;

	return firstResult;
}

//
// list handling functions

decal_filter_entry_s * GetFilterEntryByMask( std::list<decal_filter_entry_s *> *pfilters, char *  sz_Mask )
{
	decal_filter_entry_s * ret = NULL;

	std::list<decal_filter_entry_s *>::iterator iterend = pfilters->end();
	for (std::list<decal_filter_entry_s *>::iterator iter = pfilters->begin(); iter != iterend; iter++)
	{
		if( !strcmp( (*iter)->sz_Mask, sz_Mask ) )
		{
			ret = *iter;
			break;
		}
	}

	return ret;
}

void RemoveFilterEntry( std::list<decal_filter_entry_s *> *pfilters, decal_filter_entry_s *pentry )
{
	pfilters->remove(pentry);
	free(pentry->sz_Mask);
	free(pentry);

	g_decals_hook.b_ListActive = !(pfilters->empty());
}

void CreateFilterEntry( std::list<decal_filter_entry_s *> *pfilters, const char * sz_Mask, const int i_TargetDecalIndex )
{
	decal_filter_entry_s *entry = (decal_filter_entry_s *)malloc(sizeof(decal_filter_entry_s));

	size_t bufLen = strlen(sz_Mask)+1;
	entry->sz_Mask = (char *)malloc(sizeof(char)*bufLen);
	strcpy_s(entry->sz_Mask, bufLen, sz_Mask);
	entry->i_TargetDecalIndex = i_TargetDecalIndex;

	pfilters->push_back(entry);

	g_decals_hook.b_ListActive = true;
}

//
// advanced decalfilter functions for the user

void DecalFilter_Add(char * sz_Mask, char * sz_Decalname)
{
	if(!pEngfuncs->GetEntityByIndex(0))
	{
		pEngfuncs->Con_Printf("Error: Game must be running in order for this feature to work.\n");
		return;
	}

	// get's the decal index and also causes the game to cache the texture:
	// (if sz_Decalname is not a valid decalname, the game also supplies a default decal)
	int i_decalindex = pEngfuncs->pEfxAPI->Draw_DecalIndex(
			pEngfuncs->pEfxAPI->Draw_DecalIndexFromName( sz_Decalname )
	);

	// check if there is already an entry:
	decal_filter_entry_s *entry = GetFilterEntryByMask( &(g_decals_hook.filters), sz_Mask );
	if( entry )
	{
		// replace old entry:
		entry->i_TargetDecalIndex = i_decalindex;
	} else {
		// create new entry:
		CreateFilterEntry( &(g_decals_hook.filters), sz_Mask, i_decalindex );
	}
}

void DecalFilter_Remove(char * sz_Mask)
{
	decal_filter_entry_s *entry = GetFilterEntryByMask( &(g_decals_hook.filters), sz_Mask );
	if( entry )
	{
		RemoveFilterEntry(&(g_decals_hook.filters), entry);
	} else pEngfuncs->Con_Printf("Mask entry not in list.\n");
}

void DecalFilter_Debug(bool bDebug)
{
	g_decals_hook.b_debugprint = bDebug;
}

void DecalFilter_List()
{
	std::list<decal_filter_entry_s *>::iterator iterend = g_decals_hook.filters.end();
	for (std::list<decal_filter_entry_s *>::iterator iter = g_decals_hook.filters.begin(); iter != iterend; iter++)
	{
		decal_filter_entry_s *entry = *iter;
		pEngfuncs->Con_Printf("%s -> %u\n",entry->sz_Mask,entry->i_TargetDecalIndex);
	}
	pEngfuncs->Con_Printf("-- end of list --\n");
}

void DecalFilter_Clear()
{
	while( ! g_decals_hook.filters.empty() )
		RemoveFilterEntry( &(g_decals_hook.filters), g_decals_hook.filters.front() );
}

void DecalFilter_PrintHelp()
{
	pEngfuncs->Con_Printf(
		"" PREFIX "decalfilter <command>\n"
		"where <command> is one of these:\n"
		"\t" "add <mask> <decalname> - forces all decaltextures to use the texture of decal <decalname> from decals.wad instead\n"
		"\t" "remove <mask> - removes a filtering mask again\n"
		"\t" "list - lists maks currently active\n"
		"\t" "debug 0|1 - prints incoming decal texture requests from the game to the console\n"
		"\t" "clearall - removes all filters\n"
		"<mask> - string to match, where \\* = wildcard and \\\\ = \\\n"
		"The first filter in the list always wins. For more information  HLAE online manual.\n"
	);
}

REGISTER_CMD_FUNC(decalfilter)
{
	if(!InstallHook_Draw_DecalMaterial())
	{
		pEngfuncs->Con_Printf("Error: Failed to install hook.\n");
		return;
	}

	bool bShowHelp = true;

	int argc = pEngfuncs->Cmd_Argc();

	if( argc >= 1+1 )
	{
		char * command = pEngfuncs->Cmd_Argv(1);
		if(!_stricmp(command,"clearall"))
		{
			DecalFilter_Clear();
			bShowHelp = false;
		}
		else if(!_stricmp(command,"list"))
		{
			DecalFilter_List();
			bShowHelp = false;
		}
		else if(!_stricmp(command,"debug") && argc == 1+2)
		{
			DecalFilter_Debug(0 != atoi(pEngfuncs->Cmd_Argv(2)));
			bShowHelp = false;
		}
		else if(!_stricmp(command,"remove") && argc == 1+2)
		{
			DecalFilter_Remove( pEngfuncs->Cmd_Argv(2) );
			bShowHelp = false;
		}
		else if(!_stricmp(command,"add") && argc == 1+3)
		{
			DecalFilter_Add(  pEngfuncs->Cmd_Argv(2),  pEngfuncs->Cmd_Argv(3) );
			bShowHelp = false;
		}

	}

	if( bShowHelp ) DecalFilter_PrintHelp();
}


//
// simplified noadverts command for the user

REGISTER_DEBUGCVAR(noadverts_newdecal, "{littleman", 0);
REGISTER_DEBUGCVAR(noadverts_mask, "{^IGA_\\*", 0);

REGISTER_CMD_FUNC(noadverts)
{
	if(!InstallHook_Draw_DecalMaterial())
	{
		pEngfuncs->Con_Printf("Error: Failed to install hook.\n");
		return;
	}

	if( pEngfuncs->Cmd_Argc() != 2)
	{
		pEngfuncs->Con_Printf("" PREFIX "noadverts 0|1 - disable / enable removal\n\n");
		return;
	}

	bool bNoAds = 0 != atoi(pEngfuncs->Cmd_Argv(1));

	decal_filter_entry_s *entry = GetFilterEntryByMask( &(g_decals_hook.filters), noadverts_mask->string );
	if( entry )
	{
		RemoveFilterEntry(&(g_decals_hook.filters), entry);
	}
	if( bNoAds )
	{
		if(!pEngfuncs->GetEntityByIndex(0))
		{
			pEngfuncs->Con_Printf("Error: Game must be running in order for this feature to work.\n");
			return;
		}

		CreateFilterEntry(
			&(g_decals_hook.filters),
			noadverts_mask->string,
			pEngfuncs->pEfxAPI->Draw_DecalIndex(
				pEngfuncs->pEfxAPI->Draw_DecalIndexFromName( noadverts_newdecal->string )
			)
		);
	}
}