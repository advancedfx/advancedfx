namespace AfxGui
{
    partial class MainForm
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
            this.mainMenu = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuLaunchCSGO = new System.Windows.Forms.ToolStripMenuItem();
            this.launchGoldSrcToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.menuExit = new System.Windows.Forms.ToolStripMenuItem();
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStatusBar = new System.Windows.Forms.ToolStripMenuItem();
            this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.calculatorsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuFileSize = new System.Windows.Forms.ToolStripMenuItem();
            this.goldSrcToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.demoToolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.skyManagerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.developerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuCustomLoader = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
            this.menuGuidToClipBoard = new System.Windows.Forms.ToolStripMenuItem();
            this.menuNewGuidToClipBoard = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.manualToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.checkForUpdatesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuAutoUpdateCheck = new System.Windows.Forms.ToolStripMenuItem();
            this.checkNowToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
            this.menuAdvancedFxOrg = new System.Windows.Forms.ToolStripMenuItem();
            this.donateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.statusLabelUpdate = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelIgnore = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelHide = new System.Windows.Forms.ToolStripStatusLabel();
            this.stripEnableUpdateCheck = new System.Windows.Forms.StatusStrip();
            this.checkUpdatesLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelAutoYes = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelAutoNo = new System.Windows.Forms.ToolStripStatusLabel();
            this.panel1 = new System.Windows.Forms.Panel();
            this.buttonManual = new System.Windows.Forms.Button();
            this.mainMenu.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.stripEnableUpdateCheck.SuspendLayout();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.toolsToolStripMenuItem,
            this.helpToolStripMenuItem,
            this.donateToolStripMenuItem});
            this.mainMenu.Location = new System.Drawing.Point(0, 0);
            this.mainMenu.Name = "mainMenu";
            this.mainMenu.ShowItemToolTips = true;
            this.mainMenu.Size = new System.Drawing.Size(584, 24);
            this.mainMenu.TabIndex = 0;
            this.mainMenu.Text = "mainMenu";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuLaunchCSGO,
            this.launchGoldSrcToolStripMenuItem,
            this.toolStripMenuItem1,
            this.menuExit});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(65, 20);
            this.fileToolStripMenuItem.Text = "L10n File";
            // 
            // menuLaunchCSGO
            // 
            this.menuLaunchCSGO.Name = "menuLaunchCSGO";
            this.menuLaunchCSGO.Size = new System.Drawing.Size(185, 22);
            this.menuLaunchCSGO.Text = "L10n Launch CS:GO";
            this.menuLaunchCSGO.Click += new System.EventHandler(this.MenuLaunchCSGO_Click);
            // 
            // launchGoldSrcToolStripMenuItem
            // 
            this.launchGoldSrcToolStripMenuItem.Name = "launchGoldSrcToolStripMenuItem";
            this.launchGoldSrcToolStripMenuItem.Size = new System.Drawing.Size(185, 22);
            this.launchGoldSrcToolStripMenuItem.Text = "L10n Launch GoldSrc";
            this.launchGoldSrcToolStripMenuItem.Click += new System.EventHandler(this.MenuLaunchGoldSrc_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(182, 6);
            // 
            // menuExit
            // 
            this.menuExit.Name = "menuExit";
            this.menuExit.Size = new System.Drawing.Size(185, 22);
            this.menuExit.Text = "L10n Exit";
            this.menuExit.Click += new System.EventHandler(this.MenuExit_Click);
            // 
            // viewToolStripMenuItem
            // 
            this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuStatusBar});
            this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
            this.viewToolStripMenuItem.Size = new System.Drawing.Size(72, 20);
            this.viewToolStripMenuItem.Text = "L10n View";
            // 
            // menuStatusBar
            // 
            this.menuStatusBar.Name = "menuStatusBar";
            this.menuStatusBar.Size = new System.Drawing.Size(154, 22);
            this.menuStatusBar.Text = "L10n Status Bar";
            this.menuStatusBar.Click += new System.EventHandler(this.StatusBarToolStripMenuItem_Click);
            // 
            // toolsToolStripMenuItem
            // 
            this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.calculatorsToolStripMenuItem,
            this.goldSrcToolStripMenuItem,
            this.toolStripMenuItem2,
            this.developerToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(74, 20);
            this.toolsToolStripMenuItem.Text = "L10n Tools";
            // 
            // calculatorsToolStripMenuItem
            // 
            this.calculatorsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuFileSize});
            this.calculatorsToolStripMenuItem.Name = "calculatorsToolStripMenuItem";
            this.calculatorsToolStripMenuItem.Size = new System.Drawing.Size(161, 22);
            this.calculatorsToolStripMenuItem.Text = "L10n Calculators";
            // 
            // menuFileSize
            // 
            this.menuFileSize.Name = "menuFileSize";
            this.menuFileSize.Size = new System.Drawing.Size(143, 22);
            this.menuFileSize.Text = "L10n File Size";
            this.menuFileSize.Click += new System.EventHandler(this.MenuFileSize_Click);
            // 
            // goldSrcToolStripMenuItem
            // 
            this.goldSrcToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.demoToolsToolStripMenuItem,
            this.skyManagerToolStripMenuItem});
            this.goldSrcToolStripMenuItem.Name = "goldSrcToolStripMenuItem";
            this.goldSrcToolStripMenuItem.Size = new System.Drawing.Size(161, 22);
            this.goldSrcToolStripMenuItem.Text = "L10n GoldSrc";
            // 
            // demoToolsToolStripMenuItem
            // 
            this.demoToolsToolStripMenuItem.Name = "demoToolsToolStripMenuItem";
            this.demoToolsToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
            this.demoToolsToolStripMenuItem.Text = "L10n Demo Tools";
            this.demoToolsToolStripMenuItem.Click += new System.EventHandler(this.MenuToolsGoldSrcDemoTools_Click);
            // 
            // skyManagerToolStripMenuItem
            // 
            this.skyManagerToolStripMenuItem.Name = "skyManagerToolStripMenuItem";
            this.skyManagerToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
            this.skyManagerToolStripMenuItem.Text = "L10n Sky Manager";
            this.skyManagerToolStripMenuItem.Click += new System.EventHandler(this.MenuToolsGoldSrcSkyManager_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(158, 6);
            // 
            // developerToolStripMenuItem
            // 
            this.developerToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuCustomLoader,
            this.toolStripMenuItem4,
            this.menuGuidToClipBoard,
            this.menuNewGuidToClipBoard});
            this.developerToolStripMenuItem.Name = "developerToolStripMenuItem";
            this.developerToolStripMenuItem.Size = new System.Drawing.Size(161, 22);
            this.developerToolStripMenuItem.Text = "l10n Developer";
            // 
            // menuCustomLoader
            // 
            this.menuCustomLoader.Name = "menuCustomLoader";
            this.menuCustomLoader.Size = new System.Drawing.Size(226, 22);
            this.menuCustomLoader.Text = "L10n Custom Loader";
            this.menuCustomLoader.Click += new System.EventHandler(this.MenuCustomLoader_Click);
            // 
            // toolStripMenuItem4
            // 
            this.toolStripMenuItem4.Name = "toolStripMenuItem4";
            this.toolStripMenuItem4.Size = new System.Drawing.Size(223, 6);
            // 
            // menuGuidToClipBoard
            // 
            this.menuGuidToClipBoard.Name = "menuGuidToClipBoard";
            this.menuGuidToClipBoard.Size = new System.Drawing.Size(226, 22);
            this.menuGuidToClipBoard.Text = "L10n Own GUID to ClipBoard";
            this.menuGuidToClipBoard.Click += new System.EventHandler(this.MenuGuidToClipBoard_Click);
            // 
            // menuNewGuidToClipBoard
            // 
            this.menuNewGuidToClipBoard.Name = "menuNewGuidToClipBoard";
            this.menuNewGuidToClipBoard.Size = new System.Drawing.Size(226, 22);
            this.menuNewGuidToClipBoard.Text = "L10n New GUID to ClipBoard";
            this.menuNewGuidToClipBoard.Click += new System.EventHandler(this.MenuNewGuidToClipBoard_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.manualToolStripMenuItem,
            this.toolStripMenuItem3,
            this.checkForUpdatesToolStripMenuItem,
            this.toolStripMenuItem5,
            this.menuAdvancedFxOrg});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(72, 20);
            this.helpToolStripMenuItem.Text = "L10n Help";
            // 
            // manualToolStripMenuItem
            // 
            this.manualToolStripMenuItem.Name = "manualToolStripMenuItem";
            this.manualToolStripMenuItem.Size = new System.Drawing.Size(222, 22);
            this.manualToolStripMenuItem.Text = "L10n Online Manual (LANG)";
            this.manualToolStripMenuItem.ToolTipText = "L10n URL";
            this.manualToolStripMenuItem.Click += new System.EventHandler(this.openManual_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(196, 6);
            // 
            // checkForUpdatesToolStripMenuItem
            // 
            this.checkForUpdatesToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuAutoUpdateCheck,
            this.checkNowToolStripMenuItem});
            this.checkForUpdatesToolStripMenuItem.Name = "checkForUpdatesToolStripMenuItem";
            this.checkForUpdatesToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.checkForUpdatesToolStripMenuItem.Text = "L10n Check for Updates";
            // 
            // menuAutoUpdateCheck
            // 
            this.menuAutoUpdateCheck.Name = "menuAutoUpdateCheck";
            this.menuAutoUpdateCheck.Size = new System.Drawing.Size(164, 22);
            this.menuAutoUpdateCheck.Text = "L10n Auto Check";
            this.menuAutoUpdateCheck.Click += new System.EventHandler(this.MenuAutoUpdateCheck_Click);
            // 
            // checkNowToolStripMenuItem
            // 
            this.checkNowToolStripMenuItem.Name = "checkNowToolStripMenuItem";
            this.checkNowToolStripMenuItem.Size = new System.Drawing.Size(164, 22);
            this.checkNowToolStripMenuItem.Text = "L10n Check Now";
            this.checkNowToolStripMenuItem.Click += new System.EventHandler(this.CheckNowToolStripMenuItem_Click);
            // 
            // toolStripMenuItem5
            // 
            this.toolStripMenuItem5.Name = "toolStripMenuItem5";
            this.toolStripMenuItem5.Size = new System.Drawing.Size(196, 6);
            // 
            // menuAdvancedFxOrg
            // 
            this.menuAdvancedFxOrg.Name = "menuAdvancedFxOrg";
            this.menuAdvancedFxOrg.Size = new System.Drawing.Size(199, 22);
            this.menuAdvancedFxOrg.Text = "L10n Offical website";
            this.menuAdvancedFxOrg.ToolTipText = "https://www.advancedfx.org/";
            this.menuAdvancedFxOrg.Click += new System.EventHandler(this.MenuAdvancedFxOrg_Click);
            // 
            // donateToolStripMenuItem
            // 
            this.donateToolStripMenuItem.Name = "donateToolStripMenuItem";
            this.donateToolStripMenuItem.Size = new System.Drawing.Size(85, 20);
            this.donateToolStripMenuItem.Text = "L10n Donate";
            this.donateToolStripMenuItem.ToolTipText = "https://opencollective.com/advancedfx/";
            this.donateToolStripMenuItem.Click += new System.EventHandler(this.MenuDonate_Click);
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusLabelUpdate,
            this.statusLabelIgnore,
            this.statusLabelHide});
            this.statusStrip.Location = new System.Drawing.Point(0, 337);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(584, 24);
            this.statusStrip.TabIndex = 1;
            this.statusStrip.Text = "statusStrip1";
            this.statusStrip.Visible = false;
            this.statusStrip.VisibleChanged += new System.EventHandler(this.StatusStrip_VisibleChanged);
            // 
            // statusLabelUpdate
            // 
            this.statusLabelUpdate.Name = "statusLabelUpdate";
            this.statusLabelUpdate.Size = new System.Drawing.Size(441, 19);
            this.statusLabelUpdate.Spring = true;
            this.statusLabelUpdate.Text = "L10n Update status unknown";
            this.statusLabelUpdate.Click += new System.EventHandler(this.StatusLabelUpdate_Click);
            // 
            // statusLabelIgnore
            // 
            this.statusLabelIgnore.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelIgnore.Name = "statusLabelIgnore";
            this.statusLabelIgnore.Size = new System.Drawing.Size(73, 19);
            this.statusLabelIgnore.Text = "L10n Ignore";
            this.statusLabelIgnore.Visible = false;
            this.statusLabelIgnore.Click += new System.EventHandler(this.StatusLabelIgnore_Click);
            // 
            // statusLabelHide
            // 
            this.statusLabelHide.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelHide.Name = "statusLabelHide";
            this.statusLabelHide.Size = new System.Drawing.Size(55, 19);
            this.statusLabelHide.Text = "L10n OK";
            this.statusLabelHide.Click += new System.EventHandler(this.StatusLabelHide_Click);
            // 
            // stripEnableUpdateCheck
            // 
            this.stripEnableUpdateCheck.Dock = System.Windows.Forms.DockStyle.Top;
            this.stripEnableUpdateCheck.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.checkUpdatesLabel,
            this.statusLabelAutoYes,
            this.statusLabelAutoNo});
            this.stripEnableUpdateCheck.Location = new System.Drawing.Point(0, 0);
            this.stripEnableUpdateCheck.Name = "stripEnableUpdateCheck";
            this.stripEnableUpdateCheck.Size = new System.Drawing.Size(584, 32);
            this.stripEnableUpdateCheck.SizingGrip = false;
            this.stripEnableUpdateCheck.TabIndex = 2;
            this.stripEnableUpdateCheck.Text = "statusStrip1";
            this.stripEnableUpdateCheck.Visible = false;
            // 
            // checkUpdatesLabel
            // 
            this.checkUpdatesLabel.BackColor = System.Drawing.SystemColors.Info;
            this.checkUpdatesLabel.ForeColor = System.Drawing.SystemColors.InfoText;
            this.checkUpdatesLabel.Name = "checkUpdatesLabel";
            this.checkUpdatesLabel.Size = new System.Drawing.Size(442, 27);
            this.checkUpdatesLabel.Spring = true;
            this.checkUpdatesLabel.Text = "L10n Check for updates automatically?";
            // 
            // statusLabelAutoYes
            // 
            this.statusLabelAutoYes.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelAutoYes.Name = "statusLabelAutoYes";
            this.statusLabelAutoYes.Padding = new System.Windows.Forms.Padding(4);
            this.statusLabelAutoYes.Size = new System.Drawing.Size(64, 27);
            this.statusLabelAutoYes.Text = "L10n Yes";
            this.statusLabelAutoYes.Click += new System.EventHandler(this.StatusLabelAuto_Click);
            // 
            // statusLabelAutoNo
            // 
            this.statusLabelAutoNo.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelAutoNo.Name = "statusLabelAutoNo";
            this.statusLabelAutoNo.Padding = new System.Windows.Forms.Padding(4);
            this.statusLabelAutoNo.Size = new System.Drawing.Size(63, 27);
            this.statusLabelAutoNo.Text = "L10n No";
            this.statusLabelAutoNo.Click += new System.EventHandler(this.StatusLabelAuto_Click);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.buttonManual);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 24);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(584, 337);
            this.panel1.TabIndex = 4;
            // 
            // buttonManual
            // 
            this.buttonManual.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonManual.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.buttonManual.BackColor = System.Drawing.Color.MediumBlue;
            this.buttonManual.ForeColor = System.Drawing.Color.White;
            this.buttonManual.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.buttonManual.Location = new System.Drawing.Point(12, 261);
            this.buttonManual.Name = "buttonManual";
            this.buttonManual.Padding = new System.Windows.Forms.Padding(8);
            this.buttonManual.Size = new System.Drawing.Size(560, 64);
            this.buttonManual.TabIndex = 4;
            this.buttonManual.Text = "L10n Open Online Manual (LANG)";
            this.buttonManual.UseVisualStyleBackColor = false;
            this.buttonManual.Click += new System.EventHandler(this.openManual_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.mainMenu);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.stripEnableUpdateCheck);
            this.MainMenuStrip = this.mainMenu;
            this.MinimumSize = new System.Drawing.Size(600, 400);
            this.Name = "MainForm";
            this.Text = "L10n Half-Life Advanced Effects";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Shown += new System.EventHandler(this.MainForm_Shown);
            this.mainMenu.ResumeLayout(false);
            this.mainMenu.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.stripEnableUpdateCheck.ResumeLayout(false);
            this.stripEnableUpdateCheck.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip mainMenu;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem menuExit;
        private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuStatusBar;
        private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem calculatorsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuFileSize;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem developerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuCustomLoader;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuAdvancedFxOrg;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem checkForUpdatesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuAutoUpdateCheck;
        private System.Windows.Forms.ToolStripMenuItem checkNowToolStripMenuItem;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelUpdate;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelHide;
        private System.Windows.Forms.StatusStrip stripEnableUpdateCheck;
        private System.Windows.Forms.ToolStripStatusLabel checkUpdatesLabel;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelAutoYes;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelAutoNo;
        private System.Windows.Forms.ToolStripMenuItem menuGuidToClipBoard;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelIgnore;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem menuNewGuidToClipBoard;
        private System.Windows.Forms.ToolStripMenuItem menuLaunchCSGO;
        private System.Windows.Forms.ToolStripMenuItem launchGoldSrcToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem goldSrcToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem demoToolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem skyManagerToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem donateToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem manualToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonManual;
    }
}