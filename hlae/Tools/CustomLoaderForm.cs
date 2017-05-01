using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui.Tools
{
    public partial class CustomLoaderForm : Form
    {
        //
        // Public members:


        //
        // Internal members:

        internal CustomLoaderForm()
        {
            InitializeComponent();

            this.openFileDialog.InitialDirectory = AfxGui.Program.BaseDir;
        }
        
        internal String HookDll
        {
            get { return this.textDll.Text; }
			set { this.textDll.Text = value; }
		}

        internal String Program
        {
			get { return this.textProgram.Text; }
			set { this.textProgram.Text = value; }
		}

        internal String CmdLine
        {
			get { return this.textCmdLine.Text; }
			set { this.textCmdLine.Text = value; }
		}

        //
        // Private memebers:

        private void buttonSelectHook_Click(object sender, EventArgs e)
        {
		    openFileDialog.DefaultExt = "*.dll";
		    openFileDialog.Filter = "Hook DLL (.dll)|*.dll";
		    if(DialogResult.OK == openFileDialog.ShowDialog(this))
			    this.textDll.Text = openFileDialog.FileName;
        }

        private void buttonSelectProgram_Click(object sender, EventArgs e)
        {
		    openFileDialog.DefaultExt = "*.exe";
		    openFileDialog.Filter = "Program to launch and hook (.exe)|*.exe";
		    if(DialogResult.OK == openFileDialog.ShowDialog(this))
			    this.textProgram.Text = openFileDialog.FileName;
        }
    }
}
