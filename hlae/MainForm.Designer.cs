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
            this.menuContact = new System.Windows.Forms.ToolStripMenuItem();
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
            this.panel2 = new System.Windows.Forms.Panel();
            this.groupBoxConfigError = new System.Windows.Forms.GroupBox();
            this.buttonConfigOverwrite = new System.Windows.Forms.Button();
            this.buttonConfigReload = new System.Windows.Forms.Button();
            this.labelConfigError = new System.Windows.Forms.Label();
            this.pictureBoxConfigError = new System.Windows.Forms.PictureBox();
            this.groupBoxPathError = new System.Windows.Forms.GroupBox();
            this.labelHlaePathError = new System.Windows.Forms.Label();
            this.textBoxHlaePath = new System.Windows.Forms.TextBox();
            this.labelHlaePath = new System.Windows.Forms.Label();
            this.pictureBoxPathError = new System.Windows.Forms.PictureBox();
            this.groupBoxHelp = new System.Windows.Forms.GroupBox();
            this.comboBoxHelpEntries = new System.Windows.Forms.ComboBox();
            this.labelHelpSelection = new System.Windows.Forms.Label();
            this.comboBoxHelpLanguage = new System.Windows.Forms.ComboBox();
            this.labelHelpLanguage = new System.Windows.Forms.Label();
            this.buttonManual = new System.Windows.Forms.Button();
            this.pictureBoxHelp = new System.Windows.Forms.PictureBox();
            this.mainMenu.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.stripEnableUpdateCheck.SuspendLayout();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.groupBoxConfigError.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxConfigError)).BeginInit();
            this.groupBoxPathError.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPathError)).BeginInit();
            this.groupBoxHelp.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxHelp)).BeginInit();
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
            this.menuAfxRgbaLut,
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
            this.calculatorsToolStripMenuItem.Size = new System.Drawing.Size(164, 22);
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
            this.menuAfxRgbaLut.Size = new System.Drawing.Size(164, 22);
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
            this.goldSrcToolStripMenuItem.Size = new System.Drawing.Size(164, 22);
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
            this.toolStripMenuItem2.Size = new System.Drawing.Size(161, 6);
            // 
            // developerToolStripMenuItem
            // 
            this.developerToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuCustomLoader,
            this.toolStripMenuItem4,
            this.menuGuidToClipBoard,
            this.menuNewGuidToClipBoard});
            this.developerToolStripMenuItem.Name = "developerToolStripMenuItem";
            this.developerToolStripMenuItem.Size = new System.Drawing.Size(164, 22);
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
            this.menuContact,
            this.menuAdvancedFxOrg});
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
            this.checkForUpdatesToolStripMenuItem.Size = new System.Drawing.Size(371, 22);
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
            this.toolStripMenuItem5.Size = new System.Drawing.Size(368, 6);
            // 
            // menuContact
            // 
            this.menuContact.Name = "menuContact";
            this.menuContact.Size = new System.Drawing.Size(371, 22);
            this.menuContact.Text = "L10n Contact / Privacy Policy / Imprint (advancedfx.org)";
            this.menuContact.Click += new System.EventHandler(this.menuContact_Click);
            // 
            // menuAdvancedFxOrg
            // 
            this.menuAdvancedFxOrg.Name = "menuAdvancedFxOrg";
            this.menuAdvancedFxOrg.Size = new System.Drawing.Size(371, 22);
            this.menuAdvancedFxOrg.Text = "L10n Offical website";
            this.menuAdvancedFxOrg.ToolTipText = "https://www.advancedfx.org/";
            this.menuAdvancedFxOrg.Click += new System.EventHandler(this.MenuAdvancedFxOrg_Click);
            // 
            // donateToolStripMenuItem
            // 
            this.donateToolStripMenuItem.Name = "donateToolStripMenuItem";
            this.donateToolStripMenuItem.Size = new System.Drawing.Size(85, 20);
            this.donateToolStripMenuItem.Text = "L10n Donate";
            this.donateToolStripMenuItem.ToolTipText = "https://www.advancedfx.org/credits/#donors";
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
            this.panel1.AutoScroll = true;
            this.panel1.AutoSize = true;
            this.panel1.Controls.Add(this.panel2);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 24);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(584, 337);
            this.panel1.TabIndex = 6;
            // 
            // panel2
            // 
            this.panel2.AutoSize = true;
            this.panel2.Controls.Add(this.groupBoxConfigError);
            this.panel2.Controls.Add(this.groupBoxPathError);
            this.panel2.Controls.Add(this.groupBoxHelp);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel2.Location = new System.Drawing.Point(0, 8);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(584, 329);
            this.panel2.TabIndex = 5;
            // 
            // groupBoxConfigError
            // 
            this.groupBoxConfigError.BackColor = System.Drawing.Color.MistyRose;
            this.groupBoxConfigError.Controls.Add(this.buttonConfigOverwrite);
            this.groupBoxConfigError.Controls.Add(this.buttonConfigReload);
            this.groupBoxConfigError.Controls.Add(this.labelConfigError);
            this.groupBoxConfigError.Controls.Add(this.pictureBoxConfigError);
            this.groupBoxConfigError.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.groupBoxConfigError.ForeColor = System.Drawing.Color.Black;
            this.groupBoxConfigError.Location = new System.Drawing.Point(0, 0);
            this.groupBoxConfigError.Name = "groupBoxConfigError";
            this.groupBoxConfigError.Size = new System.Drawing.Size(584, 96);
            this.groupBoxConfigError.TabIndex = 13;
            this.groupBoxConfigError.TabStop = false;
            this.groupBoxConfigError.Text = "L10n Config Access Error";
            // 
            // buttonConfigOverwrite
            // 
            this.buttonConfigOverwrite.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonConfigOverwrite.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.buttonConfigOverwrite.Location = new System.Drawing.Point(380, 55);
            this.buttonConfigOverwrite.Name = "buttonConfigOverwrite";
            this.buttonConfigOverwrite.Size = new System.Drawing.Size(192, 23);
            this.buttonConfigOverwrite.TabIndex = 6;
            this.buttonConfigOverwrite.Text = "L10n Overwrite";
            this.buttonConfigOverwrite.UseVisualStyleBackColor = true;
            this.buttonConfigOverwrite.Click += new System.EventHandler(this.buttonConfigOverwrite_Click);
            // 
            // buttonConfigReload
            // 
            this.buttonConfigReload.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonConfigReload.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.buttonConfigReload.Location = new System.Drawing.Point(69, 55);
            this.buttonConfigReload.Name = "buttonConfigReload";
            this.buttonConfigReload.Size = new System.Drawing.Size(192, 23);
            this.buttonConfigReload.TabIndex = 5;
            this.buttonConfigReload.Text = "L10n Reload";
            this.buttonConfigReload.UseVisualStyleBackColor = true;
            this.buttonConfigReload.Click += new System.EventHandler(this.buttonConfigReload_Click);
            // 
            // labelConfigError
            // 
            this.labelConfigError.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelConfigError.Location = new System.Drawing.Point(66, 19);
            this.labelConfigError.Name = "labelConfigError";
            this.labelConfigError.Size = new System.Drawing.Size(509, 36);
            this.labelConfigError.TabIndex = 4;
            this.labelConfigError.Text = "L10n Config Access Error";
            // 
            // pictureBoxConfigError
            // 
            this.pictureBoxConfigError.Location = new System.Drawing.Point(6, 19);
            this.pictureBoxConfigError.Name = "pictureBoxConfigError";
            this.pictureBoxConfigError.Size = new System.Drawing.Size(48, 48);
            this.pictureBoxConfigError.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBoxConfigError.TabIndex = 1;
            this.pictureBoxConfigError.TabStop = false;
            // 
            // groupBoxPathError
            // 
            this.groupBoxPathError.BackColor = System.Drawing.Color.MistyRose;
            this.groupBoxPathError.Controls.Add(this.labelHlaePathError);
            this.groupBoxPathError.Controls.Add(this.textBoxHlaePath);
            this.groupBoxPathError.Controls.Add(this.labelHlaePath);
            this.groupBoxPathError.Controls.Add(this.pictureBoxPathError);
            this.groupBoxPathError.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.groupBoxPathError.ForeColor = System.Drawing.Color.Black;
            this.groupBoxPathError.Location = new System.Drawing.Point(0, 96);
            this.groupBoxPathError.Name = "groupBoxPathError";
            this.groupBoxPathError.Size = new System.Drawing.Size(584, 96);
            this.groupBoxPathError.TabIndex = 12;
            this.groupBoxPathError.TabStop = false;
            this.groupBoxPathError.Text = "L10n Path Error";
            // 
            // labelHlaePathError
            // 
            this.labelHlaePathError.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.labelHlaePathError.Location = new System.Drawing.Point(66, 42);
            this.labelHlaePathError.Name = "labelHlaePathError";
            this.labelHlaePathError.Size = new System.Drawing.Size(509, 51);
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
            this.textBoxHlaePath.Size = new System.Drawing.Size(346, 20);
            this.textBoxHlaePath.TabIndex = 3;
            // 
            // labelHlaePath
            // 
            this.labelHlaePath.AutoSize = true;
            this.labelHlaePath.Location = new System.Drawing.Point(66, 22);
            this.labelHlaePath.Name = "labelHlaePath";
            this.labelHlaePath.Size = new System.Drawing.Size(90, 13);
            this.labelHlaePath.TabIndex = 2;
            this.labelHlaePath.Text = "L10n HLAE Path:";
            // 
            // pictureBoxPathError
            // 
            this.pictureBoxPathError.Location = new System.Drawing.Point(6, 19);
            this.pictureBoxPathError.Name = "pictureBoxPathError";
            this.pictureBoxPathError.Size = new System.Drawing.Size(48, 48);
            this.pictureBoxPathError.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBoxPathError.TabIndex = 1;
            this.pictureBoxPathError.TabStop = false;
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
            this.groupBoxHelp.Location = new System.Drawing.Point(0, 192);
            this.groupBoxHelp.Name = "groupBoxHelp";
            this.groupBoxHelp.Size = new System.Drawing.Size(584, 137);
            this.groupBoxHelp.TabIndex = 11;
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
            this.buttonManual.Location = new System.Drawing.Point(6, 89);
            this.buttonManual.Name = "buttonManual";
            this.buttonManual.Padding = new System.Windows.Forms.Padding(8);
            this.buttonManual.Size = new System.Drawing.Size(572, 42);
            this.buttonManual.TabIndex = 0;
            this.buttonManual.Text = "[URL]";
            this.buttonManual.UseVisualStyleBackColor = false;
            // 
            // pictureBoxHelp
            // 
            this.pictureBoxHelp.Location = new System.Drawing.Point(9, 19);
            this.pictureBoxHelp.Name = "pictureBoxHelp";
            this.pictureBoxHelp.Size = new System.Drawing.Size(48, 48);
            this.pictureBoxHelp.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.pictureBoxHelp.TabIndex = 0;
            this.pictureBoxHelp.TabStop = false;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.mainMenu);
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
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.groupBoxConfigError.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxConfigError)).EndInit();
            this.groupBoxPathError.ResumeLayout(false);
            this.groupBoxPathError.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxPathError)).EndInit();
            this.groupBoxHelp.ResumeLayout(false);
            this.groupBoxHelp.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBoxHelp)).EndInit();
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
        private System.Windows.Forms.ToolStripMenuItem menuAfxRgbaLut;
        private System.Windows.Forms.ToolStripMenuItem menuVoronoi;
        private System.Windows.Forms.ToolStripMenuItem menuContact;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.GroupBox groupBoxConfigError;
        private System.Windows.Forms.Button buttonConfigOverwrite;
        private System.Windows.Forms.Button buttonConfigReload;
        private System.Windows.Forms.Label labelConfigError;
        private System.Windows.Forms.PictureBox pictureBoxConfigError;
        private System.Windows.Forms.GroupBox groupBoxPathError;
        private System.Windows.Forms.Label labelHlaePathError;
        private System.Windows.Forms.TextBox textBoxHlaePath;
        private System.Windows.Forms.Label labelHlaePath;
        private System.Windows.Forms.PictureBox pictureBoxPathError;
        private System.Windows.Forms.GroupBox groupBoxHelp;
        private System.Windows.Forms.ComboBox comboBoxHelpEntries;
        private System.Windows.Forms.Label labelHelpSelection;
        private System.Windows.Forms.ComboBox comboBoxHelpLanguage;
        private System.Windows.Forms.Label labelHelpLanguage;
        private System.Windows.Forms.Button buttonManual;
        private System.Windows.Forms.PictureBox pictureBoxHelp;
    }
}