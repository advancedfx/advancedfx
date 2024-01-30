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
    public partial class LaunchCsgoForm : Form
    {
        public LaunchCsgoForm()
        {
            InitializeComponent();

            this.Icon = Program.Icon;

            this.Text = L10n._p("Launch CS:GO dialog", "Launch CS:GO ...");
            this.groupBoxGame.Text = L10n._p("Launch CS:GO dialog", "Game");
            this.labelExe.Text = L10n._p("Launch CS:GO dialog", "csgo.exe file:");
            this.buttonExe.Text = L10n._p("Launch CS:GO dialog", "Browse ...");
            this.groupBoxMmcfg.Text = L10n._p("Launch CS:GO dialog", "Movie making config parent folder");
            this.buttonMmcfgInfo.Text = L10n._p("Launch CS:GO dialog", "What's this?");
            this.checkBoxEnableMmcfg.Text = L10n._p("Launch CS:GO dialog", "enable");
            this.buttonMmcfg.Text = L10n._p("Launch CS:GO dialog", "Browse ...");
            this.groupBoxRes.Text = L10n._p("Launch CS:GO dialog", "Graphics Resolution");
            this.checkBoxEnableGfx.Text = L10n._p("Launch CS:GO dialog", "enable");
            this.labelGfxWidth.Text = L10n._p("Launch CS:GO dialog", "Width:");
            this.labelGfxHeight.Text = L10n._p("Launch CS:GO dialog", "Height:");
            this.labelGfxInfo.Text = L10n._p("Launch CS:GO dialog", "Actual results depend on the game.");
            this.checkBoxGfxFull.Text = L10n._p("Launch CS:GO dialog", "full screen");
            this.groupBoxCmdOpts.Text = L10n._p("Launch CS:GO dialog", "Custom command line options");
            this.checkBoxAvoidVac.Text = L10n._p("Launch CS:GO dialog", "{0} (prevents joining VAC secured server / VAC bans)", "-insecure");
            this.checkBoxRemeber.Text = L10n._p("Launch CS:GO dialog", "remember my changes");
            this.buttonOK.Text = L10n._p("Launch CS:GO dialog", "L&aunch");
            this.buttonCancel.Text = L10n._p("Launch CS:GO dialog", "Can&cel");
        }

        internal CfgLauncherCsgo Config
        {
            get
            {
                CfgLauncherCsgo cfg = new CfgLauncherCsgo();

                cfg.CsgoExe = textBoxExe.Text;
                cfg.MmcfgEnabled = checkBoxEnableMmcfg.Checked;
                cfg.Mmmcfg = textBoxMmcfg.Text;
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
                textBoxExe.Text = value.CsgoExe;
                checkBoxEnableMmcfg.Checked = value.MmcfgEnabled;
                textBoxMmcfg.Text = value.Mmmcfg;
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

        private void checkBoxAvoidVac_Click(object sender, EventArgs e)
        {
            this.checkBoxAvoidVac.Checked = true;

            /*
            if (!this.checkBoxAvoidVac.Checked)
            {
                DialogResult dr = MessageBox.Show(
                    "You should leave this checkbox checked\n"
                    + "in order to avoid VAC bans!\n" // line rewritten 2017-05-01T16:20Z to avoid trivial copyright issues.
                    + "\n"
                    + "Do you still want to uncheck it?", // line rewritten 2017-05-01T16:20Z to avoid trivial copyright issues.
                    "Really risk VAC bans?", // line rewritten 2017-05-01T16:20Z to avoid trivial copyright issues.
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Exclamation,
                    MessageBoxDefaultButton.Button2
                );
                if (System.Windows.Forms.DialogResult.Yes != dr)
                {
                    this.checkBoxAvoidVac.Checked = true;
                }
            }
            */
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
                L10n._p("Launch CS:GO dialog | MmcfgFolder Description", "When enabled you can set a parent folder for your movie making config for the game.\nThe game will create a sub-folder called cfg there and store config.cfg\nand video settings in that folder.\nAlso you can put your movie making config into that cfg sub-folder,\nhowever the game will load those only if they are not present in the csgo/cfg folder already."),
                L10n._p("Launch CS:GO dialog | MmcfgFolder Title", "About movie making config parent folder"),
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
