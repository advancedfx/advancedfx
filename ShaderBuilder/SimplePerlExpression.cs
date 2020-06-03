// This code is based on Andreas Gieriet's code:
// "Invent your own Dynamic LINQ parser"
// http://www.codeproject.com/Articles/355513/Invent-your-own-Dynamic-LINQ-parser
// v1.7 (2014-10-14)
//
// The code was licensed under the MIT license, which we replicate bellow:
/*
Copyright (c) 2014 Andreas Gieriet



Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:



The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Text.RegularExpressions;


namespace ShaderBuilder {

class SimplePerlExpression
{
    //
    // Public members:

    public static bool IntToBool(int value)
    {
        return Convert.ToBoolean(value);
    }

    /// <throws>ApplicationException</throws>
    public SimplePerlExpression(string code)
    {
        _tokens = MakeTokens(code);
        Move();

        m_Func = Parse().Compile();
    }

    public void DefineVariable(string name, int value)
    {
        m_Variables[name] = value;
    }

    public bool DefinedVariable(string name)
    {
        return m_Variables.Keys.Contains(name);
    }

    public int GetVariable(string name)
    {
        int value;

        if (m_Variables.TryGetValue(name, out value))
            return value;

        return 0;
    }

    public bool Eval()
    {
        return m_Func(this);
    }

    //
    // Private members:

    private class Token
    {
        public enum Types
        {
            Invalid,
            BracketOpen,
            BracketClose,
            Variable,
            Number,
            Defined,
            OpEqual,
            OpNotEqual,
            OpLogicalNot,
            OpLogicalAnd,
            OpLogicalOr,
            OpLess,
            OpLessEqual,
            OpGreater,
            OpGreaterEqual
        }

        public Types Type;

        public string Content;

        public Token(Types type, string content)
        {
            Type = type;
            Content = content;
        }
    }

    private Dictionary<string, int> m_Variables = new Dictionary<string,int>();
    private IEnumerator<Token> _tokens;
    private Func<SimplePerlExpression, bool> m_Func;

    private bool Move()
    {
        return _tokens.MoveNext();
    }

    private bool IsOfType(Token.Types type)
    {
        Token token = Curr;

        return null != token && token.Type == type;
    }

    private void Abort(string msg)
    {
        throw new ArgumentException("Error: " + (msg ?? "unknown error"));
    }

    private Token Curr
    {
        get
        {
            return _tokens.Current;
        }
    }

    private Token CurrAndNext
    {
        get {
            Token t = Curr;
            if (!Move()) Abort("data expected");
            return t;
        }
    }

    private Token CurrOptNext
    {
        get {
            Token t = Curr;
            Move();
            return t;
        }
    }

    private Token CurrOpAndNext(params Token.Types[] ops)
    {
        Token token = null;
        if(ops.Contains(Curr.Type)) token = Curr;

        if(token != null && !Move()) Abort("data expected");

        return token;
    }

    private static ConstantExpression Const(object v) { return Expression.Constant(v); }

    private static readonly Type _bool = typeof(bool);
    private static readonly Type _int = typeof(int);
    private static readonly Type[] _prom = new Type[]
    { typeof(decimal), typeof(double), typeof(float), typeof(ulong), typeof(long), typeof(uint),
        typeof(int), typeof(ushort), typeof(char), typeof(short), typeof(byte), typeof(sbyte) };

    private Expression<Func<SimplePerlExpression, bool>> Lambda(Expression expr)
    {
        if (expr.Type == _int)
            expr = Expression.Convert(expr, _bool, _methodIntToBool);

        return Expression.Lambda<Func<SimplePerlExpression, bool>>(expr, _param);
    }

    private Expression<Func<SimplePerlExpression,bool>> Parse()
    {
        return Lambda(ParseExpression());
    }

    private Expression ParseExpression()
    {
        return ParseOr();
    }

    private Expression ParseOr()
    {
        return ParseBinary(ParseAnd, Token.Types.OpLogicalOr);
    }

    private Expression ParseAnd()
    {
        return ParseBinary(ParseEquality, Token.Types.OpLogicalAnd);
    }

    private Expression ParseEquality()
    {
        return ParseBinary(ParseRelation, Token.Types.OpEqual, Token.Types.OpNotEqual);
    }

    private Expression ParseRelation()
    {
        return ParseBinary(ParseUnary, Token.Types.OpLess, Token.Types.OpLessEqual, Token.Types.OpGreaterEqual, Token.Types.OpGreater);
    }

    private Expression ParseUnary()
    {
        return CurrOpAndNext(Token.Types.OpLogicalNot) != null ? _unOp[Token.Types.OpLogicalNot](ParseUnary()) : ParsePrimary();
    }

    private Expression ParseDefined()
    {
        return ExprDefinedVariable(CurrOptNext.Content);
    }

    private Expression ParseVariable()
    {
        return ExprGetVariable(CurrOptNext.Content);
    }

    private Expression ParseNumber()
    {
        return Const(int.Parse(CurrOptNext.Content));
    }

    private Expression ParsePrimary()
    {
        if (IsOfType(Token.Types.Defined)) return ParseDefined();
        if (IsOfType(Token.Types.Variable)) return ParseVariable();
        if (IsOfType(Token.Types.Number)) return ParseNumber();
        return ParseNested();
    }

    private Expression ParseNested()
    {
        if (CurrAndNext.Type != Token.Types.BracketOpen) Abort("(...) expected");
        Expression expr = ParseExpression();
        if (CurrOptNext.Type != Token.Types.BracketClose) Abort("')' expected");
        return expr;
    }

    private Expression ParseBinary(Func<Expression> parse, params Token.Types[] ops)
    {
        Expression expr = parse();

        Token token;
        while ((token = CurrOpAndNext(ops)) != null) expr = _binOp[token.Type](expr, parse());

        return expr;
    }

    private static Expression Coerce(Expression expr, Type type)
    {
        if (type == _bool && expr.Type == _int)
            return Expression.Convert(expr, _bool, _methodIntToBool);

        return expr.Type == type ? expr : Expression.Convert(expr, type);
    }

    private static Expression Coerce(Expression expr, Expression sibling)
    {
        if (expr.Type != sibling.Type)
        {
            Type maxType = MaxType(expr.Type, sibling.Type);
            if (maxType != expr.Type) expr = Expression.Convert(expr, maxType);
        }
        return expr;
    }

    private static Type MaxType(Type a, Type b) { return a==b?a:(_prom.FirstOrDefault(t=>t==a||t==b)??a); }

    private static readonly Dictionary<Token.Types, Func<Expression, Expression, Expression>> _binOp = new Dictionary<Token.Types, Func<Expression, Expression, Expression>>()
    {
        { Token.Types.OpLogicalOr, (a,b)=>Expression.OrElse(Coerce(a, _bool), Coerce(b, _bool)) },
        { Token.Types.OpLogicalAnd, (a,b)=>Expression.AndAlso(Coerce(a, _bool), Coerce(b, _bool)) },
        { Token.Types.OpEqual, (a,b)=>Expression.Equal(Coerce(a,b), Coerce(b,a)) },
        { Token.Types.OpNotEqual, (a,b)=>Expression.NotEqual(Coerce(a,b), Coerce(b,a)) },
        { Token.Types.OpLess, (a,b)=>Expression.LessThan(Coerce(a,b), Coerce(b,a)) },
        { Token.Types.OpLessEqual, (a,b)=>Expression.LessThanOrEqual(Coerce(a,b), Coerce(b,a)) },
        { Token.Types.OpGreaterEqual, (a,b)=>Expression.GreaterThanOrEqual(Coerce(a,b), Coerce(b,a)) },
        { Token.Types.OpGreater, (a,b)=>Expression.GreaterThan(Coerce(a,b), Coerce(b,a)) },
    };

    private static readonly Dictionary<Token.Types, Func<Expression, Expression>> _unOp =
            new Dictionary<Token.Types, Func<Expression, Expression>>()
    {
        { Token.Types.OpLogicalNot, a=>Expression.Not(Coerce(a, _bool)) },
    };

    private static readonly ParameterExpression _param = Expression.Parameter(typeof(SimplePerlExpression), "_p_");
    private static readonly System.Reflection.MethodInfo _methodDefinedVariable = typeof(SimplePerlExpression).GetMethod("DefinedVariable");
    private static readonly System.Reflection.MethodInfo _methodGetVariable = typeof(SimplePerlExpression).GetMethod("GetVariable");
    private static readonly System.Reflection.MethodInfo _methodIntToBool = typeof(SimplePerlExpression).GetMethod("IntToBool");

    private Expression ExprDefinedVariable(string name)
    {
        return Expression.Call(
            _param,
            _methodDefinedVariable,
            new Expression[] { Expression.Constant(name) }
        );
    }

    private Expression ExprGetVariable(string name)
    {
        return Expression.Call(
            _param,
            _methodGetVariable,
            new Expression[] { Expression.Constant(name) }
        );
    }

    private IEnumerator<Token> MakeTokens(string code)
    {
        LinkedList<Token> tokens = new LinkedList<Token>();

        string remainder = code;

        Regex regexWhiteSpace = new Regex(
            @"^\s+"
        );

        Regex regexDefined = new Regex(
            @"^defined\s+\$(\w+)"
        );

        Regex regexVariable = new Regex(
            @"^\$(\w+)"
        );

        Regex regexNumber = new Regex(
            @"^(\d+)"
        );

        while (0 < remainder.Length)
        {
            Match matchWhiteSpace = regexWhiteSpace.Match(remainder);
            if (matchWhiteSpace.Success)
            {
                remainder = remainder.Substring(matchWhiteSpace.Index + matchWhiteSpace.Length);
                continue;
            }

            char firstChar = remainder[0];
            char secondChar = 1 < remainder.Length ? remainder[1] : '\0';

            switch (firstChar)
            {
            case '(':
                tokens.AddLast(new Token(Token.Types.BracketOpen, ""));
                remainder = remainder.Substring(1);
                continue;
            case ')':
                tokens.AddLast(new Token(Token.Types.BracketClose, ""));
                remainder = remainder.Substring(1);
                continue;
            case '!':
                switch (secondChar)
                {
                case '=':
                    tokens.AddLast(new Token(Token.Types.OpNotEqual, ""));
                    remainder = remainder.Substring(2);
                    continue;
                default:
                    tokens.AddLast(new Token(Token.Types.OpLogicalNot, ""));
                    remainder = remainder.Substring(1);
                    continue;
                }
            case '=':
                switch (secondChar)
                {
                case '=':
                    tokens.AddLast(new Token(Token.Types.OpEqual, ""));
                    remainder = remainder.Substring(2);
                    continue;
                }
                break;
            case '<':
                switch (secondChar)
                {
                case '=':
                    tokens.AddLast(new Token(Token.Types.OpLessEqual, ""));
                    remainder = remainder.Substring(2);
                    continue;
                default:
                    tokens.AddLast(new Token(Token.Types.OpLess, ""));
                    remainder = remainder.Substring(1);
                    continue;
                }
            case '>':
                switch (secondChar)
                {
                case '=':
                    tokens.AddLast(new Token(Token.Types.OpGreaterEqual, ""));
                    remainder = remainder.Substring(2);
                    continue;
                default:
                    tokens.AddLast(new Token(Token.Types.OpGreater, ""));
                    remainder = remainder.Substring(1);
                    continue;
                }
            case '&':
                switch (secondChar)
                {
                case '&':
                    tokens.AddLast(new Token(Token.Types.OpLogicalAnd, ""));
                    remainder = remainder.Substring(2);
                    continue;
                }
                break;
            case '|':
                switch (secondChar)
                {
                    case '|':
                        tokens.AddLast(new Token(Token.Types.OpLogicalOr, ""));
                        remainder = remainder.Substring(2);
                        continue;
                }
                break;
            }

            Match matchDefined = regexDefined.Match(remainder);
            if (matchDefined.Success)
            {
                tokens.AddLast(new Token(Token.Types.Defined, matchDefined.Groups[1].Value));
                remainder = remainder.Substring(matchDefined.Index + matchDefined.Length);
                continue;
            }

            Match matchVariable = regexVariable.Match(remainder);
            if (matchVariable.Success)
            {
                tokens.AddLast(new Token(Token.Types.Variable, matchVariable.Groups[1].Value));
                remainder = remainder.Substring(matchVariable.Index + matchVariable.Length);
                continue;
            }

            Match matchNumber = regexNumber.Match(remainder);
            if (matchNumber.Success)
            {
                tokens.AddLast(new Token(Token.Types.Number, matchNumber.Groups[1].Value));
                remainder = remainder.Substring(matchNumber.Index + matchNumber.Length);
                continue;
            }

            tokens.AddLast(new Token(Token.Types.Invalid, remainder));
            remainder = "";
        }

        return tokens.GetEnumerator();
    }

}

} // namespace ShaderBuilder {