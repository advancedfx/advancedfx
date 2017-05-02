using System;
using System.Windows.Forms;

namespace injector
{
    class Program
    {
        static int Main(string[] args)
        {
            bool bOk = false;

            string strArgs = string.Join(" ", args);

            try
            {
                int delimiterPos = strArgs.IndexOf(" ");
                string strDwProcessId = strArgs.Substring(0, delimiterPos);
                UInt32 dwProcessId = UInt32.Parse(strDwProcessId);

                string strDllPath = strArgs.Substring(delimiterPos + 1);

                bOk = advancedfx.Injector.Inject(dwProcessId, strDllPath);
            }
            catch(Exception e)
            {
                System.Windows.Forms.MessageBox.Show(e.ToString(), "injector Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return 1;
            }

            return bOk ? 0 : 1;
        }
    }
}
