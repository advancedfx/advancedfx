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

        internal static System.Drawing.Icon Icon
        {
            get
            {
                return m_Icon;
            }
        }


        //
        // Private members:

        static String m_BaseDir;
        static System.Drawing.Icon m_Icon;
        static bool m_CustomLoaderHadHookDllPath = false;

        static void ProcessArgsCustomLoader(string[] args)
        {
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
                }
            }

            // backup potential config changes:
            GlobalConfig.Instance.BackUp();
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

            // backup potential config changes:
            GlobalConfig.Instance.BackUp();
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

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            m_BaseDir = System.Windows.Forms.Application.StartupPath;

            m_Icon = System.Drawing.Icon.ExtractAssociatedIcon(System.Windows.Forms.Application.ExecutablePath);

            GlobalConfig.Instance = Config.LoadOrCreate(String.Concat(m_BaseDir, "\\hlaeconfig.xml"));
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

            // start-up CustomLoader if requested (i.e. by command line)
            if (Globals.AutoStartCustomLoader)
            {
                List<Loader.GetHookPathDelegate> getHookPaths = new List<Loader.GetHookPathDelegate>();
                foreach (CfgInjectDll dll in GlobalConfig.Instance.Settings.CustomLoader.InjectDlls) getHookPaths.Add(isProcess64Bit => dll.Path);

                if (!Loader.Load(getHookPaths, GlobalConfig.Instance.Settings.CustomLoader.ProgramPath, GlobalConfig.Instance.Settings.CustomLoader.CmdLine))
                    bOk = false;
            }

            ////

            if (!Globals.NoGui)
            {
                Application.Run(new MainForm());
            }

            GlobalConfig.Instance.BackUp();

            GlobalUpdateCheck.Instance.Dispose();

            return bOk ? 0 : 1;
        }
    }
}
