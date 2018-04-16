using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui
{
    public partial class ErrorDialogue : Form
    {
        public ErrorDialogue()
        {
            InitializeComponent();

            this.Icon = SystemIcons.Error;
            this.pictureBoxIcon.Image = SystemIcons.Error.ToBitmap();

            m_NoTitleString = this.labelTitle.Text;
            m_NoDescriptionString = this.textBoxDescription.Text;
            m_NoSolutionString = this.textBoxSolution.Text;

            m_NoSolutionBackColor = this.textBoxSolution.BackColor;
            m_NoSolutionForeColor = this.textBoxSolution.ForeColor;
        }

        internal advancedfx.AfxError Error {
            get
            {
                return m_Error;
            }

            set
            {
                m_Error = value;

                this.labelTitle.Text = m_Error.Title ?? m_NoTitleString;
                this.labelCodeValue.Text = m_Error.Code.ToString();
                this.textBoxDescription.Text = m_Error.Description ?? m_NoDescriptionString;
                
                if(null != m_Error.Solution)
                {
                    this.textBoxSolution.Text = m_Error.Solution;
                    this.textBoxSolution.BackColor = Color.LightGreen;
                    this.textBoxSolution.ForeColor = Color.Black;
                }
                else
                {
                    this.textBoxSolution.Text = m_NoSolutionString;
                    this.textBoxSolution.BackColor = m_NoSolutionBackColor;
                    this.textBoxSolution.ForeColor = m_NoSolutionForeColor;
                }
            }
        
        }

        private string m_NoTitleString;
        private string m_NoDescriptionString;
        private string m_NoSolutionString;
        private Color m_NoSolutionBackColor;
        private Color m_NoSolutionForeColor;

        private advancedfx.AfxError m_Error;

        private void buttonCopyToClipboard_Click(object sender, EventArgs e)
        {
            StringBuilder stringBuilder = new StringBuilder();

            stringBuilder.AppendLine("```");
            stringBuilder.Append(m_Error.ToString());
            stringBuilder.AppendLine("```");

            Clipboard.SetText(stringBuilder.ToString());
        }
    }
}
