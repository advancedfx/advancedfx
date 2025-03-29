using System;
using System.Security.Principal;

namespace DeleteHlaeAppData
{
    class Program
    {
        static void Main(string[] args)
        {     
            if (args.Length == 3 && args[0].Equals("EXECUTE") && int.TryParse(args[1], out int wixBundleAction) && int.TryParse(args[2], out int hlaeRemoveAppData) && 3 == wixBundleAction && 1 == hlaeRemoveAppData)
            {
                try
                {
                    string hlaeAppDataPath = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "HLAE");

                    if (System.IO.Directory.Exists(hlaeAppDataPath)) System.IO.Directory.Delete(hlaeAppDataPath, true);
                }
                catch (Exception)
                {
                }
            }
        }
    }
}
