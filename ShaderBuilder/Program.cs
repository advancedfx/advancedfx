using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace ShaderBuilder
{
    static class Program
    {
        static int Main(string[] args)
        {
            if(0 == args.Length)
            {
                Console.WriteLine("Usage: ShaderBuilder.exe --profile <fxcProfile> --in <inputFile> --outPrefix <outputFilesPrefix>");
                Console.WriteLine();
                Console.WriteLine("<fxcProfile> can be one of:");

                foreach (var value in System.Enum.GetValues(typeof(FxcCompile.Profile)))
                {
                    Console.WriteLine(value);
                }

                return 0;
            }

            try
            {

                string strProfile = null;
                string strInputFile = null;
                string strOutPrefix = null;

                for(int i=0; i<args.Length; ++i)
                {
                    if(args[i].Equals("--profile") && i + 1 < args.Length)
                    {
                        ++i;
                        strProfile = args[i];
                    }
                    else if (args[i].Equals("--in") && i + 1 < args.Length)
                    {
                        ++i;
                        strInputFile = args[i];
                    }
                    else if (args[i].Equals("--outPrefix") && i + 1 < args.Length)
                    {
                        ++i;
                        strOutPrefix = args[i];
                    }
                }

                if(null == strProfile)
                {
                    Console.Error.WriteLine("Error: --profile option missing.");
                    return 1;
                }

                if (null == strInputFile)
                {
                    Console.Error.WriteLine("Error: --in option missing.");
                    return 1;
                }

                if (null == strInputFile)
                {
                    Console.Error.WriteLine("Error: --outPrefix option missing.");
                    return 1;
                }

                FxcCompile.Profile profile;
                if (!System.Enum.TryParse<FxcCompile.Profile>(strProfile, out profile))
                {
                    Console.Error.WriteLine("Error: Invalid profile "+strProfile+".");
                    return 1;
                }

                int lastTick = System.Environment.TickCount;

                FxcCompile fxcCompile = new FxcCompile();

                fxcCompile.Error = (FxcCompile o, string messsage) =>
                {
                    Console.Error.WriteLine("Error: "+messsage);
                };
                fxcCompile.Status = (FxcCompile o, string messsage) =>
                {
                    Console.WriteLine("Status: "+messsage);
                };
                fxcCompile.Progress = (FxcCompile o, double relativeValue) =>
                {
                    int currentTick = System.Environment.TickCount;
                    if(Math.Abs(currentTick -lastTick) >= 200) {
                        lastTick = currentTick;
                        Console.WriteLine("Progress: " + (relativeValue * 100.0).ToString() + "%");
                    }
                };

                if (!fxcCompile.Compile(strInputFile, strOutPrefix, profile))
                {
                    Console.Error.WriteLine("Error: Compile failed!");
                    return 1;
                }

                return 0;
            }
            catch(Exception e)
            {
                Console.Error.WriteLine(e.ToString());
                return 1;
            }
        }
    }
}
