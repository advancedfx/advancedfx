﻿namespace AfxGui
{
    partial class LaunchCs2Form
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
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.checkBoxRemeber = new System.Windows.Forms.CheckBox();
            this.groupBoxGame = new System.Windows.Forms.GroupBox();
            this.buttonExe = new System.Windows.Forms.Button();
            this.textBoxExe = new System.Windows.Forms.TextBox();
            this.labelExe = new System.Windows.Forms.Label();
            this.groupBoxCmdOpts = new System.Windows.Forms.GroupBox();
            this.checkBoxAvoidVac = new System.Windows.Forms.CheckBox();
            this.textBoxCustomCmd = new System.Windows.Forms.TextBox();
            this.groupBoxRes = new System.Windows.Forms.GroupBox();
            this.checkBoxEnableGfx = new System.Windows.Forms.CheckBox();
            this.labelGfxInfo = new System.Windows.Forms.Label();
            this.checkBoxGfxFull = new System.Windows.Forms.CheckBox();
            this.textBoxGfxHeight = new System.Windows.Forms.TextBox();
            this.labelGfxHeight = new System.Windows.Forms.Label();
            this.textBoxGfxWidth = new System.Windows.Forms.TextBox();
            this.labelGfxWidth = new System.Windows.Forms.Label();
            this.groupBoxMmcfg = new System.Windows.Forms.GroupBox();
            this.buttonMmcfgInfo = new System.Windows.Forms.Button();
            this.checkBoxEnableMmcfg = new System.Windows.Forms.CheckBox();
            this.buttonMmcfg = new System.Windows.Forms.Button();
            this.textBoxMmcfg = new System.Windows.Forms.TextBox();
            this.openFileDialogExe = new System.Windows.Forms.OpenFileDialog();
            this.folderBrowserDialogMMcfg = new System.Windows.Forms.FolderBrowserDialog();
            this.groupBoxGame.SuspendLayout();
            this.groupBoxCmdOpts.SuspendLayout();
            this.groupBoxRes.SuspendLayout();
            this.groupBoxMmcfg.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonCancel
            // 
            this.buttonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(474, 321);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(102, 28);
            this.buttonCancel.TabIndex = 1;
            this.buttonCancel.Text = "L10n Can&cel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(366, 321);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(102, 28);
            this.buttonOK.TabIndex = 0;
            this.buttonOK.Text = "L10n L&aunch";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // checkBoxRemeber
            // 
            this.checkBoxRemeber.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.checkBoxRemeber.AutoSize = true;
            this.checkBoxRemeber.Location = new System.Drawing.Point(22, 321);
            this.checkBoxRemeber.Name = "checkBoxRemeber";
            this.checkBoxRemeber.Size = new System.Drawing.Size(159, 17);
            this.checkBoxRemeber.TabIndex = 2;
            this.checkBoxRemeber.Text = "L10n remember my changes";
            this.checkBoxRemeber.UseVisualStyleBackColor = true;
            // 
            // groupBoxGame
            // 
            this.groupBoxGame.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBoxGame.Controls.Add(this.buttonExe);
            this.groupBoxGame.Controls.Add(this.textBoxExe);
            this.groupBoxGame.Controls.Add(this.labelExe);
            this.groupBoxGame.Location = new System.Drawing.Point(12, 12);
            this.groupBoxGame.Name = "groupBoxGame";
            this.groupBoxGame.Size = new System.Drawing.Size(564, 48);
            this.groupBoxGame.TabIndex = 3;
            this.groupBoxGame.TabStop = false;
            this.groupBoxGame.Text = "L10n Game";
            // 
            // buttonExe
            // 
            this.buttonExe.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonExe.Location = new System.Drawing.Point(462, 15);
            this.buttonExe.Name = "buttonExe";
            this.buttonExe.Size = new System.Drawing.Size(96, 23);
            this.buttonExe.TabIndex = 1;
            this.buttonExe.Text = "L10n Browse ...";
            this.buttonExe.UseVisualStyleBackColor = true;
            this.buttonExe.Click += new System.EventHandler(this.buttonExe_Click);
            // 
            // textBoxExe
            // 
            this.textBoxExe.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxExe.Location = new System.Drawing.Point(112, 17);
            this.textBoxExe.Name = "textBoxExe";
            this.textBoxExe.Size = new System.Drawing.Size(344, 20);
            this.textBoxExe.TabIndex = 2;
            // 
            // labelExe
            // 
            this.labelExe.AutoSize = true;
            this.labelExe.Location = new System.Drawing.Point(7, 20);
            this.labelExe.Name = "labelExe";
            this.labelExe.Size = new System.Drawing.Size(96, 13);
            this.labelExe.TabIndex = 0;
            this.labelExe.Text = "L10n cs2.exe file:";
            // 
            // groupBoxCmdOpts
            // 
            this.groupBoxCmdOpts.Controls.Add(this.checkBoxAvoidVac);
            this.groupBoxCmdOpts.Controls.Add(this.textBoxCustomCmd);
            this.groupBoxCmdOpts.Location = new System.Drawing.Point(12, 244);
            this.groupBoxCmdOpts.Name = "groupBoxCmdOpts";
            this.groupBoxCmdOpts.Size = new System.Drawing.Size(564, 71);
            this.groupBoxCmdOpts.TabIndex = 6;
            this.groupBoxCmdOpts.TabStop = false;
            this.groupBoxCmdOpts.Text = "L10n Custom command line options";
            // 
            // checkBoxAvoidVac
            // 
            this.checkBoxAvoidVac.AutoSize = true;
            this.checkBoxAvoidVac.Checked = true;
            this.checkBoxAvoidVac.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxAvoidVac.Enabled = false;
            this.checkBoxAvoidVac.Location = new System.Drawing.Point(10, 21);
            this.checkBoxAvoidVac.Name = "checkBoxAvoidVac";
            this.checkBoxAvoidVac.Size = new System.Drawing.Size(334, 17);
            this.checkBoxAvoidVac.TabIndex = 0;
            this.checkBoxAvoidVac.Text = "L10n -insecure (prevents joining VAC secured server / VAC bans)";
            this.checkBoxAvoidVac.UseVisualStyleBackColor = true;
            // 
            // textBoxCustomCmd
            // 
            this.textBoxCustomCmd.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxCustomCmd.Location = new System.Drawing.Point(10, 44);
            this.textBoxCustomCmd.Name = "textBoxCustomCmd";
            this.textBoxCustomCmd.Size = new System.Drawing.Size(547, 20);
            this.textBoxCustomCmd.TabIndex = 1;
            // 
            // groupBoxRes
            // 
            this.groupBoxRes.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBoxRes.Controls.Add(this.checkBoxEnableGfx);
            this.groupBoxRes.Controls.Add(this.labelGfxInfo);
            this.groupBoxRes.Controls.Add(this.checkBoxGfxFull);
            this.groupBoxRes.Controls.Add(this.textBoxGfxHeight);
            this.groupBoxRes.Controls.Add(this.labelGfxHeight);
            this.groupBoxRes.Controls.Add(this.textBoxGfxWidth);
            this.groupBoxRes.Controls.Add(this.labelGfxWidth);
            this.groupBoxRes.Location = new System.Drawing.Point(12, 145);
            this.groupBoxRes.Name = "groupBoxRes";
            this.groupBoxRes.Size = new System.Drawing.Size(564, 93);
            this.groupBoxRes.TabIndex = 5;
            this.groupBoxRes.TabStop = false;
            this.groupBoxRes.Text = "L10n Graphics Resolution";
            // 
            // checkBoxEnableGfx
            // 
            this.checkBoxEnableGfx.AutoSize = true;
            this.checkBoxEnableGfx.Location = new System.Drawing.Point(10, 19);
            this.checkBoxEnableGfx.Name = "checkBoxEnableGfx";
            this.checkBoxEnableGfx.Size = new System.Drawing.Size(85, 17);
            this.checkBoxEnableGfx.TabIndex = 0;
            this.checkBoxEnableGfx.Text = "L10n enable";
            this.checkBoxEnableGfx.UseVisualStyleBackColor = true;
            this.checkBoxEnableGfx.CheckedChanged += new System.EventHandler(this.checkBoxEnableGfx_CheckedChanged);
            // 
            // labelGfxInfo
            // 
            this.labelGfxInfo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelGfxInfo.Location = new System.Drawing.Point(301, 45);
            this.labelGfxInfo.Name = "labelGfxInfo";
            this.labelGfxInfo.Size = new System.Drawing.Size(256, 35);
            this.labelGfxInfo.TabIndex = 6;
            this.labelGfxInfo.Text = "L10n Actual results depend on the game.";
            // 
            // checkBoxGfxFull
            // 
            this.checkBoxGfxFull.AutoSize = true;
            this.checkBoxGfxFull.Location = new System.Drawing.Point(164, 64);
            this.checkBoxGfxFull.Name = "checkBoxGfxFull";
            this.checkBoxGfxFull.Size = new System.Drawing.Size(101, 17);
            this.checkBoxGfxFull.TabIndex = 5;
            this.checkBoxGfxFull.Text = "L10n full screen";
            this.checkBoxGfxFull.UseVisualStyleBackColor = true;
            // 
            // textBoxGfxHeight
            // 
            this.textBoxGfxHeight.Location = new System.Drawing.Point(85, 62);
            this.textBoxGfxHeight.Name = "textBoxGfxHeight";
            this.textBoxGfxHeight.Size = new System.Drawing.Size(56, 20);
            this.textBoxGfxHeight.TabIndex = 4;
            // 
            // labelGfxHeight
            // 
            this.labelGfxHeight.AutoSize = true;
            this.labelGfxHeight.Location = new System.Drawing.Point(82, 45);
            this.labelGfxHeight.Name = "labelGfxHeight";
            this.labelGfxHeight.Size = new System.Drawing.Size(68, 13);
            this.labelGfxHeight.TabIndex = 3;
            this.labelGfxHeight.Text = "L10n Height:";
            // 
            // textBoxGfxWidth
            // 
            this.textBoxGfxWidth.Location = new System.Drawing.Point(10, 62);
            this.textBoxGfxWidth.Name = "textBoxGfxWidth";
            this.textBoxGfxWidth.Size = new System.Drawing.Size(56, 20);
            this.textBoxGfxWidth.TabIndex = 2;
            // 
            // labelGfxWidth
            // 
            this.labelGfxWidth.AutoSize = true;
            this.labelGfxWidth.Location = new System.Drawing.Point(7, 45);
            this.labelGfxWidth.Name = "labelGfxWidth";
            this.labelGfxWidth.Size = new System.Drawing.Size(65, 13);
            this.labelGfxWidth.TabIndex = 1;
            this.labelGfxWidth.Text = "L10n Width:";
            // 
            // groupBoxMmcfg
            // 
            this.groupBoxMmcfg.Controls.Add(this.buttonMmcfgInfo);
            this.groupBoxMmcfg.Controls.Add(this.checkBoxEnableMmcfg);
            this.groupBoxMmcfg.Controls.Add(this.buttonMmcfg);
            this.groupBoxMmcfg.Controls.Add(this.textBoxMmcfg);
            this.groupBoxMmcfg.Location = new System.Drawing.Point(12, 66);
            this.groupBoxMmcfg.Name = "groupBoxMmcfg";
            this.groupBoxMmcfg.Size = new System.Drawing.Size(564, 73);
            this.groupBoxMmcfg.TabIndex = 4;
            this.groupBoxMmcfg.TabStop = false;
            this.groupBoxMmcfg.Text = "L10n Moviemaking config parent folder";
            // 
            // buttonMmcfgInfo
            // 
            this.buttonMmcfgInfo.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonMmcfgInfo.BackColor = System.Drawing.SystemColors.Highlight;
            this.buttonMmcfgInfo.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.buttonMmcfgInfo.ForeColor = System.Drawing.SystemColors.HighlightText;
            this.buttonMmcfgInfo.Location = new System.Drawing.Point(354, 15);
            this.buttonMmcfgInfo.Name = "buttonMmcfgInfo";
            this.buttonMmcfgInfo.Size = new System.Drawing.Size(203, 23);
            this.buttonMmcfgInfo.TabIndex = 1;
            this.buttonMmcfgInfo.Text = "L10n Information";
            this.buttonMmcfgInfo.UseVisualStyleBackColor = false;
            this.buttonMmcfgInfo.Click += new System.EventHandler(this.buttonMmcfgInfo_Click);
            // 
            // checkBoxEnableMmcfg
            // 
            this.checkBoxEnableMmcfg.AutoSize = true;
            this.checkBoxEnableMmcfg.Location = new System.Drawing.Point(10, 19);
            this.checkBoxEnableMmcfg.Name = "checkBoxEnableMmcfg";
            this.checkBoxEnableMmcfg.Size = new System.Drawing.Size(85, 17);
            this.checkBoxEnableMmcfg.TabIndex = 0;
            this.checkBoxEnableMmcfg.Text = "L10n enable";
            this.checkBoxEnableMmcfg.UseVisualStyleBackColor = true;
            this.checkBoxEnableMmcfg.CheckedChanged += new System.EventHandler(this.checkBoxEnableMmcfg_CheckedChanged);
            // 
            // buttonMmcfg
            // 
            this.buttonMmcfg.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonMmcfg.Location = new System.Drawing.Point(462, 44);
            this.buttonMmcfg.Name = "buttonMmcfg";
            this.buttonMmcfg.Size = new System.Drawing.Size(96, 23);
            this.buttonMmcfg.TabIndex = 2;
            this.buttonMmcfg.Text = "L10n Browse ...";
            this.buttonMmcfg.UseVisualStyleBackColor = true;
            this.buttonMmcfg.Click += new System.EventHandler(this.buttonMmcfg_Click);
            // 
            // textBoxMmcfg
            // 
            this.textBoxMmcfg.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxMmcfg.Location = new System.Drawing.Point(10, 46);
            this.textBoxMmcfg.Name = "textBoxMmcfg";
            this.textBoxMmcfg.Size = new System.Drawing.Size(446, 20);
            this.textBoxMmcfg.TabIndex = 3;
            // 
            // openFileDialogExe
            // 
            this.openFileDialogExe.DefaultExt = "exe";
            this.openFileDialogExe.FileName = "cs2.exe";
            this.openFileDialogExe.Filter = "CS2 executable|cs2.exe";
            // 
            // folderBrowserDialogMMcfg
            // 
            this.folderBrowserDialogMMcfg.Description = "Select moviemaking config parent folder";
            // 
            // LaunchCs2Form
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.groupBoxMmcfg);
            this.Controls.Add(this.groupBoxCmdOpts);
            this.Controls.Add(this.groupBoxRes);
            this.Controls.Add(this.groupBoxGame);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.checkBoxRemeber);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(600, 400);
            this.Name = "LaunchCs2Form";
            this.ShowInTaskbar = false;
            this.Text = "L10n Launch CS2 ...";
            this.groupBoxGame.ResumeLayout(false);
            this.groupBoxGame.PerformLayout();
            this.groupBoxCmdOpts.ResumeLayout(false);
            this.groupBoxCmdOpts.PerformLayout();
            this.groupBoxRes.ResumeLayout(false);
            this.groupBoxRes.PerformLayout();
            this.groupBoxMmcfg.ResumeLayout(false);
            this.groupBoxMmcfg.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.CheckBox checkBoxRemeber;
        private System.Windows.Forms.GroupBox groupBoxGame;
        private System.Windows.Forms.Button buttonExe;
        private System.Windows.Forms.TextBox textBoxExe;
        private System.Windows.Forms.Label labelExe;
        private System.Windows.Forms.GroupBox groupBoxCmdOpts;
        private System.Windows.Forms.TextBox textBoxCustomCmd;
        private System.Windows.Forms.GroupBox groupBoxRes;
        private System.Windows.Forms.CheckBox checkBoxGfxFull;
        private System.Windows.Forms.TextBox textBoxGfxHeight;
        private System.Windows.Forms.Label labelGfxHeight;
        private System.Windows.Forms.TextBox textBoxGfxWidth;
        private System.Windows.Forms.Label labelGfxWidth;
        private System.Windows.Forms.CheckBox checkBoxAvoidVac;
        private System.Windows.Forms.Label labelGfxInfo;
        private System.Windows.Forms.GroupBox groupBoxMmcfg;
        private System.Windows.Forms.Button buttonMmcfg;
        private System.Windows.Forms.TextBox textBoxMmcfg;
        private System.Windows.Forms.CheckBox checkBoxEnableGfx;
        private System.Windows.Forms.Button buttonMmcfgInfo;
        private System.Windows.Forms.CheckBox checkBoxEnableMmcfg;
        private System.Windows.Forms.OpenFileDialog openFileDialogExe;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogMMcfg;
    }
}
