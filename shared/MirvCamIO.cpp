#include "stdafx.h"

#include "MirvCamIO.h"

#include "StringTools.h"

void MirvCamIO_ConsoleCommand(advancedfx::ICommandArgs * args, CamImport * & refPCamImport, CamExport * & refPCamExport, MirvCamIO_GetTimeFn_t fnGetTime) {
	int argc = args->ArgC();

	char const * cmd0 = args->ArgV(0);

	if (2 <= argc)
	{
		char const * cmd1 = args->ArgV(1);

		if (0 == _stricmp("export", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (0 == _stricmp("start", cmd2) && 4 <= argc)
				{
					if (0 != refPCamExport)
					{
						delete refPCamExport;
						refPCamExport = 0;
					}

					std::wstring fileName(L"");

					if (UTF8StringToWideString(args->ArgV(3), fileName))
					{
						refPCamExport = new CamExport(fileName.c_str());
					}
					else
						advancedfx::Warning("Error: Can not convert \"%s\" from UTF-8 to WideString.\n", args->ArgV(3));


					return;
				}
				else if (0 == _stricmp("end", cmd2))
				{
					if (0 != refPCamExport)
					{
						delete refPCamExport;
						refPCamExport = 0;
					}
					else
						advancedfx::Warning("No cam export was active.\n");

					return;
				}
			}

			advancedfx::Message(
				"%s export start <fileName> - Starts exporting to file <fileName>.\n"
				"%s export end - Stops exporting.\n"
				, cmd0
				, cmd0
			);
			return;
		}
		else if (0 == _stricmp("import", cmd1))
		{
			if (3 <= argc)
			{
				char const * cmd2 = args->ArgV(2);

				if (0 == _stricmp("start", cmd2) && 4 <= argc)
				{
					if (0 != refPCamImport)
					{
						delete refPCamImport;
						refPCamImport = 0;
					}

					refPCamImport = new CamImport(args->ArgV(3), fnGetTime());
					if (refPCamImport->IsBad()) advancedfx::Warning("Error importing CAM file \"%s\"\n", args->ArgV(3));
					return;
				}
				else if (0 == _stricmp("end", cmd2))
				{
					delete refPCamImport;
					refPCamImport = 0;
					return;
				}

			}

			advancedfx::Message(
				"%s import start <fileName> - Starts importing cam from file <fileName>.\n"
				"%s import end - Stops importing.\n"
				, cmd0
				, cmd0
			);
			return;
		}
	}

	advancedfx::Message(
		"%s export [...] - Controls export of new camera motion data.\n"
		"%s import [...] - Controls import of new camera motion data.\n"
		, cmd0
		, cmd0
	);
}
