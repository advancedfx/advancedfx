using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.IO;
using System.Security.Cryptography;
using Microsoft.Deployment.WindowsInstaller;
using System.Windows.Forms;

namespace HlaeCoreExtension
{
    public class CustomActions
    {
        const long ticksFfmpegDownload = 100 * 1000 * 1000;
        const long ticksFfmpegExctract = 250 * 1000 * 1000;

        const int msidbComponentAttributesLocalOnly = 0x0000;

        const string ffmpegWin64Url = "https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-latest-win64-static.zip";
        //const string ffmpegWin64Sha512 = "BC9670EE122F099E9133343C94E5337DA07CD72875985F694231BDD0644E262EA46B61361C815092AFD89686B236717AEE092D4F27090D7FEA90F2DEA3F8F4B1";

        const string ffmpegWin32Url = "https://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-latest-win32-static.zip";
        //const string ffmpegWin32Sha512 = "3F00BDC729D5F6B5A3BE025074B68F9EB07939078E8C38E2FB0E45C798F607E1791EEDB964BB5886820DD9B3CAA4ED0E18A7CD2900031BAFFBE7B2B3ECA4818A";

        class Progress
        {
            public Progress(Session session)
            {
                this.session = session;
            }

            public MessageResult Init(string name, string description, string template, long ticks)
            {
                this.ticks = ticks;
                this.lastTick = 0;
                this.lastTickCount = System.Environment.TickCount;

                return SetupProgress(session, name, description, template, ticks);
            }

            public MessageResult SetAbsTick(long tick, object templateArg1, object templateArg2)
            {
                if (tick < 0) tick = 0;
                else if (ticks < tick) tick = ticks;

                if (tick <= lastTick) return MessageResult.None;

                long deltaTicks = tick - lastTick;

                if (tick < lastTick && Math.Abs(System.Environment.TickCount - lastTickCount) < 100) return MessageResult.None;

                lastTickCount = System.Environment.TickCount;

                lastTick = tick;

                return TickProgress(session, deltaTicks, templateArg1, templateArg2);
            }

            private readonly Session session;
            private long ticks;
            private long lastTick;
            private int lastTickCount;

            private Record actionRec = new Record(3);
            private Record progressRec = new Record(3);

            private MessageResult SetupProgress(Session session, string name, string description, string template, long ticks)
            {
                // https://docs.microsoft.com/en-us/windows/win32/msi/adding-custom-actions-to-the-progressbar
                // https://docs.microsoft.com/en-us/windows/win32/msi/session-message
                // http://bonemanblog.blogspot.com/2004/09/making-progress-bars-behave-nicely.html

                MessageResult result;

                //

                actionRec[1] = name;
                actionRec[2] = description;
                actionRec[3] = template;

                result = session.Message(InstallMessage.ActionStart, actionRec);

                if (result == MessageResult.Cancel) return result;

                // Tell the installer to use explicit progress messages.

                progressRec[1] = 1;
                progressRec[2] = 1;
                progressRec[3] = 0;

                result = session.Message(InstallMessage.Progress, progressRec);

                //Specify that an update of the progress bar's position in
                //this case means to move it forward by one increment.

                progressRec[1] = 2;
                progressRec[2] = 1;
                progressRec[3] = 0;

                //


                return result;
            }

            private MessageResult TickProgress(Session session, long deltaTicks, object templateArg1, object templateArg2)
            {
                MessageResult result = MessageResult.OK;

                actionRec[1] = templateArg1;
                actionRec[2] = templateArg2;

                result = session.Message(InstallMessage.ActionData, actionRec);
                if (result == MessageResult.Cancel) return result;

                progressRec[2] = deltaTicks;

                result = session.Message(InstallMessage.Progress, progressRec);
                if (result == MessageResult.Cancel) return result;

                return result;
            }
        }

        [CustomAction]
        public static ActionResult ValidateAppFolderIsLatin(Session session)
        {
            try
            {
                bool ok = System.Text.RegularExpressions.Regex.IsMatch(session["APPLICATIONFOLDER"], @"^\p{IsBasicLatin}*$");

                Record record = new Record(1);
                record[1] = 25001;

                if (!ok) session.Message(InstallMessage.Error, record);

                return ok ? ActionResult.Success : ActionResult.Failure;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }

            return ActionResult.Failure;
        }


