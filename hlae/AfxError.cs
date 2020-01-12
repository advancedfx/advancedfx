using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace advancedfx
{
    public class AfxError : Exception
    {
        public AfxError(int code, string title = null, string description = null, string solution = null, Exception innerException = null)
            : base("AfxError", innerException)
        {
            m_Code = code;
            m_Title = title;
            m_Description = description;
            m_Solution = solution;
        }

        public int Code { get { return m_Code; } }

        /// <remarks>
        /// Can be null to indicate no description.
        /// </remarks>
        public string Description { get { return m_Description; } }

        /// <remarks>
        /// Can be null to indicate no title.
        /// </remarks>
        public string Title { get { return m_Title;  } }

        /// <remarks>
        /// Can be null to indicate no solution.
        /// </remarks>
        public string Solution { get { return m_Solution; } }

        public override string ToString()
        {
            StringBuilder stringBuilder = new StringBuilder();

            stringBuilder.AppendLine("AfxError #"+ m_Code.ToString()+": ");
            if (null != m_Title) { stringBuilder.Append("Title: "); stringBuilder.AppendLine(m_Title); }
            if (null != m_Description) { stringBuilder.Append("Description: "); stringBuilder.AppendLine(m_Description); }
            if (null != m_Solution) { stringBuilder.Append("Solution: "); stringBuilder.AppendLine(m_Solution); }
            if (null != this.InnerException) { stringBuilder.Append("Inner exception: "); stringBuilder.AppendLine(this.InnerException.ToString()); }

            return stringBuilder.ToString();
        }

        private int m_Code;
        private string m_Description;
        private string m_Title;
        private string m_Solution;
    }
}
