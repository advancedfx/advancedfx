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

            m_UpdateCheckNotification = new UpdateCheckNotificationTarget(this, new UpdateCheckedDelegate(OnUpdateChecked));
        }

        //
        // Private members:

        Guid m_LastUpdateGuid;
        UpdateCheckNotificationTarget m_UpdateCheckNotification;

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
                    statusLabelUpdate.Text = "Update available!";
                    statusLabelUpdate.ForeColor = Color.Black;
                    statusLabelUpdate.BackColor = Color.Orange;
                }
                else
                {
                    // Is recent:
                    statusLabelIgnore.Visible = false;
                    statusLabelUpdate.IsLink = false;
                    statusLabelUpdate.Text = "Your version is up to date :)";
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
                statusLabelUpdate.Text = "Update check failed :(";
                statusLabelUpdate.ForeColor = Color.Black;
                statusLabelUpdate.BackColor = Color.LightCoral;
            }
        }

        void StartUpdateCheck()
        {
            this.statusLabelUpdate.IsLink = false;
            this.statusLabelUpdate.Text = "Checking for updates ...";
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
            System.Diagnostics.Process.Start("https://opencollective.com/advancedfx/");
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
    }
}