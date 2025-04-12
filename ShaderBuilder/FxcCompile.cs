using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text.RegularExpressions;
using System.Threading;
using System.Runtime.InteropServices;

namespace ShaderBuilder {

class FxcCompile
{
    //
    // Public members:

    public enum Profile
    {
        vs_2_0,
        ps_2_0,
        ps_2_b,
        vs_3_0,
        ps_3_0,
		vs_5_0,
		ps_5_0,
        cs_5_0
    }

    public delegate void MessageDelegate(FxcCompile o, string message);

    public delegate void ProgressDelegate(FxcCompile o, double relativeValue);

    public MessageDelegate Error;

    public MessageDelegate Status;

    public ProgressDelegate Progress;

    public FxcCompile()
    {
        InitProgressSections();
    }

    public bool Compile(string filePath, string outputPrefix, Profile profile)
    {
        try
        {
            PreventSleepBegin();

            DoProgress(0, 0);
            DoStatus("Started compilation ...");

            m_Profile = profile;

            m_Source.Clear();

            if (!ReadInputFile(filePath))
                return false;

            if (!FilterComobos())
                return false;

            try
            {
                m_Expression = new SimplePerlExpression(m_PerlSkipCode);
            }
            catch (ApplicationException e)
            {
                DoError("Error when creating SimplePerlExpression: " + e.ToString());
                return false;
            }

            if (!WriteHeaderFile(outputPrefix))
                return false;

            if (!CompileCombos())
                return false;

            if (!WriteCompileResults(outputPrefix))
                return false;

            foreach (var v in m_CompilationResults.Values)
            {
                v.Dispose();
            }
        }
        finally
        {
            PreventSleepEnd();
        }

        DoStatus("Finished compilation successfully.");
        return true;
    }

    //
    // Private members:

    private double[] m_ProgressSectionWeights = new double[6] {
        0.05, // ReadInputFile
        0.05, // FilterComobos
        0.4, // CompileCombos queue combos
        0.2, // CompileCombos wait finish
        0.1, // WriteCompileResults.Indices
        0.2 // WriteCompileResults.Shaders
    };

    private struct Combo
    {
        public string Name;
        public int Min;
        public int Max;

        public Combo(string name, int min, int max)
        {
            Name = name;
            Min = min;
            Max = max;
        }

        public override bool Equals(object obj)
        {
            return Name.Equals(obj);
        }

        public override int GetHashCode()
        {
            return Name.GetHashCode();
        }
    }

    private enum Platform
    {
        PC,
        XBOX
    }

    [FlagsAttribute]
    public enum EXECUTION_STATE : uint
    {
        ES_AWAYMODE_REQUIRED = 0x00000040,
        ES_CONTINUOUS = 0x80000000,
        ES_DISPLAY_REQUIRED = 0x00000002,
        ES_SYSTEM_REQUIRED = 0x00000001
        // Legacy flag, should not be used.
        // ES_USER_PRESENT = 0x00000004
    }

