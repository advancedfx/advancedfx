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
            System.Globalization.CultureInfo cultureInfo = new System.Globalization.CultureInfo("en-US", false);
            IFormatProvider provider;
            System.Globalization.NumberStyles style = System.Globalization.NumberStyles.Number;

            cultureInfo.NumberFormat.NumberDecimalDigits = 4;
            provider = cultureInfo.NumberFormat;

            if (!System.Decimal.TryParse(textWidth.Text, style, provider, out width))
            {
                bOk = false;
                errorProvider.SetError(textWidth, "Parsing failed.");
            }
            else
                errorProvider.SetError(textWidth, "");

            if (!System.Decimal.TryParse(textHeight.Text, style, provider, out height))
            {
                bOk = false;
                errorProvider.SetError(textHeight, "Parsing failed.");
            }
            else
                errorProvider.SetError(textHeight, "");

            if (!System.Decimal.TryParse(textFps.Text, style, provider, out fps))
            {
                bOk = false;
                errorProvider.SetError(textFps, "Parsing failed.");
            }
            else
                errorProvider.SetError(textFps, "");

            if (!System.Decimal.TryParse(textMin.Text, style, provider, out mins))
            {
                bOk = false;
                errorProvider.SetError(textMin, "Parsing failed.");
            }
            else
                errorProvider.SetError(textMin, "");

            if (!System.Decimal.TryParse(textSec.Text, style, provider, out secs))
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


			textSize.Text = String.Format("{0} {1}", result.ToString("N", provider), sunit);
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
