using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Text;

namespace injector
{
    class Program
    {
        static Program()
        {
            //TODO: Maybe move this into main and make it one-time initalized and handle errors more gracefully.

            IntPtr hKernel32Dll = GetModuleHandle("Kernel32.dll");

            if (IntPtr.Zero == hKernel32Dll)
                throw new ApplicationException("Can not get Kernel32.dll handle.");

            m_PGetModuleHandleW = GetProcAddress(hKernel32Dll, "GetModuleHandleW");
            m_PGetProcAddress = GetProcAddress(hKernel32Dll, "GetProcAddress");
        }

        static void Main()
        {
            System.Windows.Forms.MessageBox.Show("HI");

            IFormatter formatter = new advancedfx.injector.interop.Formatter();

            using (Stream stdIn = System.Console.OpenStandardInput())
            {
                using (Stream stdOut = System.Console.OpenStandardOutput())
                {
                    advancedfx.injector.interop.InjectMessage injectMessage = (advancedfx.injector.interop.InjectMessage)formatter.Deserialize(stdIn);

                    System.Windows.Forms.MessageBox.Show(injectMessage.ToString());

                    bool bOk = false;
                    try
                    {
                        bOk = Inject(injectMessage, formatter, stdIn, stdOut);
                    }
                    catch (Exception e)
                    {
                        bOk = false;

                        advancedfx.injector.interop.ExceptionError r = new advancedfx.injector.interop.ExceptionError();
                        r.ExceptionText = e.ToString();
                        formatter.Serialize(stdOut, r);
                    }

                    {
                        advancedfx.injector.interop.InjectResponse r = new advancedfx.injector.interop.InjectResponse();
                        r.Response = bOk;
                        formatter.Serialize(stdOut, r);
                        stdOut.Flush();
                    }
                }
            }
        }

