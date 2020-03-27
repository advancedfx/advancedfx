using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace AfxGui.Tools {

class CustomLoader
{
    public static bool RunCustomLoader(IWin32Window owner)
    {
	    CfgCustomLoader cfg = GlobalConfig.Instance.Settings.CustomLoader;

        bool bOk;

        using (CustomLoaderForm frm = new CustomLoaderForm())
        {
            frm.Icon = Program.Icon;
            frm.ShowInTaskbar = false;

            frm.Program = cfg.ProgramPath;
            frm.CmdLine = cfg.CmdLine;
            foreach (CfgInjectDll dll in cfg.InjectDlls) frm.HookDlls.Add(dll.Path);

            frm.AddEnvironmentVars = cfg.AddEnvironmentVars.Replace("\n", System.Environment.NewLine);

            DialogResult dr = frm.ShowDialog(owner);

            if (DialogResult.OK == dr)
            {
                    List<Loader.GetHookPathDelegate> getHookPaths = new List<Loader.GetHookPathDelegate>();

                    cfg.InjectDlls.Clear();
                    foreach (object o in frm.HookDlls)
                    {
                        string path = o as string;

                        if (null != path)
                        {
                            CfgInjectDll dll = new CfgInjectDll();
                            dll.Path = path;

                            cfg.InjectDlls.Add(dll);

                            getHookPaths.Add(isProcess64Bit => dll.FullPath);
                        }
                    }

                cfg.ProgramPath = frm.Program;
                cfg.CmdLine = frm.CmdLine;
                cfg.AddEnvironmentVars = frm.AddEnvironmentVars.Replace(System.Environment.NewLine, "\n");

                GlobalConfig.Instance.BackUp();

                string[] envVars = cfg.AddEnvironmentVars.Split(new char[]{ '\n'}, StringSplitOptions.RemoveEmptyEntries);
                string environment = null;        
                foreach(string line in envVars)
                {
                        if (null == environment)
                        {
                            environment = "";
                            foreach (System.Collections.DictionaryEntry kv in Environment.GetEnvironmentVariables())
                            {
                                environment += kv.Key + "=" + kv.Value + "\0";
                            }
                        }
                    environment += line + "\0";
                }
                if (null != environment)
                {
                    environment += "\0\0";
                }

                bOk = Loader.Load(getHookPaths, frm.Program, frm.CmdLine, environment);

                if (!bOk)
                    MessageBox.Show(L10n._p("Custom Loader dialog", "CustomLoader failed"), L10n._("Error"), MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
                bOk = true;
        }

        return bOk;
    }
}

} // namespace AfxGui.Tools {
