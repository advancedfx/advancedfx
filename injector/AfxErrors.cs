using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace advancedfx
{
    internal class Errors
    {
        public static Errors Instance
        {
            get
            {
                if (null == m_Instance) m_Instance = new Errors();

                return m_Instance;
            }
        }

        protected class ErrorStrings
        {
            public const string NoSolutionKnown = "No Solution known.";
        }
        
        public static readonly Error Unknown = new Error(-1, "Unknown error", ErrorStrings.NoSolutionKnown);

        public Error GetById(int id)
        {
            Error error;

            if (!Init.m_Errors.TryGetValue(id, out error))
                error = Unknown;

            return error;
        }

        public class Error
        {
            public Error(int code, string text, string solution)
            {
                if (0 == code)
                    throw new System.ApplicationException("Programming error: Code " + code + " is reserved.");

                if (Init.m_Errors.Keys.Contains(code))
                    throw new System.ApplicationException("Programming error: Code " + code + " already in use.");

                m_Code = code;
                m_Text = text;
                m_Solution = solution;

                Init.m_Errors[code] = this;
            }

            public int Code { get { return m_Code; } }

            public string Text { get { return m_Text; } }

            public string Solution { get { return m_Solution; } }

            private int m_Code;
            private string m_Text;
            private string m_Solution;
        }

        protected Errors()
        {
        }

        private class Init
        {
            public static Dictionary<int, Error> m_Errors = new Dictionary<int, Error>();
        }

        private static Errors m_Instance;
    }
}