        internal static bool Inject(advancedfx.injector.interop.InjectMessage injectMessage, IFormatter formatter, Stream stdIn, Stream stdOut)
        {
            string baseDirectory = System.IO.Path.GetDirectoryName(injectMessage.DllPath);

            byte[] datDllPath = Encoding.Unicode.GetBytes(injectMessage.DllPath + "\0");
            byte[] datBaseDirectory = Encoding.Unicode.GetBytes(baseDirectory + "\0");
            byte[] image = null;

            IntPtr argDllDir = IntPtr.Zero;
            IntPtr argDllFilePath = IntPtr.Zero;
            UIntPtr dllDirectorySz = new UIntPtr((ulong)datBaseDirectory.LongLength);
            UIntPtr dllFilePathSz = new UIntPtr((ulong)datDllPath.LongLength);
            UIntPtr imageSz = UIntPtr.Zero;
            IntPtr hProc = IntPtr.Zero;
            IntPtr hThread = IntPtr.Zero;
            IntPtr imageAfxHook = IntPtr.Zero;

            bool bOk = true;

            try
            {
                bOk = true;

                if(bOk && IntPtr.Zero == (hProc = OpenProcess(createThreadAccess, false, injectMessage.ProcessId)))
                {
                    bOk = false;
                    advancedfx.injector.interop.OpenProcessError e = new advancedfx.injector.interop.OpenProcessError();
                    e.GetLastError = Marshal.GetLastWin32Error();

                    formatter.Serialize(stdOut, e);
                }

                if (bOk && IntPtr.Zero == (argDllDir = VirtualAllocEx(hProc, IntPtr.Zero, dllDirectorySz, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ReadWrite)))
                {
                    bOk = false;
                    advancedfx.injector.interop.VirtualAllocExArgDllDirError e = new advancedfx.injector.interop.VirtualAllocExArgDllDirError();
                    e.GetLastError = Marshal.GetLastWin32Error();

                    formatter.Serialize(stdOut, e);
                }

                if (bOk && IntPtr.Zero == (argDllFilePath = VirtualAllocEx(hProc, IntPtr.Zero, dllFilePathSz, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ReadWrite)))
                {
                    bOk = false;
                    advancedfx.injector.interop.VirtualAllocExArgDllFilePathError e = new advancedfx.injector.interop.VirtualAllocExArgDllFilePathError();
                    e.GetLastError = Marshal.GetLastWin32Error();

                    formatter.Serialize(stdOut, e);
                }

                if(bOk && null == (image = GetImage(m_PGetModuleHandleW, m_PGetProcAddress, argDllDir, argDllFilePath)))
                {
                    bOk = false;
                    advancedfx.injector.interop.GetImageError e = new advancedfx.injector.interop.GetImageError();

                    formatter.Serialize(stdOut, e);
                }

                if (bOk)
                {
                    imageSz = new UIntPtr((ulong)image.LongLength);

                    if (bOk && IntPtr.Zero == (imageAfxHook = VirtualAllocEx(hProc, IntPtr.Zero, imageSz, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ExecuteReadWrite)))
                    {
                        bOk = false;
                        advancedfx.injector.interop.VirtualAllocExImageError e = new advancedfx.injector.interop.VirtualAllocExImageError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }

                    if(bOk && !WriteProcessMemory(hProc, argDllDir, datBaseDirectory, dllDirectorySz, IntPtr.Zero))
                    {
                        bOk = false;
                        advancedfx.injector.interop.WriteProcessMemoryArgDllDirError e = new advancedfx.injector.interop.WriteProcessMemoryArgDllDirError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }

                    if(bOk && ! WriteProcessMemory(hProc, argDllFilePath, datDllPath, dllFilePathSz, IntPtr.Zero))
                    {
                        bOk = false;
                        advancedfx.injector.interop.WriteProcessMemoryArgDllFilePathError e = new advancedfx.injector.interop.WriteProcessMemoryArgDllFilePathError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }

                    if (bOk && !WriteProcessMemory(hProc, imageAfxHook, image, imageSz, IntPtr.Zero))
                    {
                        bOk = false;
                        advancedfx.injector.interop.WriteProcessMemoryImageError e = new advancedfx.injector.interop.WriteProcessMemoryImageError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }

                    if(bOk && !FlushInstructionCache(hProc, imageAfxHook, imageSz))
                    {
                        bOk = false;
                        advancedfx.injector.interop.FlushInstructionCacheError e = new advancedfx.injector.interop.FlushInstructionCacheError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }

                    if (bOk && IntPtr.Zero == (hThread = CreateRemoteThread(hProc, IntPtr.Zero, UIntPtr.Zero, imageAfxHook, IntPtr.Zero, 0, IntPtr.Zero)))
                    {
                        bOk = false;
                        advancedfx.injector.interop.CreateRemoteThreadError e = new advancedfx.injector.interop.CreateRemoteThreadError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }

                    if (bOk)
                    {
                        bOk = false;
                        bool bWait;

                        do
                        {
                            bWait = false;

                            for (int i = 0; i < 60; i++)
                            {
                                if (WAIT_OBJECT_0 == WaitForSingleObject(hThread, 1000))
                                {
                                    bOk = true;
                                    break;
                                }
                            }

                            if (!bOk)
                            {
                                advancedfx.injector.interop.ContinueWaitingQuestion e = new advancedfx.injector.interop.ContinueWaitingQuestion();

                                formatter.Serialize(stdOut, e);
                                stdOut.Flush();

                                advancedfx.injector.interop.ContinueWaiting r = (advancedfx.injector.interop.ContinueWaiting)formatter.Deserialize(stdIn);

                                bWait = r.Response;
                            }

                        } while (bWait);

                        if (!bOk)
                        {
                            TerminateThread(hThread, 1);
                        }
                        else
                        {
                            if(!GetExitCodeThread(hThread, out UInt32 exitCode))
                            {
                                bOk = false;
                                advancedfx.injector.interop.GetExitCodeThreadError e = new advancedfx.injector.interop.GetExitCodeThreadError();
                                e.GetLastError = Marshal.GetLastWin32Error();

                                formatter.Serialize(stdOut, e);
                            }

                            if (bOk)
                            {
                                if (0 != exitCode)
                                {
                                    bOk = false;

                                    if (1 <= exitCode && exitCode <= 15)
                                    {
                                        advancedfx.injector.interop.KnownExitCodeError e = new advancedfx.injector.interop.KnownExitCodeError();
                                        e.ThreadExitCode = exitCode;

                                        formatter.Serialize(stdOut, e);
                                    }
                                    else
                                    {
                                        advancedfx.injector.interop.InvalidExitCodeError e = new advancedfx.injector.interop.InvalidExitCodeError();

                                        formatter.Serialize(stdOut, e);
                                    }
                                }
                            }
                        }
                    }
                }

            }
            finally
            {
                if (IntPtr.Zero != hThread)
                {
                    if(!CloseHandle(hThread))
                    {
                        bOk = false;
                        advancedfx.injector.interop.CloseHandleThreadError e = new advancedfx.injector.interop.CloseHandleThreadError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }
                }

                if (IntPtr.Zero != imageAfxHook)
                {
                    if(!VirtualFreeEx(hProc, imageAfxHook, UIntPtr.Zero, AllocationType.Release))
                    {
                        bOk = false;
                        advancedfx.injector.interop.VirtualFreeExImageError e = new advancedfx.injector.interop.VirtualFreeExImageError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }
                }

                if (IntPtr.Zero != argDllFilePath)
                {
                    if(!VirtualFreeEx(hProc, argDllFilePath, UIntPtr.Zero, AllocationType.Release))
                    {
                        bOk = false;
                        advancedfx.injector.interop.VirtualFreeExArgFilePathError e = new advancedfx.injector.interop.VirtualFreeExArgFilePathError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }
                }

                if (IntPtr.Zero != argDllDir)
                {
                    if (!VirtualFreeEx(hProc, argDllDir, UIntPtr.Zero, AllocationType.Release))
                    {
                        bOk = false;
                        advancedfx.injector.interop.VirtualFreeExArgDllDirError e = new advancedfx.injector.interop.VirtualFreeExArgDllDirError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }
                }

                if (IntPtr.Zero != hProc)
                {
                    if(!CloseHandle(hProc))
                    {
                        bOk = false;
                        advancedfx.injector.interop.CloseHandleThreadError e = new advancedfx.injector.interop.CloseHandleThreadError();
                        e.GetLastError = Marshal.GetLastWin32Error();

                        formatter.Serialize(stdOut, e);
                    }
                }
            }

            return bOk;
        }

