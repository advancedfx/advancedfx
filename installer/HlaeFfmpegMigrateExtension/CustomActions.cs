using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.IO;
using System.Security.Cryptography;
using WixToolset.Dtf.WindowsInstaller;
using System.Windows.Forms;
using System.Globalization;
using System.Threading;
using Microsoft.Win32;
using System.Runtime.InteropServices;
using System.Linq;

namespace HlaeFfmpegMigrateExtension
{
    public class CustomActions
    {
        [CustomAction]
        public static ActionResult RemoveFolder(Session session)
        {
            try
            {
                session.Log("Begin RemoveFolder");

                string afxFolder = session.CustomActionData["AFX_REMOVEFOLDER"].TrimEnd('/', '\\');

                if (0 < afxFolder.Length && Directory.Exists(afxFolder)) Directory.Delete(afxFolder, true);

                return ActionResult.Success;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }
            return ActionResult.Failure;
        }

        [CustomAction]
        public static ActionResult CreateDirectory(Session session)
        {
            try
            {
                session.Log("Begin CreateFolder");

                string afxFolder = session.CustomActionData["AFX_CREATEFOLDER"].TrimEnd('/', '\\');

                if (0 < afxFolder.Length && !Directory.Exists(afxFolder)) Directory.CreateDirectory(afxFolder);

                return ActionResult.Success;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }

            return ActionResult.Failure;
        }


        [CustomAction]
        public static ActionResult MigrateFfmpegInstall(Session session)
        {
            try
            {
                session.Log("Begin CopyDirectory");

                string afxFrom = session.CustomActionData["AFX_FROM"].TrimEnd('/', '\\');
                string afxTo = session.CustomActionData["AFX_TO"].TrimEnd('/', '\\');
                bool bCustom = session.CustomActionData.ContainsKey("FFMPEG_CUSTOM");
                string afxFfmpegExe = bCustom ? session.CustomActionData["FFMPEG_CUSTOM"] : afxTo + "\\bin\\ffmpeg.exe";

                if (
                    0 < afxFrom.Length && Directory.Exists(afxFrom)
                    && 0 < afxTo.Length && Directory.Exists(afxTo)
                    && 0 < afxFfmpegExe.Length
                    ) {
                        if(!bCustom) {
                           Directory.Delete(afxTo, true);
                           Directory.Move(afxFrom, afxTo);
                        }
                        WritePrivateProfileString("Ffmpeg", "Path", afxFfmpegExe, afxTo + "\\ffmpeg.ini");
                }

                return ActionResult.Success;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }
            return ActionResult.Failure;
        }

        [DllImport("kernel32.dll", EntryPoint = "GetPrivateProfileString")]
        public static extern int GetPrivateProfileString(string SectionName, string KeyName, string Default, StringBuilder Return_StringBuilder_Name, int Size, string FileName);

        [DllImport("kernel32", EntryPoint = "WritePrivateProfileString")]
        private static extern long WritePrivateProfileString(string section, string key, string val, string filePath);

        private static void CopyDirectoryEx(string sourceDir, string destinationDir, bool recursive)
        {
            // Get information about the source directory
            var dir = new DirectoryInfo(sourceDir);

            // Check if the source directory exists
            if (!dir.Exists)
                throw new DirectoryNotFoundException($"Source directory not found: {dir.FullName}");

            // Cache directories before we start copying
            DirectoryInfo[] dirs = dir.GetDirectories();

            // Create the destination directory
            Directory.CreateDirectory(destinationDir);

            // Get the files in the source directory and copy to the destination directory
            foreach (FileInfo file in dir.GetFiles())
            {
                string targetFilePath = Path.Combine(destinationDir, file.Name);
                file.CopyTo(targetFilePath);
            }

            // If recursive and copying subdirectories, recursively call this method
            if (recursive)
            {
                foreach (DirectoryInfo subDir in dirs)
                {
                    string newDestinationDir = Path.Combine(destinationDir, subDir.Name);
                    CopyDirectoryEx(subDir.FullName, newDestinationDir, true);
                }
            }
        }
    }
}
