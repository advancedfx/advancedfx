using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace injector
{
    internal class Injector
    {
        private static bool CheckError(bool condition, InjectorErrors.Error onError, ref InjectorErrors.Error resultError)
        {
            if(null != resultError)
                return false;

            if (condition)
                return true;

            resultError = onError;
            return false;
        }

        public static InjectorErrors.Error Inject(UInt32 dwProcessId, string dllPath)
        {

            string baseDirectory = System.IO.Path.GetDirectoryName(dllPath);

            byte[] datDllPath = Encoding.Unicode.GetBytes(dllPath + "\0");
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

            InjectorErrors.Error error = null;
            bool bOk = true;

            try
            {
                bOk = true
                    && CheckError(IntPtr.Zero != (hProc = OpenProcess(createThreadAccess, false, dwProcessId)), InjectorErrors.OpenProcessFailed, ref error)
                    && CheckError(IntPtr.Zero != (argDllDir = VirtualAllocEx(hProc, IntPtr.Zero, dllDirectorySz, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ReadWrite)), InjectorErrors.VirtualAllocExReadWriteFailed, ref error)
                    && CheckError(IntPtr.Zero != (argDllFilePath = VirtualAllocEx(hProc, IntPtr.Zero, dllFilePathSz, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ReadWrite)), InjectorErrors.VirtualAllocExReadWriteFailed, ref error)
                    && CheckError(null != (image = GetImage(m_PGetModuleHandleW, m_PGetProcAddress, argDllDir, argDllFilePath)), InjectorErrors.GetImageFailed, ref error)
                ;

                if (bOk)
                {
                    imageSz = new UIntPtr((ulong)image.LongLength);

                    bOk = true
                        && CheckError(IntPtr.Zero != (imageAfxHook = VirtualAllocEx(hProc, IntPtr.Zero, imageSz, AllocationType.Reserve | AllocationType.Commit, MemoryProtection.ExecuteReadWrite)), InjectorErrors.VirtualAllocExReadWriteExecuteFailed, ref error)
                        && CheckError(WriteProcessMemory(hProc, argDllDir, datBaseDirectory, dllDirectorySz, IntPtr.Zero), InjectorErrors.WriteProcessMemoryFailed, ref error)
                        && CheckError(WriteProcessMemory(hProc, argDllFilePath, datDllPath, dllFilePathSz, IntPtr.Zero), InjectorErrors.WriteProcessMemoryFailed, ref error)
                        && CheckError(WriteProcessMemory(hProc, imageAfxHook, image, imageSz, IntPtr.Zero), InjectorErrors.WriteProcessMemoryFailed, ref error)
                        && CheckError(FlushInstructionCache(hProc, imageAfxHook, imageSz), InjectorErrors.FlushInstructionCacheFailed, ref error)
                        && CheckError(IntPtr.Zero != (hThread = CreateRemoteThread(hProc, IntPtr.Zero, UIntPtr.Zero, imageAfxHook, IntPtr.Zero, 0, IntPtr.Zero)), InjectorErrors.CreateRemoteThreadFailed, ref error)
                    ;

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
                                bWait = DialogResult.Yes == MessageBox.Show("Image injection problem.\nContinue waiting?", "injector Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
                            }

                        } while (bWait);

                        if (!bOk)
                        {
                            TerminateThread(hThread, 1);
                        }
                        else
                        {
                            UInt32 exitCode;

                            bOk = GetExitCodeThread(hThread, out exitCode);

                            if (bOk)
                            {
                                if (0 != exitCode)
                                {
                                    bOk = false;

                                    if (1 <= exitCode && exitCode <= 15)
                                        error = InjectorErrors.GetById((int)exitCode);
                                    else
                                    {
                                        error = InjectorErrors.AfxHookUnknown;
                                        MessageBox.Show("Unknown AfxHook exit code: " + exitCode.ToString(), "injector Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    }
                                }
                            }
                        }
                    }
                }

            }
            finally
            {
                if (IntPtr.Zero != hThread) CloseHandle(hThread);

                if (IntPtr.Zero != imageAfxHook) VirtualFreeEx(hProc, imageAfxHook, UIntPtr.Zero, AllocationType.Release);
                if (IntPtr.Zero != argDllDir) VirtualFreeEx(hProc, argDllFilePath, UIntPtr.Zero, AllocationType.Release);
                if (IntPtr.Zero != argDllFilePath) VirtualFreeEx(hProc, argDllDir, UIntPtr.Zero, AllocationType.Release);

                if (IntPtr.Zero != hProc) CloseHandle(hProc);
            }

            CheckError(bOk, InjectorErrors.Unknown, ref error);

            return error;
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
            catch(Exception)
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
        static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, UIntPtr nSize, /*out*/ IntPtr lpNumberOfBytesWritten);

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, UIntPtr dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, UInt32 dwCreationFlags, /*out*/ IntPtr lpThreadId);

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern bool FlushInstructionCache(IntPtr hProcess, IntPtr lpBaseAddress, UIntPtr dwSize);

        const UInt32 INFINITE = 0xFFFFFFFF;
        const UInt32 WAIT_ABANDONED = 0x00000080;
        const UInt32 WAIT_OBJECT_0 = 0x00000000;
        const UInt32 WAIT_TIMEOUT = 0x00000102;

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern UInt32 WaitForSingleObject(IntPtr hHandle, UInt32 dwMilliseconds);

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern bool TerminateThread(IntPtr hThread, UInt32 dwExitCode);

        [DllImport("Kernel32.dll", SetLastError = true)]
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

        static Injector()
        {
            IntPtr hKernel32Dll = GetModuleHandle("Kernel32.dll");

            if(IntPtr.Zero == hKernel32Dll)
                throw new ApplicationException("Can not get Kernel32.dll handle.");

            m_PGetModuleHandleW = GetProcAddress(hKernel32Dll, "GetModuleHandleW");
            m_PGetProcAddress = GetProcAddress(hKernel32Dll, "GetProcAddress");
        }
    }
}
