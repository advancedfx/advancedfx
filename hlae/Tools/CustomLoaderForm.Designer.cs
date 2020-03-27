namespace AfxGui.Tools
{
    partial class CustomLoaderForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.buttonSelectProgram = new System.Windows.Forms.Button();
            this.buttonAbort = new System.Windows.Forms.Button();
            this.buttonOk = new System.Windows.Forms.Button();
            this.labelCmdLine = new System.Windows.Forms.Label();
            this.textCmdLine = new System.Windows.Forms.TextBox();
            this.labelProgram = new System.Windows.Forms.Label();
            this.textProgram = new System.Windows.Forms.TextBox();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.groupBoxInjectDlls = new System.Windows.Forms.GroupBox();
            this.labelDllsHint = new System.Windows.Forms.Label();
            this.listBoxHookDlls = new System.Windows.Forms.ListBox();
            this.buttonHookDelete = new System.Windows.Forms.Button();
            this.buttonHookUp = new System.Windows.Forms.Button();
            this.buttonHookDown = new System.Windows.Forms.Button();
            this.buttonHookBrowse = new System.Windows.Forms.Button();
            this.groupEnv = new System.Windows.Forms.GroupBox();
            this.textEnv = new System.Windows.Forms.TextBox();
            this.groupBoxInjectDlls.SuspendLayout();
            this.groupEnv.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonSelectProgram
            // 
            this.buttonSelectProgram.Location = new System.Drawing.Point(468, 11);
            this.buttonSelectProgram.Name = "buttonSelectProgram";
            this.buttonSelectProgram.Size = new System.Drawing.Size(104, 22);
            this.buttonSelectProgram.TabIndex = 15;
            this.buttonSelectProgram.Text = "L10n Browse ...";
            this.buttonSelectProgram.UseVisualStyleBackColor = true;
            this.buttonSelectProgram.Click += new System.EventHandler(this.buttonSelectProgram_Click);
            // 
            // buttonAbort
            // 
            this.buttonAbort.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonAbort.Location = new System.Drawing.Point(452, 317);
            this.buttonAbort.Name = "buttonAbort";
            this.buttonAbort.Size = new System.Drawing.Size(120, 32);
            this.buttonAbort.TabIndex = 19;
            this.buttonAbort.Text = "L10n &Abort";
            this.buttonAbort.UseVisualStyleBackColor = true;
            // 
            // buttonOk
            // 
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(12, 317);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(120, 32);
            this.buttonOk.TabIndex = 18;
            this.buttonOk.Text = "L10n &Ok";
            this.buttonOk.UseVisualStyleBackColor = true;
            // 
            // labelCmdLine
            // 
            this.labelCmdLine.AutoSize = true;
            this.labelCmdLine.Location = new System.Drawing.Point(12, 41);
            this.labelCmdLine.Name = "labelCmdLine";
            this.labelCmdLine.Size = new System.Drawing.Size(103, 13);
            this.labelCmdLine.TabIndex = 16;
            this.labelCmdLine.Text = "L10 n Commandline:";
            // 
            // textCmdLine
            // 
            this.textCmdLine.Location = new System.Drawing.Point(116, 38);
            this.textCmdLine.Name = "textCmdLine";
            this.textCmdLine.Size = new System.Drawing.Size(456, 20);
            this.textCmdLine.TabIndex = 17;
            // 
            // labelProgram
            // 
            this.labelProgram.AutoSize = true;
            this.labelProgram.Location = new System.Drawing.Point(12, 15);
            this.labelProgram.Name = "labelProgram";
            this.labelProgram.Size = new System.Drawing.Size(100, 13);
            this.labelProgram.TabIndex = 13;
            this.labelProgram.Text = "L10n Program path:";
            // 
            // textProgram
            // 
            this.textProgram.Location = new System.Drawing.Point(116, 12);
            this.textProgram.Name = "textProgram";
            this.textProgram.Size = new System.Drawing.Size(346, 20);
            this.textProgram.TabIndex = 14;
            // 
            // openFileDialog
            // 
            this.openFileDialog.Title = "Select ...";
            // 
            // groupBoxInjectDlls
            // 
            this.groupBoxInjectDlls.Controls.Add(this.labelDllsHint);
            this.groupBoxInjectDlls.Controls.Add(this.listBoxHookDlls);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookDelete);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookUp);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookDown);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookBrowse);
            this.groupBoxInjectDlls.Location = new System.Drawing.Point(2, 65);
            this.groupBoxInjectDlls.Name = "groupBoxInjectDlls";
            this.groupBoxInjectDlls.Size = new System.Drawing.Size(579, 158);
            this.groupBoxInjectDlls.TabIndex = 20;
            this.groupBoxInjectDlls.TabStop = false;
            this.groupBoxInjectDlls.Text = "L10n DLLs to inject";
            // 
            // labelDllsHint
            // 
            this.labelDllsHint.AutoSize = true;
            this.labelDllsHint.Location = new System.Drawing.Point(6, 108);
            this.labelDllsHint.Name = "labelDllsHint";
            this.labelDllsHint.Size = new System.Drawing.Size(246, 13);
            this.labelDllsHint.TabIndex = 20;
            this.labelDllsHint.Text = "Hint: You can Drag && Drop DLLs in the box above.";
            // 
            // listBoxHookDlls
            // 
            this.listBoxHookDlls.AllowDrop = true;
            this.listBoxHookDlls.HorizontalScrollbar = true;
            this.listBoxHookDlls.Location = new System.Drawing.Point(6, 49);
            this.listBoxHookDlls.Name = "listBoxHookDlls";
            this.listBoxHookDlls.SelectionMode = System.Windows.Forms.SelectionMode.MultiSimple;
            this.listBoxHookDlls.Size = new System.Drawing.Size(548, 56);
            this.listBoxHookDlls.TabIndex = 19;
            this.listBoxHookDlls.DragDrop += new System.Windows.Forms.DragEventHandler(this.listBoxHookDlls_DragDrop);
            this.listBoxHookDlls.DragEnter += new System.Windows.Forms.DragEventHandler(this.listBoxHookDlls_DragEnter);
            this.listBoxHookDlls.KeyDown += new System.Windows.Forms.KeyEventHandler(this.listBoxHookDlls_KeyDown);
            // 
            // buttonHookDelete
            // 
            this.buttonHookDelete.Location = new System.Drawing.Point(214, 19);
            this.buttonHookDelete.Name = "buttonHookDelete";
            this.buttonHookDelete.Size = new System.Drawing.Size(104, 22);
            this.buttonHookDelete.TabIndex = 17;
            this.buttonHookDelete.Text = "L10n Delete";
            this.buttonHookDelete.UseVisualStyleBackColor = true;
            this.buttonHookDelete.Click += new System.EventHandler(this.buttonHookDelete_Click);
            // 
            // buttonHookUp
            // 
            this.buttonHookUp.Location = new System.Drawing.Point(340, 19);
            this.buttonHookUp.Name = "buttonHookUp";
            this.buttonHookUp.Size = new System.Drawing.Size(104, 22);
            this.buttonHookUp.TabIndex = 16;
            this.buttonHookUp.Text = "L10n Up";
            this.buttonHookUp.UseVisualStyleBackColor = true;
            this.buttonHookUp.Click += new System.EventHandler(this.buttonHookUp_Click);
            // 
            // buttonHookDown
            // 
            this.buttonHookDown.Location = new System.Drawing.Point(450, 19);
            this.buttonHookDown.Name = "buttonHookDown";
            this.buttonHookDown.Size = new System.Drawing.Size(104, 22);
            this.buttonHookDown.TabIndex = 15;
            this.buttonHookDown.Text = "Down";
            this.buttonHookDown.UseVisualStyleBackColor = true;
            this.buttonHookDown.Click += new System.EventHandler(this.buttonHookDown_Click);
            // 
            // buttonHookBrowse
            // 
            this.buttonHookBrowse.Location = new System.Drawing.Point(6, 19);
            this.buttonHookBrowse.Name = "buttonHookBrowse";
            this.buttonHookBrowse.Size = new System.Drawing.Size(104, 22);
            this.buttonHookBrowse.TabIndex = 14;
            this.buttonHookBrowse.Text = "L10n Browse ...";
            this.buttonHookBrowse.UseVisualStyleBackColor = true;
            this.buttonHookBrowse.Click += new System.EventHandler(this.buttonHookBrowse_Click);
            // 
            // groupEnv
            // 
            this.groupEnv.Controls.Add(this.textEnv);
            this.groupEnv.Location = new System.Drawing.Point(2, 229);
            this.groupEnv.Name = "groupEnv";
            this.groupEnv.Size = new System.Drawing.Size(579, 82);
            this.groupEnv.TabIndex = 21;
            this.groupEnv.TabStop = false;
            this.groupEnv.Text = "L10n Add Environment Variables: Name=Value";
            // 
            // textEnv
            // 
            this.textEnv.Location = new System.Drawing.Point(6, 19);
            this.textEnv.Multiline = true;
            this.textEnv.Name = "textEnv";
            this.textEnv.Size = new System.Drawing.Size(567, 57);
            this.textEnv.TabIndex = 0;
            this.textEnv.Enter += new System.EventHandler(this.textEnv_Enter);
            this.textEnv.Leave += new System.EventHandler(this.textEnv_Leave);
            // 
            // CustomLoaderForm
            // 
            this.AcceptButton = this.buttonOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonAbort;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.groupEnv);
            this.Controls.Add(this.groupBoxInjectDlls);
            this.Controls.Add(this.buttonSelectProgram);
            this.Controls.Add(this.buttonAbort);
            this.Controls.Add(this.buttonOk);
            this.Controls.Add(this.labelCmdLine);
            this.Controls.Add(this.textCmdLine);
            this.Controls.Add(this.labelProgram);
            this.Controls.Add(this.textProgram);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "CustomLoaderForm";
            this.Text = "L10n Custom Loader";
            this.groupBoxInjectDlls.ResumeLayout(false);
            this.groupBoxInjectDlls.PerformLayout();
            this.groupEnv.ResumeLayout(false);
            this.groupEnv.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonSelectProgram;
        private System.Windows.Forms.Button buttonAbort;
        private System.Windows.Forms.Button buttonOk;
        private System.Windows.Forms.Label labelCmdLine;
        private System.Windows.Forms.TextBox textCmdLine;
        private System.Windows.Forms.Label labelProgram;
        private System.Windows.Forms.TextBox textProgram;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.GroupBox groupBoxInjectDlls;
        private System.Windows.Forms.Button buttonHookDelete;
        private System.Windows.Forms.Button buttonHookUp;
        private System.Windows.Forms.Button buttonHookDown;
        private System.Windows.Forms.Button buttonHookBrowse;
        private System.Windows.Forms.Label labelDllsHint;
        private System.Windows.Forms.ListBox listBoxHookDlls;
        private System.Windows.Forms.GroupBox groupEnv;
        private System.Windows.Forms.TextBox textEnv;
    }
}