using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Windows.Forms;

namespace AfxGui
{
    internal class Loader
    {
        public delegate string GetHookPathDelegate(bool isProcess64Bit);

        public static bool Load(GetHookPathDelegate getHookPath, string programPath, string cmdLine, string environment = null, bool showErrorMessage = true)
        {
            return Load(new GetHookPathDelegate[] { getHookPath }, programPath, cmdLine, environment, showErrorMessage);
        }

        public static bool Load(IEnumerable<GetHookPathDelegate> getHookPathCollection, string programPath, string cmdLine, string environment = null, bool showErrorMessage = true)
        {
            try
            {
                string programOptions = "\"" + programPath + "\" " + cmdLine;
                string programDirectory = System.IO.Path.GetDirectoryName(programPath);

                PROCESS_INFORMATION processInfo = new PROCESS_INFORMATION();

                STARTUPINFOW startupInfo = new STARTUPINFOW();
                startupInfo.cb = (UInt32)Marshal.SizeOf(startupInfo);

                if (!CreateProcessW(
                    programPath
                    , programOptions
                    , null
                    , null
                    , true // inherit handles
                    , //CREATE_DEFAULT_ERROR_MODE|
                        CREATE_NEW_PROCESS_GROUP |
                        DETACHED_PROCESS |
                        CREATE_SUSPENDED
                        //DEBUG_ONLY_THIS_PROCESS|
                        //DEBUG_PROCESS				// we want to catch debug event's (sadly also of childs)
                        | CREATE_UNICODE_ENVIRONMENT
                    , environment
                    , programDirectory
                    , ref startupInfo
                    , out processInfo)
                    )
                    throw HlaeErrors.LoaderCreateProcessException(Marshal.GetLastWin32Error());

                try
                {
                    bool isProcess64Bit = IsProcess64Bit(processInfo.hProcess);

                    foreach (GetHookPathDelegate getHookPath in getHookPathCollection)
                    {
                        string hookPath = getHookPath(isProcess64Bit);

                        using (System.Diagnostics.Process injector = new System.Diagnostics.Process())
                        {
                            injector.StartInfo.UseShellExecute = false;
                            injector.StartInfo.FileName = System.AppDomain.CurrentDomain.BaseDirectory + (isProcess64Bit ? "\\x64" : "") + "\\injector.exe";
                            injector.StartInfo.CreateNoWindow = true;
                            injector.StartInfo.RedirectStandardInput = true;
                            injector.StartInfo.RedirectStandardOutput = true;

                            try
                            {
                                injector.Start();
                            }
                            catch (Exception e)
                            {
                                throw HlaeErrors.InjectorStartException(
                                    injector.StartInfo.FileName,
                                    e
                                );
                            }

                            advancedfx.AfxError error = null;

                            IFormatter formatter = new advancedfx.injector.interop.Formatter();

                            using (Stream injectorIn = injector.StandardInput.BaseStream)
                            {
                                using (Stream injectorOut = injector.StandardOutput.BaseStream)
                                {
                                    advancedfx.injector.interop.InjectMessage injectMessage = new advancedfx.injector.interop.InjectMessage();
                                    injectMessage.ProcessId = processInfo.dwProcessId;
                                    injectMessage.DllPath = hookPath;

                                    formatter.Serialize(injectorIn, injectMessage);
                                    injectorIn.Flush();

                                    bool injectorExit = false;

                                    while (!injectorExit)
                                    {
                                        advancedfx.injector.interop.ProcessMessage m = (advancedfx.injector.interop.ProcessMessage)formatter.Deserialize(injectorOut);

                                        switch (m)
                                        {
                                            case advancedfx.injector.interop.ExceptionError exceptionError:
                                                if(null == error) error = HlaeErrors.Unknown;
                                                break;
                                            case advancedfx.injector.interop.OpenProcessError openProcessError:
                                                if (null == error) error = HlaeErrors.OpenProcessFailed;
                                                break;
                                            case advancedfx.injector.interop.VirtualAllocExArgDllDirError virtualAllocExArgDllDirError:
                                                if (null == error) error = HlaeErrors.VirtualAllocExReadWriteFailed;
                                                break;
                                            case advancedfx.injector.interop.VirtualAllocExArgDllFilePathError virtualAllocExArgDllFilePathError:
                                                if (null == error) error = HlaeErrors.VirtualAllocExReadWriteFailed;
                                                break;
                                            case advancedfx.injector.interop.GetImageError getImageError:
                                                if (null == error) error = HlaeErrors.GetImageFailed;
                                                break;
                                            case advancedfx.injector.interop.VirtualAllocExImageError virtualAllocExImageError:
                                                if (null == error) error = HlaeErrors.VirtualAllocExReadWriteExecuteFailed;
                                                break;
                                            case advancedfx.injector.interop.WriteProcessMemoryArgDllDirError writeProcessMemoryArgDllDirError:
                                                if (null == error) error = HlaeErrors.WriteProcessMemoryFailed;
                                                break;
                                            case advancedfx.injector.interop.WriteProcessMemoryArgDllFilePathError writeProcessMemoryArgDllFilePathError:
                                                if (null == error) error = HlaeErrors.WriteProcessMemoryFailed;
                                                break;
                                            case advancedfx.injector.interop.WriteProcessMemoryImageError writeProcessMemoryImageError:
                                                if (null == error) error = HlaeErrors.WriteProcessMemoryFailed;
                                                break;
                                            case advancedfx.injector.interop.FlushInstructionCacheError flushInstructionCacheError:
                                                if (null == error) error = HlaeErrors.FlushInstructionCacheFailed;
                                                break;
                                            case advancedfx.injector.interop.CreateRemoteThreadError createRemoteThreadError:
                                                if (null == error) error = HlaeErrors.CreateRemoteThreadFailed;
                                                break;
                                            case advancedfx.injector.interop.ContinueWaitingQuestion contineWaitingQuestion:
                                                {
                                                    advancedfx.injector.interop.ContinueWaiting r = new advancedfx.injector.interop.ContinueWaiting();
                                                    r.Response = DialogResult.Yes == MessageBox.Show("Image injection problem.\nContinue waiting?", "injector Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

                                                    formatter.Serialize(injectorIn, r);
                                                    injectorIn.Flush();
                                                }
                                                break;
                                            case advancedfx.injector.interop.TerminateThreadError terminateThreadError:
                                                // ignore for now
                                                break;
                                            case advancedfx.injector.interop.GetExitCodeThreadError getExitCodeThreadError:
                                                // ignore for now
                                                break;
                                            case advancedfx.injector.interop.InvalidExitCodeError invalidExitCodeError:
                                                if (null == error) error = InjectorErrors.AfxHookUnknown;
                                                break;
                                            case advancedfx.injector.interop.KnownExitCodeError knownExitCodeError:
                                                if (null == error) error = InjectorErrors.Instance.GetById((int)knownExitCodeError.ThreadExitCode);
                                                break;
                                            case advancedfx.injector.interop.CloseHandleThreadError closeHandleError:
                                                // ignore for now
                                                break;
                                            case advancedfx.injector.interop.VirtualFreeExImageError virtualFreeExImageError:
                                                // ignore for now
                                                break;
                                            case advancedfx.injector.interop.VirtualFreeExArgFilePathError virtualFreeExArgFilePathError:
                                                // ignore for now
                                                break;
                                            case advancedfx.injector.interop.VirtualFreeExArgDllDirError virtualFreeExArgDllDirError:
                                                // ignore for now
                                                break;
                                            case advancedfx.injector.interop.CloseHandleProcessError closeHandleProcessError:
                                                // ignore for now
                                                break;
                                            case advancedfx.injector.interop.InjectResponse injectResponse:
                                                bool injectorOk = injectResponse.Response;
                                                injector.WaitForExit();
                                                if (!injectorOk) throw null == error ? HlaeErrors.Unknown : error;
                                                injectorExit = true;
                                                break;
                                            default:
                                                throw HlaeErrors.Unknown;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                finally
                {

                    System.Threading.Thread.Sleep(2000);

                    ResumeThread(processInfo.hThread);

                    CloseHandle(processInfo.hThread);
                    CloseHandle(processInfo.hProcess);
                }
            }
            catch(advancedfx.AfxError e)
            {
                if (showErrorMessage)
                {
                    using (ErrorDialogue frm = new ErrorDialogue())
                    {
                        frm.Error = e;
                        frm.ShowDialog();
                    }
                }

                return false;
            }
            catch(Exception e)
            {
                if(showErrorMessage)
                {
                    using (ErrorDialogue frm = new ErrorDialogue())
                    {
                        frm.Error = HlaeErrors.LoaderException(e);
                        frm.ShowDialog();
                    }
                }

                return false;
            }

            return true;
        }

        private const uint CREATE_SUSPENDED = 0x00000004;
        private const uint DETACHED_PROCESS = 0x00000008;
        private const uint CREATE_NEW_PROCESS_GROUP = 0x00000200;
        private const uint CREATE_UNICODE_ENVIRONMENT = 0x00000400;

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        private struct STARTUPINFOW
        {
            public UInt32 cb;
            public string lpReserved;
            public string lpDesktop;
            public string lpTitle;
            public UInt32 dwX;
            public UInt32 dwY;
            public UInt32 dwXSize;
            public UInt32 dwYSize;
            public UInt32 dwXCountChars;
            public UInt32 dwYCountChars;
            public UInt32 dwFillAttribute;
            public UInt32 dwFlags;
            public UInt16 wShowWindow;
            public UInt16 cbReserved2;
            public IntPtr lpReserved2;
            public IntPtr hStdInput;
            public IntPtr hStdOutput;
            public IntPtr hStdError;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct PROCESS_INFORMATION
        {
            public IntPtr hProcess;
            public IntPtr hThread;
            public UInt32 dwProcessId;
            public UInt32 dwThreadId;
        }

        [StructLayout(LayoutKind.Sequential)]
        private class SECURITY_ATTRIBUTES
        {
            public UInt32 nLength;
            public IntPtr lpSecurityDescriptor;
            public Boolean bInheritHandle;
        }

        [DllImport("Kernel32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("Kernel32", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("Kernel32.dll", SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static extern bool IsWow64Process(
            [In] IntPtr processHandle,
            [Out, MarshalAs(UnmanagedType.Bool)] out bool wow64Process);

        [DllImport("Kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode, ExactSpelling = true)]
        private static extern bool CreateProcessW(
            string lpApplicationName,
            string lpCommandLine,
            SECURITY_ATTRIBUTES lpProcessAttributes,
            SECURITY_ATTRIBUTES lpThreadAttributes,
            bool bInheritHandles,
            uint dwCreationFlags,
            string lpEnvironment,
            string lpCurrentDirectory,
            [In] ref STARTUPINFOW lpStartupInfo,
            out PROCESS_INFORMATION lpProcessInformation);

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern uint ResumeThread(IntPtr hThread);

        [DllImport("Kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool CloseHandle(IntPtr hObject);

        private static bool m_HasIsWow64Process;

        static Loader()
        {
            IntPtr hKernel32Dll = GetModuleHandle("Kernel32.dll");

            if (IntPtr.Zero == hKernel32Dll)
                throw new ApplicationException("Can not get Kernel32.dll handle.");

            m_HasIsWow64Process = IntPtr.Zero != GetProcAddress(hKernel32Dll, "IsWow64Process");
        }

        private static bool IsProcess64Bit(IntPtr hProcess)
        {
            if (!m_HasIsWow64Process)
                return Environment.Is64BitOperatingSystem;

            bool wow64Process;

            if (!IsWow64Process(hProcess, out wow64Process))
                throw new ApplicationException("IsWow64Process failed, error code: " + Marshal.GetLastWin32Error());

            return !(wow64Process || !Environment.Is64BitOperatingSystem);
        }
    }
}
