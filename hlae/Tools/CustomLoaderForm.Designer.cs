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
            this.label1 = new System.Windows.Forms.Label();
            this.listBoxHookDlls = new System.Windows.Forms.ListBox();
            this.buttonHookDelete = new System.Windows.Forms.Button();
            this.buttonHookUp = new System.Windows.Forms.Button();
            this.buttonHookDown = new System.Windows.Forms.Button();
            this.buttonHookBrowse = new System.Windows.Forms.Button();
            this.groupBoxInjectDlls.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonSelectProgram
            // 
            this.buttonSelectProgram.Location = new System.Drawing.Point(373, 10);
            this.buttonSelectProgram.Name = "buttonSelectProgram";
            this.buttonSelectProgram.Size = new System.Drawing.Size(84, 23);
            this.buttonSelectProgram.TabIndex = 15;
            this.buttonSelectProgram.Text = "Browse";
            this.buttonSelectProgram.UseVisualStyleBackColor = true;
            this.buttonSelectProgram.Click += new System.EventHandler(this.buttonSelectProgram_Click);
            // 
            // buttonAbort
            // 
            this.buttonAbort.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonAbort.Location = new System.Drawing.Point(334, 318);
            this.buttonAbort.Name = "buttonAbort";
            this.buttonAbort.Size = new System.Drawing.Size(123, 31);
            this.buttonAbort.TabIndex = 19;
            this.buttonAbort.Text = "&Abort";
            this.buttonAbort.UseVisualStyleBackColor = true;
            // 
            // buttonOk
            // 
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(15, 318);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(123, 31);
            this.buttonOk.TabIndex = 18;
            this.buttonOk.Text = "&Ok";
            this.buttonOk.UseVisualStyleBackColor = true;
            // 
            // labelCmdLine
            // 
            this.labelCmdLine.AutoSize = true;
            this.labelCmdLine.Location = new System.Drawing.Point(12, 41);
            this.labelCmdLine.Name = "labelCmdLine";
            this.labelCmdLine.Size = new System.Drawing.Size(77, 13);
            this.labelCmdLine.TabIndex = 16;
            this.labelCmdLine.Text = "CommandLine:";
            // 
            // textCmdLine
            // 
            this.textCmdLine.Location = new System.Drawing.Point(105, 38);
            this.textCmdLine.Name = "textCmdLine";
            this.textCmdLine.Size = new System.Drawing.Size(352, 20);
            this.textCmdLine.TabIndex = 17;
            // 
            // labelProgram
            // 
            this.labelProgram.AutoSize = true;
            this.labelProgram.Location = new System.Drawing.Point(12, 15);
            this.labelProgram.Name = "labelProgram";
            this.labelProgram.Size = new System.Drawing.Size(71, 13);
            this.labelProgram.TabIndex = 13;
            this.labelProgram.Text = "ProgramPath:";
            // 
            // textProgram
            // 
            this.textProgram.Location = new System.Drawing.Point(105, 12);
            this.textProgram.Name = "textProgram";
            this.textProgram.Size = new System.Drawing.Size(262, 20);
            this.textProgram.TabIndex = 14;
            // 
            // openFileDialog
            // 
            this.openFileDialog.Title = "Select ...";
            // 
            // groupBoxInjectDlls
            // 
            this.groupBoxInjectDlls.Controls.Add(this.label1);
            this.groupBoxInjectDlls.Controls.Add(this.listBoxHookDlls);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookDelete);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookUp);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookDown);
            this.groupBoxInjectDlls.Controls.Add(this.buttonHookBrowse);
            this.groupBoxInjectDlls.Location = new System.Drawing.Point(12, 65);
            this.groupBoxInjectDlls.Name = "groupBoxInjectDlls";
            this.groupBoxInjectDlls.Size = new System.Drawing.Size(445, 234);
            this.groupBoxInjectDlls.TabIndex = 20;
            this.groupBoxInjectDlls.TabStop = false;
            this.groupBoxInjectDlls.Text = " DLLs to inject: ";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(7, 215);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(246, 13);
            this.label1.TabIndex = 20;
            this.label1.Text = "Hint: You can Drag && Drop DLLs in the box above.";
            // 
            // listBoxHookDlls
            // 
            this.listBoxHookDlls.AllowDrop = true;
            this.listBoxHookDlls.HorizontalScrollbar = true;
            this.listBoxHookDlls.Location = new System.Drawing.Point(6, 49);
            this.listBoxHookDlls.Name = "listBoxHookDlls";
            this.listBoxHookDlls.SelectionMode = System.Windows.Forms.SelectionMode.MultiSimple;
            this.listBoxHookDlls.Size = new System.Drawing.Size(433, 160);
            this.listBoxHookDlls.TabIndex = 19;
            this.listBoxHookDlls.DragDrop += new System.Windows.Forms.DragEventHandler(this.listBoxHookDlls_DragDrop);
            this.listBoxHookDlls.DragEnter += new System.Windows.Forms.DragEventHandler(this.listBoxHookDlls_DragEnter);
            this.listBoxHookDlls.KeyDown += new System.Windows.Forms.KeyEventHandler(this.listBoxHookDlls_KeyDown);
            // 
            // buttonHookDelete
            // 
            this.buttonHookDelete.Location = new System.Drawing.Point(179, 19);
            this.buttonHookDelete.Name = "buttonHookDelete";
            this.buttonHookDelete.Size = new System.Drawing.Size(80, 23);
            this.buttonHookDelete.TabIndex = 17;
            this.buttonHookDelete.Text = "Delete";
            this.buttonHookDelete.UseVisualStyleBackColor = true;
            this.buttonHookDelete.Click += new System.EventHandler(this.buttonHookDelete_Click);
            // 
            // buttonHookUp
            // 
            this.buttonHookUp.Location = new System.Drawing.Point(273, 19);
            this.buttonHookUp.Name = "buttonHookUp";
            this.buttonHookUp.Size = new System.Drawing.Size(80, 23);
            this.buttonHookUp.TabIndex = 16;
            this.buttonHookUp.Text = "Up";
            this.buttonHookUp.UseVisualStyleBackColor = true;
            this.buttonHookUp.Click += new System.EventHandler(this.buttonHookUp_Click);
            // 
            // buttonHookDown
            // 
            this.buttonHookDown.Location = new System.Drawing.Point(359, 19);
            this.buttonHookDown.Name = "buttonHookDown";
            this.buttonHookDown.Size = new System.Drawing.Size(80, 23);
            this.buttonHookDown.TabIndex = 15;
            this.buttonHookDown.Text = "Down";
            this.buttonHookDown.UseVisualStyleBackColor = true;
            this.buttonHookDown.Click += new System.EventHandler(this.buttonHookDown_Click);
            // 
            // buttonHookBrowse
            // 
            this.buttonHookBrowse.Location = new System.Drawing.Point(6, 19);
            this.buttonHookBrowse.Name = "buttonHookBrowse";
            this.buttonHookBrowse.Size = new System.Drawing.Size(80, 23);
            this.buttonHookBrowse.TabIndex = 14;
            this.buttonHookBrowse.Text = "Browse";
            this.buttonHookBrowse.UseVisualStyleBackColor = true;
            this.buttonHookBrowse.Click += new System.EventHandler(this.buttonHookBrowse_Click);
            // 
            // CustomLoaderForm
            // 
            this.AcceptButton = this.buttonOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonAbort;
            this.ClientSize = new System.Drawing.Size(469, 361);
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
            this.Text = "Custom Loader";
            this.groupBoxInjectDlls.ResumeLayout(false);
            this.groupBoxInjectDlls.PerformLayout();
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
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListBox listBoxHookDlls;
    }
}