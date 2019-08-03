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

                GlobalConfig.Instance.BackUp();

                bOk = Loader.Load(getHookPaths, frm.Program, frm.CmdLine);

                if (!bOk)
                    MessageBox.Show("CustomLoader failed", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
                bOk = true;
        }

        return bOk;
    }
}

} // namespace AfxGui.Tools {
