using System;
using System.Diagnostics;
using System.IO;
using WixToolset.Dtf.WindowsInstaller;
using System.Windows.Forms;

namespace HlaeCoreExtension
{
    public class CustomActions
    {
        const int msidbComponentAttributesLocalOnly = 0x0000;

        [CustomAction]
        public static ActionResult ValidateAppFolderIsLatin(Session session)
        {
            try
            {
                bool ok = System.Text.RegularExpressions.Regex.IsMatch(session["APPLICATIONFOLDER"], "^\\p{IsBasicLatin}*$");

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
        public static ActionResult ValidateTargetPath(Session session)
        {
            try
            {
                if (session["_BrowseProperty"].Equals("APPLICATIONFOLDER"))
                {
                    bool ok = System.Text.RegularExpressions.Regex.IsMatch(session["APPLICATIONFOLDER"], "^\\p{IsBasicLatin}*$");

                    if (!ok) MessageBox.Show(null, ((string)session.Database.ExecuteScalar("SELECT `Message` FROM `Error` WHERE `Error`=25001")), null, MessageBoxButtons.OK, MessageBoxIcon.Error);

                    session["AFX_TARGETPATH_OK"] = ok ? "1" : "0";
                    return ActionResult.Success;
                }
            }
            catch (Exception e)
            {
                session.Log("Error: " + e.ToString());
                session["AFX_TARGETPATH_OK"] = "0";
                return ActionResult.Failure;
            }

            session["AFX_TARGETPATH_OK"] = "1";
            return ActionResult.Success;
        }

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
    }
}