    private class CompileThreadClass
        : SharpDX.D3DCompiler.Include
    {
        public CompileThreadClass(
            FxcCompile fxcCompile,
            string shaderSource,
            string profile,
            SharpDX.Direct3D.ShaderMacro[] defines,
            int comboId
            )
        {
            m_FxcCompile = fxcCompile;
            m_ShaderSource = shaderSource;
            m_Profile = profile;
            m_Defines = defines;
            m_ComboId = comboId;

            Interlocked.Increment(ref fxcCompile.m_OutstandingCompiles);
        }

        public void DoCompile()
        {
            if (!m_FxcCompile.m_CompileError) // don't compile more stuff if we already have a known error
            {
                SharpDX.D3DCompiler.CompilationResult result = null;
                string exception = null;

                try
                {
                    result = SharpDX.D3DCompiler.ShaderBytecode.Compile(
                         m_ShaderSource,
                         "main",
                         m_Profile,
                         SharpDX.D3DCompiler.ShaderFlags.OptimizationLevel3,
                         SharpDX.D3DCompiler.EffectFlags.None,
                         m_Defines,
                         this,
                         "shaderCombo_" + m_ComboId
                     );
                }
                catch (SharpDX.SharpDXException e)
                {
                    exception = e.ToString();
                    result = null;
                }

                if (null == result || result.HasErrors)
                {
                    m_FxcCompile.m_CompileError = true;

                    string compileErrors =
                        "shaderCombo_" + m_ComboId + ": "
                        + "message: " + (null == result ? "[none]" : result.Message)
                        + ", exception: " + (null == exception ? "[none]" : exception)
                        + ", defines:"
                    ;

                    foreach (SharpDX.Direct3D.ShaderMacro define in m_Defines)
                    {
                        compileErrors += " /D" + define.Name.ToString() + "=" + define.Definition.ToString();
                    }

                    lock (m_FxcCompile.m_CompileErrors)
                    {
                        m_FxcCompile.m_CompileErrors.AddLast(
                            compileErrors
                        );
                    }

                    result = null;
                }

                lock (m_FxcCompile.m_CompilationResults)
                {
                    m_FxcCompile.m_CompilationResults[m_ComboId] = result;
                }
            }

            Interlocked.Decrement(ref m_FxcCompile.m_OutstandingCompiles);
        }

        private FxcCompile m_FxcCompile;
        private string m_ShaderSource;
        private string m_Profile;
        private SharpDX.Direct3D.ShaderMacro[] m_Defines;
        private int m_ComboId;

        public void Close(System.IO.Stream stream)
        {
            throw new NotImplementedException();
        }

        public System.IO.Stream Open(SharpDX.D3DCompiler.IncludeType type, string fileName, System.IO.Stream parentStream)
        {
            throw new NotImplementedException();
        }

        public IDisposable Shadow
        {
            get
            {
                return m_Shadow;
            }
            set
            {
                m_Shadow = value;
            }
        }

        public void Dispose()
        {
            if(null != m_Shadow) m_Shadow.Dispose();
        }

        private IDisposable m_Shadow;
    };

    private double[] m_ProgressSectionStarts;

    private Profile m_Profile;

    private Platform m_Platform = Platform.PC;

    private bool m_SFM = false;

    private LinkedList<string> m_Source = new LinkedList<string>();

    private LinkedList<Combo> m_StaticCombos = new LinkedList<Combo>();

    private LinkedList<Combo> m_DynamicCombos = new LinkedList<Combo>();

    private LinkedList<Combo> m_AfxCombos = new LinkedList<Combo>();

    private string m_PerlSkipCode;

    private SimplePerlExpression m_Expression;

    private decimal m_NumCombos;
    private decimal m_NumAfxCombos;
    private decimal m_NumDynamicCombos;

    private SortedDictionary<int, SharpDX.D3DCompiler.CompilationResult> m_CompilationResults = new SortedDictionary<int, SharpDX.D3DCompiler.CompilationResult>();

    private int m_OutstandingCompiles;
    private bool m_CompileError;
    private LinkedList<string> m_CompileErrors = new LinkedList<string>();

    int m_LastProgress;
    int m_LastStatus;

