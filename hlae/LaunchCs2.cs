using System;
using System.Collections;
using System.Windows.Forms;

namespace AfxGui
{
    class LaunchCs2
    {
        public static bool RunLauncherDialog(IWin32Window dialogOwner)
        {
            bool bOk;

            using (LaunchCs2Form frm = new LaunchCs2Form())
            {
                frm.Config = GlobalConfig.Instance.Settings.LauncherCs2;

                if (DialogResult.OK == frm.ShowDialog(dialogOwner))
                {
                    CfgLauncherCs2 cfg = frm.Config;

                    if (cfg.RememberChanges)
                    {
                        GlobalConfig.Instance.Settings.LauncherCs2 = cfg;
                    }
                    else
                    {
                        GlobalConfig.Instance.Settings.LauncherCs2.RememberChanges = cfg.RememberChanges;
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
#if DEBUG
            return System.Windows.Forms.Application.StartupPath + "\\x64\\AfxHookSource2_d.dll";
#else
            return System.Windows.Forms.Application.StartupPath + "\\x64\\AfxHookSource2.dll";
#endif
        }

        public static bool Launch(CfgLauncherCs2 config)
        {
            String environment = null;

            String programPath = config.Cs2Exe;

            String cmdLine = "-steam -insecure";

            if (config.GfxEnabled)
                cmdLine += " " + (config.GfxFull ? "-full" : "-sw") + " -w " + config.GfxWidth + " -h " + config.GfxHeight;

            if (config.MmcfgEnabled)
			{
				cmdLine += " -afxDisableSteamStorage";
			}

            if (0 < config.CustomLaunchOptions.Length)
                cmdLine += " " + config.CustomLaunchOptions;

            environment = "";
            foreach (DictionaryEntry kv in Environment.GetEnvironmentVariables())
            {
                environment += kv.Key + "=" + kv.Value + "\0";
            }

            if (config.MmcfgEnabled)
            {
                environment += "USRLOCALCSGO=" + config.Mmcfg+"\0";
            }

            environment += "SteamPath=" + Program.SteamInstallPath + "\0";
            environment += "SteamClientLaunch=1" + "\0";
            environment += "SteamGameId=730" + "\0";
            environment += "SteamAppId=730" + "\0";
            environment += "SteamOverlayGameId=730" + "\0";
            environment += "\0\0";

            return Loader.Load(GetHookPath, programPath, cmdLine, environment);
        }

    }
}
