using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui
{
    public partial class MainForm : Form
    {
        //
        // Public members:


        //
        // Internal members:

        internal MainForm()
        {
            InitializeComponent();
            this.Icon = Program.Icon;
            this.pictureBoxHelp.Image = SystemIcons.Information.ToBitmap();
            this.pictureBoxPathError.Image = SystemIcons.Warning.ToBitmap();
            pictureBoxConfigError.Image = SystemIcons.Warning.ToBitmap();

            this.Text = L10n._p("Window title, args: {0} - version", "Half-Life Advanced Effects {0}", System.Diagnostics.FileVersionInfo.GetVersionInfo(System.Reflection.Assembly.GetExecutingAssembly().Location).FileVersion);

            this.fileToolStripMenuItem.Text = L10n._p("Main window | menu", "File");
            this.menuLaunchCSGO.Text = L10n._p("Main window | menu | File", "Launch CS:GO");
            this.launchGoldSrcToolStripMenuItem.Text = L10n._p("Main window | menu | File", "Launch GoldSrc");
            this.menuExit.Text = L10n._p("Main window | menu | File", "Exit");

            this.viewToolStripMenuItem.Text = L10n._p("Main Window | menu", "View");
            this.menuStatusBar.Text = L10n._p("Main window | menu | View", "Status Bar");

            this.toolsToolStripMenuItem.Text = L10n._p("Main window | menu", "Tools");            
            this.calculatorsToolStripMenuItem.Text = L10n._p("Main window | menu | Tools", "Calculators");
            this.menuFileSize.Text = L10n._p("Main window | menu | Tools | Calculators", "File Size");
            this.menuAfxRgbaLut.Text = L10n._p("Main window | menu | Tools", "Color Lookup Tables");
            this.menuVoronoi.Text = L10n._p("Main window | menu | Tools | Color Lookup Tables", "Voronoi");
            this.goldSrcToolStripMenuItem.Text = L10n._p("Main window | menu | Tools", "GoldSrc");
            this.demoToolsToolStripMenuItem.Text = L10n._p("Main window | menu | Tools | GoldSrc", "Demo Tools");
            this.skyManagerToolStripMenuItem.Text = L10n._p("Main window | menu | Tools | GoldSrc", "Sky Manager");

            this.developerToolStripMenuItem.Text = L10n._p("Main window | menu | Tools", "Developer");
            this.menuCustomLoader.Text = L10n._p("Main window | menu | Tools | Developer", "Custom Loader");
            this.menuGuidToClipBoard.Text = L10n._p("Main window | menu | Tools | Developer", "Own GUID to ClipBoard");
            this.menuNewGuidToClipBoard.Text = L10n._p("Main window | menu | Tools | Developer", "New GUID to ClipBoard");

            this.helpToolStripMenuItem.Text = L10n._p("Main window | menu", "Help");
            this.checkForUpdatesToolStripMenuItem.Text = L10n._p("Main window | menu | Help", "Check for Updates");
            this.menuAutoUpdateCheck.Text = L10n._p("Main window | menu | Help | Check for Updates", "Auto Check");
            this.checkNowToolStripMenuItem.Text = L10n._p("Main window | menu | Help | Check for Updates", "Check Now");
            this.menuAdvancedFxOrg.Text = L10n._p("Main window | menu | Help", "Official Website");

            this.donateToolStripMenuItem.Text = L10n._p("Main window | menu", "Donate");

            this.checkUpdatesLabel.Text = L10n._p("Main window | check updates strip", "Check for updates automatically?");
            this.statusLabelAutoYes.Text = L10n._p("Main window | check updates strip", "Yes");
            this.statusLabelAutoNo.Text = L10n._p("Main window | check updates strip", "No");


            this.statusLabelIgnore.Text = L10n._p("Main window | update status strip", "Ignore");
            this.statusLabelHide.Text = L10n._p("Main window | update status strip", "OK");
            this.statusLabelUpdate.Text = L10n._p("Main window | update status strip | label", "Update status unknown");

            this.menuContact.Text = L10n._("Contact / Privacy Policy / Imprint (advancedfx.org)");

            this.groupBoxHelp.Text = L10n._("Help");
            this.labelHelpLanguage.Text = L10n._("Language:");
            this.labelHelpSelection.Text = L10n._("Selection:");
            {
                HelpEntry officialEnglishSupportPage = new HelpEntry(L10n._("Official support page (English)"), "https://www.advancedfx.org/support/");
                HelpEntry chinesePage = new HelpEntry(L10n._("HLAE Chinese Station"), "https://hlae.site/");

                helpLanguages = new HelpLanguage[] {
                    new HelpLanguage("en", L10n._p("Language", "English (en)"), new HelpEntry[] {
                        officialEnglishSupportPage
                    }),
                    new HelpLanguage("zh-CN", L10n._p("Language", "Chinese (zh-CN)"), new HelpEntry[] {
                        chinesePage
                    })
                };

                this.comboBoxHelpLanguage.Items.AddRange(helpLanguages);

                string ietfLanguageTag = System.Globalization.CultureInfo.CurrentUICulture.IetfLanguageTag;
                string twoLetterIsoLanguageName = System.Globalization.CultureInfo.CurrentUICulture.TwoLetterISOLanguageName;

                // Select a default language:

                int selectIndex = -1;
                int curIndex = 0;

                foreach(HelpLanguage helpLanguage in helpLanguages)
                {
                    if(helpLanguage.Code.Equals(ietfLanguageTag))
                    {
                        selectIndex = curIndex;
                        break;
                    }
                    ++curIndex;
                }

                if(selectIndex == -1)
                {
                    curIndex = 0;
                    foreach (HelpLanguage helpLanguage in helpLanguages)
                    {
                        if (helpLanguage.Code.Equals(twoLetterIsoLanguageName))
                        {
                            selectIndex = curIndex;
                            break;
                        }
                        ++curIndex;
                    }
                }

                if (selectIndex == -1 && 0 < helpLanguages.Length) selectIndex = 0;

                if (selectIndex != -1) comboBoxHelpLanguage.SelectedIndex = selectIndex;
            }

            bool bHlaePathOk = System.Text.RegularExpressions.Regex.IsMatch(Program.BaseDir, "^\\p{IsBasicLatin}*$");
            this.groupBoxPathError.Visible = !bHlaePathOk;
            this.groupBoxPathError.Enabled = !bHlaePathOk;
            this.groupBoxPathError.Text = L10n._("HLAE path error");
            this.labelHlaePath.Text = L10n._("HLAE path:");
            this.textBoxHlaePath.Text = Program.BaseDir;
            this.labelHlaePathError.Text = L10n._("Warning: Your HLAE path shown above contains non-basic latin characters (meaning characters outside the 7-bit ASCII range), which will lead to multiple problems in-game if not fixed!");

            groupBoxConfigError.Text = L10n._("Config error");
            labelConfigError.Text = L10n._("The HLAE config is not writeable (e.g. in use by other HLAE or write-protected), thus opened it in read-only mode. Overwriting it can cause loss of changes made from other instances.");
            buttonConfigReload.Text = L10n._("Reload");
            buttonConfigOverwrite.Text = L10n._("Overwrite");
            buttonConfigOverwrite.Image = SystemIcons.Hand.ToBitmap();
            CheckConfigOkay();

            m_UpdateCheckNotification = new UpdateCheckNotificationTarget(this, new UpdateCheckedDelegate(OnUpdateChecked));
        }

        //
        // Private members:

        Guid m_LastUpdateGuid;
        UpdateCheckNotificationTarget m_UpdateCheckNotification;
        HelpLanguage[] helpLanguages;

        class HelpLanguage
        {
            public HelpLanguage(string code, string label, HelpEntry[] items)
            {
                this.Code = code;
                this.Label = label;
                this.Items = items;
            }

            public override string ToString()
            {
                return Label;
            }

            public string Code { get; }
            public string Label { get; }
            public HelpEntry[] Items { get; }
        }

        class HelpEntry 
        {
            public HelpEntry(string label, string url)
            {
                this.Label = label;
                this.Url = url;
            }

            public string Label { get; }
            public string Url { get; }

            public override string ToString()
            {
                return Label;
            }
        }

        void OnUpdateChecked(object o, IUpdateCheckResult checkResult)
        {
            if (null != checkResult)
            {
                // Has result:
                if (checkResult.IsUpdated)
                {
                    // Updated:

                    m_LastUpdateGuid = checkResult.Guid;

                    Guid ignoreGuid = GlobalConfig.Instance.Settings.IgnoreUpdateGuid;

                    bool notIgnored = null == ignoreGuid || ignoreGuid != checkResult.Guid;

                    statusLabelIgnore.Visible = notIgnored;
                    statusStrip.Visible = statusStrip.Visible || notIgnored;
                    statusLabelUpdate.IsLink = true;
                    statusLabelUpdate.Tag = null != checkResult.Uri ? checkResult.Uri.ToString() : "http://advancedfx.org/";
                    statusLabelUpdate.Text = L10n._p("Main window | update status strip | label" , "Update available!");
                    statusLabelUpdate.ForeColor = Color.Black;
                    statusLabelUpdate.BackColor = Color.Orange;
                }
                else
                {
                    // Is recent:
                    statusLabelIgnore.Visible = false;
                    statusLabelUpdate.IsLink = false;
                    statusLabelUpdate.Text = L10n._p("Main window | update status strip | label", "Your version is up to date :)");
                    statusLabelUpdate.ForeColor = Color.Black;
                    statusLabelUpdate.BackColor = Color.LightGreen;
                }
            }
            else
            {
                // Has no result (s.th. went wrong):
                statusLabelIgnore.Visible = false;
                statusStrip.Visible = true;
                statusLabelUpdate.IsLink = true;
                statusLabelUpdate.Tag = "http://advancedfx.org/";
                statusLabelUpdate.Text = L10n._p("Main window | update status strip | label", "Update check failed :(");
                statusLabelUpdate.ForeColor = Color.Black;
                statusLabelUpdate.BackColor = Color.LightCoral;
            }
        }

        void StartUpdateCheck()
        {
            this.statusLabelUpdate.IsLink = false;
            this.statusLabelUpdate.Text = L10n._p("Main window | update status strip | label", "Checking for updates ...");
            this.statusLabelUpdate.ForeColor = Color.FromKnownColor(KnownColor.ControlText);
            this.statusLabelUpdate.BackColor = Color.FromKnownColor(KnownColor.Control);

            GlobalUpdateCheck.Instance.StartCheck();
        }

        private void MenuFileSize_Click(object sender, EventArgs e)
        {
            (new Tools.Calculator()).Show();
        }

        private void MenuAdvancedFxOrg_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start("https://www.advancedfx.org/");
        }

        private void MenuDonate_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start("https://www.advancedfx.org/credits/#donors");
        }

        private void CheckNowToolStripMenuItem_Click(object sender, EventArgs e)
        {
            StartUpdateCheck();
            this.statusStrip.Visible = true;
        }

        private void StatusBarToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.statusStrip.Visible = !this.menuStatusBar.Checked;
        }

        private void MainForm_Shown(object sender, EventArgs e)
        {
            GlobalUpdateCheck.Instance.BeginCheckedNotification(m_UpdateCheckNotification);

            if (0 < GlobalConfig.Instance.Settings.UpdateCheck)
            {
                this.menuAutoUpdateCheck.Checked = true;
                StartUpdateCheck();
            }
            else if (0 == GlobalConfig.Instance.Settings.UpdateCheck)
            {
                this.stripEnableUpdateCheck.Visible = true;
            }
        }

        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            GlobalUpdateCheck.Instance.EndCheckedNotification(m_UpdateCheckNotification);
        }

        private void StatusLabelUpdate_Click(object sender, EventArgs e)
        {
            if (statusLabelUpdate.IsLink)
            {
                System.Diagnostics.Process.Start(
                     statusLabelUpdate.Tag.ToString()
                );
            }
        }

        private void StatusStrip_VisibleChanged(object sender, EventArgs e)
        {
            this.menuStatusBar.Checked = this.statusStrip.Visible;
        }

        private void StatusLabelHide_Click(object sender, EventArgs e)
        {
            this.statusStrip.Visible = false;
        }

        private void MenuAutoUpdateCheck_Click(object sender, EventArgs e)
        {
            this.stripEnableUpdateCheck.Visible = false;

            menuAutoUpdateCheck.Checked = !menuAutoUpdateCheck.Checked;
            GlobalConfig.Instance.Settings.UpdateCheck = (SByte)(menuAutoUpdateCheck.Checked ? 1 : -1);
        }

        private void StatusLabelAuto_Click(object sender, EventArgs e)
        {
            this.stripEnableUpdateCheck.Visible = false;

            if (this.statusLabelAutoYes == sender)
            {
                menuAutoUpdateCheck.Checked = true;
                GlobalConfig.Instance.Settings.UpdateCheck = (SByte)1;
                StartUpdateCheck();
            }
            else
            {
                menuAutoUpdateCheck.Checked = false;
                GlobalConfig.Instance.Settings.UpdateCheck = (SByte)(-1);
            }
        }

        private void MenuExit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void MenuCustomLoader_Click(object sender, EventArgs e)
        {
            AfxGui.Tools.CustomLoader.RunCustomLoader(this);
        }

        private void MenuGuidToClipBoard_Click(object sender, EventArgs e)
        {
            Clipboard.SetText(GlobalUpdateCheck.Instance.Guid.ToString());
        }

        private void StatusLabelIgnore_Click(object sender, EventArgs e)
        {
            GlobalConfig.Instance.Settings.IgnoreUpdateGuid = m_LastUpdateGuid;
            this.statusStrip.Visible = false;
            this.statusLabelIgnore.Visible = false;
        }

        private void MenuNewGuidToClipBoard_Click(object sender, EventArgs e)
        {
            Clipboard.SetText(Guid.NewGuid().ToString());
        }

        private void MenuLaunchCSGO_Click(object sender, EventArgs e)
        {
            LaunchCsgo.RunLauncherDialog(this);
        }

        private void MenuLaunchGoldSrc_Click(object sender, EventArgs e)
        {
            Launcher.RunLauncherDialog(this);
        }

        private void MenuToolsGoldSrcDemoTools_Click(object sender, EventArgs e)
        {
            AfxCppCli.old.tools.DemoToolsWizard demoWiz = new AfxCppCli.old.tools.DemoToolsWizard();

            demoWiz.OutputPath = GlobalConfig.Instance.Settings.DemoTools.OutputFolder;

            demoWiz.ShowDialog(this);

            GlobalConfig.Instance.Settings.DemoTools.OutputFolder = demoWiz.OutputPath;
            GlobalConfig.Instance.BackUp();

            //demoWiz.Dispose();
        }

        private void MenuToolsGoldSrcSkyManager_Click(object sender, EventArgs e)
        {
            AfxCppCli.old.tools.skymanager sm = new AfxCppCli.old.tools.skymanager(GlobalConfig.Instance.Settings.Launcher.GamePath);
            sm.Show(this);
        }

        private void comboBoxHelpEntry_SelectedIndexChanged(object sender, EventArgs e)
        {
            if(0 <= comboBoxHelpEntries.SelectedIndex && comboBoxHelpEntries.SelectedIndex < comboBoxHelpEntries.Items.Count)
            {
                HelpEntry helpEntry = (HelpEntry)comboBoxHelpEntries.Items[comboBoxHelpEntries.SelectedIndex];
                buttonManual.Text = helpEntry.Url;
            }
        }

        private void comboBoxHelpLanguage_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (0 <= comboBoxHelpLanguage.SelectedIndex && comboBoxHelpLanguage.SelectedIndex < comboBoxHelpLanguage.Items.Count)
            {
                HelpLanguage helpLanguage = (HelpLanguage)comboBoxHelpLanguage.Items[comboBoxHelpLanguage.SelectedIndex];

                comboBoxHelpEntries.Items.Clear();
                comboBoxHelpEntries.Items.AddRange(helpLanguage.Items);

                if (0 < helpLanguage.Items.Length) comboBoxHelpEntries.SelectedIndex = 0;
            }
        }

        private void buttonManual_Click(object sender, EventArgs e)
        {
            if (0 <= comboBoxHelpEntries.SelectedIndex && comboBoxHelpEntries.SelectedIndex < comboBoxHelpEntries.Items.Count)
            {
                HelpEntry helpEntry = (HelpEntry)comboBoxHelpEntries.Items[comboBoxHelpEntries.SelectedIndex];
                System.Diagnostics.Process.Start(helpEntry.Url);
            }
        }

        private void l10nToolStripMenuItem_Click(object sender, EventArgs e)
        {
            new AfxRgbaLutVoronoiGenerator().Show();
        }

        private void menuContact_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start("https://www.advancedfx.org/contact");
        }

        private void CheckConfigOkay()
        {
            groupBoxConfigError.Visible = GlobalConfig.Instance.ReadOnly;
            groupBoxConfigError.Enabled = GlobalConfig.Instance.ReadOnly;
        }

        private void buttonConfigReload_Click(object sender, EventArgs e)
        {
            Program.ReloadConfig();
            CheckConfigOkay();
        }

        private void buttonConfigOverwrite_Click(object sender, EventArgs e)
        {
            GlobalConfig.Instance.ReadOnly = !GlobalConfig.Instance.BackUp(true);
            CheckConfigOkay();
        }
    }
}