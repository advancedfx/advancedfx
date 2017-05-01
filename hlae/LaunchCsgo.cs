using System;
using System.Collections;
using System.Windows.Forms;

namespace AfxGui
{
    class LaunchCsgo
    {
        public static bool RunLauncherDialog(IWin32Window dialogOwner)
        {
            bool bOk;

            using (LaunchCsgoForm frm = new LaunchCsgoForm())
            {
                frm.Icon = Program.Icon;
                frm.ShowInTaskbar = false;

                frm.Config = GlobalConfig.Instance.Settings.LauncherCsgo;

                if (DialogResult.OK == frm.ShowDialog(dialogOwner))
                {
                    CfgLauncherCsgo cfg = frm.Config;

                    if (cfg.RememberChanges)
                    {
                        GlobalConfig.Instance.Settings.LauncherCsgo = cfg;
                        GlobalConfig.Instance.BackUp();
                    }
                    else
                    {
                        GlobalConfig.Instance.Settings.LauncherCsgo.RememberChanges = cfg.RememberChanges;
                    }

                    bOk = Launch(cfg);

                    if (!bOk)
                        MessageBox.Show("Launching failed.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                else
                    bOk = true;
            }

            return bOk;
        }

        public static bool Launch(CfgLauncherCsgo config)
        {
            String environment = null;

            String programPath = config.CsgoExe;

            String cmdLine = "-steam -game csgo";
            if (config.AvoidVac)
                cmdLine += " -insecure +sv_lan 1";
            if (config.GfxEnabled)
                cmdLine += " -w " + config.GfxWidth + " -h " + config.GfxHeight + " " + (config.GfxFull ? "-full" : "-window");
            if (0 < config.CustomLaunchOptions.Length)
                cmdLine += " " + config.CustomLaunchOptions;

            if (config.MmcfgEnabled)
            {
                environment = "";

                foreach (DictionaryEntry kv in Environment.GetEnvironmentVariables())
                {
                    environment += kv.Key + "=" + kv.Value + "\0";
                }

                environment += "USRLOCALCSGO=" + config.Mmmcfg+"\0";

                environment += "\0\0";
            }

            return AfxCppCli.AfxHook.LauchAndHook(
                programPath,
                cmdLine,
                System.Windows.Forms.Application.StartupPath + "\\AfxHookSource.dll",
                environment
                );
        }

    }
}
