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
            this.menuLaunch = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.menuExit = new System.Windows.Forms.ToolStripMenuItem();
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStatusBar = new System.Windows.Forms.ToolStripMenuItem();
            this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.demoToolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.calculatorsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuFileSize = new System.Windows.Forms.ToolStripMenuItem();
            this.skyToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.developerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuCustomLoader = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
            this.menuGuidToClipBoard = new System.Windows.Forms.ToolStripMenuItem();
            this.menuNewGuidToClipBoard = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuAdvancedFxOrg = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.checkForUpdatesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuAutoUpdateCheck = new System.Windows.Forms.ToolStripMenuItem();
            this.checkNowToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.statusLabelUpdate = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelIgnore = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelHide = new System.Windows.Forms.ToolStripStatusLabel();
            this.stripEnableUpdateCheck = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelAutoYes = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelAutoNo = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuLaunchCSGO = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenu.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.stripEnableUpdateCheck.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.toolsToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.mainMenu.Location = new System.Drawing.Point(0, 0);
            this.mainMenu.Name = "mainMenu";
            this.mainMenu.Size = new System.Drawing.Size(370, 24);
            this.mainMenu.TabIndex = 0;
            this.mainMenu.Text = "mainMenu";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuLaunchCSGO,
            this.menuLaunch,
            this.toolStripMenuItem1,
            this.menuExit});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // menuLaunch
            // 
            this.menuLaunch.Name = "menuLaunch";
            this.menuLaunch.Size = new System.Drawing.Size(204, 22);
            this.menuLaunch.Text = "Launch GoldSrc (HL1, ...)";
            this.menuLaunch.Click += new System.EventHandler(this.menuLaunch_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(201, 6);
            // 
            // menuExit
            // 
            this.menuExit.Name = "menuExit";
            this.menuExit.Size = new System.Drawing.Size(204, 22);
            this.menuExit.Text = "Exit";
            this.menuExit.Click += new System.EventHandler(this.menuExit_Click);
            // 
            // viewToolStripMenuItem
            // 
            this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuStatusBar});
            this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
            this.viewToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.viewToolStripMenuItem.Text = "View";
            // 
            // menuStatusBar
            // 
            this.menuStatusBar.Name = "menuStatusBar";
            this.menuStatusBar.Size = new System.Drawing.Size(126, 22);
            this.menuStatusBar.Text = "Status Bar";
            this.menuStatusBar.Click += new System.EventHandler(this.statusBarToolStripMenuItem_Click);
            // 
            // toolsToolStripMenuItem
            // 
            this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.demoToolsToolStripMenuItem,
            this.calculatorsToolStripMenuItem,
            this.skyToolStripMenuItem,
            this.toolStripMenuItem2,
            this.developerToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(47, 20);
            this.toolsToolStripMenuItem.Text = "Tools";
            // 
            // demoToolsToolStripMenuItem
            // 
            this.demoToolsToolStripMenuItem.Name = "demoToolsToolStripMenuItem";
            this.demoToolsToolStripMenuItem.Size = new System.Drawing.Size(137, 22);
            this.demoToolsToolStripMenuItem.Text = "Demo Tools";
            this.demoToolsToolStripMenuItem.Click += new System.EventHandler(this.demoToolsToolStripMenuItem_Click);
            // 
            // calculatorsToolStripMenuItem
            // 
            this.calculatorsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuFileSize});
            this.calculatorsToolStripMenuItem.Name = "calculatorsToolStripMenuItem";
            this.calculatorsToolStripMenuItem.Size = new System.Drawing.Size(137, 22);
            this.calculatorsToolStripMenuItem.Text = "Calculators";
            // 
            // menuFileSize
            // 
            this.menuFileSize.Name = "menuFileSize";
            this.menuFileSize.Size = new System.Drawing.Size(115, 22);
            this.menuFileSize.Text = "File Size";
            this.menuFileSize.Click += new System.EventHandler(this.menuFileSize_Click);
            // 
            // skyToolStripMenuItem
            // 
            this.skyToolStripMenuItem.Name = "skyToolStripMenuItem";
            this.skyToolStripMenuItem.Size = new System.Drawing.Size(137, 22);
            this.skyToolStripMenuItem.Text = "Sky Manger";
            this.skyToolStripMenuItem.Click += new System.EventHandler(this.skyToolStripMenuItem_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(134, 6);
            // 
            // developerToolStripMenuItem
            // 
            this.developerToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuCustomLoader,
            this.toolStripMenuItem4,
            this.menuGuidToClipBoard,
            this.menuNewGuidToClipBoard});
            this.developerToolStripMenuItem.Name = "developerToolStripMenuItem";
            this.developerToolStripMenuItem.Size = new System.Drawing.Size(137, 22);
            this.developerToolStripMenuItem.Text = "Developer";
            // 
            // menuCustomLoader
            // 
            this.menuCustomLoader.Name = "menuCustomLoader";
            this.menuCustomLoader.Size = new System.Drawing.Size(198, 22);
            this.menuCustomLoader.Text = "Custom Loader";
            this.menuCustomLoader.Click += new System.EventHandler(this.menuCustomLoader_Click);
            // 
            // toolStripMenuItem4
            // 
            this.toolStripMenuItem4.Name = "toolStripMenuItem4";
            this.toolStripMenuItem4.Size = new System.Drawing.Size(195, 6);
            // 
            // menuGuidToClipBoard
            // 
            this.menuGuidToClipBoard.Name = "menuGuidToClipBoard";
            this.menuGuidToClipBoard.Size = new System.Drawing.Size(198, 22);
            this.menuGuidToClipBoard.Text = "Own GUID to ClipBoard";
            this.menuGuidToClipBoard.Click += new System.EventHandler(this.menuGuidToClipBoard_Click);
            // 
            // menuNewGuidToClipBoard
            // 
            this.menuNewGuidToClipBoard.Name = "menuNewGuidToClipBoard";
            this.menuNewGuidToClipBoard.Size = new System.Drawing.Size(198, 22);
            this.menuNewGuidToClipBoard.Text = "New GUID to ClipBoard";
            this.menuNewGuidToClipBoard.Click += new System.EventHandler(this.menuNewGuidToClipBoard_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuAdvancedFxOrg,
            this.toolStripMenuItem3,
            this.checkForUpdatesToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // menuAdvancedFxOrg
            // 
            this.menuAdvancedFxOrg.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Underline);
            this.menuAdvancedFxOrg.Name = "menuAdvancedFxOrg";
            this.menuAdvancedFxOrg.Size = new System.Drawing.Size(171, 22);
            this.menuAdvancedFxOrg.Text = "advancedfx.org";
            this.menuAdvancedFxOrg.Click += new System.EventHandler(this.menuAdvancedFxOrg_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(168, 6);
            // 
            // checkForUpdatesToolStripMenuItem
            // 
            this.checkForUpdatesToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuAutoUpdateCheck,
            this.checkNowToolStripMenuItem});
            this.checkForUpdatesToolStripMenuItem.Name = "checkForUpdatesToolStripMenuItem";
            this.checkForUpdatesToolStripMenuItem.Size = new System.Drawing.Size(171, 22);
            this.checkForUpdatesToolStripMenuItem.Text = "Check for Updates";
            // 
            // menuAutoUpdateCheck
            // 
            this.menuAutoUpdateCheck.Name = "menuAutoUpdateCheck";
            this.menuAutoUpdateCheck.Size = new System.Drawing.Size(136, 22);
            this.menuAutoUpdateCheck.Text = "Auto Check";
            this.menuAutoUpdateCheck.Click += new System.EventHandler(this.menuAutoUpdateCheck_Click);
            // 
            // checkNowToolStripMenuItem
            // 
            this.checkNowToolStripMenuItem.Name = "checkNowToolStripMenuItem";
            this.checkNowToolStripMenuItem.Size = new System.Drawing.Size(136, 22);
            this.checkNowToolStripMenuItem.Text = "Check Now";
            this.checkNowToolStripMenuItem.Click += new System.EventHandler(this.checkNowToolStripMenuItem_Click);
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusLabelUpdate,
            this.statusLabelIgnore,
            this.statusLabelHide});
            this.statusStrip.Location = new System.Drawing.Point(0, 251);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(370, 22);
            this.statusStrip.TabIndex = 1;
            this.statusStrip.Text = "statusStrip1";
            this.statusStrip.Visible = false;
            this.statusStrip.VisibleChanged += new System.EventHandler(this.statusStrip_VisibleChanged);
            // 
            // statusLabelUpdate
            // 
            this.statusLabelUpdate.Name = "statusLabelUpdate";
            this.statusLabelUpdate.Size = new System.Drawing.Size(284, 17);
            this.statusLabelUpdate.Spring = true;
            this.statusLabelUpdate.Text = "Update status known";
            this.statusLabelUpdate.Click += new System.EventHandler(this.statusLabelUpdate_Click);
            // 
            // statusLabelIgnore
            // 
            this.statusLabelIgnore.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelIgnore.Name = "statusLabelIgnore";
            this.statusLabelIgnore.Size = new System.Drawing.Size(45, 17);
            this.statusLabelIgnore.Text = "Ignore";
            this.statusLabelIgnore.Visible = false;
            this.statusLabelIgnore.Click += new System.EventHandler(this.statusLabelIgnore_Click);
            // 
            // statusLabelHide
            // 
            this.statusLabelHide.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelHide.Name = "statusLabelHide";
            this.statusLabelHide.Size = new System.Drawing.Size(26, 17);
            this.statusLabelHide.Text = "Ok";
            this.statusLabelHide.Click += new System.EventHandler(this.statusLabelHide_Click);
            // 
            // stripEnableUpdateCheck
            // 
            this.stripEnableUpdateCheck.Dock = System.Windows.Forms.DockStyle.Top;
            this.stripEnableUpdateCheck.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.statusLabelAutoYes,
            this.statusLabelAutoNo});
            this.stripEnableUpdateCheck.Location = new System.Drawing.Point(0, 0);
            this.stripEnableUpdateCheck.Name = "stripEnableUpdateCheck";
            this.stripEnableUpdateCheck.Size = new System.Drawing.Size(370, 22);
            this.stripEnableUpdateCheck.SizingGrip = false;
            this.stripEnableUpdateCheck.TabIndex = 2;
            this.stripEnableUpdateCheck.Text = "statusStrip1";
            this.stripEnableUpdateCheck.Visible = false;
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.BackColor = System.Drawing.SystemColors.Info;
            this.toolStripStatusLabel1.ForeColor = System.Drawing.SystemColors.InfoText;
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(300, 17);
            this.toolStripStatusLabel1.Spring = true;
            this.toolStripStatusLabel1.Text = "Check for updates automatically?";
            // 
            // statusLabelAutoYes
            // 
            this.statusLabelAutoYes.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelAutoYes.Name = "statusLabelAutoYes";
            this.statusLabelAutoYes.Size = new System.Drawing.Size(28, 17);
            this.statusLabelAutoYes.Text = "Yes";
            this.statusLabelAutoYes.Click += new System.EventHandler(this.statusLabelAuto_Click);
            // 
            // statusLabelAutoNo
            // 
            this.statusLabelAutoNo.BorderSides = ((System.Windows.Forms.ToolStripStatusLabelBorderSides)((((System.Windows.Forms.ToolStripStatusLabelBorderSides.Left | System.Windows.Forms.ToolStripStatusLabelBorderSides.Top) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Right) 
            | System.Windows.Forms.ToolStripStatusLabelBorderSides.Bottom)));
            this.statusLabelAutoNo.Name = "statusLabelAutoNo";
            this.statusLabelAutoNo.Size = new System.Drawing.Size(27, 17);
            this.statusLabelAutoNo.Text = "No";
            this.statusLabelAutoNo.Click += new System.EventHandler(this.statusLabelAuto_Click);
            // 
            // menuLaunchCSGO
            // 
            this.menuLaunchCSGO.Name = "menuLaunchCSGO";
            this.menuLaunchCSGO.Size = new System.Drawing.Size(204, 22);
            this.menuLaunchCSGO.Text = "Launch CS:GO";
            this.menuLaunchCSGO.Click += new System.EventHandler(this.menuLaunchCSGO_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(370, 273);
            this.Controls.Add(this.mainMenu);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.stripEnableUpdateCheck);
            this.MainMenuStrip = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "Half-Life Advanced Effects";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Shown += new System.EventHandler(this.MainForm_Shown);
            this.mainMenu.ResumeLayout(false);
            this.mainMenu.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.stripEnableUpdateCheck.ResumeLayout(false);
            this.stripEnableUpdateCheck.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip mainMenu;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuLaunch;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem menuExit;
        private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuStatusBar;
        private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem demoToolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem calculatorsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem menuFileSize;
        private System.Windows.Forms.ToolStripMenuItem skyToolStripMenuItem;
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
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelAutoYes;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelAutoNo;
        private System.Windows.Forms.ToolStripMenuItem menuGuidToClipBoard;
        private System.Windows.Forms.ToolStripStatusLabel statusLabelIgnore;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem menuNewGuidToClipBoard;
        private System.Windows.Forms.ToolStripMenuItem menuLaunchCSGO;
    }
}