using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ShaderDisassembler
{
    class Program
    {
        static int Main(string[] args)
        {
            if(args.Length < 2)
            {
                Console.WriteLine("Usage: ShaderDisassembler.exe <shaderFile> <outPutFile>");
                return 0;
            }

            try
            {
                SharpDX.D3DCompiler.ShaderBytecode sb = SharpDX.D3DCompiler.ShaderBytecode.FromFile(args[0]);

                string code = sb.Disassemble(
                    /* SharpDX.D3DCompiler.DisassemblyFlags.EnableColorCode */
                    SharpDX.D3DCompiler.DisassemblyFlags.EnableDefaultValuePrints
                    /*| SharpDX.D3DCompiler.DisassemblyFlags.EnableInstructionCycle // this will cause an error, lol */
                    | SharpDX.D3DCompiler.DisassemblyFlags.EnableInstructionNumbering
                );

                System.IO.StreamWriter sw = new System.IO.StreamWriter(args[1]);

                sw.Write(code);

                sw.Close();

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