        [CustomAction]
        public static ActionResult InstallFfmpegPrepare(Session session)
        {
            try
            {
                Record progressRec = new Record(2);
                progressRec[1] = 3;
                progressRec[2] = ticksFfmpegDownload + ticksFfmpegExctract; // ticks
                if (MessageResult.Cancel == session.Message(InstallMessage.Progress, progressRec)) return ActionResult.UserExit;

                CustomActionData _data = new CustomActionData();

                _data["UILevel"] = session["UILevel"].Replace(";", ";;");
                _data["TEMPFOLDER"] = session["TEMPFOLDER"].Replace(";", ";;");
                _data["AFX_FFMPEGFOLDER"] = session["AFX_FFMPEGFOLDER"].Replace("; ", ";;");
                _data["InstallFfmpegConnect"] = ((string)session.Database.ExecuteScalar("SELECT `Text` FROM `UIText` WHERE `Key`='InstallFfmpegConnect'")).Replace(";", ";;");
                _data["InstallFfmpegConnect_Template"] = ((string)session.Database.ExecuteScalar("SELECT `Text` FROM `UIText` WHERE `Key`='InstallFfmpegConnect_Template'")).Replace(";", ";;");
                _data["InstallFfmpegDownload"] = ((string)session.Database.ExecuteScalar("SELECT `Text` FROM `UIText` WHERE `Key`='InstallFfmpegDownload'")).Replace(";", ";;");
                _data["InstallFfmpegDownload_Template"] = ((string)session.Database.ExecuteScalar("SELECT `Text` FROM `UIText` WHERE `Key`='InstallFfmpegDownload_Template'")).Replace(";", ";;");
                _data["InstallFfmpegExtract"] = ((string)session.Database.ExecuteScalar("SELECT `Text` FROM `UIText` WHERE `Key`='InstallFfmpegExtract'")).Replace(";", ";;");
                _data["InstallFfmpegExtract_Template"] = ((string)session.Database.ExecuteScalar("SELECT `Text` FROM `UIText` WHERE `Key`='InstallFfmpegExtract_Template'")).Replace(";", ";;");

                session["InstallFfmpegAction"] = _data.ToString();
                return ActionResult.Success;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }

            return ActionResult.Success;
        }


