using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace AfxGui
{
    internal class InjectorErrors
    {
        public class InjectorError : advancedfx.AfxError
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
            public const string AfxHookError = "AfxHook error";
            public const string AccessRightsSolution = "Make sure you run the injector and the program to inject with same access rights and that no anti virus / anti cheat is blocking its calls.";
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
        public static readonly InjectorError AfxHook12 = new InjectorError(12, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook13 = new InjectorError(13, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook14 = new InjectorError(14, InjectorErrorStrings.AfxHookError);
        public static readonly InjectorError AfxHook15 = new InjectorError(15, InjectorErrorStrings.AfxHookError);

        public static readonly InjectorError OpenProcessFailed = new InjectorError(1000, "OpenProcess failed", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError VirtualAllocExReadWriteFailed = new InjectorError(1001, "VirtualAllocEx read|write allocation failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError GetImageFailed = new InjectorError(1002, "GetImage failed.", null, "Make sure that AfxHook.dat is not missing / broken and accessible.");
        public static readonly InjectorError VirtualAllocExReadWriteExecuteFailed = new InjectorError(1003, "VirtualAllocEx read|write|execute allocation failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError WriteProcessMemoryFailed = new InjectorError(1004, "WriteProcessMemory failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError FlushInstructionCacheFailed = new InjectorError(1005, "FlushInstructionCache failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError CreateRemoteThreadFailed = new InjectorError(1006, "CreateRemoteThread failed.", null, InjectorErrorStrings.AccessRightsSolution);
        public static readonly InjectorError AfxHookUnknown = new InjectorError(1007, "AfxHook error: Unknown error code.");

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

        public static advancedfx.AfxError InjectorStartException(string injectorFileName, Exception exception)
        {
            return new advancedfx.AfxError(2000, "Failed to start injector.", "Failed to start injector: " + injectorFileName, "Check that your Anti Virus did not remove it due to a false positive. If so restore it and add an exception for injector / the HLAE folder.", exception);
        }

        public static advancedfx.AfxError LoaderException(Exception exeception)
        {
            if (exeception is advancedfx.AfxError) return exeception as advancedfx.AfxError;

            return new advancedfx.AfxError(2001, "Loader failed.", exeception.ToString(), null, exeception);
        }

        protected HlaeErrors()
        {

        }

        private static HlaeErrors m_Instance;
    }
}
