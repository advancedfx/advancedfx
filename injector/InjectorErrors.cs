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
            public const string AfxHookError = "AfxHook error";
            public const string AccessRightsSolution = "Make sure you run the injector and the program to inject with same access rights and that no anti virus / anti cheat is blocking its calls.";
        }

        public static readonly Error AfxHook1 = new Error(1, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook2 = new Error(2, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook3 = new Error(3, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook4 = new Error(4, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook5 = new Error(5, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook6 = new Error(6, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook7 = new Error(7, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook8 = new Error(8, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook9 = new Error(9, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook10 = new Error(10, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook11 = new Error(11, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook12 = new Error(12, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook13 = new Error(13, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook14 = new Error(14, InjectorErrorStrings.AfxHookError);
        public static readonly Error AfxHook15 = new Error(15, InjectorErrorStrings.AfxHookError);

        public static readonly Error OpenProcessFailed = new Error(1000, "OpenProcess failed", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error VirtualAllocExReadWriteFailed = new Error(1001, "VirtualAllocEx read|write allocation failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error GetImageFailed = new Error(1002, "GetImage failed.", null, "Make sure that AfxHook.dat is not missing / broken and accessible.");
        public static readonly Error VirtualAllocExReadWriteExecuteFailed = new Error(1003, "VirtualAllocEx read|write|execute allocation failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error WriteProcessMemoryFailed = new Error(1004, "WriteProcessMemory failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error FlushInstructionCacheFailed = new Error(1005, "FlushInstructionCache failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error CreateRemoteThreadFailed = new Error(1006, "CreateRemoteThread failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly Error AfxHookUnknown = new Error(1007, "AfxHook error: Unknown error code.");

        private static InjectorErrors m_Instance;

        protected InjectorErrors()
        {

        }
    }
}