        private static byte[] GetImage(IntPtr pGetModuleHandleW, IntPtr pGetProcAddress, IntPtr pBaseDirectory, IntPtr pDllFilePath)
        {
            byte[] image = null;

            try
            {
                image = System.IO.File.ReadAllBytes(System.AppDomain.CurrentDomain.BaseDirectory + "\\AfxHook.dat");

                const int argOfs = 32;

                if (IntPtr.Size == sizeof(UInt32))
                {
                    BitConverter.GetBytes(pGetModuleHandleW.ToInt32()).CopyTo(image, argOfs + 0 * IntPtr.Size);
                    BitConverter.GetBytes(pGetProcAddress.ToInt32()).CopyTo(image, argOfs + 1 * IntPtr.Size);
                    BitConverter.GetBytes(pBaseDirectory.ToInt32()).CopyTo(image, argOfs + 2 * IntPtr.Size);
                    BitConverter.GetBytes(pDllFilePath.ToInt32()).CopyTo(image, argOfs + 3 * IntPtr.Size);
                }
                else
                {
                    BitConverter.GetBytes(pGetModuleHandleW.ToInt64()).CopyTo(image, argOfs + 0 * IntPtr.Size);
                    BitConverter.GetBytes(pGetProcAddress.ToInt64()).CopyTo(image, argOfs + 1 * IntPtr.Size);
                    BitConverter.GetBytes(pBaseDirectory.ToInt64()).CopyTo(image, argOfs + 2 * IntPtr.Size);
                    BitConverter.GetBytes(pDllFilePath.ToInt64()).CopyTo(image, argOfs + 3 * IntPtr.Size);
                }
            }
            catch (Exception)
            {
                return null;
            }

            return image;
        }