        [CustomAction]
        public static ActionResult InstallFfmpeg(Session session)
        {
            //System.Diagnostics.Debugger.Launch();

            const string actionName = "InstallFfmpegAction";

            bool showUi = false;
            string tempFolder = null;

            try
            {
                session.Log("Begin InstallFfmpeg");

                showUi = 5 >= int.Parse(session.CustomActionData["UILevel"]);

                tempFolder = session.CustomActionData["TEMPFOLDER"].TrimEnd('/', '\\') + "\\" + Path.GetRandomFileName();

                string afxFfmpegFolder = session.CustomActionData["AFX_FFMPEGFOLDER"].TrimEnd('/', '\\');
                string locInstallFfmpegConnect = session.CustomActionData["InstallFfmpegConnect"];
                string locInstallFfmpegConnect_Template = session.CustomActionData["InstallFfmpegConnect_Template"];
                string locInstallFfmpegDownload = session.CustomActionData["InstallFfmpegDownload"];
                string locInstallFfmpegDownload_Template = session.CustomActionData["InstallFfmpegDownload_Template"];
                string locInstallFfmpegExtract = session.CustomActionData["InstallFfmpegExtract"];
                string locInstallFfmpegExtract_Template = session.CustomActionData["InstallFfmpegExtract_Template"];

                bool is64BitOs = System.Environment.Is64BitOperatingSystem;

                string downloadUrl = is64BitOs
                    ? ffmpegWin64Url
                    : ffmpegWin32Url
                    ;

                string fileName = is64BitOs
                    ? "ffmpeg-latest-win64-static.zip"
                    : "ffmpeg-latest-win32-static.zip"
                    ;

                //string targetHash = is64BitOs
                //   ? ffmpegWin64Sha512
                //    : ffmpegWin32Sha512
                 //   ;

                if (!Directory.Exists(tempFolder)) Directory.CreateDirectory(tempFolder);

                ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;

                Progress progress = new Progress(session);

                if (MessageResult.Cancel == progress.Init(actionName, locInstallFfmpegConnect, locInstallFfmpegConnect_Template, 1)) return ActionResult.UserExit;

                System.Net.WebRequest.DefaultWebProxy = System.Net.WebRequest.GetSystemWebProxy();
                System.Net.WebRequest.DefaultWebProxy.Credentials = System.Net.CredentialCache.DefaultNetworkCredentials;

                HttpWebRequest request = (HttpWebRequest)WebRequest.Create(downloadUrl);

                using (FileStream download = new FileStream(tempFolder + "\\" + fileName, FileMode.Create))
                {
                    int read = 0;
                    byte[] buffer = new byte[1000];
                    long length = -1;
                    long lengthRead = 0;

                    WebResponse response = request.GetResponse();

                    length = response.ContentLength;

                    if (MessageResult.Cancel == progress.SetAbsTick(1, 1, 1)) return ActionResult.UserExit;

                    if (MessageResult.Cancel == progress.Init(actionName, locInstallFfmpegDownload, locInstallFfmpegDownload_Template, ticksFfmpegDownload)) return ActionResult.UserExit;

                    using (Stream stream = response.GetResponseStream())
                    {
                        while ((read = stream.Read(buffer, 0, buffer.Length)) != 0)
                        {
                            lengthRead += read;
                            download.Write(buffer, 0, read);

                            if (0 < length && length <= int.MaxValue) progress.SetAbsTick((long)Math.Round(lengthRead * (double)ticksFfmpegDownload / length), lengthRead, length);
                        }

                        stream.Close();
                    }

                    download.Close();

                    if (MessageResult.Cancel == progress.SetAbsTick(ticksFfmpegDownload, lengthRead, length)) return ActionResult.UserExit;
                }

                /*
                if (MessageResult.Cancel == progress.Init("InstallFfmpegAction", "Verifiying checksum", "...", 1)) throw new ApplicationException("Aborted by user.");

                string tempFile = tempFolder+"\\ffmpeg-20190712-81d3d7d-win64-static.zip";

                using (FileStream fs = File.OpenRead(
                    tempFile
                    //directoryPath + "\\" + fileName
                    ))
                {
                    HashAlgorithm sha512 = new SHA512CryptoServiceProvider();
                    string hash = BitConverter.ToString(sha512.ComputeHash(fs)).ToUpper().Replace("-", "");
                    if(0 != hash.CompareTo(targetHash)) throw new ApplicationException("SHA512 hash mismatch.");
                }

                progress.SetAbsTick(1)) return ActionResult.UserExit;

                */

                using (System.IO.Compression.ZipArchive zip = new System.IO.Compression.ZipArchive(new FileStream(
                    tempFolder + "\\" + fileName,
                    FileMode.Open), System.IO.Compression.ZipArchiveMode.Read))
                {
                    int entryNr = 0;

                    if (MessageResult.Cancel == progress.Init(actionName, locInstallFfmpegExtract, locInstallFfmpegExtract_Template, ticksFfmpegExctract)) return ActionResult.UserExit;

                    foreach (System.IO.Compression.ZipArchiveEntry entry in zip.Entries)
                    {
                        // Strip out root directory

                        string[] split = entry.FullName.Trim(new char[] { '/', '\\' }).Split(new char[] { '/', '\\' });

                        if (1 <= split.Length)
                        {

                        }
                        else
                        {
                            throw new ApplicationException("Unexpected ZIP contents.");
                        }

                        string orgName = string.Join("\\", split, 1, split.Length -1);
                        string fullName = afxFfmpegFolder + "\\" + orgName;
                        string fullPath = Path.GetFullPath(fullName);
                        string parentFolder = 2 <= split.Length ? string.Join("\\", split, 1, split.Length - 2) : "";
                        string ownName = string.Join("\\", split, split.Length - 1, 1);

                        if (!(fullPath.IndexOf(afxFfmpegFolder) == 0))
                        {
                            throw new ApplicationException("Bogous ZIP directory structure.");
                        }

                        bool isFile = !(null == entry.Name || 0 == entry.Name.Length);

                        if (isFile)
                        {
                            using (Stream entryStream = entry.Open())
                            {
                                using (FileStream outEntryStream = new FileStream(fullPath, FileMode.Create))
                                {
                                    int read = 0;
                                    byte[] buffer = new byte[1000];
                                    long length = entry.Length;
                                    long lengthRead = 0;

                                    while ((read = entryStream.Read(buffer, 0, buffer.Length)) != 0)
                                    {
                                        lengthRead += read;
                                        outEntryStream.Write(buffer, 0, read);
                                    }

                                    outEntryStream.Close();
                                }

                                entryStream.Close();
                            }
                        }
                        else
                        {
                            if (!Directory.Exists(fullPath)) Directory.CreateDirectory(fullPath);
                        }

                        ++entryNr;

                        if (MessageResult.Cancel == progress.SetAbsTick((long)Math.Round(entryNr * (double)ticksFfmpegExctract / zip.Entries.Count), entryNr, zip.Entries.Count)) return ActionResult.UserExit;
                    }

                    if (MessageResult.Cancel == progress.SetAbsTick(ticksFfmpegExctract, entryNr, zip.Entries.Count)) return ActionResult.UserExit;
                }

                return ActionResult.Success;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }
            finally
            {
                try
                {
                    if (null != tempFolder) Directory.Delete(tempFolder, true);
                }
                catch (Exception e2)
                {
                    session.Log("Error deleting temporary ffmpeg directory: " + e2.ToString());
                }
            }

            return ActionResult.Failure;
        }

        [CustomAction]
        public static ActionResult UninstallFfmpegPrepare(Session session)
        {
            try
            {
                CustomActionData _data = new CustomActionData();

                _data["UILevel"] = session["UILevel"].Replace(";", ";;");
                _data["AFX_FFMPEGFOLDER"] = session["AFX_FFMPEGFOLDER"].Replace("; ", ";;");

                session["RollbackFfmpegAction"] = _data.ToString();
                session["UninstallFfmpegAction"] = _data.ToString();
                return ActionResult.Success;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }

            return ActionResult.Success;
        }

        [CustomAction]
        public static ActionResult UninstallFfmpeg(Session session)
        {
            bool showUi = false;

            try
            {
                session.Log("Begin UninstallFfmpeg");

                showUi = 5 >= int.Parse(session.CustomActionData["UILevel"]);

                string afxFfmpegFolder = session.CustomActionData["AFX_FFMPEGFOLDER"].TrimEnd('/', '\\');

                if(0 < afxFfmpegFolder.Length) Directory.Delete(afxFfmpegFolder, true);

                return ActionResult.Success;
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
            }

            return ActionResult.Success;
        }
    }
}
