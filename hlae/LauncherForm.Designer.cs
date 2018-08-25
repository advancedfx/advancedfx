namespace AfxGui
{
    partial class LauncherForm
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
            this.components = new System.ComponentModel.Container();
            this.labelResDepth = new System.Windows.Forms.Label();
            this.textBoxResHeight = new System.Windows.Forms.TextBox();
            this.labelResHeight = new System.Windows.Forms.Label();
            this.checkBoxVisbility = new System.Windows.Forms.CheckBox();
            this.comboBoxResDepth = new System.Windows.Forms.ComboBox();
            this.checkBoxDesktopRes = new System.Windows.Forms.CheckBox();
            this.checkBoxForceAlpha = new System.Windows.Forms.CheckBox();
            this.groupBoxGame = new System.Windows.Forms.GroupBox();
            this.textBoxCustMod = new System.Windows.Forms.TextBox();
            this.comboBoxModSel = new System.Windows.Forms.ComboBox();
            this.labelModSel = new System.Windows.Forms.Label();
            this.buttonExe = new System.Windows.Forms.Button();
            this.textBoxExe = new System.Windows.Forms.TextBox();
            this.labelExe = new System.Windows.Forms.Label();
            this.checkBoxFullScreen = new System.Windows.Forms.CheckBox();
            this.comboBoxRenderMode = new System.Windows.Forms.ComboBox();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.textBoxCmdAdd = new System.Windows.Forms.TextBox();
            this.groupBoxCmdOpts = new System.Windows.Forms.GroupBox();
            this.labelRenderMode = new System.Windows.Forms.Label();
            this.openFileDialogExe = new System.Windows.Forms.OpenFileDialog();
            this.groupBoxMisc = new System.Windows.Forms.GroupBox();
            this.checkBoxResForce = new System.Windows.Forms.CheckBox();
            this.textBoxResWidth = new System.Windows.Forms.TextBox();
            this.groupBoxRes = new System.Windows.Forms.GroupBox();
            this.labelResWidth = new System.Windows.Forms.Label();
            this.checkBoxRemeber = new System.Windows.Forms.CheckBox();
            this.errorProvider = new System.Windows.Forms.ErrorProvider(this.components);
            this.groupBoxGame.SuspendLayout();
            this.groupBoxCmdOpts.SuspendLayout();
            this.groupBoxMisc.SuspendLayout();
            this.groupBoxRes.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).BeginInit();
            this.SuspendLayout();
            // 
            // labelResDepth
            // 
            this.labelResDepth.AutoSize = true;
            this.labelResDepth.Location = new System.Drawing.Point(154, 18);
            this.labelResDepth.Name = "labelResDepth";
            this.labelResDepth.Size = new System.Drawing.Size(66, 13);
            this.labelResDepth.TabIndex = 4;
            this.labelResDepth.Text = "Color Depth:";
            // 
            // textBoxResHeight
            // 
            this.textBoxResHeight.Location = new System.Drawing.Point(85, 35);
            this.textBoxResHeight.Name = "textBoxResHeight";
            this.textBoxResHeight.Size = new System.Drawing.Size(56, 20);
            this.textBoxResHeight.TabIndex = 3;
            this.textBoxResHeight.TextChanged += new System.EventHandler(this.textBoxResHeight_TextChanged);
            // 
            // labelResHeight
            // 
            this.labelResHeight.AutoSize = true;
            this.labelResHeight.Location = new System.Drawing.Point(82, 18);
            this.labelResHeight.Name = "labelResHeight";
            this.labelResHeight.Size = new System.Drawing.Size(41, 13);
            this.labelResHeight.TabIndex = 2;
            this.labelResHeight.Text = "Height:";
            // 
            // checkBoxVisbility
            // 
            this.checkBoxVisbility.AutoSize = true;
            this.checkBoxVisbility.Location = new System.Drawing.Point(10, 42);
            this.checkBoxVisbility.Name = "checkBoxVisbility";
            this.checkBoxVisbility.Size = new System.Drawing.Size(195, 17);
            this.checkBoxVisbility.TabIndex = 1;
            this.checkBoxVisbility.Text = "Optimize window visibilty on capture";
            this.checkBoxVisbility.UseVisualStyleBackColor = true;
            // 
            // comboBoxResDepth
            // 
            this.comboBoxResDepth.FormattingEnabled = true;
            this.comboBoxResDepth.Items.AddRange(new object[] {
            "32 (High)",
            "24 (Medium)",
            "16 (Low)"});
            this.comboBoxResDepth.Location = new System.Drawing.Point(157, 35);
            this.comboBoxResDepth.Name = "comboBoxResDepth";
            this.comboBoxResDepth.Size = new System.Drawing.Size(88, 21);
            this.comboBoxResDepth.TabIndex = 5;
            // 
            // checkBoxDesktopRes
            // 
            this.checkBoxDesktopRes.AutoSize = true;
            this.checkBoxDesktopRes.Enabled = false;
            this.checkBoxDesktopRes.Location = new System.Drawing.Point(10, 65);
            this.checkBoxDesktopRes.Name = "checkBoxDesktopRes";
            this.checkBoxDesktopRes.Size = new System.Drawing.Size(155, 17);
            this.checkBoxDesktopRes.TabIndex = 4;
            this.checkBoxDesktopRes.Text = "Optimize desktop resolution";
            this.checkBoxDesktopRes.UseVisualStyleBackColor = true;
            // 
            // checkBoxForceAlpha
            // 
            this.checkBoxForceAlpha.AutoSize = true;
            this.checkBoxForceAlpha.Location = new System.Drawing.Point(10, 19);
            this.checkBoxForceAlpha.Name = "checkBoxForceAlpha";
            this.checkBoxForceAlpha.Size = new System.Drawing.Size(146, 17);
            this.checkBoxForceAlpha.TabIndex = 0;
            this.checkBoxForceAlpha.Text = "Force 8 bit alpha channel";
            this.checkBoxForceAlpha.UseVisualStyleBackColor = true;
            // 
            // groupBoxGame
            // 
            this.groupBoxGame.Controls.Add(this.textBoxCustMod);
            this.groupBoxGame.Controls.Add(this.comboBoxModSel);
            this.groupBoxGame.Controls.Add(this.labelModSel);
            this.groupBoxGame.Controls.Add(this.buttonExe);
            this.groupBoxGame.Controls.Add(this.textBoxExe);
            this.groupBoxGame.Controls.Add(this.labelExe);
            this.groupBoxGame.Location = new System.Drawing.Point(2, 7);
            this.groupBoxGame.Name = "groupBoxGame";
            this.groupBoxGame.Size = new System.Drawing.Size(469, 86);
            this.groupBoxGame.TabIndex = 10;
            this.groupBoxGame.TabStop = false;
            this.groupBoxGame.Text = "Game";
            // 
            // textBoxCustMod
            // 
            this.textBoxCustMod.Enabled = false;
            this.textBoxCustMod.Location = new System.Drawing.Point(342, 51);
            this.textBoxCustMod.Name = "textBoxCustMod";
            this.textBoxCustMod.Size = new System.Drawing.Size(113, 20);
            this.textBoxCustMod.TabIndex = 5;
            this.textBoxCustMod.Text = "custom";
            // 
            // comboBoxModSel
            // 
            this.comboBoxModSel.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxModSel.FormattingEnabled = true;
            this.comboBoxModSel.Items.AddRange(new object[] {
            "cstrike (Counter-Strike)",
            "dod (Day of Defeat)",
            "tfc (Team Fortress Classic)",
            "valve (Half-Life)",
            "Other modification:"});
            this.comboBoxModSel.Location = new System.Drawing.Point(112, 51);
            this.comboBoxModSel.Name = "comboBoxModSel";
            this.comboBoxModSel.Size = new System.Drawing.Size(224, 21);
            this.comboBoxModSel.TabIndex = 4;
            this.comboBoxModSel.SelectedIndexChanged += new System.EventHandler(this.comboBoxModSel_SelectedIndexChanged);
            // 
            // labelModSel
            // 
            this.labelModSel.AutoSize = true;
            this.labelModSel.Location = new System.Drawing.Point(7, 54);
            this.labelModSel.Name = "labelModSel";
            this.labelModSel.Size = new System.Drawing.Size(67, 13);
            this.labelModSel.TabIndex = 3;
            this.labelModSel.Text = "Modification:";
            // 
            // buttonExe
            // 
            this.buttonExe.Location = new System.Drawing.Point(342, 15);
            this.buttonExe.Name = "buttonExe";
            this.buttonExe.Size = new System.Drawing.Size(96, 23);
            this.buttonExe.TabIndex = 1;
            this.buttonExe.Text = "Browse ...";
            this.buttonExe.UseVisualStyleBackColor = true;
            this.buttonExe.Click += new System.EventHandler(this.buttonExe_Click);
            // 
            // textBoxExe
            // 
            this.textBoxExe.Location = new System.Drawing.Point(112, 17);
            this.textBoxExe.Name = "textBoxExe";
            this.textBoxExe.Size = new System.Drawing.Size(224, 20);
            this.textBoxExe.TabIndex = 2;
            // 
            // labelExe
            // 
            this.labelExe.AutoSize = true;
            this.labelExe.Location = new System.Drawing.Point(7, 20);
            this.labelExe.Name = "labelExe";
            this.labelExe.Size = new System.Drawing.Size(54, 13);
            this.labelExe.TabIndex = 0;
            this.labelExe.Text = "hl.exe file:";
            // 
            // checkBoxFullScreen
            // 
            this.checkBoxFullScreen.AutoSize = true;
            this.checkBoxFullScreen.Location = new System.Drawing.Point(275, 39);
            this.checkBoxFullScreen.Name = "checkBoxFullScreen";
            this.checkBoxFullScreen.Size = new System.Drawing.Size(74, 17);
            this.checkBoxFullScreen.TabIndex = 7;
            this.checkBoxFullScreen.Text = "full screen";
            this.checkBoxFullScreen.UseVisualStyleBackColor = true;
            this.checkBoxFullScreen.Click += new System.EventHandler(this.checkBoxFullScreen_Click);
            // 
            // comboBoxRenderMode
            // 
            this.comboBoxRenderMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxRenderMode.FormattingEnabled = true;
            this.comboBoxRenderMode.Items.AddRange(new object[] {
            "Standard",
            "FrameBuffer Object",
            "Memory DC"});
            this.comboBoxRenderMode.Location = new System.Drawing.Point(259, 32);
            this.comboBoxRenderMode.Name = "comboBoxRenderMode";
            this.comboBoxRenderMode.Size = new System.Drawing.Size(181, 21);
            this.comboBoxRenderMode.TabIndex = 3;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(385, 332);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 28);
            this.buttonCancel.TabIndex = 8;
            this.buttonCancel.Text = "Can&cel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(292, 332);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 28);
            this.buttonOK.TabIndex = 7;
            this.buttonOK.Text = "L&aunch";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // textBoxCmdAdd
            // 
            this.textBoxCmdAdd.Location = new System.Drawing.Point(10, 19);
            this.textBoxCmdAdd.Name = "textBoxCmdAdd";
            this.textBoxCmdAdd.Size = new System.Drawing.Size(446, 20);
            this.textBoxCmdAdd.TabIndex = 0;
            // 
            // groupBoxCmdOpts
            // 
            this.groupBoxCmdOpts.Controls.Add(this.textBoxCmdAdd);
            this.groupBoxCmdOpts.Location = new System.Drawing.Point(2, 99);
            this.groupBoxCmdOpts.Name = "groupBoxCmdOpts";
            this.groupBoxCmdOpts.Size = new System.Drawing.Size(469, 50);
            this.groupBoxCmdOpts.TabIndex = 11;
            this.groupBoxCmdOpts.TabStop = false;
            this.groupBoxCmdOpts.Text = "Custom command line options";
            // 
            // labelRenderMode
            // 
            this.labelRenderMode.AutoSize = true;
            this.labelRenderMode.Location = new System.Drawing.Point(256, 16);
            this.labelRenderMode.Name = "labelRenderMode";
            this.labelRenderMode.Size = new System.Drawing.Size(72, 13);
            this.labelRenderMode.TabIndex = 2;
            this.labelRenderMode.Text = "RenderMode:";
            // 
            // openFileDialogExe
            // 
            this.openFileDialogExe.DefaultExt = "exe";
            this.openFileDialogExe.FileName = "hl.exe";
            this.openFileDialogExe.Filter = "Half-Life executeable|hl.exe";
            // 
            // groupBoxMisc
            // 
            this.groupBoxMisc.Controls.Add(this.checkBoxVisbility);
            this.groupBoxMisc.Controls.Add(this.checkBoxDesktopRes);
            this.groupBoxMisc.Controls.Add(this.checkBoxForceAlpha);
            this.groupBoxMisc.Controls.Add(this.comboBoxRenderMode);
            this.groupBoxMisc.Controls.Add(this.labelRenderMode);
            this.groupBoxMisc.Location = new System.Drawing.Point(2, 224);
            this.groupBoxMisc.Name = "groupBoxMisc";
            this.groupBoxMisc.Size = new System.Drawing.Size(469, 98);
            this.groupBoxMisc.TabIndex = 13;
            this.groupBoxMisc.TabStop = false;
            this.groupBoxMisc.Text = "Advanced Settings";
            // 
            // checkBoxResForce
            // 
            this.checkBoxResForce.AutoSize = true;
            this.checkBoxResForce.Location = new System.Drawing.Point(275, 14);
            this.checkBoxResForce.Name = "checkBoxResForce";
            this.checkBoxResForce.Size = new System.Drawing.Size(98, 17);
            this.checkBoxResForce.TabIndex = 6;
            this.checkBoxResForce.Text = "force resolution";
            this.checkBoxResForce.UseVisualStyleBackColor = true;
            // 
            // textBoxResWidth
            // 
            this.textBoxResWidth.Location = new System.Drawing.Point(10, 35);
            this.textBoxResWidth.Name = "textBoxResWidth";
            this.textBoxResWidth.Size = new System.Drawing.Size(56, 20);
            this.textBoxResWidth.TabIndex = 1;
            // 
            // groupBoxRes
            // 
            this.groupBoxRes.Controls.Add(this.checkBoxFullScreen);
            this.groupBoxRes.Controls.Add(this.comboBoxResDepth);
            this.groupBoxRes.Controls.Add(this.labelResDepth);
            this.groupBoxRes.Controls.Add(this.textBoxResHeight);
            this.groupBoxRes.Controls.Add(this.labelResHeight);
            this.groupBoxRes.Controls.Add(this.textBoxResWidth);
            this.groupBoxRes.Controls.Add(this.labelResWidth);
            this.groupBoxRes.Controls.Add(this.checkBoxResForce);
            this.groupBoxRes.Location = new System.Drawing.Point(2, 155);
            this.groupBoxRes.Name = "groupBoxRes";
            this.groupBoxRes.Size = new System.Drawing.Size(469, 63);
            this.groupBoxRes.TabIndex = 12;
            this.groupBoxRes.TabStop = false;
            this.groupBoxRes.Text = "Graphic Resolution";
            // 
            // labelResWidth
            // 
            this.labelResWidth.AutoSize = true;
            this.labelResWidth.Location = new System.Drawing.Point(7, 19);
            this.labelResWidth.Name = "labelResWidth";
            this.labelResWidth.Size = new System.Drawing.Size(38, 13);
            this.labelResWidth.TabIndex = 0;
            this.labelResWidth.Text = "Width:";
            // 
            // checkBoxRemeber
            // 
            this.checkBoxRemeber.AutoSize = true;
            this.checkBoxRemeber.Location = new System.Drawing.Point(12, 332);
            this.checkBoxRemeber.Name = "checkBoxRemeber";
            this.checkBoxRemeber.Size = new System.Drawing.Size(132, 17);
            this.checkBoxRemeber.TabIndex = 9;
            this.checkBoxRemeber.Text = "remember my changes";
            this.checkBoxRemeber.UseVisualStyleBackColor = true;
            // 
            // errorProvider
            // 
            this.errorProvider.ContainerControl = this;
            // 
            // LauncherForm
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(472, 366);
            this.Controls.Add(this.groupBoxGame);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.groupBoxCmdOpts);
            this.Controls.Add(this.groupBoxMisc);
            this.Controls.Add(this.groupBoxRes);
            this.Controls.Add(this.checkBoxRemeber);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "LauncherForm";
            this.Text = "Launch GoldSrc ...";
            this.groupBoxGame.ResumeLayout(false);
            this.groupBoxGame.PerformLayout();
            this.groupBoxCmdOpts.ResumeLayout(false);
            this.groupBoxCmdOpts.PerformLayout();
            this.groupBoxMisc.ResumeLayout(false);
            this.groupBoxMisc.PerformLayout();
            this.groupBoxRes.ResumeLayout(false);
            this.groupBoxRes.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelResDepth;
        private System.Windows.Forms.TextBox textBoxResHeight;
        private System.Windows.Forms.Label labelResHeight;
        private System.Windows.Forms.CheckBox checkBoxVisbility;
        private System.Windows.Forms.ComboBox comboBoxResDepth;
        private System.Windows.Forms.CheckBox checkBoxDesktopRes;
        private System.Windows.Forms.CheckBox checkBoxForceAlpha;
        private System.Windows.Forms.GroupBox groupBoxGame;
        private System.Windows.Forms.TextBox textBoxCustMod;
        private System.Windows.Forms.ComboBox comboBoxModSel;
        private System.Windows.Forms.Label labelModSel;
        private System.Windows.Forms.Button buttonExe;
        private System.Windows.Forms.TextBox textBoxExe;
        private System.Windows.Forms.Label labelExe;
        private System.Windows.Forms.CheckBox checkBoxFullScreen;
        private System.Windows.Forms.ComboBox comboBoxRenderMode;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.TextBox textBoxCmdAdd;
        private System.Windows.Forms.GroupBox groupBoxCmdOpts;
        private System.Windows.Forms.Label labelRenderMode;
        private System.Windows.Forms.OpenFileDialog openFileDialogExe;
        private System.Windows.Forms.GroupBox groupBoxMisc;
        private System.Windows.Forms.CheckBox checkBoxResForce;
        private System.Windows.Forms.TextBox textBoxResWidth;
        private System.Windows.Forms.GroupBox groupBoxRes;
        private System.Windows.Forms.Label labelResWidth;
        private System.Windows.Forms.CheckBox checkBoxRemeber;
        private System.Windows.Forms.ErrorProvider errorProvider;
    }
}