        [Flags]
        public enum AllocationType : UInt32
        {
            Commit = 0x1000,
            Reserve = 0x2000,
            Decommit = 0x4000,
            Release = 0x8000,
            Reset = 0x80000,
            Physical = 0x400000,
            TopDown = 0x100000,
            WriteWatch = 0x200000,
            LargePages = 0x20000000
        }

        [Flags]
        public enum MemoryProtection : UInt32
        {
            Execute = 0x10,
            ExecuteRead = 0x20,
            ExecuteReadWrite = 0x40,
            ExecuteWriteCopy = 0x80,
            NoAccess = 0x01,
            ReadOnly = 0x02,
            ReadWrite = 0x04,
            WriteCopy = 0x08,
            GuardModifierflag = 0x100,
            NoCacheModifierflag = 0x200,
            WriteCombineModifierflag = 0x400
        }

        [DllImport("Kernel32.dll", SetLastError = true, ExactSpelling = true)]
        static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, UIntPtr dwSize, AllocationType flAllocationType, MemoryProtection flProtect);

        [DllImport("Kernel32.dll", SetLastError = true, ExactSpelling = true)]
        static extern bool VirtualFreeEx(IntPtr hProcess, IntPtr lpAddress, UIntPtr dwSize, AllocationType dwFreeType);

        [Flags]
        public enum ProcessAccessFlags : UInt32
        {
            All = 0x001F0FFF,
            Terminate = 0x00000001,
            CreateThread = 0x00000002,
            VirtualMemoryOperation = 0x00000008,
            VirtualMemoryRead = 0x00000010,
            VirtualMemoryWrite = 0x00000020,
            DuplicateHandle = 0x00000040,
            CreateProcess = 0x000000080,
            SetQuota = 0x00000100,
            SetInformation = 0x00000200,
            QueryInformation = 0x00000400,
            QueryLimitedInformation = 0x00001000,
            Synchronize = 0x00100000
        }

        const ProcessAccessFlags createThreadAccess = ProcessAccessFlags.CreateThread | ProcessAccessFlags.QueryInformation | ProcessAccessFlags.VirtualMemoryOperation | ProcessAccessFlags.VirtualMemoryWrite | ProcessAccessFlags.VirtualMemoryRead;

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr OpenProcess(ProcessAccessFlags dwDesiredAccess, bool bInheritHandle, UInt32 dwProcessId);

        [DllImport("Kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, UIntPtr nSize, /*out*/ IntPtr lpNumberOfBytesWritten);

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, UIntPtr dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, UInt32 dwCreationFlags, /*out*/ IntPtr lpThreadId);

        [DllImport("Kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool FlushInstructionCache(IntPtr hProcess, IntPtr lpBaseAddress, UIntPtr dwSize);

        const UInt32 INFINITE = 0xFFFFFFFF;
        const UInt32 WAIT_ABANDONED = 0x00000080;
        const UInt32 WAIT_OBJECT_0 = 0x00000000;
        const UInt32 WAIT_TIMEOUT = 0x00000102;

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern UInt32 WaitForSingleObject(IntPtr hHandle, UInt32 dwMilliseconds);

        [DllImport("Kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool TerminateThread(IntPtr hThread, UInt32 dwExitCode);

        [DllImport("Kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool GetExitCodeThread(IntPtr hThread, out UInt32 lpExitCode);

        [DllImport("Kernel32.dll", CharSet = CharSet.Auto)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("Kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("Kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool CloseHandle(IntPtr hObject);

        private static IntPtr m_PGetModuleHandleW;
        private static IntPtr m_PGetProcAddress;
    }
}
