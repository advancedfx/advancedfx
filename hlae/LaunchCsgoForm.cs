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
            MessageBox.Show(
                "HLAE users requested this option to be made mandatory.\n"
                +"The hook will refuse to work without it, thus it can not be removed.",
                "Mandatory option ...",
                MessageBoxButtons.OK,
                MessageBoxIcon.Stop
            );

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
                "When enabled you can set a parent folder for your movie making config for the game.\n"
                + "The game will create a sub-folder called cfg there and store config.cfg\n"
                + "and video settings in that folder.\n"
                + "Also you can put your movie making config into that cfg sub-folder,\n"
                + "however the game will load those only if they are not present in"
                + "the csgo/cfg folder already.",
                "About movie making config parent folder",
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