    [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    static extern EXECUTION_STATE SetThreadExecutionState(EXECUTION_STATE esFlags);

    private static void PreventSleepBegin()
    {
        SetThreadExecutionState(EXECUTION_STATE.ES_CONTINUOUS | EXECUTION_STATE.ES_SYSTEM_REQUIRED);
    }

    private static void PreventSleepEnd()
    {
        SetThreadExecutionState(EXECUTION_STATE.ES_CONTINUOUS);
    }

    private void InitProgressSections()
    {
        double totalSize = 0.0;

        foreach (double val in m_ProgressSectionWeights)
        {
            totalSize += val;
        }

        m_ProgressSectionStarts = new double[m_ProgressSectionWeights.Length];

        double size = 0.0;

        for (int i = 0; i < m_ProgressSectionWeights.Length; ++i)
        {
            m_ProgressSectionStarts[i] = size;
            size += (m_ProgressSectionWeights[i] = (0.0 != totalSize ? m_ProgressSectionWeights[i] / totalSize : 0.0));
        }
    }

    private void DoError(string description)
    {
        if (null != Error) Error(this, description);
        DoStatus("Error occured!");
    }

    private void DoStatus(string description, bool reliable = true)
    {
        int cur = System.Environment.TickCount;
        if (!reliable && 100 > Math.Abs(cur -m_LastStatus))
            return;

        m_LastStatus = cur;

        if (null != Status) Status(this, description);
    }

    private void DoProgress(int section, double subProgress, bool reliable = true)
    {
        int cur = System.Environment.TickCount;
        if (!reliable && 100 > Math.Abs(cur - m_LastProgress))
            return;

        m_LastProgress = cur;

        if (null == Progress)
            return;

        double value = m_ProgressSectionStarts[section] + subProgress * m_ProgressSectionWeights[section];
        Progress(this, value);
    }

    private bool WriteHeaderFile(string outputPrefix)
    {
        DoStatus("Writing header file ...");

        System.IO.StreamWriter sw = null;

        string className = outputPrefix.Split(new char[2] { '/', '\\' }).Last();
        string compileTime = System.DateTime.UtcNow.ToString("yyyy'-'MM'-'dd'T'HH':'mm':'ss'.'fff'Z'", System.Globalization.CultureInfo.InvariantCulture);

        try
        {
            sw = new System.IO.StreamWriter(outputPrefix + ".h");

            sw.WriteLine("#pragma once");
            sw.WriteLine();
            sw.WriteLine("////////////////////////////////////////////////");
            sw.WriteLine("// AUTOGENERATED FILE, CHANGES WILL GET LOST! //");
            sw.WriteLine("////////////////////////////////////////////////");
            sw.WriteLine();
            sw.WriteLine("// Copyright (c) advancedfx.org");
            sw.WriteLine("//");
            sw.WriteLine("// Last changes:");
            sw.WriteLine("// " + compileTime + " ShaderBuilder (auto)");
            sw.WriteLine("//");
            sw.WriteLine("// First changes:");
            sw.WriteLine("// " + compileTime + " ShaderBuilder (auto)");
            sw.WriteLine();
            sw.WriteLine("class ShaderCombo_" + className);
            sw.WriteLine("{");

            sw.WriteLine("public:");

            foreach (var combo in m_AfxCombos)
            {
                sw.WriteLine("\tenum " + combo.Name + "_e");
                sw.WriteLine("\t{");

                bool first = true;
                for (int i = combo.Min; i <= combo.Max; ++i)
                {
                    sw.WriteLine("\t\t" + (first ? "" : ", ") + combo.Name + "_" + i+" = "+i);
                    first = false;
                }

                sw.WriteLine("\t};");
            }
            foreach (var combo in m_DynamicCombos)
            {
                sw.WriteLine("\tenum " + combo.Name + "_e");
                sw.WriteLine("\t{");

                bool first = true;
                for (int i = combo.Min; i <= combo.Max; ++i)
                {
                    sw.WriteLine("\t\t" + (first ? "" : ", ") + combo.Name + "_" + i + " = " + i);
                    first = false;
                }

                sw.WriteLine("\t};");
            }
            foreach (var combo in m_StaticCombos)
            {
                sw.WriteLine("\tenum " + combo.Name + "_e");
                sw.WriteLine("\t{");

                bool first = true;
                for (int i = combo.Min; i <= combo.Max; ++i)
                {
                    sw.WriteLine("\t\t" + (first ? "" : ", ") + combo.Name + "_" + i + " = " + i);
                    first = false;
                }

                sw.WriteLine("\t};");
            }

            {
                sw.WriteLine("\tstatic int GetCombo(");

                bool first = true;

                foreach (var combo in m_AfxCombos)
                {
                    sw.WriteLine("\t\t" + (first ? "" : ", ") + combo.Name + "_e a_" + combo.Name);
                    first = false;
                }
                foreach (var combo in m_DynamicCombos)
                {
                    sw.WriteLine("\t\t" + (first ? "" : ", ") + combo.Name + "_e a_" + combo.Name);
                    first = false;
                }
                foreach (var combo in m_StaticCombos)
                {
                    sw.WriteLine("\t\t" + (first ? "" : ", ") + combo.Name + "_e a_" + combo.Name);
                    first = false;
                }
                sw.WriteLine("\t\t)");

                sw.WriteLine("\t{");

                int scale = 1;

                sw.WriteLine("\t\treturn 0");
                foreach (var combo in m_AfxCombos)
                {
                    sw.WriteLine("\t\t\t+(a_" + combo.Name + " - " + combo.Min + ") * " + scale);
                    scale *= combo.Max - combo.Min + 1;
                }
                foreach (var combo in m_DynamicCombos)
                {
                    sw.WriteLine("\t\t\t+(a_" + combo.Name + " - " + combo.Min + ") * " + scale);
                    scale *= combo.Max - combo.Min + 1;
                }
                foreach (var combo in m_StaticCombos)
                {
                    sw.WriteLine("\t\t\t+(a_" + combo.Name + " - " + combo.Min + ") * " + scale);
                    scale *= combo.Max - combo.Min + 1;
                }
                sw.WriteLine("\t\t\t;");

                sw.WriteLine("\t}");
            }

            sw.WriteLine("};");

            sw.Close();
        }
        catch (System.IO.IOException e)
        {
            DoError("Error writing header file: " + e);
            if (null != sw) sw.Close();

            return false;
        }

        return true;
    }

    private void CalcNumCombos()
    {
        m_NumCombos = 1;
        m_NumAfxCombos = 1;
        m_NumDynamicCombos = 1;

        foreach(Combo combo in m_AfxCombos)
        {
            decimal d = combo.Max - combo.Min + 1;
            m_NumCombos *= d;
            m_NumAfxCombos *= d;
        }
        
        foreach (Combo combo in m_DynamicCombos)
        {
            decimal d = combo.Max - combo.Min + 1;
            m_NumCombos *= d;
            m_NumDynamicCombos *= d;
        }

        foreach (Combo combo in m_StaticCombos)
        {
            decimal d = combo.Max - combo.Min + 1;
            m_NumCombos *= d;
        }
    }

    private void DefineCombos(int num)
    {
        foreach (Combo combo in m_AfxCombos)
        {
            int d = num % (combo.Max - combo.Min + 1) + combo.Min;
            m_Expression.DefineVariable(combo.Name, (int)d);
            num = num / (combo.Max - combo.Min + 1);
        }

        foreach (Combo combo in m_DynamicCombos)
        {
            int d = num % (combo.Max - combo.Min + 1) + combo.Min;
            m_Expression.DefineVariable(combo.Name, (int)d);
            num = num / (combo.Max - combo.Min + 1);
        }

        foreach (Combo combo in m_StaticCombos)
        {
            int d = num % (combo.Max - combo.Min + 1) + combo.Min;
            m_Expression.DefineVariable(combo.Name, (int)d);
            num = num / (combo.Max - combo.Min + 1);
        }
    }

    private SharpDX.Direct3D.ShaderMacro[] MakeMacros(int num)
    {
        SharpDX.Direct3D.ShaderMacro[] macros = new SharpDX.Direct3D.ShaderMacro[m_AfxCombos.Count + m_DynamicCombos.Count + m_StaticCombos.Count + 1];

        int idx = 0;

        foreach (Combo combo in m_AfxCombos)
        {
            int d = num % (combo.Max - combo.Min + 1) + combo.Min;
            macros[idx++] = new SharpDX.Direct3D.ShaderMacro(combo.Name, d);
            num = num / (combo.Max - combo.Min + 1);
        }

        foreach (Combo combo in m_DynamicCombos)
        {
            int d = num % (combo.Max - combo.Min + 1) + combo.Min;
            macros[idx++] = new SharpDX.Direct3D.ShaderMacro(combo.Name, d);
            num = num / (combo.Max - combo.Min + 1);
        }

        foreach (Combo combo in m_StaticCombos)
        {
            int d = num % (combo.Max - combo.Min + 1) + combo.Min;
            macros[idx++] = new SharpDX.Direct3D.ShaderMacro(combo.Name, d);
            num = num / (combo.Max - combo.Min + 1);
        }

        macros[idx++] = new SharpDX.Direct3D.ShaderMacro("SHADER_MODEL_" + m_Profile.ToString().ToUpper(), 1);

        return macros;
    }

    private void WriteSourceString(out string outString)
    {
        outString = "";

        foreach (string line in m_Source)
        {
            outString += line + System.Environment.NewLine;
        }
    }

    private static string EscapeCmdArgument(string arg)
    {
        return "\"" + arg.Replace ("\\", "\\\\").Replace("\"", "\\\"") + "\"";

    }

    private static void CompileThreadProc(object stateInfo)
    {
        CompileThreadClass ctc = (CompileThreadClass)stateInfo;

        ctc.DoCompile();
    }

    private bool CompileCombos()
    {
        DoProgress(2, 0);
        DoStatus("Compiling combos ...");

        m_CompilationResults.Clear();
        m_CompileError = false;
        m_CompileErrors.Clear();

        CalcNumCombos();

        if (m_NumCombos > 0x7fffffff)
        {
            DoError("Too many combos.");
            return false;
        }

        string sourceString;
        string profileString = m_Profile.ToString();

        WriteSourceString(out sourceString);

        int numCombos = (int)m_NumCombos;
        int queuedComobos = 0;

        for (int i = 0; i < numCombos; ++i)
        {
            DoProgress(2, (double)i / (double)m_NumCombos, false);

            DefineCombos(i);

            bool skip = m_Expression.Eval();

            if (!skip)
            {
                // abort as we learn about past compile errors:
                if (m_CompileError)
                    break;

                DoStatus("Queueing combo " + (i + 1) + "/" + m_NumCombos + " for compilation ...", false);

                CompileThreadClass ctc = new CompileThreadClass(
                    this,
                    sourceString,
                    profileString,
                    MakeMacros(i),
                    i
                    );

                if (!ThreadPool.QueueUserWorkItem(
                    CompileThreadProc,
                    ctc
                ))
                {
                    DoError("Failed to queue on ThreadPool for shaderCombo_" + i);
                    return false;
                }

                ++queuedComobos;
            }
        }

        int remainingCompiles = 0;

        while (0 != (remainingCompiles = Interlocked.CompareExchange(ref m_OutstandingCompiles, 0, 0)))
        {
            DoProgress(3, (double)(queuedComobos - remainingCompiles) / (double)queuedComobos);
            DoStatus("Waiting for combo compilation to finish (" + remainingCompiles + " remaining) ...");
            Thread.Sleep(100);
        }

        if (0 < m_CompileErrors.Count)
        {
            DoError("Found compile errors, limiting to one error:");
            DoError(m_CompileErrors.First.Value);

            return false;
        }

        DoProgress(3, 1);

        return true;
    }

    private bool WriteCompileResults(string outputPrefix)
    {
        DoProgress(4, 0);
        DoStatus("Starting to write results ...");

        // One could think about finding duplicate shaders here,
        // but we can skip this step, because the compiler is
        // not deterministic (probably due to timestamp or s.th.
        // like that).

        DoStatus("Writing .acs shader file ...");

        System.IO.BinaryWriter bw = null;

        try
        {
            System.IO.Stream fs = new System.IO.FileStream(outputPrefix + ".acs", System.IO.FileMode.Create);
            bw = new System.IO.BinaryWriter(fs);

            int acsHeaderSize =
                4 * 1 // version
                + 4 * 1 // index size
                + 4 * 2 * m_CompilationResults.Count // indices
                ;

            bw.Write((int)0); // Version.

            DoStatus("Writing .acs index tree ...");
            {

                bw.Write((int)(m_CompilationResults.Count)); // Index size

                int i = 0;
                int curOfs = acsHeaderSize;

                foreach (var kv in m_CompilationResults)
                {
                    DoProgress(4, (double)i / (double)m_CompilationResults.Count, false);

                    bw.Write((int)kv.Key);
                    bw.Write((int)curOfs);

                    curOfs += kv.Value.Bytecode.Data.Length;
                    ++i;
                }
            }

            DoProgress(5, 0);
            DoStatus("Writing .acs shaders ...");
            {
                int i = 0;

                foreach (var v in m_CompilationResults.Values)
                {
                    DoProgress(5, (double)i / (double)m_CompilationResults.Count, false);
                    
                    bw.Write(v.Bytecode.Data);
                    
                    ++i;
                }
            }

            bw.Close();
        }
        catch (System.IO.IOException e)
        {
            DoError("Error writing .acs shader file: "+e.ToString());
            if (null != bw) bw.Close();

            return false;
        }

        DoProgress(5, 1);
        return true;
    }

    private bool FilterComobos()
    {
        DoProgress(1, 0);
        DoStatus("Filtering combos ...");

        m_StaticCombos.Clear();
        m_DynamicCombos.Clear();
        m_AfxCombos.Clear();
        m_PerlSkipCode = "";

        Regex regExPc = new Regex(
            @"\[PC\]"
        );

        Regex regExXbox = new Regex(
            @"\[XBOX\]"
        );

        Regex regExSfm = new Regex(
            @"\[SFM\]"
        );

        Regex regExNotSfm = new Regex(
            @"\[!SFM\]"
        );

        Regex regExPs = new Regex(
            @"(?i)\[(ps\d+\w?)\]"
        );

        Regex regExVs = new Regex(
            @"(?i)\[(vs\d+\w?)\]"
        );

        Regex regExInitExpr = new Regex(
            @"\[\s*\=\s*([^\]]+)\]"
        );

        Regex regExBracketed = new Regex(
            @"\[[^\[\]]*\]"
        );

        Regex regExEmptyLine = new Regex(
            @"^\s*$"
        );

        Regex regExStaticCombo = new Regex(
            @"^\s*\/\/\s*STATIC\s*\:\s*" +"\""+ @"(.*)" +"\""+ @"\s+"+ "\""+ @"(\d+)\.\.(\d+)" +"\""
        );

        Regex regExDynamicCombo = new Regex(
            @"^\s*\/\/\s*DYNAMIC\s*\:\s*" + "\"" + @"(.*)" + "\"" + @"\s+" + "\"" + @"(\d+)\.\.(\d+)" + "\""
        );

        Regex regExAfxCombo = new Regex(
            @"^\s*\/\/\s*AFX\s*\:\s*" + "\"" + @"(.*)" + "\"" + @"\s+" + "\"" + @"(\d+)\.\.(\d+)" + "\""
        );

        Regex regExSkipCode = new Regex(
            @"^\s*\/\/\s*SKIP\s*\s*\:\s*(.*)$"
        );
        
        string profileStringLower = m_Profile.ToString().Replace("_","").ToLower();

        int lineNum = 0;
        double orgCount = m_Source.Count;

        for (LinkedListNode<string> node = m_Source.First; null != node; )
        {
            string orginalLine = node.Value;

            DoProgress(1, lineNum / orgCount);

            try
            {
                string value = node.Value;

                Match matchPc = regExPc.Match(value);
                Match matchXbox = regExXbox.Match(value);
                Match matchSfm = regExSfm.Match(value);
                Match matchNotSfm = regExNotSfm.Match(value);
                Match matchPs = regExPs.Match(value);
                Match matchVs = regExVs.Match(value);

                if (
                    matchPc.Success && m_Platform != Platform.PC
                    || matchXbox.Success && m_Platform != Platform.XBOX
                    || matchSfm.Success && !m_SFM
                    || matchNotSfm.Success && m_SFM
                    || matchPs.Success && 0 != matchPs.Groups[1].Value.ToLower().CompareTo(profileStringLower)
                    || matchVs.Success && 0 != matchVs.Groups[1].Value.ToLower().CompareTo(profileStringLower)
                )
                {
                    LinkedListNode<string> next = node.Next;
                    m_Source.Remove(node);
                    node = next;
                    ++lineNum;
                    continue;
                }

                string initExpression = null;
                Match matchInitExpr = regExInitExpr.Match(value);
                if (matchInitExpr.Success) initExpression = matchInitExpr.Groups[1].Value;

                value = regExBracketed.Replace(value, "");

                Match matchEmptyLine = regExEmptyLine.Match(value);
                if (matchEmptyLine.Success)
                {
                    node = node.Next;
                    ++lineNum;
                    continue;
                }

                Match matchStaticCombo = regExStaticCombo.Match(value);
                if (matchStaticCombo.Success)
                {
                    string name = matchStaticCombo.Groups[1].Value;
                    string min = matchStaticCombo.Groups[2].Value;
                    string max = matchStaticCombo.Groups[3].Value;

                    Combo combo = new Combo(name, int.Parse(min), int.Parse(max));

                    if (m_StaticCombos.Contains(combo))
                        throw new ArgumentException("m_StaticCombos already contains a combo with name " + name + ".");
                    m_StaticCombos.AddLast(combo);
                }

                Match matchDynamicCombo = regExDynamicCombo.Match(value);
                if (matchDynamicCombo.Success)
                {
                    string name = matchDynamicCombo.Groups[1].Value;
                    string min = matchDynamicCombo.Groups[2].Value;
                    string max = matchDynamicCombo.Groups[3].Value;

                    Combo combo = new Combo(name, int.Parse(min), int.Parse(max));

                    if (m_DynamicCombos.Contains(combo))
                        throw new ArgumentException("m_DynamicCombos already contains a combo with name " + name + ".");
                    m_DynamicCombos.AddLast(combo);
                }

                Match matchAfxCombo = regExAfxCombo.Match(value);
                if (matchAfxCombo.Success)
                {
                    string name = matchAfxCombo.Groups[1].Value;
                    string min = matchAfxCombo.Groups[2].Value;
                    string max = matchAfxCombo.Groups[3].Value;

                    Combo combo = new Combo(name, int.Parse(min), int.Parse(max));

                    if (m_AfxCombos.Contains(combo))
                        throw new ArgumentException("m_AfxCombos already contains a combo with name " + name + ".");
                    m_AfxCombos.AddLast(combo);
                }

                Match matchSkipCode = regExSkipCode.Match(value);
                if (matchSkipCode.Success)
                {
                    m_PerlSkipCode += "(" + matchSkipCode.Groups[1].Value + ")||";
                }
            }
            catch(ApplicationException e)
            {
                DoError("FilterComobos: Error in: Line #" + lineNum + " (" + orginalLine + "): " + e);
                return false;
            }

            node = node.Next;
            ++lineNum;
        }

        m_PerlSkipCode = m_PerlSkipCode + "0";

        DoProgress(1, 1);

        return true;
    }

    private bool ReadInputFile(String filePath)
    {
        DoProgress(0, 0);
        DoStatus("Processing input files(s) ...");

        System.IO.StreamReader sr = null;

        Regex includeRegEx = new Regex(
            @"(?i)^\s*\#include\s" + "\"" + @"(.*)" + "\""
        );

        try
        {
            sr = new System.IO.StreamReader(filePath);

            string line;

            int lineNum = 0;

            while(null != (line = sr.ReadLine()))
            {
                Match match = includeRegEx.Match(line);

                if (match.Success)
                {
                    string basePath = System.IO.Path.GetDirectoryName(filePath);
                    string subFilePath = basePath + "\\" + match.Groups[1].Value;

                    bool subFileOk = ReadInputFile(subFilePath);

                    if (!subFileOk)
                    {
                        DoError("Error doing include in \"" + filePath + "\" in line " + lineNum + ".");

                        sr.Close();
                        return false;
                    }

                    match.NextMatch();
                }
                else
                    m_Source.AddLast(line);

                ++lineNum;
            }

            sr.Close();
        }
        catch(System.IO.IOException e)
        {
            DoError("Error reading file \"" + filePath + "\": " + e);
            if (null != sr) sr.Close();

            return false;
        }

        DoProgress(1, 1);

        return true;
    }

}

} // namespace ShaderBuilder {
