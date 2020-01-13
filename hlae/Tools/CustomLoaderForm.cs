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

            this.Text = L10n._p("Custom Loader dialog", "Custom Loader");
            this.labelProgram.Text = L10n._p("Custom Loader dialog | Program", "ProgramPath:");
            this.labelCmdLine.Text = L10n._p("Custom Loader dialog | Program", "CommandLine:");
            this.buttonSelectProgram.Text = L10n._p("Custom Loader dialog | Program", "Browse ...");
            this.groupBoxInjectDlls.Text = L10n._p("Custom Loader dialog | DLLS", "DLLs to inject");
            this.buttonHookBrowse.Text = L10n._p("Custom Loader dialog | DLLS", "Browse ...");
            this.buttonHookDelete.Text = L10n._p("Custom Loader dialog | DLLS", "Delete");
            this.buttonHookUp.Text = L10n._p("Custom Loader dialog | DLLS", "Up");
            this.buttonHookDown.Text = L10n._p("Custom Loader dialog | DLLS", "Down");
            this.labelDllsHint.Text = L10n._p("Custom Loader dialog | DLLS", "Hint: You can Drag && Drop DLLs in the box above.");
            this.buttonOk.Text = L10n._p("Custom Loader dialog", "&OK");
            this.buttonAbort.Text = L10n._p("Custom Loader dialog", "&Abort");

            this.openFileDialog.InitialDirectory = AfxGui.Program.BaseDir;
        }
        
        internal ListBox.ObjectCollection HookDlls
        {
            get { return listBoxHookDlls.Items; }
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

        private void buttonSelectProgram_Click(object sender, EventArgs e)
        {
		    openFileDialog.DefaultExt = "*.exe";
		    openFileDialog.Filter = L10n._p("Custom Loader dialog | Program filter", "Program to launch and hook (.exe)|*.exe");
		    if(DialogResult.OK == openFileDialog.ShowDialog(this))
			    this.textProgram.Text = openFileDialog.FileName;
        }

        private void buttonHookBrowse_Click(object sender, EventArgs e)
        {
            openFileDialog.DefaultExt = "*.dll";
            openFileDialog.Filter = L10n._p("Custom Loader dialog | DLL filter", "Hook DLL (.dll)|*.dll");
            if (DialogResult.OK == openFileDialog.ShowDialog(this))
            {
                this.listBoxHookDlls.Items.Add(openFileDialog.FileName);
            }
        }

        private void HookDeleteSelected()
        {
            listBoxHookDlls.BeginUpdate();
            while (0 < listBoxHookDlls.SelectedIndices.Count)
                listBoxHookDlls.Items.RemoveAt(listBoxHookDlls.SelectedIndices[0]);
            listBoxHookDlls.EndUpdate();
        }

        private void HookMoveSelected(int index)
        {
            if(0 < listBoxHookDlls.SelectedIndices.Count && 0 <= index && index <= listBoxHookDlls.Items.Count)
            {
                listBoxHookDlls.BeginUpdate();

                List<object> objs = new List<object>();

                while(0 < listBoxHookDlls.SelectedIndices.Count)
                {
                    if (index > listBoxHookDlls.SelectedIndices[0])
                        --index;

                    object obj = listBoxHookDlls.SelectedItems[0];

                    objs.Insert(0, obj);
                    listBoxHookDlls.Items.RemoveAt(listBoxHookDlls.SelectedIndices[0]);
                }

                foreach (object obj in objs)
                {
                    listBoxHookDlls.Items.Insert(index, obj);
                    listBoxHookDlls.SetSelected(index, true);
                }

                listBoxHookDlls.EndUpdate();
            }
        }

        private void buttonHookDelete_Click(object sender, EventArgs e)
        {
            HookDeleteSelected();
        }

        private void listBoxHookDlls_DragEnter(object sender, DragEventArgs e)
        {
            if(e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                e.Effect = DragDropEffects.Link;
            }
        }

        private void listBoxHookDlls_DragDrop(object sender, DragEventArgs e)
        {
            if(e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                listBoxHookDlls.BeginUpdate();
                try
                {
                    string[] files = e.Data.GetData(DataFormats.FileDrop) as string[];
                    if(null != files)
                    {
                        foreach(string file in files)
                        {
                            listBoxHookDlls.Items.Add(file);
                        }                        
                    }
                }
                catch
                {
                }
                listBoxHookDlls.EndUpdate();
            }
        }

        private void listBoxHookDlls_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.Control && e.KeyCode == Keys.A)
            {
                listBoxHookDlls.BeginUpdate();

                if (listBoxHookDlls.SelectedIndices.Count < listBoxHookDlls.Items.Count)
                {
                    for (int i = 0; i < listBoxHookDlls.Items.Count; ++i)
                        if (!listBoxHookDlls.GetSelected(i)) listBoxHookDlls.SetSelected(i, true);
                }
                else
                    listBoxHookDlls.ClearSelected();

                listBoxHookDlls.EndUpdate();
            }
            else if(e.KeyCode == Keys.Delete)
            {
                HookDeleteSelected();
            }
        }

        private void buttonHookUp_Click(object sender, EventArgs e)
        {
            if (0 < listBoxHookDlls.SelectedItems.Count)
            {
                HookMoveSelected(Math.Max(listBoxHookDlls.SelectedIndices[0] -1, 0));
            }
        }

        private void buttonHookDown_Click(object sender, EventArgs e)
        {
            if (0 < listBoxHookDlls.SelectedItems.Count)
            {
                HookMoveSelected(Math.Min(listBoxHookDlls.SelectedIndices[listBoxHookDlls.SelectedIndices.Count-1] +2, listBoxHookDlls.Items.Count));
            }
        }
    }
}