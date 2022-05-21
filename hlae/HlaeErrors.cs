using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace AfxGui
{
    internal class InjectorErrors
    {
        public class InjectorError : AfxError
        {
            public InjectorError(int code, string title = null, string description = null, string solution = null, Exception innerException = null)
                : base(code, title, description, solution, innerException)
            {
                if (0 == code)
                    throw new System.ApplicationException("Programming error: Code " + code + " is reserved.");

                if (Init.m_Errors.Keys.Contains(code))
                    throw new System.ApplicationException("Programming error: Code " + code + " already in use.");


                Init.m_Errors[code] = this;
            }
        }

        public static InjectorErrors Instance
        {
            get
            {
                if (null == m_Instance) m_Instance = new InjectorErrors();

                return m_Instance;
            }
        }

        public InjectorError GetById(int id)
        {
            if (!Init.m_Errors.TryGetValue(id, out InjectorError error))
                error = Unknown;

            return error;
        }

        protected class InjectorErrorStrings
        {
            public static string AfxHookError = L10n._p("HLAE errors", "AfxHook error");
            public static string AccessRightsSolution = L10n._p("HLAE errors", "Make sure you run the injector and the program to inject with same access rights and that no antivirus or anti-cheat is blocking its calls.");
            public static string CloseAntiCheatsSolution = L10n._p("HLAE errors", "Make sure to exit anti-cheat software such as the ESEA client, Faceit client or Challengeme.gg before using HLAE.");
        }

        public static readonly InjectorError Unknown = new InjectorError(-1);
        public static readonly InjectorError AfxHook1 = new InjectorError(1, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook2 = new InjectorError(2, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook3 = new InjectorError(3, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook4 = new InjectorError(4, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook5 = new InjectorError(5, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook6 = new InjectorError(6, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook7 = new InjectorError(7, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook8 = new InjectorError(8, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook9 = new InjectorError(9, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook10 = new InjectorError(10, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook11 = new InjectorError(11, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook12 = new InjectorError(12, InjectorErrorStrings.AfxHookError, L10n._p("HLAE errors", "Could not access / enter directory of DLL to inject", "Is the path given invalid? Is the directory really there or missing? Does HLAE have enough rights to access that path?"));
        public static readonly InjectorError AfxHook13 = new InjectorError(13, InjectorErrorStrings.AfxHookError, L10n._p("HLAE errors", "DLL can't be found.", "Remove DLLs, re-add them and maybe check antivirus."));
        public static readonly InjectorError AfxHook14 = new InjectorError(14, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook15 = new InjectorError(15, InjectorErrorStrings.AfxHookError);

        public static readonly InjectorError OpenProcessFailed = new InjectorError(1000, L10n._p("HLAE errors", "OpenProcess failed"), null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError VirtualAllocExReadWriteFailed = new InjectorError(1001, L10n._p("HLAE errors", "VirtualAllocEx read|write allocation failed."), null, InjectorErrorStrings.CloseAntiCheatsSolution);
        public static readonly InjectorError GetImageFailed = new InjectorError(1002, L10n._p("HLAE errors", "GetImage failed."), L10n._p("HLAE errors", "AfxHook.dat missing, broken or not accessible"), L10n._p("HLAE errors", "Try re-extracting or repair HLAE."));
        public static readonly InjectorError VirtualAllocExReadWriteExecuteFailed = new InjectorError(1003, L10n._p("HLAE errors", "VirtualAllocEx read|write|execute allocation failed."), null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError WriteProcessMemoryFailed = new InjectorError(1004, L10n._p("HLAE errors", "WriteProcessMemory failed."), null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError FlushInstructionCacheFailed = new InjectorError(1005, L10n._p("HLAE errors", "FlushInstructionCache failed."), null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError CreateRemoteThreadFailed = new InjectorError(1006, L10n._p("HLAE errors", "CreateRemoteThread failed."), null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError AfxHookUnknown = new InjectorError(1007, L10n._p("HLAE errors", "AfxHook error: Unknown error code."), InjectorErrorStrings.CloseAntiCheatsSolution);

        protected InjectorErrors()
        {

        }

        private class Init
        {
            public static Dictionary<int, InjectorError> m_Errors = new Dictionary<int, InjectorError>();
        }

        private static InjectorErrors m_Instance;
    }

    internal class HlaeErrors : InjectorErrors
    {
        public static new HlaeErrors Instance
        {
            get
            {
                if (null == m_Instance) m_Instance = new HlaeErrors();

                return m_Instance;
            }
        }

        protected class HlaeErrorStrings
        {
            public static string IncorrectPath = L10n._p("HLAE errors", "Make sure you entered the correct path to the game .exe and included the .exe file. Copy the path from the game properties in Steam to make sure you are using the correct path.");
        }

        public static AfxError InjectorStartException(string injectorFileName, Exception exception)
        {
            return new AfxError(2000, L10n._p("HLAE errors", "Failed to start injector."), L10n._p("HLAE errors", "Failed to start injector: {0}", injectorFileName), L10n._p("HLAE errors", "injector.exe blocked. Probably caused by antivirus. Check it and re-extract HLAE (HLAE NEEDS to be extracted, don't run it from a .zip file)."), exception);
        }

        public static AfxError LoaderException(Exception exeception)
        {
            if (exeception is AfxError) return exeception as AfxError;

            return new AfxError(2001, L10n._p("HLAE errors", "Loader failed."), exeception.ToString(), null, exeception);
        }

        public static AfxError LoaderCreateProcessException(int getLastWin32ErrorValue)
        {
            string solution = null;

            switch(getLastWin32ErrorValue)
            {
                case 2:
                    solution = HlaeErrorStrings.IncorrectPath;
                    break;
                case 5:
                    solution = L10n._p("HLAE errors", "Make sure to close any anti-cheat software and that the path to the game .exe is correct.");
                    break;
                case 267:
                    solution = HlaeErrorStrings.IncorrectPath;
                    break;
                case 123:
                    solution = HlaeErrorStrings.IncorrectPath;
                    break;
                case 740:
                    solution = L10n._p("HLAE errors", "Make sure neither HLAE.exe, Steam.exe nor csgo.exe are set to run as admin.");
                    break;
            }

            return new AfxError(2002, L10n._p("HLAE errors", "Loader could not create requested process."), L10n._p("HLAE errors", "GetLastWin32Error = {0}: {1}", getLastWin32ErrorValue, new Win32Exception(Marshal.GetLastWin32Error()).Message), solution);
        }

        protected HlaeErrors()
        {

        }

        private static HlaeErrors m_Instance;
    }
}
