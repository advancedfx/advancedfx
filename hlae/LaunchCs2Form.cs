using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui
{
    public partial class LaunchCs2Form : Form
    {
        public LaunchCs2Form()
        {
            InitializeComponent();

            this.Icon = Program.Icon;

            this.Text = L10n._p("Launch CS2 dialog", "Launch CS2 ...");
            this.groupBoxGame.Text = L10n._p("Launch CS2 dialog", "Game");
            this.labelExe.Text = L10n._p("Launch CS2 dialog", "cs2.exe file:");
            this.buttonExe.Text = L10n._p("Launch CS2 dialog", "Browse ...");
            this.groupBoxMmcfg.Text = L10n._p("Launch CS2 dialog", "Moviemaking config parent folder");
            this.buttonMmcfgInfo.Text = L10n._p("Launch CS2 dialog", "What's this?");
            this.checkBoxEnableMmcfg.Text = L10n._p("Launch CS2 dialog", "enable");
            this.buttonMmcfg.Text = L10n._p("Launch CS2 dialog", "Browse ...");
            this.groupBoxRes.Text = L10n._p("Launch CS2 dialog", "Graphics Resolution");
            this.checkBoxEnableGfx.Text = L10n._p("Launch CS2 dialog", "enable");
            this.labelGfxWidth.Text = L10n._p("Launch CS2 dialog", "Width:");
            this.labelGfxHeight.Text = L10n._p("Launch CS2 dialog", "Height:");
            this.labelGfxInfo.Text = L10n._p("Launch CS2 dialog", "Actual results depend on the game.");
            this.checkBoxGfxFull.Text = L10n._p("Launch CS2 dialog", "full screen");
            this.groupBoxCmdOpts.Text = L10n._p("Launch CS2 dialog", "Custom command line options");
            this.checkBoxAvoidVac.Text = L10n._p("Launch CS2 dialog", "{0} (prevents joining VAC secured server / VAC bans)", "-insecure");
            this.checkBoxRemeber.Text = L10n._p("Launch CS2 dialog", "remember my changes");
            this.buttonOK.Text = L10n._p("Launch CS2 dialog", "L&aunch");
            this.buttonCancel.Text = L10n._p("Launch CS2 dialog", "Can&cel");
        }

        internal CfgLauncherCs2 Config
        {
            get
            {
                CfgLauncherCs2 cfg = new CfgLauncherCs2();

                cfg.Cs2Exe = textBoxExe.Text;
                cfg.MmcfgEnabled = checkBoxEnableMmcfg.Checked;
                cfg.Mmcfg = textBoxMmcfg.Text;
                cfg.GfxEnabled = checkBoxEnableGfx.Checked;
                UInt16.TryParse(textBoxGfxWidth.Text, out cfg.GfxWidth);
                UInt16.TryParse(textBoxGfxHeight.Text, out cfg.GfxHeight);
                cfg.GfxFull = checkBoxGfxFull.Checked;
                cfg.AvoidVac = checkBoxAvoidVac.Checked;
                cfg.CustomLaunchOptions = textBoxCustomCmd.Text;
                cfg.RememberChanges = checkBoxRemeber.Checked;

                return cfg;
            }

            set
            {
                textBoxExe.Text = value.Cs2Exe;
                checkBoxEnableMmcfg.Checked = value.MmcfgEnabled;
                textBoxMmcfg.Text = value.Mmcfg;
                checkBoxEnableGfx.Checked = value.GfxEnabled;
                textBoxGfxWidth.Text = value.GfxWidth.ToString();
                textBoxGfxHeight.Text = value.GfxHeight.ToString();
                checkBoxGfxFull.Checked = value.GfxFull;
                checkBoxAvoidVac.Checked = true;
                textBoxCustomCmd.Text = value.CustomLaunchOptions;
                checkBoxRemeber.Checked = value.RememberChanges;

                EnableGfx(checkBoxEnableGfx.Checked);
                EnableMmcfg(checkBoxEnableMmcfg.Checked);
            }
        }

        private void EnableMmcfg(bool enabled)
        {
            textBoxMmcfg.Enabled = enabled;
            buttonMmcfg.Enabled = enabled;
        }

        private void checkBoxEnableMmcfg_CheckedChanged(object sender, EventArgs e)
        {
            EnableMmcfg(checkBoxEnableMmcfg.Checked);
        }
        
        private void EnableGfx(bool enabled)
        {
            labelGfxHeight.Enabled = enabled;
            labelGfxWidth.Enabled = enabled;
            labelGfxHeight.Enabled = enabled;
            checkBoxGfxFull.Enabled = enabled;
            textBoxGfxWidth.Enabled = enabled;
            textBoxGfxHeight.Enabled = enabled;
        }

        private void checkBoxEnableGfx_CheckedChanged(object sender, EventArgs e)
        {
            EnableGfx(checkBoxEnableGfx.Checked);
        }

        private void buttonMmcfgInfo_Click(object sender, EventArgs e)
        {
            MessageBox.Show(
                L10n._p("Launch CS2 dialog | MmcfgFolder Description", "When enabled you can set a parent folder for your moviemaking config for the game.\nThe game will create a sub-folder called cfg there and store configuration/video settings in that folder.\nAlso you can put your moviemaking config into that cfg sub-folder."),
                L10n._p("Launch CS2 dialog | MmcfgFolder Title", "About moviemaking config parent folder"),
                MessageBoxButtons.OK,
                MessageBoxIcon.Information,
                MessageBoxDefaultButton.Button1
                );
        }

        private void buttonExe_Click(object sender, EventArgs e)
        {
            if (openFileDialogExe.ShowDialog(this) == DialogResult.OK)
            {
                textBoxExe.Text = openFileDialogExe.FileName;
            }
        }

        private void buttonMmcfg_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialogMMcfg.ShowDialog(this) == DialogResult.OK)
            {
                textBoxMmcfg.Text = folderBrowserDialogMMcfg.SelectedPath;
            }
        }
    }
}
