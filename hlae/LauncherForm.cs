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
        }


        internal void ReadFromConfig(CfgLauncher cfg)
        {
	        this.checkBoxRemeber.Checked = cfg.RememberChanges;
	        this.textBoxExe.Text = cfg.GamePath;

	        int iInd = -1;
	        for( int i = 0; i < this.comboBoxModSel.Items.Count; i++)
	        {

		        String stext = this.comboBoxModSel.Items[i].ToString();
                stext = stext.Split(new char[]{' '}, 2)[0];
                    
		        if(0 == String.Compare(stext,cfg.Modification))
		        {
			        iInd = i;
			        break;
		        }
	        }

	        if(iInd<0 || iInd >= this.comboBoxModSel.Items.Count-1)
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
            char[] spaceDelim = new char[] { ' ' };

	        cfg.RememberChanges = this.checkBoxRemeber.Checked;

	        cfg.GamePath = this.textBoxExe.Text;

	        if( this.comboBoxModSel.SelectedIndex ==  this.comboBoxModSel.Items.Count-1)
	        {
		        cfg.Modification = this.textBoxCustMod.Text;
	        } else {
                cfg.Modification = this.comboBoxModSel.Text.Split(spaceDelim, 2)[0];
	        }

	        cfg.CustomCmdLine = this.textBoxCmdAdd.Text;
	        cfg.GfxForce = this.checkBoxResForce.Checked;
	        UInt16.TryParse( this.textBoxResWidth.Text, out cfg.GfxWidth );
	        UInt16.TryParse( this.textBoxResHeight.Text, out cfg.GfxHeight );
            Byte.TryParse(this.comboBoxResDepth.Text.Split(spaceDelim, 2)[0], out cfg.GfxBpp);

	        // Advanced Settings:
	        cfg.ForceAlpha = this.checkBoxForceAlpha.Checked;
	        cfg.OptimizeDesktopRes = this.checkBoxDesktopRes.Checked;
	        cfg.OptimizeVisibilty = this.checkBoxVisbility.Checked;
	        cfg.RenderMode = (byte)this.comboBoxRenderMode.SelectedIndex;
	        cfg.FullScreen = this.checkBoxFullScreen.Checked;
        }

        //
        // Private members:

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
