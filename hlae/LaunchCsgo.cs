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
                }
                else
                    bOk = true;
            }

            return bOk;
        }

        private static string GetHookPath(bool isProcess64Bit)
        {
            if (isProcess64Bit) throw new System.ApplicationException(L10n._("64 Bit CS:GO is not supported."));

            return System.Windows.Forms.Application.StartupPath + "\\AfxHookSource.dll";
        }

        public static bool Launch(CfgLauncherCsgo config)
        {
            String environment = null;

            String programPath = config.CsgoExe;

            String cmdLine = "-steam -game csgo";
            //if (config.AvoidVac)
            cmdLine += " -insecure";
            if (config.GfxEnabled)
                cmdLine += " -w " + config.GfxWidth + " -h " + config.GfxHeight + " " + (config.GfxFull ? "-full" : "-window");
            if (0 < config.CustomLaunchOptions.Length)
                cmdLine += " " + config.CustomLaunchOptions;

            environment = "";
            foreach (DictionaryEntry kv in Environment.GetEnvironmentVariables())
            {
                environment += kv.Key + "=" + kv.Value + "\0";
            }

            if (config.MmcfgEnabled)
            {
                environment += "USRLOCALCSGO=" + config.Mmmcfg+"\0";
            }

            environment += "SteamOverlayGameId=730" + "\0";
            environment += "\0\0";

            return Loader.Load(GetHookPath, programPath, cmdLine, environment);
        }

    }
}
