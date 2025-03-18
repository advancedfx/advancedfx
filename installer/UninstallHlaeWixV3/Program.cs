using Microsoft.Win32;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

namespace UninstallHlaeWixV3
{
    class Program
    {
        static int Main(string[] args)
        {
            if (!(args.Length == 3 && args[0].Equals("EXECUTE") && int.TryParse(args[1], out int wixBundleAction) && int.TryParse(args[2], out int hlaeRemoveWixV3) && 3 != wixBundleAction && 1 == hlaeRemoveWixV3)) return 0;

            try
            {
                string systemFolder = Environment.GetFolderPath(Environment.SpecialFolder.System);

                Version versionHlaeWixV5 = new Version(2, 181, 0, 0);

                var productCodes = GetProductCodes(new Guid("{33FEF63B-4A17-4D59-ABC9-B7A06BD07F07}"));

                if (0 == productCodes.Count) return 0;

                var uinstallCodes = new System.Collections.Generic.HashSet<string>();

                using (RegistryKey keyUninstall = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"))
                {
                    if(null != keyUninstall)
                    {
                        foreach (string productCode in keyUninstall.GetSubKeyNames())
                        {
                            if (string.IsNullOrEmpty(productCode)) continue;
                            try
                            {
                                if (productCodes.Contains(new Guid(productCode)))
                                {                                    
                                    using (RegistryKey keyProduct = keyUninstall.OpenSubKey(productCode))
                                    {
                                        if (null != keyProduct)
                                        {
                                            string strVersion = (string)keyProduct.GetValue("DisplayVersion");
                                            if (string.IsNullOrEmpty(strVersion)) continue;

                                            if (new Version(strVersion) < versionHlaeWixV5)
                                            {
                                                uinstallCodes.Add(productCode);
                                            }
                                        }
                                    }
                                }
                            }
                            catch (Exception)
                            {
                                continue;
                            }
                        }
                    }
                }

                foreach(string uinstallCode in uinstallCodes)
                {
                    try
                    {
                        ProcessStartInfo startInfo = new ProcessStartInfo
                        {
                            CreateNoWindow = true,
                            UseShellExecute = false,
                            FileName = systemFolder + "\\msiexec.exe",
                            Arguments = "/x \"" + uinstallCode + "\" /passive /qn"
                        };

                        using (Process exeProcess = Process.Start(startInfo))
                        {
                            exeProcess.WaitForExit();
                            if (exeProcess.ExitCode != 0)
                            {
                                //throw new ApplicationException("Removing product \"" + uinstallCode + "\" failed.");
                            }
                        }

                    }
                    catch (Exception)
                    {

                    }
                }

                return 0;
            }
            catch (Exception)
            {
                //MessageBox.Show("Error: " + e.ToString());
            }        
            return 1;
        }

        // https://stackoverflow.com/a/35753513/26347992

        private const string UpgradeCodeRegistryKey = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UpgradeCodes";

        private static readonly int[] GuidRegistryFormatPattern = new[] { 8, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2 };

        public static System.Collections.Generic.ICollection<Guid> GetProductCodes(Guid upgradeCode)
        {
            // Convert the product code to the format found in the registry
            var productCodeSearchString = ConvertToRegistryFormat(upgradeCode);

            // Open the upgrade code registry key
            var upgradeCodeRegistryRoot = GetRegistryKey(Path.Combine(UpgradeCodeRegistryKey, productCodeSearchString));

            if (upgradeCodeRegistryRoot == null)
                return null;

            var dict = new System.Collections.Generic.HashSet<Guid>();

            foreach(var uninstallCode in upgradeCodeRegistryRoot.GetValueNames()) {
                if (string.IsNullOrEmpty(uninstallCode)) continue;

                try {
                    // Convert it back to a Guid
                    dict.Add(ConvertFromRegistryFormat(uninstallCode));
                }
                catch(Exception) {

                }
            }
            return dict;
        }

        private static string ConvertToRegistryFormat(Guid productCode)
        {
            return Reverse(productCode, GuidRegistryFormatPattern);
        }

        private static Guid ConvertFromRegistryFormat(string upgradeCode)
        {
            if (upgradeCode == null || upgradeCode.Length != 32)
                throw new FormatException("Product code was in an invalid format");

            upgradeCode = Reverse(upgradeCode, GuidRegistryFormatPattern);

            return Guid.Parse(upgradeCode);
        }

        private static string Reverse(object value, params int[] pattern)
        {
            // Strip the hyphens
            var inputString = value.ToString().Replace("-", "");

            var returnString = new StringBuilder();

            var index = 0;

            // Iterate over the reversal pattern
            foreach (var length in pattern)
            {
                // Reverse the sub-string and append it
                returnString.Append(inputString.Substring(index, length).Reverse().ToArray());

                // Increment our posistion in the string
                index += length;
            }

            return returnString.ToString();
        }

        static RegistryKey GetRegistryKey(string registryPath)
        {
            var hklm64 = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64);
            var registryKey64 = hklm64.OpenSubKey(registryPath);
            if (((bool?)registryKey64?.GetValueNames()?.Any()).GetValueOrDefault())
            {
                return registryKey64;
            }

            var hklm32 = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32);
            return hklm32.OpenSubKey(registryPath);
        }
    }
}
