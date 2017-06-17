using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace injector
{
    internal class InjectorErrors : advancedfx.Errors
    {
        public static new InjectorErrors Instance
        {
            get
            {
                if (null == m_Instance) m_Instance = new InjectorErrors();

                return m_Instance;
            }
        }

        protected class InjectorErrorStrings
        {
            public const string AccessRightsSolution = "Make sure you run the injector and the program to inject with same access rights and that no anti virus / anti cheat is blocking its calls.";
        }

        public static readonly Error AfxHook1 = new Error(1, "AfxHook error: 1", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook2 = new Error(2, "AfxHook error: 2", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook3 = new Error(3, "AfxHook error: 3", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook4 = new Error(4, "AfxHook error: 4", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook5 = new Error(5, "AfxHook error: 5", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook6 = new Error(6, "AfxHook error: 6", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook7 = new Error(7, "AfxHook error: 7", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook8 = new Error(8, "AfxHook error: 8", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook9 = new Error(9, "AfxHook error: 9", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook10 = new Error(10, "AfxHook error: 10", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook11 = new Error(11, "AfxHook error: 11", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook12 = new Error(12, "AfxHook error: 12", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook13 = new Error(13, "AfxHook error: 13", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook14 = new Error(14, "AfxHook error: 14", ErrorStrings.NoSolutionKnown);
        public static readonly Error AfxHook15 = new Error(15, "AfxHook error: 15", ErrorStrings.NoSolutionKnown);

        public static readonly Error OpenProcessFailed = new Error(1000, "OpenProcess failed", InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error VirtualAllocExReadWriteFailed = new Error(1001, "VirtualAllocEx read|write allocation failed.", InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error GetImageFailed = new Error(1002, "GetImage failed.", "Make sure that AfxHook.dat is not missing / broken and accessible.");
        public static readonly Error VirtualAllocExReadWriteExecuteFailed = new Error(1003, "VirtualAllocEx read|write|execute allocation failed.", InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error WriteProcessMemoryFailed = new Error(1004, "WriteProcessMemory failed.", InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error FlushInstructionCacheFailed = new Error(1005, "FlushInstructionCache failed.", InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error CreateRemoteThreadFailed = new Error(1006, "CreateRemoteThread failed.", InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error AfxHookUnknown = new Error(1007, "AfxHook error: Unknown error code.", ErrorStrings.NoSolutionKnown);

        private static InjectorErrors m_Instance;

        protected InjectorErrors()
        {

        }
    }
}
