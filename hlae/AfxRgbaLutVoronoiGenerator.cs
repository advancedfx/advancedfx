using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui
{
    public partial class AfxRgbaLutVoronoiGenerator : Form
    {
        public AfxRgbaLutVoronoiGenerator()
        {
            InitializeComponent();

            m_Colors.Add(new HlaeCommentedColor(0.878431f, 0.686275f, 0.337255f, 1, "T"), new HlaeColor(0.902f, 0.525f, 0.196f, 1));
            m_Colors.Add(new HlaeCommentedColor(0.447059f, 0.607843f, 0.866667f, 1, "CT"), new HlaeColor(0.384f, 0.388f, 0.741f, 1));
            //m_Colors.Add(new HlaeCommentedColor(0.662745f, 0.647059f, 0.601961f, 1, "T/CT primary neutral"), new HlaeColor(0.637258f, 0.637258f, 0.637258f));
            m_Colors.Add(new HlaeCommentedColor(0.894f, 0.486f, 0f, 1, "T aim"), new HlaeColor(0.902f, 0.525f, 0.196f, 1));
            m_Colors.Add(new HlaeCommentedColor(0f, 0.447f, 0.922f, 1, "CT aim"), new HlaeColor(0.384f, 0.388f, 0.741f, 1));
            //m_Colors.Add(new HlaeCommentedColor(0.984f, 0.4815f, 0.922f, 1, "T/CT aim neutral"), new HlaeColor(0.795833f, 0.795833f, 0.795833, 1));

            m_Colors.Add(new HlaeCommentedColor(0, 0, 0, 0), new HlaeColor(0, 0, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 0, 0, 1), new HlaeColor(0, 0, 0, 1));
            m_Colors.Add(new HlaeCommentedColor(0, 0, 1, 0), new HlaeColor(0, 0, 1, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 0, 1, 1), new HlaeColor(0, 0, 1, 1));
            m_Colors.Add(new HlaeCommentedColor(0, 1, 0, 0), new HlaeColor(0, 1, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 1, 0, 1), new HlaeColor(0, 1, 0, 1));
            m_Colors.Add(new HlaeCommentedColor(0, 1, 1, 0), new HlaeColor(0, 1, 1, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 1, 1, 1), new HlaeColor(0, 1, 1, 1));
            m_Colors.Add(new HlaeCommentedColor(1, 0, 0, 0), new HlaeColor(1, 0, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(1, 0, 0, 1), new HlaeColor(1, 0, 0, 1));
            m_Colors.Add(new HlaeCommentedColor(1, 0, 1, 0), new HlaeColor(1, 0, 1, 0));
            m_Colors.Add(new HlaeCommentedColor(1, 0, 1, 1), new HlaeColor(1, 0, 1, 1));
            m_Colors.Add(new HlaeCommentedColor(1, 1, 0, 0), new HlaeColor(1, 1, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(1, 1, 0, 1), new HlaeColor(1, 1, 0, 1));
            m_Colors.Add(new HlaeCommentedColor(1, 1, 1, 0), new HlaeColor(1, 1, 1, 0));
            m_Colors.Add(new HlaeCommentedColor(1, 1, 1, 1), new HlaeColor(1, 1, 1, 1));

            foreach (KeyValuePair<HlaeCommentedColor, HlaeColor> kvp in m_Colors)
            {
                int idx = dataGridViewColors.Rows.Add(
                    VavlueToByte(kvp.Key.Color.R),
                    VavlueToByte(kvp.Key.Color.G),
                    VavlueToByte(kvp.Key.Color.B),
                    VavlueToByte(kvp.Key.Color.A),
                    kvp.Key.Comment,
                    VavlueToByte(kvp.Value.R),
                    VavlueToByte(kvp.Value.G),
                    VavlueToByte(kvp.Value.B),
                    VavlueToByte(kvp.Value.A)
                );
            }
        }

        private byte VavlueToByte(float value)
        {
            return (byte)Math.Min(Math.Max(0, value * 255f + 0.5f), 255f);
        }

        public struct HlaeColorUc
        {
            public byte R { get; set; }
            public byte G { get; set; }
            public byte B { get; set; }
            public byte A { get; set; }


            public HlaeColorUc(HlaeColor value)
            {
                R = (byte)Math.Min(Math.Max(0, value.R * 255f + 0.5f), 255f);
                G = (byte)Math.Min(Math.Max(0, value.G * 255f + 0.5f), 255f);
                B = (byte)Math.Min(Math.Max(0, value.B * 255f + 0.5f), 255f);
                A = (byte)Math.Min(Math.Max(0, value.A * 255f + 0.5f), 255f);
            }

            public override string ToString() 
            {
                return R.ToString("X2") + G.ToString("X2") + B.ToString("X2") + A.ToString("X2");
            }

            public System.Drawing.Color ToColor()
            {
                return System.Drawing.Color.FromArgb(A, R, G, B);
            }
        }

        public struct HlaeColor
        {
            public float R { get; set; }
            public float G { get; set; }
            public float B { get; set; }
            public float A { get; set; }

            public HlaeColor(float r, float g, float b, float a)
            {
                this.R = r;
                this.G = g;
                this.B = b;
                this.A = a;
            }

            public override string ToString()
            {
                return new HlaeColorUc(this).ToString();
            }

            public HlaeColor MullAdd(float mul, float add)
            {
                return new HlaeColor(mul * R + add, mul * G + add, mul * B + add, mul * A + add);
            }
        }

        public struct HlaeCommentedColor
        {
            public HlaeColor Color { get; set; }
            public string Comment { get; set; }

            public HlaeCommentedColor(float r, float g, float b, float a, string comment = "")
            {
                Color = new HlaeColor(r,g,b,a);
                Comment = comment;
            }
        }


        private Dictionary<HlaeCommentedColor, HlaeColor> m_Colors = new Dictionary<HlaeCommentedColor, HlaeColor>();

        private System.Drawing.Color ForeColorValue(System.Drawing.Color color)
        {
            return System.Drawing.Color.FromArgb(
                color.A,
                color.R < 64 || color.R >= 192 ? 255 - color.R : (255 - color.R + 128) % 256,
                color.G < 64 || color.G >= 192 ? 255 - color.G : (255 - color.G + 128) % 256,
                color.B < 64 || color.B >= 192 ? 255 - color.B : (255 - color.B + 128) % 256
            );
        }

        private void StyleRow(DataGridViewRow row)
        {
            bool isNull = true;
            for (int i = 0; i < 9; ++i) isNull = isNull && null == row.Cells[i].Value;
            if (isNull) return;

            byte sR = 255, sG = 255, sB = 255, sA = 255, dR = 255, dG = 255, dB = 255, dA = 255;

            bool bSR = null != row.Cells[0].Value ? byte.TryParse(row.Cells[0].Value.ToString(), out sR) : false;
            bool bSG = null != row.Cells[1].Value ? byte.TryParse(row.Cells[1].Value.ToString(), out sG) : false;
            bool bSB = null != row.Cells[2].Value ? byte.TryParse(row.Cells[2].Value.ToString(), out sB) : false;
            bool bSA = null != row.Cells[3].Value ? byte.TryParse(row.Cells[3].Value.ToString(), out sA) : false;
            bool bDR = null != row.Cells[5].Value ? byte.TryParse(row.Cells[5].Value.ToString(), out dR) : false;
            bool bDG = null != row.Cells[6].Value ? byte.TryParse(row.Cells[6].Value.ToString(), out dG) : false;
            bool bDB = null != row.Cells[7].Value ? byte.TryParse(row.Cells[7].Value.ToString(), out dB) : false;
            bool bDA = null != row.Cells[8].Value ? byte.TryParse(row.Cells[8].Value.ToString(), out dA) : false;

            System.Drawing.Color bcS = System.Drawing.Color.FromArgb(255, sR, sG, sB);
            System.Drawing.Color bcSf = ForeColorValue(bcS);
            System.Drawing.Color bcD = System.Drawing.Color.FromArgb(255, dR, dG, dB);
            System.Drawing.Color bcDf = ForeColorValue(bcD);

            row.Cells[0].Style.SelectionBackColor = row.Cells[0].Style.BackColor = bSR ? bcS : bcSf;
            row.Cells[0].Style.SelectionForeColor = row.Cells[0].Style.ForeColor = bSR ? bcSf : bcS;

            row.Cells[1].Style.SelectionBackColor = row.Cells[1].Style.BackColor = bSG ? bcS : bcSf;
            row.Cells[1].Style.SelectionForeColor = row.Cells[1].Style.ForeColor = bSG ? bcSf : bcS;
            row.Cells[2].Style.SelectionBackColor = row.Cells[2].Style.BackColor = bSB ? bcS : bcSf;
            row.Cells[2].Style.SelectionForeColor = row.Cells[2].Style.ForeColor = bSB ? bcSf : bcS;
            row.Cells[3].Style.SelectionBackColor = row.Cells[3].Style.BackColor = bSA ? System.Drawing.Color.FromArgb(255, sA, sA, sA) : System.Drawing.Color.FromArgb(255, 255, 0, 0);
            row.Cells[3].Style.SelectionForeColor = row.Cells[3].Style.ForeColor = ForeColorValue(row.Cells[3].Style.BackColor);

            row.Cells[5].Style.SelectionBackColor = row.Cells[5].Style.BackColor = bDR ? bcD : bcDf;
            row.Cells[5].Style.SelectionForeColor = row.Cells[5].Style.ForeColor = bDR ? bcDf : bcD;
            row.Cells[6].Style.SelectionBackColor = row.Cells[6].Style.BackColor = bDG ? bcD : bcDf;
            row.Cells[6].Style.SelectionForeColor = row.Cells[6].Style.ForeColor = bDG ? bcDf : bcD;
            row.Cells[7].Style.SelectionBackColor = row.Cells[7].Style.BackColor = bDB ? bcD : bcDf;
            row.Cells[7].Style.SelectionForeColor = row.Cells[7].Style.ForeColor = bDB ? bcDf : bcD;
            row.Cells[8].Style.SelectionBackColor = row.Cells[8].Style.BackColor = bDA ? System.Drawing.Color.FromArgb(255, dA, dA, dA) : System.Drawing.Color.FromArgb(255, 255, 0, 0);
            row.Cells[8].Style.SelectionForeColor = row.Cells[8].Style.ForeColor = ForeColorValue(row.Cells[8].Style.BackColor);
        }

        private void ColorCell(DataGridViewCell cell)
        {
            if (null != cell && null != cell.Value && 0 <= cell.RowIndex) StyleRow(dataGridViewColors.Rows[cell.RowIndex]);                
        }

        private void dataGridViewColors_CellValueChanged(object sender, DataGridViewCellEventArgs e)
        {
            if(0 <= e.RowIndex && 0 <= e.ColumnIndex) ColorCell((sender as DataGridView).Rows[e.RowIndex].Cells[e.ColumnIndex]);
        }

        private void dataGridViewColors_RowsAdded(object sender, DataGridViewRowsAddedEventArgs e)
        {
            for(int i=e.RowIndex; i< e.RowIndex + e.RowCount; ++i)
            {
                StyleRow(dataGridViewColors.Rows[i]);
            }
        }
    }
}
