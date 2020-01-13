using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui.Tools
{
    public partial class Calculator : Form
    {
        internal Calculator()
        {
            InitializeComponent();
            this.Icon = Program.Icon;

            this.Text = L10n._p("File Size Calculator", "File Size Calculator");
            this.labelWidth.Text = L10n._p("File Size Calculator", "Width:");
            this.labelHeight.Text = L10n._p("File Size Calculator", "Height:");
            this.labelFPS.Text = L10n._p("File Size Calculator", "FPS:");
            this.labelDuration.Text = L10n._p("File Size Calculator", "Duration:");
            this.labelMin.Text = L10n._p("File Size Calculator | Duration", "min");
            this.labelSec.Text = L10n._p("File Size Calculator | Duration", "sec");
            this.groupBoxEstimate.Text = L10n._p("File Size Calculator", "Estimated Disk Usage");
            this.checkHuffYuv.Text = L10n._p("File Size Calculator | Estimate", "HuffYuv encoded output");

            this.textWidth.Text = ((int)1920).ToString();
            this.textHeight.Text = ((int)1080).ToString();
            this.textFps.Text = (60.0f).ToString("N1");
            this.textMin.Text = 1.ToString();
            this.textSec.Text = 0.ToString();

            this.textWidth.TextChanged += text_TextChanged;
            this.textHeight.TextChanged += text_TextChanged;
            this.textFps.TextChanged += text_TextChanged;
            this.textMin.TextChanged += text_TextChanged;
            this.textSec.TextChanged += text_TextChanged;

            DoRecalc();
        }

        private void DoRecalc()
        {
            bool bOk = true;
            Decimal width = 0;
            Decimal height = 0;
            Decimal fps = 0;
            Decimal mins = 0;
            Decimal secs = 0;

            if (!System.Decimal.TryParse(textWidth.Text, out width))
            {
                bOk = false;
                errorProvider.SetError(textWidth, "Parsing failed.");
            }
            else
                errorProvider.SetError(textWidth, "");

            if (!System.Decimal.TryParse(textHeight.Text, out height))
            {
                bOk = false;
                errorProvider.SetError(textHeight, "Parsing failed.");
            }
            else
                errorProvider.SetError(textHeight, "");

            if (!System.Decimal.TryParse(textFps.Text, out fps))
            {
                bOk = false;
                errorProvider.SetError(textFps, "Parsing failed.");
            }
            else
                errorProvider.SetError(textFps, "");

            if (!System.Decimal.TryParse(textMin.Text, out mins))
            {
                bOk = false;
                errorProvider.SetError(textMin, "Parsing failed.");
            }
            else
                errorProvider.SetError(textMin, "");

            if (!System.Decimal.TryParse(textSec.Text, out secs))
            {
                bOk = false;
                errorProvider.SetError(textSec, "Parsing failed.");
            }
            else
                errorProvider.SetError(textSec, "");

			if( bOk )
			{
				System.Decimal result;
				System.String sunit = "Byte";

				result = width * height * fps * (mins * 60 + secs) * 3;

				if( this.checkHuffYuv.Checked )
					result = result / new System.Decimal(2.5);

				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "KiB";
				}
				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "MiB";
				}
				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "GiB";
				}
				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "TiB";
				}
				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "PiB";
				}
				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "EiB";
				}
				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "ZiB";
				}
				if( result >= 1024)
				{
					result = result / new System.Decimal(1024);
					sunit = "YiB";
				}

			System.Globalization.CultureInfo MyCI = new System.Globalization.CultureInfo("en-US", false);
			System.Globalization.NumberFormatInfo nfi = MyCI.NumberFormat;
			nfi.NumberDecimalDigits = 4;


			textSize.Text = String.Format("{0} {1}", result.ToString("N4"), sunit);
			}
			else
			{
                textSize.Text = "invalid input";
			}
        }

        private void text_TextChanged(object sender, EventArgs e)
        {
            DoRecalc();
        }
    }


}
