using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui
{
    public partial class LauncherForm : Form
    {
        //
        // Internal members:

        internal LauncherForm()
        {
            InitializeComponent();

            this.Text = L10n._p("Launch GoldSrc dialog", "Launch GoldSrc ...");
            this.groupBoxGame.Text = L10n._p("Launch GoldSrc dialog", "Game");
            this.labelExe.Text = L10n._p("Launch GoldSrc dialog", "hl.exe");
            this.buttonExe.Text = L10n._p("Launch GoldSrc dialog", "Browse ...");
            this.labelModSel.Text = L10n._p("Launch GoldSrc dialog", "Modification:");

            this.modificiationValues = new string[] { "cstrike", "dod", "tfc", "valve" };
            this.comboBoxModSel.Items.AddRange(new string[]{
                L10n._p("Launch GoldSrc dialog", "{0} (Counter-Strike)", this.modificiationValues[0]),
                L10n._p("Launch GoldSrc dialog", "{0} (Day of Defeat)", this.modificiationValues[1]),
                L10n._p("Launch GoldSrc dialog", "{0} (Team Fortress Classic)", this.modificiationValues[2]),
                L10n._p("Launch GoldSrc dialog", "{0} (Half-Life)", this.modificiationValues[3]),
                L10n._p("Launch GoldSrc dialog", "Other modification:"),
            });

            this.groupBoxCmdOpts.Text = L10n._p("Launch GoldSrc dialog", "Custom command line options");
            this.groupBoxRes.Text = L10n._p("Launch GoldSrc dialog", "Graphic Resolution");
            this.labelResWidth.Text = L10n._p("Launch GoldSrc dialog", "Width:");
            this.labelResHeight.Text = L10n._p("Launch GoldSrc dialog", "Height:");
            this.labelResDepth.Text = L10n._p("Launch GoldSrc dialog", "Color Depth:");
            this.checkBoxResForce.Text = L10n._p("Launch GoldSrc dialog", "force resolution");
            this.checkBoxFullScreen.Text = L10n._p("Launch GoldSrc dialog", "full screen");

            this.bppValues = new byte[] { 32, 24, 16 };
            this.comboBoxResDepth.Items.AddRange(new string[]{
                L10n._p("Launch GoldSrc dialog bpp", "32 (High)"),
                L10n._p("Launch GoldSrc dialog bpp", "24 (Medium)"),
                L10n._p("Launch GoldSrc dialog bpp", "16 (Low)"),
            });

            this.groupBoxMisc.Text = L10n._p("Launch GoldSrc dialog", "Advanced Settings");
            this.checkBoxForceAlpha.Text = L10n._p("Launch GoldSrc dialog", "Force 8 bit alpha channel");
            this.checkBoxVisbility.Text = L10n._p("Launch GoldSrc dialog", "Optimize window visibilty on capture");
            this.checkBoxDesktopRes.Text = L10n._p("Launch GoldSrc dialog", "Optimize desktop resolution");

            this.labelRenderMode.Text = L10n._p("Launch GoldSrc dialog", "Render mode:");
            this.comboBoxRenderMode.Items.AddRange(new string[]
            {
                L10n._p("Launch GoldSrc dialog render mode", "Standard"),
                L10n._p("Launch GoldSrc dialog render mode", "FrameBuffer Object"),
                L10n._p("Launch GoldSrc dialog render mode", "Memory DC")
            });

            this.checkBoxRemeber.Text = L10n._p("Launch GoldSrc dialog", "remember my changes");
            this.buttonOK.Text = L10n._p("Launch GoldSrc dialog", "L&aunch");
            this.buttonCancel.Text = L10n._p("Launch GoldSrc dialog", "Can&cel");
        }


        internal void ReadFromConfig(CfgLauncher cfg)
        {
	        this.checkBoxRemeber.Checked = cfg.RememberChanges;
	        this.textBoxExe.Text = cfg.GamePath;

	        int iInd = -1;
	        for( int i = 0; i < this.modificiationValues.Length; i++)
	        {
		        if(0 == String.Compare(this.modificiationValues[i], cfg.Modification))
		        {
			        iInd = i;
			        break;
		        }
	        }

	        if(iInd<0 || iInd >= this.modificiationValues.Length)
	        {
		        this.textBoxCustMod.Enabled = true;
		        this.comboBoxModSel.SelectedIndex = this.comboBoxModSel.Items.Count - 1;
		        this.textBoxCustMod.Text = cfg.Modification;
	        }
	        else
	        {
		        this.textBoxCustMod.Enabled = false;
		        this.comboBoxModSel.SelectedIndex = iInd;
	        }

	        this.textBoxCmdAdd.Text = cfg.CustomCmdLine;
	        this.checkBoxResForce.Checked = cfg.GfxForce;
	        this.textBoxResWidth.Text = cfg.GfxWidth.ToString();
	        this.textBoxResHeight.Text = cfg.GfxHeight.ToString();
	        this.comboBoxResDepth.Text = cfg.GfxBpp.ToString();
	        // Advanced Settings:
	        this.checkBoxForceAlpha.Checked = cfg.ForceAlpha;
	        this.checkBoxDesktopRes.Checked = cfg.OptimizeDesktopRes;
	        this.checkBoxVisbility.Checked = cfg.OptimizeVisibilty;
	        //
	        if( cfg.RenderMode < this.comboBoxRenderMode.Items.Count )
		        this.comboBoxRenderMode.SelectedIndex = cfg.RenderMode;
	        //
	        checkBoxFullScreen.Checked = cfg.FullScreen;
        }

        internal void WriteToConfig(CfgLauncher cfg)
        {
	        cfg.RememberChanges = this.checkBoxRemeber.Checked;

	        cfg.GamePath = this.textBoxExe.Text;

	        if( this.comboBoxModSel.SelectedIndex < 0 || this.comboBoxModSel.SelectedIndex >= this.modificiationValues.Length)
	        {
		        cfg.Modification = this.textBoxCustMod.Text;
	        } else {
                cfg.Modification = this.modificiationValues[this.comboBoxModSel.SelectedIndex];
	        }

	        cfg.CustomCmdLine = this.textBoxCmdAdd.Text;
	        cfg.GfxForce = this.checkBoxResForce.Checked;
	        UInt16.TryParse( this.textBoxResWidth.Text, out cfg.GfxWidth );
	        UInt16.TryParse( this.textBoxResHeight.Text, out cfg.GfxHeight );

            if (this.comboBoxResDepth.SelectedIndex < 0 || this.comboBoxResDepth.SelectedIndex >= this.bppValues.Length)
            {
                Byte.TryParse(this.comboBoxResDepth.Text, out cfg.GfxBpp);
            }
            else
            {
                cfg.GfxBpp = this.bppValues[this.comboBoxResDepth.SelectedIndex];
            }

	        // Advanced Settings:
	        cfg.ForceAlpha = this.checkBoxForceAlpha.Checked;
	        cfg.OptimizeDesktopRes = this.checkBoxDesktopRes.Checked;
	        cfg.OptimizeVisibilty = this.checkBoxVisbility.Checked;
	        cfg.RenderMode = (byte)this.comboBoxRenderMode.SelectedIndex;
	        cfg.FullScreen = this.checkBoxFullScreen.Checked;
        }

        //
        // Private members:

        private string[] modificiationValues;
        private Byte[] bppValues;

        private void buttonExe_Click(object sender, EventArgs e)
        {
            if (DialogResult.OK == openFileDialogExe.ShowDialog(this))
            {
                this.textBoxExe.Text = openFileDialogExe.FileName;
            }
        }

        private void checkBoxFullScreen_Click(object sender, EventArgs e)
        {


            if (this.checkBoxFullScreen.Checked)
            {
                DialogResult dr = MessageBox.Show(
                    "Switching to full screen is not recommended,\n"
                    + "i.e. full screen supports much fewer resolutions.\n"
                    + "\n"
                    + "Do you still want to switch to fullscreen?",
                    "Really switch to full screen?",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Information,
                    MessageBoxDefaultButton.Button2
                );
                if (System.Windows.Forms.DialogResult.Yes == dr)
                {
                    this.checkBoxVisbility.Checked = false;
                }
                else this.checkBoxFullScreen.Checked = false;
            }
        }

        private void comboBoxModSel_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (this.comboBoxModSel.SelectedIndex == this.comboBoxModSel.Items.Count - 1)
            {
                this.textBoxCustMod.Enabled = true;
            }
            else
            {
                this.textBoxCustMod.Enabled = false;
            }
        }

        private void textBoxResHeight_TextChanged(object sender, EventArgs e)
        {
            UInt16 height;

            if (!UInt16.TryParse(this.textBoxResHeight.Text, out height))
                errorProvider.SetError(this.textBoxResHeight, "Invalid value.");
            else if(height < 480)
                errorProvider.SetError(this.textBoxResHeight, "Warning: Values bellow 480 won't work properly\nunless an EngineFont for this case is added to TrackerScheme.res\nin Half-Life\\platform\\resource\\TracherScheme.res.");
            else
                errorProvider.SetError(this.textBoxResHeight, null);
        }
    }
}
