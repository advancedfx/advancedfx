using System;
using System.Security.Principal;

namespace DeleteHlaeAppData
{
    class Program
    {
        static void Main(string[] args)
        {
            int remove = 0;            

            if (args.Length == 2 && args[0].Equals("EXECUTE") && int.TryParse(args[1], out remove) && 1 == remove)
            {
                try
                {
                    string hlaeAppDataPath = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "HLAE");

                    if (System.IO.Directory.Exists(hlaeAppDataPath)) System.IO.Directory.Delete(hlaeAppDataPath, true);
                }
                catch (Exception e)
                {
                }
            }
        }
    }
}
