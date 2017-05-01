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
            this.buttonSelectHook = new System.Windows.Forms.Button();
            this.buttonAbort = new System.Windows.Forms.Button();
            this.buttonOk = new System.Windows.Forms.Button();
            this.labelCmdLine = new System.Windows.Forms.Label();
            this.textCmdLine = new System.Windows.Forms.TextBox();
            this.labelProgram = new System.Windows.Forms.Label();
            this.textProgram = new System.Windows.Forms.TextBox();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.labelDll = new System.Windows.Forms.Label();
            this.textDll = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // buttonSelectProgram
            // 
            this.buttonSelectProgram.Location = new System.Drawing.Point(373, 37);
            this.buttonSelectProgram.Name = "buttonSelectProgram";
            this.buttonSelectProgram.Size = new System.Drawing.Size(84, 23);
            this.buttonSelectProgram.TabIndex = 15;
            this.buttonSelectProgram.Text = "browse";
            this.buttonSelectProgram.UseVisualStyleBackColor = true;
            this.buttonSelectProgram.Click += new System.EventHandler(this.buttonSelectProgram_Click);
            // 
            // buttonSelectHook
            // 
            this.buttonSelectHook.Location = new System.Drawing.Point(373, 11);
            this.buttonSelectHook.Name = "buttonSelectHook";
            this.buttonSelectHook.Size = new System.Drawing.Size(84, 23);
            this.buttonSelectHook.TabIndex = 12;
            this.buttonSelectHook.Text = "browse";
            this.buttonSelectHook.UseVisualStyleBackColor = true;
            this.buttonSelectHook.Click += new System.EventHandler(this.buttonSelectHook_Click);
            // 
            // buttonAbort
            // 
            this.buttonAbort.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonAbort.Location = new System.Drawing.Point(334, 109);
            this.buttonAbort.Name = "buttonAbort";
            this.buttonAbort.Size = new System.Drawing.Size(123, 31);
            this.buttonAbort.TabIndex = 19;
            this.buttonAbort.Text = "&Abort";
            this.buttonAbort.UseVisualStyleBackColor = true;
            // 
            // buttonOk
            // 
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(15, 109);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(123, 31);
            this.buttonOk.TabIndex = 18;
            this.buttonOk.Text = "&Ok";
            this.buttonOk.UseVisualStyleBackColor = true;
            // 
            // labelCmdLine
            // 
            this.labelCmdLine.AutoSize = true;
            this.labelCmdLine.Location = new System.Drawing.Point(12, 68);
            this.labelCmdLine.Name = "labelCmdLine";
            this.labelCmdLine.Size = new System.Drawing.Size(77, 13);
            this.labelCmdLine.TabIndex = 16;
            this.labelCmdLine.Text = "CommandLine:";
            // 
            // textCmdLine
            // 
            this.textCmdLine.Location = new System.Drawing.Point(105, 65);
            this.textCmdLine.Name = "textCmdLine";
            this.textCmdLine.Size = new System.Drawing.Size(352, 20);
            this.textCmdLine.TabIndex = 17;
            // 
            // labelProgram
            // 
            this.labelProgram.AutoSize = true;
            this.labelProgram.Location = new System.Drawing.Point(12, 42);
            this.labelProgram.Name = "labelProgram";
            this.labelProgram.Size = new System.Drawing.Size(71, 13);
            this.labelProgram.TabIndex = 13;
            this.labelProgram.Text = "ProgramPath:";
            // 
            // textProgram
            // 
            this.textProgram.Location = new System.Drawing.Point(105, 39);
            this.textProgram.Name = "textProgram";
            this.textProgram.Size = new System.Drawing.Size(262, 20);
            this.textProgram.TabIndex = 14;
            // 
            // openFileDialog
            // 
            this.openFileDialog.Title = "Select ...";
            // 
            // labelDll
            // 
            this.labelDll.AutoSize = true;
            this.labelDll.Location = new System.Drawing.Point(12, 16);
            this.labelDll.Name = "labelDll";
            this.labelDll.Size = new System.Drawing.Size(56, 13);
            this.labelDll.TabIndex = 10;
            this.labelDll.Text = "HookDLL:";
            // 
            // textDll
            // 
            this.textDll.Location = new System.Drawing.Point(105, 13);
            this.textDll.Name = "textDll";
            this.textDll.Size = new System.Drawing.Size(262, 20);
            this.textDll.TabIndex = 11;
            // 
            // label1
            // 
            this.label1.BackColor = System.Drawing.Color.Maroon;
            this.label1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.label1.ForeColor = System.Drawing.Color.White;
            this.label1.Location = new System.Drawing.Point(12, 155);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(445, 55);
            this.label1.TabIndex = 20;
            this.label1.Text = "Dear CS:GO users, while you can still use this, please note that a CS:GO specifc " +
    "dialog has been added to the File menu entry!";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // CustomLoaderForm
            // 
            this.AcceptButton = this.buttonOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonAbort;
            this.ClientSize = new System.Drawing.Size(469, 219);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonSelectProgram);
            this.Controls.Add(this.buttonSelectHook);
            this.Controls.Add(this.buttonAbort);
            this.Controls.Add(this.buttonOk);
            this.Controls.Add(this.labelCmdLine);
            this.Controls.Add(this.textCmdLine);
            this.Controls.Add(this.labelProgram);
            this.Controls.Add(this.textProgram);
            this.Controls.Add(this.labelDll);
            this.Controls.Add(this.textDll);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "CustomLoaderForm";
            this.Text = "Custom Loader";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonSelectProgram;
        private System.Windows.Forms.Button buttonSelectHook;
        private System.Windows.Forms.Button buttonAbort;
        private System.Windows.Forms.Button buttonOk;
        private System.Windows.Forms.Label labelCmdLine;
        private System.Windows.Forms.TextBox textCmdLine;
        private System.Windows.Forms.Label labelProgram;
        private System.Windows.Forms.TextBox textProgram;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.Label labelDll;
        private System.Windows.Forms.TextBox textDll;
        private System.Windows.Forms.Label label1;
    }
}