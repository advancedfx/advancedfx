using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Net;

namespace AfxGui
{
    static class Program
    {


        //
        // Internal members:

        internal static String BaseDir
        {
            get
            {
                return m_BaseDir;
            }
        }

        internal static String AppDataDir
        {
            get
            {
                return m_AppDataDir;
            }
        }

        internal static System.Drawing.Icon Icon
        {
            get
            {
                return m_Icon;
            }
        }

        internal static string SteamInstallPath
        {
            get
            {
                return m_SteamInstallPath;
            }
        }

        //
        // Private members:

        static String m_BaseDir;
        static String m_AppDataDir;
        static System.Drawing.Icon m_Icon;
        static string m_SteamInstallPath;
        static bool m_CustomLoaderHadHookDllPath = false;

        static void ProcessArgsAfxHookGoldSrc(string[] args)
        {
            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                switch (arg)
                {
                    case "-autoStart":
                        Globals.AutoStartAfxHookGoldSrc = true;
                        break;
                    case "-noGui":
                        Globals.NoGui = true;
                        break;
                    case "-gamePath":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.Launcher.GamePath = args[i + 1];
                            i++;
                        }
                        break;
                    case "-modification":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.Launcher.Modification = args[i + 1];
                            i++;
                        }
                        break;
                    case "-customCmdLine":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.Launcher.CustomCmdLine = args[i + 1];
                            i++;
                        }
                        break;
                    case "-gfxForce":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.GfxForce);
                            i++;
                        }
                        break;
                    case "-gfxWidth":
                        if (i + 1 < args.Length)
                        {
                            UInt16.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.GfxWidth);
                            i++;
                        }
                        break;
                    case "-gfxHeight":
                        if (i + 1 < args.Length)
                        {
                            UInt16.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.GfxHeight);
                            i++;
                        }
                        break;
                    case "-gfxBpp":
                        if (i + 1 < args.Length)
                        {
                            Byte.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.GfxBpp);
                            i++;
                        }
                        break;
                    //case "-rememberChanges":
                    case "-foreceAlpha":
                    case "-forceAlpha":
                        if (arg == "-foreceAlpha")
                            MessageBox.Show("-foreceAlpha is deprecated, use -forceAlpha instead!", "Deprecated launch option!", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1);

                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.ForceAlpha);
                            i++;
                        }
                        break;
                    case "-optimizeDesktopRes":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.OptimizeDesktopRes);
                            i++;
                        }
                        break;
                    case "-optimizeVisibilty":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.OptimizeVisibilty);
                            i++;
                        }
                        break;
                    case "-fullScreen":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.FullScreen);
                            i++;
                        }
                        break;
                    case "-renderMode":
                        if (i + 1 < args.Length)
                        {
                            Byte.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.Launcher.RenderMode);
                            i++;
                        }
                        break;
                }
            }
        }

        static void ProcessArgsCustomLoader(string[] args)
        {
            bool firstEnv = true;

            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                switch (arg)
                {
                    case "-autoStart":
                        Globals.AutoStartCustomLoader = true;
                        break;
                    case "-noGui":
                        Globals.NoGui = true;
                        break;
                    case "-hookDllPath":
                        if (i + 1 < args.Length)
                        {
                            if(!m_CustomLoaderHadHookDllPath)
                            {
                                GlobalConfig.Instance.Settings.CustomLoader.InjectDlls.Clear();

                                m_CustomLoaderHadHookDllPath = true;
                            }

                            CfgInjectDll dll = new CfgInjectDll();

                            dll.Path = args[i + 1];
                            i++;

                            GlobalConfig.Instance.Settings.CustomLoader.InjectDlls.Add(dll);
                        }
                        break;
                    case "-programPath":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.CustomLoader.ProgramPath = args[i + 1];
                            i++;
                        }
                        break;
                    case "-cmdLine":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.CustomLoader.CmdLine = args[i + 1];
                            i++;
                        }
                        break;
                    case "-addEnv":
                        if (i + 1 < args.Length)
                        {
                            if (firstEnv)
                            {
                                firstEnv = false;

                                GlobalConfig.Instance.Settings.CustomLoader.AddEnvironmentVars = "";
                            }
                            GlobalConfig.Instance.Settings.CustomLoader.AddEnvironmentVars += args[i + 1] + "\n";
                            i++;
                        }
                        break;
                }
            }
        }

        static void ProcessArgsCsgoLauncher(string[] args)
        {
            for (int i = 0; i < args.Length; i++)
            {
                string arg = args[i];
                switch (arg)
                {
                    case "-autoStart":
                        Globals.AutoStartCsgo = true;
                        break;
                    case "-noGui":
                        Globals.NoGui = true;
                        break;
                    case "-csgoExe":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.LauncherCsgo.CsgoExe = args[i + 1];
                            i++;
                        }
                        break;
                    case "-mmcfgEnabled":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.LauncherCsgo.MmcfgEnabled);
                            i++;
                        }
                        break;
                    case "-mmcfg":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.LauncherCsgo.Mmmcfg = args[i + 1];
                            i++;
                        }
                        break;
                    case "-gfxEnabled":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.LauncherCsgo.GfxEnabled);
                            i++;
                        }
                        break;
                    case "-gfxWidth":
                        if (i + 1 < args.Length)
                        {
                            UInt16.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.LauncherCsgo.GfxWidth);
                            i++;
                        }
                        break;
                    case "-gfxHeight":
                        if (i + 1 < args.Length)
                        {
                            UInt16.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.LauncherCsgo.GfxHeight);
                            i++;
                        }
                        break;
                    case "-gfxFull":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.LauncherCsgo.GfxFull);
                            i++;
                        }
                        break;
                    case "-avoidVac":
                        if (i + 1 < args.Length)
                        {
                            Boolean.TryParse(args[i + 1], out GlobalConfig.Instance.Settings.LauncherCsgo.AvoidVac);
                            i++;
                        }
                        break;
                    case "-customLaunchOptions":
                        if (i + 1 < args.Length)
                        {
                            GlobalConfig.Instance.Settings.LauncherCsgo.CustomLaunchOptions = args[i + 1];
                            i++;
                        }
                        break;
                }
            }
        }

        static void ProcessCommandLine()
        {
            string [] argv = new string[0];
            
            try
            {
                argv = System.Environment.GetCommandLineArgs();
            }
            catch(NotSupportedException)
            {
            }

            if (0 < argv.Length)
            {
                string[] tmp = new string[argv.Length - 1];
                Array.Copy(argv, 1, tmp, 0, argv.Length - 1);
                argv = tmp;
            }

            if (Array.Exists<string>(argv, p => p == "-noConfig"))
            {
				GlobalConfig.Instance = new Config();
				Globals.NoConfig = true;
            }

            if (Array.Exists<string>(argv, p => p == "-customLoader"))
            {
                ProcessArgsCustomLoader(argv);
            }
            else if (Array.Exists<string>(argv, p => p == "-csgoLauncher"))
            {
                ProcessArgsCsgoLauncher(argv);
            }
        }

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static int Main()
        {
            ServicePointManager.SecurityProtocol = (SecurityProtocolType)3072; // Force TLS v1.2 (Will only work where .NET 4.5 is installed).

            // Use default system proxy in case we use WebRequest (which we don't) atm:
            System.Net.WebRequest.DefaultWebProxy = System.Net.WebRequest.GetSystemWebProxy();
            System.Net.WebRequest.DefaultWebProxy.Credentials = System.Net.CredentialCache.DefaultNetworkCredentials;

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            m_BaseDir = System.IO.Path.GetFullPath(System.Windows.Forms.Application.StartupPath).TrimEnd('\\','/');
            m_AppDataDir = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "HLAE");

            const string defaultSteamPath = "C:\\Program Files (x86)\\Steam";

            try
            {
                m_SteamInstallPath = Microsoft.Win32.Registry.GetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath", defaultSteamPath) as string;
            }
            catch
            {
                m_SteamInstallPath = null;
            }
            if (null == m_SteamInstallPath) m_SteamInstallPath = defaultSteamPath;

            if (!System.IO.Directory.Exists(m_AppDataDir)) System.IO.Directory.CreateDirectory(m_AppDataDir);

            m_Icon = System.Drawing.Icon.ExtractAssociatedIcon(System.Windows.Forms.Application.ExecutablePath);

            GlobalConfig.Instance = Config.LoadOrCreate(System.IO.Path.Combine(m_AppDataDir, "hlaeconfig.xml"), System.IO.Path.Combine(m_BaseDir, "hlaeconfig.xml"));
            GlobalUpdateCheck.Instance = new UpdateCheck();

            ProcessCommandLine();

            ////

            bool bOk = true;

            // start-up CS:GO if requested (i.e. by command line)
            if (Globals.AutoStartCsgo)
            {
                if (!LaunchCsgo.Launch(GlobalConfig.Instance.Settings.LauncherCsgo))
                    bOk = false;
            }

            // start-up AfxHookGoldSrc if requested (i.e. by command line)
            if (Globals.AutoStartAfxHookGoldSrc)
            {
                if (!Launcher.Launch(GlobalConfig.Instance.Settings.Launcher))
                    bOk = false;
            }

            // start-up CustomLoader if requested (i.e. by command line)
            if (Globals.AutoStartCustomLoader)
            {
                List<Loader.GetHookPathDelegate> getHookPaths = new List<Loader.GetHookPathDelegate>();
                foreach (CfgInjectDll dll in GlobalConfig.Instance.Settings.CustomLoader.InjectDlls) getHookPaths.Add(isProcess64Bit => dll.FullPath);

                string[] envVars = GlobalConfig.Instance.Settings.CustomLoader.AddEnvironmentVars.Split(new char[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
                string environment = null;
                foreach (string line in envVars)
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

                if (!Loader.Load(getHookPaths, GlobalConfig.Instance.Settings.CustomLoader.ProgramPath, GlobalConfig.Instance.Settings.CustomLoader.CmdLine, environment))
                    bOk = false;
            }

            ////

            if (!Globals.NoGui)
            {
                Application.Run(new MainForm());
            }
			
			if(!Globals.NoConfig)
			{
				GlobalConfig.Instance.BackUp();
			}

            GlobalUpdateCheck.Instance.Dispose();

            return bOk ? 0 : 1;
        }
    }
}
