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
            this.menuAfxRgbaLut = new System.Windows.Forms.ToolStripMenuItem();
            this.menuVoronoi = new System.Windows.Forms.ToolStripMenuItem();
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
            this.checkForUpdatesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuAutoUpdateCheck = new System.Windows.Forms.ToolStripMenuItem();
            this.checkNowToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
            this.menuGitHub = new System.Windows.Forms.ToolStripMenuItem();
            this.menuFaq = new System.Windows.Forms.ToolStripMenuItem();
            this.menuContact = new System.Windows.Forms.ToolStripMenuItem();
            this.donateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.statusLabelUpdate = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelIgnore = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelHide = new System.Windows.Forms.ToolStripStatusLabel();
            this.stripEnableUpdateCheck = new System.Windows.Forms.StatusStrip();
            this.checkUpdatesLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelAutoYes = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusLabelAutoNo = new System.Windows.Forms.ToolStripStatusLabel();
            this.groupBoxHelp = new System.Windows.Forms.GroupBox();
            this.comboBoxHelpEntries = new System.Windows.Forms.ComboBox();
            this.labelHelpSelection = new System.Windows.Forms.Label();
            this.comboBoxHelpLanguage = new System.Windows.Forms.ComboBox();
            this.labelHelpLanguage = new System.Windows.Forms.Label();
            this.buttonManual = new System.Windows.Forms.Button();
            this.pictureBoxHelp = new System.Windows.Forms.PictureBox();
            this.groupBoxPathError = new System.Windows.Forms.GroupBox();
            this.labelHlaePathError = new System.Windows.Forms.Label();
            this.textBoxHlaePath = new System.Windows.Forms.TextBox();
            this.labelHlaePath = new System.Windows.Forms.Label();
            this.pictureBoxPathError = new System.Windows.Forms.PictureBox();
            this.groupBoxVac = new System.Windows.Forms.GroupBox();
            this.pictureBoxVac = new System.Windows.Forms.PictureBox();
            this.labelVac = new System.Windows.Forms.Label();
            this.buttonVacOk = new System.Windows.Forms.Button();
            this.groupBoxEpilepsy = new System.Windows.Forms.GroupBox();
            this.pictureBoxEpilepsy = new System.Windows.Forms.PictureBox();
            this.labelEpilepsy = new System.Windows.Forms.Label();
            this.buttonEpilepsyOk = new System.Windows.Forms.Button();
            this.panelNotifications = new System.Windows.Forms.Panel();
            this.menuLaunchCS2 = new System.Windows.Forms.ToolStripMenuItem();
            this.mainMenu.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.stripEnableUpdateCheck.SuspendLayout();
            this.groupBoxHelp.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxHelp)).BeginInit();
            this.groupBoxPathError.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPathError)).BeginInit();
            this.groupBoxVac.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxVac)).BeginInit();
            this.groupBoxEpilepsy.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxEpilepsy)).BeginInit();
            this.panelNotifications.SuspendLayout();
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
            this.menuLaunchCS2,
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
            this.menuAfxRgbaLut,
            this.goldSrcToolStripMenuItem,
            this.toolStripMenuItem2,
            this.developerToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(75, 20);
            this.toolsToolStripMenuItem.Text = "L10n Tools";
            // 
            // calculatorsToolStripMenuItem
            // 
            this.calculatorsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuFileSize});
            this.calculatorsToolStripMenuItem.Name = "calculatorsToolStripMenuItem";
            this.calculatorsToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.calculatorsToolStripMenuItem.Text = "L10n Calculators";
            // 
            // menuFileSize
            // 
            this.menuFileSize.Name = "menuFileSize";
            this.menuFileSize.Size = new System.Drawing.Size(143, 22);
            this.menuFileSize.Text = "L10n File Size";
            this.menuFileSize.Click += new System.EventHandler(this.MenuFileSize_Click);
            // 
            // menuAfxRgbaLut
            // 
            this.menuAfxRgbaLut.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuVoronoi});
            this.menuAfxRgbaLut.Name = "menuAfxRgbaLut";
            this.menuAfxRgbaLut.Size = new System.Drawing.Size(163, 22);
            this.menuAfxRgbaLut.Text = "L10n AfxRgbaLut";
            // 
            // menuVoronoi
            // 
            this.menuVoronoi.Name = "menuVoronoi";
            this.menuVoronoi.Size = new System.Drawing.Size(143, 22);
            this.menuVoronoi.Text = "L10n Voronoi";
            this.menuVoronoi.Click += new System.EventHandler(this.l10nToolStripMenuItem_Click);
            // 
            // goldSrcToolStripMenuItem
            // 
            this.goldSrcToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.demoToolsToolStripMenuItem,
            this.skyManagerToolStripMenuItem});
            this.goldSrcToolStripMenuItem.Name = "goldSrcToolStripMenuItem";
            this.goldSrcToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
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
            this.toolStripMenuItem2.Size = new System.Drawing.Size(160, 6);
            // 
            // developerToolStripMenuItem
            // 
            this.developerToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuCustomLoader,
            this.toolStripMenuItem4,
            this.menuGuidToClipBoard,
            this.menuNewGuidToClipBoard});
            this.developerToolStripMenuItem.Name = "developerToolStripMenuItem";
            this.developerToolStripMenuItem.Size = new System.Drawing.Size(163, 22);
            this.developerToolStripMenuItem.Text = "L10n Developer";
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
            this.checkForUpdatesToolStripMenuItem,
            this.toolStripMenuItem5,
            this.menuGitHub,
            this.menuFaq,
            this.menuContact});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(72, 20);
            this.helpToolStripMenuItem.Text = "L10n Help";
            // 
            // checkForUpdatesToolStripMenuItem
            // 
            this.checkForUpdatesToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuAutoUpdateCheck,
            this.checkNowToolStripMenuItem});
            this.checkForUpdatesToolStripMenuItem.Name = "checkForUpdatesToolStripMenuItem";
            this.checkForUpdatesToolStripMenuItem.Size = new System.Drawing.Size(282, 22);
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
            this.toolStripMenuItem5.Size = new System.Drawing.Size(279, 6);
            // 
            // menuGitHub
            // 
            this.menuGitHub.Name = "menuGitHub";
            this.menuGitHub.Size = new System.Drawing.Size(282, 22);
            this.menuGitHub.Text = "L10n Offical GitHub";
            this.menuGitHub.ToolTipText = "https://github.com/advancedfx/advancedfx";
            this.menuGitHub.Click += new System.EventHandler(this.MenuGitHub_Click);
            // 
            // menuFaq
            // 
            this.menuFaq.Name = "menuFaq";
            this.menuFaq.Size = new System.Drawing.Size(282, 22);
            this.menuFaq.Text = "L10n Frequently Asked Questions (FAQ)";
            this.menuFaq.ToolTipText = "https://github.com/advancedfx/advancedfx/wiki/FAQ";
            this.menuFaq.Click += new System.EventHandler(this.menuFaq_Click);
            // 
            // menuContact
            // 
            this.menuContact.Name = "menuContact";
            this.menuContact.Size = new System.Drawing.Size(282, 22);
            this.menuContact.Text = "L10n Contact";
            this.menuContact.ToolTipText = "https://github.com/advancedfx#contact";
            this.menuContact.Click += new System.EventHandler(this.menuContact_Click);
            // 
            // donateToolStripMenuItem
            // 
            this.donateToolStripMenuItem.Name = "donateToolStripMenuItem";
            this.donateToolStripMenuItem.Size = new System.Drawing.Size(85, 20);
            this.donateToolStripMenuItem.Text = "L10n Donate";
            this.donateToolStripMenuItem.ToolTipText = "https://github.com/advancedfx/advancedfx/blob/main/CREDITS.md#donors";
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
            // groupBoxHelp
            // 
            this.groupBoxHelp.BackColor = System.Drawing.Color.LightBlue;
            this.groupBoxHelp.Controls.Add(this.comboBoxHelpEntries);
            this.groupBoxHelp.Controls.Add(this.labelHelpSelection);
            this.groupBoxHelp.Controls.Add(this.comboBoxHelpLanguage);
            this.groupBoxHelp.Controls.Add(this.labelHelpLanguage);
            this.groupBoxHelp.Controls.Add(this.buttonManual);
            this.groupBoxHelp.Controls.Add(this.pictureBoxHelp);
            this.groupBoxHelp.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.groupBoxHelp.ForeColor = System.Drawing.Color.Black;
            this.groupBoxHelp.Location = new System.Drawing.Point(0, 244);
            this.groupBoxHelp.Name = "groupBoxHelp";
            this.groupBoxHelp.Size = new System.Drawing.Size(584, 117);
            this.groupBoxHelp.TabIndex = 0;
            this.groupBoxHelp.TabStop = false;
            this.groupBoxHelp.Text = "L10n Help";
            // 
            // comboBoxHelpEntries
            // 
            this.comboBoxHelpEntries.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxHelpEntries.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxHelpEntries.FormattingEnabled = true;
            this.comboBoxHelpEntries.Location = new System.Drawing.Point(229, 46);
            this.comboBoxHelpEntries.Name = "comboBoxHelpEntries";
            this.comboBoxHelpEntries.Size = new System.Drawing.Size(346, 21);
            this.comboBoxHelpEntries.TabIndex = 4;
            this.comboBoxHelpEntries.SelectedIndexChanged += new System.EventHandler(this.comboBoxHelpEntry_SelectedIndexChanged);
            // 
            // labelHelpSelection
            // 
            this.labelHelpSelection.AutoSize = true;
            this.labelHelpSelection.Location = new System.Drawing.Point(63, 49);
            this.labelHelpSelection.Name = "labelHelpSelection";
            this.labelHelpSelection.Size = new System.Drawing.Size(81, 13);
            this.labelHelpSelection.TabIndex = 3;
            this.labelHelpSelection.Text = "L10n Selection:";
            // 
            // comboBoxHelpLanguage
            // 
            this.comboBoxHelpLanguage.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.comboBoxHelpLanguage.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxHelpLanguage.FormattingEnabled = true;
            this.comboBoxHelpLanguage.Location = new System.Drawing.Point(229, 19);
            this.comboBoxHelpLanguage.Name = "comboBoxHelpLanguage";
            this.comboBoxHelpLanguage.Size = new System.Drawing.Size(346, 21);
            this.comboBoxHelpLanguage.TabIndex = 2;
            this.comboBoxHelpLanguage.SelectedIndexChanged += new System.EventHandler(this.comboBoxHelpLanguage_SelectedIndexChanged);
            // 
            // labelHelpLanguage
            // 
            this.labelHelpLanguage.AutoSize = true;
            this.labelHelpLanguage.Location = new System.Drawing.Point(63, 22);
            this.labelHelpLanguage.Name = "labelHelpLanguage";
            this.labelHelpLanguage.Size = new System.Drawing.Size(85, 13);
            this.labelHelpLanguage.TabIndex = 1;
            this.labelHelpLanguage.Text = "L10n Language:";
            // 
            // buttonManual
            // 
            this.buttonManual.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonManual.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.buttonManual.BackColor = System.Drawing.Color.MediumBlue;
            this.buttonManual.FlatAppearance.BorderColor = System.Drawing.Color.White;
            this.buttonManual.FlatAppearance.BorderSize = 2;
            this.buttonManual.FlatAppearance.MouseDownBackColor = System.Drawing.Color.Cyan;
            this.buttonManual.FlatAppearance.MouseOverBackColor = System.Drawing.Color.LightCyan;
            this.buttonManual.ForeColor = System.Drawing.Color.White;
            this.buttonManual.Location = new System.Drawing.Point(6, 73);
            this.buttonManual.Name = "buttonManual";
            this.buttonManual.Padding = new System.Windows.Forms.Padding(8);
            this.buttonManual.Size = new System.Drawing.Size(572, 38);
            this.buttonManual.TabIndex = 0;
            this.buttonManual.Text = "[URL]";
            this.buttonManual.UseVisualStyleBackColor = false;
            this.buttonManual.Click += new System.EventHandler(this.buttonManual_Click);
            // 
            // pictureBoxHelp
            // 
            this.pictureBoxHelp.Location = new System.Drawing.Point(9, 19);
            this.pictureBoxHelp.Name = "pictureBoxHelp";
            this.pictureBoxHelp.Size = new System.Drawing.Size(48, 48);
            this.pictureBoxHelp.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pictureBoxHelp.TabIndex = 0;
            this.pictureBoxHelp.TabStop = false;
            // 
            // groupBoxPathError
            // 
            this.groupBoxPathError.AutoSize = true;
            this.groupBoxPathError.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.groupBoxPathError.BackColor = System.Drawing.Color.MistyRose;
            this.groupBoxPathError.Controls.Add(this.labelHlaePathError);
            this.groupBoxPathError.Controls.Add(this.textBoxHlaePath);
            this.groupBoxPathError.Controls.Add(this.labelHlaePath);
            this.groupBoxPathError.Controls.Add(this.pictureBoxPathError);
            this.groupBoxPathError.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBoxPathError.ForeColor = System.Drawing.Color.Black;
            this.groupBoxPathError.Location = new System.Drawing.Point(0, 0);
            this.groupBoxPathError.Name = "groupBoxPathError";
            this.groupBoxPathError.Size = new System.Drawing.Size(567, 113);
            this.groupBoxPathError.TabIndex = 3;
            this.groupBoxPathError.TabStop = false;
            this.groupBoxPathError.Text = "L10n Path Error";
            // 
            // labelHlaePathError
            // 
            this.labelHlaePathError.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelHlaePathError.Location = new System.Drawing.Point(57, 42);
            this.labelHlaePathError.Name = "labelHlaePathError";
            this.labelHlaePathError.Size = new System.Drawing.Size(507, 55);
            this.labelHlaePathError.TabIndex = 4;
            this.labelHlaePathError.Text = "L10n Path error text here ...";
            // 
            // textBoxHlaePath
            // 
            this.textBoxHlaePath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxHlaePath.Location = new System.Drawing.Point(229, 19);
            this.textBoxHlaePath.Name = "textBoxHlaePath";
            this.textBoxHlaePath.ReadOnly = true;
            this.textBoxHlaePath.Size = new System.Drawing.Size(335, 20);
            this.textBoxHlaePath.TabIndex = 3;
            // 
            // labelHlaePath
            // 
            this.labelHlaePath.AutoSize = true;
            this.labelHlaePath.Location = new System.Drawing.Point(57, 22);
            this.labelHlaePath.Name = "labelHlaePath";
            this.labelHlaePath.Size = new System.Drawing.Size(90, 13);
            this.labelHlaePath.TabIndex = 2;
            this.labelHlaePath.Text = "L10n HLAE Path:";
            // 
            // pictureBoxPathError
            // 
            this.pictureBoxPathError.Location = new System.Drawing.Point(3, 19);
            this.pictureBoxPathError.Name = "pictureBoxPathError";
            this.pictureBoxPathError.Size = new System.Drawing.Size(48, 48);
            this.pictureBoxPathError.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBoxPathError.TabIndex = 1;
            this.pictureBoxPathError.TabStop = false;
            // 
            // groupBoxVac
            // 
            this.groupBoxVac.AutoSize = true;
            this.groupBoxVac.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.groupBoxVac.BackColor = System.Drawing.Color.LightYellow;
            this.groupBoxVac.Controls.Add(this.pictureBoxVac);
            this.groupBoxVac.Controls.Add(this.labelVac);
            this.groupBoxVac.Controls.Add(this.buttonVacOk);
            this.groupBoxVac.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBoxVac.ForeColor = System.Drawing.Color.Black;
            this.groupBoxVac.Location = new System.Drawing.Point(0, 204);
            this.groupBoxVac.Name = "groupBoxVac";
            this.groupBoxVac.Size = new System.Drawing.Size(567, 115);
            this.groupBoxVac.TabIndex = 4;
            this.groupBoxVac.TabStop = false;
            this.groupBoxVac.Text = "L10n VAC warning";
            // 
            // pictureBoxVac
            // 
            this.pictureBoxVac.Location = new System.Drawing.Point(3, 19);
            this.pictureBoxVac.Name = "pictureBoxVac";
            this.pictureBoxVac.Size = new System.Drawing.Size(24, 24);
            this.pictureBoxVac.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBoxVac.TabIndex = 1;
            this.pictureBoxVac.TabStop = false;
            // 
            // labelVac
            // 
            this.labelVac.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelVac.Location = new System.Drawing.Point(33, 19);
            this.labelVac.Name = "labelVac";
            this.labelVac.Size = new System.Drawing.Size(531, 51);
            this.labelVac.TabIndex = 2;
            this.labelVac.Text = "L10n The HLAE tool is technically a hack, therefore you should only use it for ma" +
    "king gaming videos or watching demos. Joining VAC protected servers with HLAE wi" +
    "ll probably get you VAC banned.";
            // 
            // buttonVacOk
            // 
            this.buttonVacOk.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonVacOk.Location = new System.Drawing.Point(3, 73);
            this.buttonVacOk.Name = "buttonVacOk";
            this.buttonVacOk.Size = new System.Drawing.Size(561, 23);
            this.buttonVacOk.TabIndex = 3;
            this.buttonVacOk.Text = "L10n OK";
            this.buttonVacOk.UseVisualStyleBackColor = true;
            this.buttonVacOk.Click += new System.EventHandler(this.buttonVacOk_Click);
            // 
            // groupBoxEpilepsy
            // 
            this.groupBoxEpilepsy.AutoSize = true;
            this.groupBoxEpilepsy.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.groupBoxEpilepsy.BackColor = System.Drawing.Color.LightYellow;
            this.groupBoxEpilepsy.Controls.Add(this.pictureBoxEpilepsy);
            this.groupBoxEpilepsy.Controls.Add(this.labelEpilepsy);
            this.groupBoxEpilepsy.Controls.Add(this.buttonEpilepsyOk);
            this.groupBoxEpilepsy.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBoxEpilepsy.ForeColor = System.Drawing.Color.Black;
            this.groupBoxEpilepsy.Location = new System.Drawing.Point(0, 113);
            this.groupBoxEpilepsy.Name = "groupBoxEpilepsy";
            this.groupBoxEpilepsy.Size = new System.Drawing.Size(567, 91);
            this.groupBoxEpilepsy.TabIndex = 5;
            this.groupBoxEpilepsy.TabStop = false;
            this.groupBoxEpilepsy.Text = "L10n Epilepsy warning";
            // 
            // pictureBoxEpilepsy
            // 
            this.pictureBoxEpilepsy.Location = new System.Drawing.Point(3, 19);
            this.pictureBoxEpilepsy.Name = "pictureBoxEpilepsy";
            this.pictureBoxEpilepsy.Size = new System.Drawing.Size(24, 24);
            this.pictureBoxEpilepsy.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBoxEpilepsy.TabIndex = 1;
            this.pictureBoxEpilepsy.TabStop = false;
            // 
            // labelEpilepsy
            // 
            this.labelEpilepsy.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelEpilepsy.Location = new System.Drawing.Point(33, 19);
            this.labelEpilepsy.Name = "labelEpilepsy";
            this.labelEpilepsy.Size = new System.Drawing.Size(531, 27);
            this.labelEpilepsy.TabIndex = 2;
            this.labelEpilepsy.Text = "L10n This software may cause fast-changing images and colors on your screen.";
            // 
            // buttonEpilepsyOk
            // 
            this.buttonEpilepsyOk.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonEpilepsyOk.Location = new System.Drawing.Point(3, 49);
            this.buttonEpilepsyOk.Name = "buttonEpilepsyOk";
            this.buttonEpilepsyOk.Size = new System.Drawing.Size(561, 23);
            this.buttonEpilepsyOk.TabIndex = 3;
            this.buttonEpilepsyOk.Text = "L10n OK";
            this.buttonEpilepsyOk.UseVisualStyleBackColor = true;
            this.buttonEpilepsyOk.Click += new System.EventHandler(this.buttonEpilepsyOk_Click);
            // 
            // panelNotifications
            // 
            this.panelNotifications.AutoScroll = true;
            this.panelNotifications.Controls.Add(this.groupBoxVac);
            this.panelNotifications.Controls.Add(this.groupBoxEpilepsy);
            this.panelNotifications.Controls.Add(this.groupBoxPathError);
            this.panelNotifications.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panelNotifications.Location = new System.Drawing.Point(0, 24);
            this.panelNotifications.Name = "panelNotifications";
            this.panelNotifications.Size = new System.Drawing.Size(584, 220);
            this.panelNotifications.TabIndex = 6;
            // 
            // menuLaunchCS2
            // 
            this.menuLaunchCS2.Name = "menuLaunchCS2";
            this.menuLaunchCS2.Size = new System.Drawing.Size(185, 22);
            this.menuLaunchCS2.Text = "L10n Launch CS2";
            this.menuLaunchCS2.Click += new System.EventHandler(this.MenuLaunchCS2_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.panelNotifications);
            this.Controls.Add(this.mainMenu);
            this.Controls.Add(this.groupBoxHelp);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.stripEnableUpdateCheck);
            this.MainMenuStrip = this.mainMenu;
            this.MinimumSize = new System.Drawing.Size(600, 400);
            this.Name = "MainForm";
            this.Text = "L10n Half-Life Advanced Effects 0.0.0.0";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.MainForm_FormClosed);
            this.Shown += new System.EventHandler(this.MainForm_Shown);
            this.mainMenu.ResumeLayout(false);
            this.mainMenu.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.stripEnableUpdateCheck.ResumeLayout(false);
            this.stripEnableUpdateCheck.PerformLayout();
            this.groupBoxHelp.ResumeLayout(false);
            this.groupBoxHelp.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxHelp)).EndInit();
            this.groupBoxPathError.ResumeLayout(false);
            this.groupBoxPathError.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPathError)).EndInit();
            this.groupBoxVac.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxVac)).EndInit();
            this.groupBoxEpilepsy.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxEpilepsy)).EndInit();
            this.panelNotifications.ResumeLayout(false);
            this.panelNotifications.PerformLayout();
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
        private System.Windows.Forms.ToolStripMenuItem menuGitHub;
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
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
        private System.Windows.Forms.GroupBox groupBoxHelp;
        private System.Windows.Forms.ComboBox comboBoxHelpEntries;
        private System.Windows.Forms.Label labelHelpSelection;
        private System.Windows.Forms.ComboBox comboBoxHelpLanguage;
        private System.Windows.Forms.Label labelHelpLanguage;
        private System.Windows.Forms.Button buttonManual;
        private System.Windows.Forms.PictureBox pictureBoxHelp;
        private System.Windows.Forms.ToolStripMenuItem menuAfxRgbaLut;
        private System.Windows.Forms.ToolStripMenuItem menuVoronoi;
        private System.Windows.Forms.ToolStripMenuItem menuContact;
        private System.Windows.Forms.GroupBox groupBoxPathError;
        private System.Windows.Forms.Label labelHlaePathError;
        private System.Windows.Forms.TextBox textBoxHlaePath;
        private System.Windows.Forms.Label labelHlaePath;
        private System.Windows.Forms.PictureBox pictureBoxPathError;
        private System.Windows.Forms.GroupBox groupBoxVac;
        private System.Windows.Forms.Label labelVac;
        private System.Windows.Forms.PictureBox pictureBoxVac;
        private System.Windows.Forms.Button buttonVacOk;
        private System.Windows.Forms.GroupBox groupBoxEpilepsy;
        private System.Windows.Forms.Button buttonEpilepsyOk;
        private System.Windows.Forms.Label labelEpilepsy;
        private System.Windows.Forms.PictureBox pictureBoxEpilepsy;
        private System.Windows.Forms.Panel panelNotifications;
        private System.Windows.Forms.ToolStripMenuItem menuFaq;
        private System.Windows.Forms.ToolStripMenuItem menuLaunchCS2;
    }
}