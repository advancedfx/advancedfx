using System;
using System.Windows.Forms;

namespace injector
{
    class Program
    {
        static int Main(string[] args)
        {
            InjectorErrors.Error result = null;

            string strArgs = string.Join(" ", args);

            try
            {
                int delimiterPos = strArgs.IndexOf(" ");
                string strDwProcessId = strArgs.Substring(0, delimiterPos);
                UInt32 dwProcessId = UInt32.Parse(strDwProcessId);

                string strDllPath = strArgs.Substring(delimiterPos + 1);

                result = Injector.Inject(dwProcessId, strDllPath);
            }
            catch(Exception e)
            {
                System.Windows.Forms.MessageBox.Show(e.ToString(), "injector Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                result = InjectorErrors.Unknown;
            }

            return (int)(null == result ? 0 : result.Code);
        }
    }
}
