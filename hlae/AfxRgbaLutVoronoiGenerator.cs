using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AfxGui
{
    public partial class AfxRgbaLutVoronoiGenerator : Form, IDisposable
    {
        public AfxRgbaLutVoronoiGenerator()
        {
            InitializeComponent();
            this.Icon = Program.Icon;

            this.Text = L10n._("Voronoi Color Lookup Table");
            this.tabMap.Text = L10n._("Color map");
            this.tabGenerate.Text = L10n._("Generate");
            this.buttonCheck.Text = L10n._("Check");
            this.buttonCopy.Text = L10n._("Copy all to clipboard");
            this.buttonPaste.Text = L10n._("Paste all from clipboard");
            this.colIndex.HeaderText = L10n._p("Number/Key","Index");
            this.colComment.HeaderText = L10n._("Comment");
            this.colDstWeight.HeaderText = L10n._("Weight");
            grpRes.Text = L10n._p("image", "Resolution");
            labelMemEstimate.Text = L10n._p("image", "Memory estimate (Bytes):");
            grpPreview.Text = L10n._("Preview");
            labelHori.Text = L10n._("Horizontal:");
            labelVert.Text = L10n._("Vertical:");
            buttonStart.Text = L10n._("Generate");
            buttonSave.Text = L10n._("Save");

            suspendPreview = true;

            m_Colors.Add(new HlaeCommentedColor(224, 175, 86, 0, L10n._p("terrorist", "T")), new HlaeWeightedColor(255, 0, 0, 0, 128f));
            m_Colors.Add(new HlaeCommentedColor(224, 175, 86, 255, L10n._p("terrorist", "T")), new HlaeWeightedColor(255, 0, 0, 255, 128f));
            m_Colors.Add(new HlaeCommentedColor(144, 155, 221, 0, L10n._p("counter-terrorist", "CT")), new HlaeWeightedColor(0, 0, 255, 0, 128f));
            m_Colors.Add(new HlaeCommentedColor(144, 155, 221, 255, L10n._p("counter-terrorist", "CT")), new HlaeWeightedColor(0, 0, 255, 255, 128f));
            m_Colors.Add(new HlaeCommentedColor(0.662745f, 0.647059f, 0.601961f, 0.0f, L10n._p("terrorist / counter-terrorist", "T/CT neutral")), new HlaeWeightedColor(0.656863f, 0.656863f, 0.656863f, 0));
            m_Colors.Add(new HlaeCommentedColor(0.662745f, 0.647059f, 0.601961f, 1.0f, L10n._p("terrorist / counter-terrorist", "T/CT neutral")), new HlaeWeightedColor(0.656863f, 0.656863f, 0.656863f, 1));

            m_Colors.Add(new HlaeCommentedColor(230, 128, 0, 0, L10n._p("terrorist", "T aim")), new HlaeWeightedColor(255, 0, 0, 0, 128f));
            m_Colors.Add(new HlaeCommentedColor(230, 128, 0, 255, L10n._p("terrorist", "T aim")), new HlaeWeightedColor(255, 0, 0, 255, 128f));
            m_Colors.Add(new HlaeCommentedColor(0, 120, 240, 0, L10n._p("counter-terrorist", "CT aim")), new HlaeWeightedColor(0, 0, 255, 0, 128f));
            m_Colors.Add(new HlaeCommentedColor(0, 120, 240, 255, L10n._p("counter-terrorist", "CT aim")), new HlaeWeightedColor(0, 0, 255, 255, 128f));
            m_Colors.Add(new HlaeCommentedColor(0.450980f, 0.486275f, 0.470588f, 0, L10n._p("terrorist / counter-terrorist", "T/CT aim neutral")), new HlaeWeightedColor(0.469281f, 0.469281f, 0.469281f, 0));
            m_Colors.Add(new HlaeCommentedColor(0.450980f, 0.486275f, 0.470588f, 1, L10n._p("terrorist / counter-terrorist", "T/CT aim neutral")), new HlaeWeightedColor(0.469281f, 0.469281f, 0.469281f, 1));
            
            m_Colors.Add(new HlaeCommentedColor(0, 0, 0, 0), new HlaeWeightedColor(0, 0, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 0, 0, 255), new HlaeWeightedColor(0, 0, 0, 255));
            m_Colors.Add(new HlaeCommentedColor(0, 0, 255, 0), new HlaeWeightedColor(0, 0, 255, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 0, 255, 255), new HlaeWeightedColor(0, 0, 255, 255));
            m_Colors.Add(new HlaeCommentedColor(0, 255, 0, 0), new HlaeWeightedColor(0, 255, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 255, 0, 255), new HlaeWeightedColor(0, 255, 0, 255));
            m_Colors.Add(new HlaeCommentedColor(0, 255, 255, 0), new HlaeWeightedColor(0, 255, 255, 0));
            m_Colors.Add(new HlaeCommentedColor(0, 255, 255, 255), new HlaeWeightedColor(0, 255, 255, 255));
            m_Colors.Add(new HlaeCommentedColor(255, 0, 0, 0), new HlaeWeightedColor(255, 0, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(255, 0, 0, 255), new HlaeWeightedColor(255, 0, 0, 255));
            m_Colors.Add(new HlaeCommentedColor(255, 0, 255, 0), new HlaeWeightedColor(255, 0, 255, 0));
            m_Colors.Add(new HlaeCommentedColor(255, 0, 255, 255), new HlaeWeightedColor(255, 0, 255, 255));
            m_Colors.Add(new HlaeCommentedColor(255, 255, 0, 0), new HlaeWeightedColor(255, 255, 0, 0));
            m_Colors.Add(new HlaeCommentedColor(255, 255, 0, 255), new HlaeWeightedColor(255, 255, 0, 255));
            m_Colors.Add(new HlaeCommentedColor(255, 255, 255, 0), new HlaeWeightedColor(255, 255, 255, 0));
            m_Colors.Add(new HlaeCommentedColor(255, 255, 255, 255), new HlaeWeightedColor(255, 255, 255, 255));

            int idx = 0;

            foreach (KeyValuePair<HlaeCommentedColor, HlaeWeightedColor> kvp in m_Colors)
            {
                StyleRow(dataGridViewColors.Rows[dataGridViewColors.Rows.Add(
                    idx,
                    ValueToByte(kvp.Key.Color.R),
                    ValueToByte(kvp.Key.Color.G),
                    ValueToByte(kvp.Key.Color.B),
                    ValueToByte(kvp.Key.Color.A),
                    kvp.Key.Comment,
                    ValueToByte(kvp.Value.Color.R),
                    ValueToByte(kvp.Value.Color.G),
                    ValueToByte(kvp.Value.Color.B),
                    ValueToByte(kvp.Value.Color.A),
                    kvp.Value.Weight.ToString()
                )]);

                ++idx;
            }

            drawMode.Items.AddRange(new string[]{
                L10n._p("Image", "Target"),
                L10n._p("Image", "Source"),
                L10n._p("Image", "RGB difference")
            });
            drawMode.SelectedIndex = 0;

            UpdateEstimate();

            suspendPreview = false;

            UpdatePreview();
        }

        private AfxCppCli.ColorLutTools colorLutTools;
        private Bitmap previewBitmap;
        private bool suspendPreview = false;

        private byte ValueToByte(float value)
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
                R = (byte)Math.Min(Math.Max(0, value.R * 255f), 255f);
                G = (byte)Math.Min(Math.Max(0, value.G * 255f), 255f);
                B = (byte)Math.Min(Math.Max(0, value.B * 255f), 255f);
                A = (byte)Math.Min(Math.Max(0, value.A * 255f), 255f);
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


            public HlaeColor(byte r, byte g, byte b, byte a)
            {
                this.R = r / 255.0f;
                this.G = g / 255.0f;
                this.B = b / 255.0f;
                this.A = a / 255.0f;
            }

            public override string ToString()
            {
                return new HlaeColorUc(this).ToString();
            }

            public static HlaeColor operator +(HlaeColor a, HlaeColor b)
            {
                return new HlaeColor(a.R + b.R, a.G + b.G, a.B + b.B, a.A + b.A);
            }


            public static HlaeColor operator *(HlaeColor a, float b)
            {
                return new HlaeColor(a.R * b, a.G * b, a.B * b, a.A * b);
            }

            public static HlaeColor operator /(HlaeColor a, float b)
            {
                return new HlaeColor(a.R / b, a.G / b, a.B / b, a.A / b);
            }

            public float CalcDistanceSquared(HlaeColor other)
            {
                float dR = other.R - R;
                float dG = other.G - G;
                float dB = other.B - B;
                float dA = other.A - A;

                return dR * dR + dG * dG + dB * dB + dA * dA;
            }
        }

        public struct HlaeCommentedColor
        {
            public HlaeColor Color { get; set; }
            public string Comment { get; set; }

            public HlaeCommentedColor(byte r, byte g, byte b, byte a, string comment = "")
            {
                Color = new HlaeColor(r, g, b, a);
                Comment = comment;
            }

            public HlaeCommentedColor(float r, float g, float b, float a, string comment = "")
            {
                Color = new HlaeColor(r, g, b, a);
                Comment = comment;
            }

            public override int GetHashCode()
            {
                return Color.GetHashCode();
            }

            public override bool Equals(object obj)
            {
                if (null == obj) return false;

                if (!(obj is HlaeCommentedColor)) return false;

                HlaeCommentedColor ccol = (HlaeCommentedColor)obj;

                return Color.Equals(ccol.Color);
            }
        }

        public struct HlaeWeightedColor
        {
            public HlaeColor Color { get; set; }
            public float Weight { get; set; }

            public HlaeWeightedColor(byte r, byte g, byte b, byte a, float weight = 1)
            {
                Color = new HlaeColor(r, g, b, a);
                Weight = weight;
            }

            public HlaeWeightedColor(float r, float g, float b, float a, float weight = 1)
            {
                Color = new HlaeColor(r, g, b, a);
                Weight = weight;
            }

            public override int GetHashCode()
            {
                return Color.GetHashCode();
            }

            public override bool Equals(object obj)
            {
                if (null == obj) return false;

                if (!(obj is HlaeCommentedColor)) return false;

                HlaeCommentedColor ccol = (HlaeCommentedColor)obj;

                return Color.Equals(ccol.Color);
            }
        }


        private Dictionary<HlaeCommentedColor, HlaeWeightedColor> m_Colors = new Dictionary<HlaeCommentedColor, HlaeWeightedColor>();

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

            bool bSR = null != row.Cells[1].Value ? byte.TryParse(row.Cells[1].Value.ToString(), out sR) : false;
            bool bSG = null != row.Cells[2].Value ? byte.TryParse(row.Cells[2].Value.ToString(), out sG) : false;
            bool bSB = null != row.Cells[3].Value ? byte.TryParse(row.Cells[3].Value.ToString(), out sB) : false;
            bool bSA = null != row.Cells[4].Value ? byte.TryParse(row.Cells[4].Value.ToString(), out sA) : false;
            bool bDR = null != row.Cells[6].Value ? byte.TryParse(row.Cells[6].Value.ToString(), out dR) : false;
            bool bDG = null != row.Cells[7].Value ? byte.TryParse(row.Cells[7].Value.ToString(), out dG) : false;
            bool bDB = null != row.Cells[8].Value ? byte.TryParse(row.Cells[8].Value.ToString(), out dB) : false;
            bool bDA = null != row.Cells[9].Value ? byte.TryParse(row.Cells[9].Value.ToString(), out dA) : false;

            System.Drawing.Color bcS = System.Drawing.Color.FromArgb(255, sR, sG, sB);
            System.Drawing.Color bcSf = ForeColorValue(bcS);
            System.Drawing.Color bcD = System.Drawing.Color.FromArgb(255, dR, dG, dB);
            System.Drawing.Color bcDf = ForeColorValue(bcD);

            row.Cells[1].Style.SelectionBackColor = row.Cells[1].Style.BackColor = bSR ? bcS : bcSf;
            row.Cells[1].Style.SelectionForeColor = row.Cells[1].Style.ForeColor = bSR ? bcSf : bcS;
            row.Cells[2].Style.SelectionBackColor = row.Cells[2].Style.BackColor = bSG ? bcS : bcSf;
            row.Cells[2].Style.SelectionForeColor = row.Cells[2].Style.ForeColor = bSG ? bcSf : bcS;
            row.Cells[3].Style.SelectionBackColor = row.Cells[3].Style.BackColor = bSB ? bcS : bcSf;
            row.Cells[3].Style.SelectionForeColor = row.Cells[3].Style.ForeColor = bSB ? bcSf : bcS;
            row.Cells[4].Style.SelectionBackColor = row.Cells[4].Style.BackColor = bSA ? System.Drawing.Color.FromArgb(255, sA, sA, sA) : System.Drawing.Color.FromArgb(255, 255, 0, 0);
            row.Cells[4].Style.SelectionForeColor = row.Cells[4].Style.ForeColor = ForeColorValue(row.Cells[4].Style.BackColor);

            row.Cells[6].Style.SelectionBackColor = row.Cells[6].Style.BackColor = bDR ? bcD : bcDf;
            row.Cells[6].Style.SelectionForeColor = row.Cells[6].Style.ForeColor = bDR ? bcDf : bcD;
            row.Cells[7].Style.SelectionBackColor = row.Cells[7].Style.BackColor = bDG ? bcD : bcDf;
            row.Cells[7].Style.SelectionForeColor = row.Cells[7].Style.ForeColor = bDG ? bcDf : bcD;
            row.Cells[8].Style.SelectionBackColor = row.Cells[8].Style.BackColor = bDB ? bcD : bcDf;
            row.Cells[8].Style.SelectionForeColor = row.Cells[8].Style.ForeColor = bDB ? bcDf : bcD;
            row.Cells[9].Style.SelectionBackColor = row.Cells[9].Style.BackColor = bDA ? System.Drawing.Color.FromArgb(255, dA, dA, dA) : System.Drawing.Color.FromArgb(255, 255, 0, 0);
            row.Cells[9].Style.SelectionForeColor = row.Cells[9].Style.ForeColor = ForeColorValue(row.Cells[9].Style.BackColor);
        }

        private void dataGridViewColors_CellValueChanged(object sender, DataGridViewCellEventArgs e)
        {
            if (0 <= e.RowIndex && e.RowIndex < (sender as DataGridView).Rows.Count)
            {
                dataGridViewColors.Rows[e.RowIndex].Cells[0].Value = e.RowIndex.ToString();
                StyleRow((sender as DataGridView).Rows[e.RowIndex]);
            }
        }

        private void dataGridViewColors_RowsRemoved(object sender, DataGridViewRowsRemovedEventArgs e)
        {
            for (int i = e.RowIndex; i < (sender as DataGridView).Rows.Count; ++i)
            {
                dataGridViewColors.Rows[i].Cells[0].Value = e.RowIndex.ToString();
            }
        }

        private void buttonCopy_Click(object sender, EventArgs e)
        {
            string value = "";

            for (int i = 0; i < dataGridViewColors.Rows.Count; ++i)
            {
                if (0 < i) value = value + System.Environment.NewLine;
                for (int j = 1; j < dataGridViewColors.Rows[i].Cells.Count; ++j)
                {
                    if (1 < j) value = value + "\t";
                    value = value + (null != dataGridViewColors.Rows[i].Cells[j].Value ? dataGridViewColors.Rows[i].Cells[j].Value.ToString() : "");
                }
            }

            System.Windows.Forms.Clipboard.SetText(value);
        }

        private void buttonPaste_Click(object sender, EventArgs e)
        {
            dataGridViewColors.Rows.Clear();

            string value = System.Windows.Forms.Clipboard.GetText();
            if (null == value) value = "";

            string[] rows = value.Split('\n');
            for (int i = 0; i < rows.Length; ++i)
            {
                string[] cells = rows[i].TrimEnd('\r').Split('\t');

                StyleRow(dataGridViewColors.Rows[dataGridViewColors.Rows.Add(
                    i,
                    1 <= cells.Length ? cells[0] : "",
                    2 <= cells.Length ? cells[1] : "",
                    3 <= cells.Length ? cells[2] : "",
                    4 <= cells.Length ? cells[3] : "",
                    5 <= cells.Length ? cells[4] : "",
                    6 <= cells.Length ? cells[5] : "",
                    7 <= cells.Length ? cells[6] : "",
                    8 <= cells.Length ? cells[7] : "",
                    9 <= cells.Length ? cells[8] : ""
                )]);
            }
        }

        private void RowColumnError(int row, string column)
        {
            MessageBox.Show(this,
                 L10n._("Invalid / missing value in row {0}, column {1}.", row, column),
                 L10n._("Error"),
                 MessageBoxButtons.OK,
                 MessageBoxIcon.Error);
        }

        private bool GetRowColor(DataGridViewRow row, out HlaeCommentedColor src, out HlaeWeightedColor dst)
        {
            bool bOk = true;
            string[] colNames = { "Index", "R1", "G1", "B1", "A1", "Comment", "R2", "G2", "B2", "A2" };
            int[] cols = { 1, 2, 3, 4, 6, 7, 8, 9 };
            byte[] vals = { 255, 255, 255, 255, 255, 255, 255, 255 };
            string comment;
            float weight;

            for (int i = 0; i < cols.Length && bOk; ++i)
            {
                bOk = bOk && null != row.Cells[cols[i]].Value;
                if (bOk) bOk = bOk && byte.TryParse(row.Cells[cols[i]].Value.ToString(), out vals[i]);
                if (!bOk) RowColumnError(row.Index, colNames[cols[i]]);
            }          

            comment = null == row.Cells[5].Value ? "" : row.Cells[5].Value.ToString();

            if (null == row.Cells[10].Value) weight = 1.0f;
            else if (!float.TryParse(row.Cells[10].Value.ToString(), out weight)) RowColumnError(row.Index, "Weight");

            src = new HlaeCommentedColor(vals[0] / 255.0f, vals[1] / 255.0f, vals[2] / 255.0f, vals[3] / 255.0f, comment);
            dst = new HlaeWeightedColor(vals[4] / 255.0f, vals[5] / 255.0f, vals[6] / 255.0f, vals[7] / 255.0f, weight);

            return bOk;
        }

        private bool GetColors()
        {
            m_Colors.Clear();

            for (int i = 0; i < dataGridViewColors.Rows.Count; ++i)
            {
                if (dataGridViewColors.Rows[i].IsNewRow) continue;

                HlaeCommentedColor src;
                HlaeWeightedColor dst;
                if (!GetRowColor(dataGridViewColors.Rows[i], out src, out dst)) return false;

                if (m_Colors.ContainsKey(src))
                {
                    int idx = 0;
                    foreach (KeyValuePair<HlaeCommentedColor, HlaeWeightedColor> kvp in m_Colors)
                    {
                        if (kvp.Key.Equals(src))
                        {
                            MessageBox.Show(this,
                                L10n._("Duplicate source color in row {0} (of row {1}).", i, idx),
                                L10n._("Error"),
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Error);
                        }
                        ++idx;
                    }

                    return false;
                }

                m_Colors.Add(src, dst);
            }

            if (m_Colors.Count <= 0)
            {
                MessageBox.Show(this,
                    L10n._("Color map must not be empty."),
                    L10n._("Error"),
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);

                return false;
            }

            return true;
        }

        private void buttonCheck_Click(object sender, EventArgs e)
        {
            if (GetColors())
            {
                MessageBox.Show(
                    L10n._("No Errors"),
                    L10n._("Check passed"),
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Information);
            }
        }

        private void UpdateEstimate()
        {
            decimal value = resR.Value * resG.Value * resB.Value * resA.Value * 4;

            memEstimate.Text = value.ToString();
        }

        private void ResValueChanged(object sender, EventArgs e)
        {
            UpdateEstimate();
        }

        private void UpdatePictureBoxBg(PictureBox pb, int size, Color col)
        {
            if (size == 0 && pb.BackgroundImage != null)
            {
                pb.BackgroundImage.Dispose();
                pb.BackgroundImage = null;
                return;
            }
            Bitmap bmp = new Bitmap(size * 2, size * 2);
            using (SolidBrush brush = new SolidBrush(col))
            using (Graphics G = Graphics.FromImage(bmp))
            {
                G.FillRectangle(brush, 0, 0, size, size);
                G.FillRectangle(brush, size, size, size, size);
            }
            pb.BackgroundImage = bmp;
            pb.BackgroundImageLayout = ImageLayout.Tile;
        }

        int lastTick = 0;

        private bool Iterate(float r, float g, float b, float a, out float outR, out float outG, out float outB, out float outA)
        {
            HlaeColor x = new HlaeColor(r, g, b, a);
            HlaeColor y = new HlaeColor(0, 0, 0, 0);
            float s = 0;

            for(int iR = 0; iR < (int)resR.Value; ++iR)
            {
                float fiR = iR / (float)(resR.Value - 1);
                for (int iG = 0; iG < (int)resG.Value; ++iG)
                {
                    float fiG = iG / (float)(resG.Value - 1);
                    for (int iB = 0; iB < (int)resB.Value; ++iB)
                    {
                        float fiB = iB / (float)(resB.Value - 1);
                        for (int iA = 0; iA < (int)resA.Value; ++iA)
                        {
                            float fiA = iA / (float)(resA.Value - 1);

                            HlaeColor xx = new HlaeColor(fiR, fiG, fiB, fiA);

                            bool hasBest = false;
                            KeyValuePair<HlaeCommentedColor, HlaeWeightedColor> best = m_Colors.First();
                            float bestDist = 0;
                            float dist;

                            foreach (KeyValuePair<HlaeCommentedColor, HlaeWeightedColor> kvp in m_Colors)
                            {
                                dist = xx.CalcDistanceSquared(kvp.Key.Color);

                                if (!hasBest || dist < bestDist)
                                {
                                    hasBest = true;
                                    bestDist = dist;
                                    best = kvp;
                                }
                            }

                            if (!hasBest)
                            {
                                outR = best.Value.Color.R;
                                outG = best.Value.Color.G;
                                outB = best.Value.Color.B;
                                outA = best.Value.Color.A;
                                return false;
                            }

                            dist = xx.CalcDistanceSquared(x);
                            if(dist < bestDist)
                            {
                                y = y + best.Value.Color * best.Value.Weight;
                                s = s + best.Value.Weight;
                            }
                        }
                    }
                }
            }

            if(0 != s) y = y / s;

            outR = y.R;
            outG = y.G;
            outB = y.B;
            outA = y.A;

            previewBitmap.SetPixel(Math.Max(Math.Min(previewBitmap.Width - 1, (int)(r * (float)(previewBitmap.Width - 1) + 0.5f)), 0), Math.Max(Math.Min(previewBitmap.Height - 1, (int)(g * (float)(previewBitmap.Height - 1) + 0.5f)), 0), new HlaeColorUc(y).ToColor());

            int curTick = System.Environment.TickCount;

            if (1000 <= Math.Abs(curTick - lastTick))
            {
                lastTick = curTick;

                using (Graphics gfx = Graphics.FromImage(preview.Image))
                {
                    gfx.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.Half;
                    gfx.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
                    gfx.DrawImage(previewBitmap, 0, 0, preview.Width, preview.Height);
                }
                trackZ.Value = Math.Max(Math.Min(trackZ.Maximum, (int)(b * (float)(trackZ.Maximum))), 0);
                trackW.Value = Math.Max(Math.Min(trackW.Maximum, (int)(a * (float)(trackW.Maximum))), 0);
                preview.Refresh();
            }

            return true;
        }

        private void buttonGenerateLut_Click(object sender, EventArgs e)
        {
            if (!GetColors()) return;

            UpdateAxis();

            suspendPreview = true;

            comboX.SelectedIndex = 0;
            comboY.SelectedIndex = 1;
            trackZ.Value = 0;
            trackW.Value = 0;
            drawMode.SelectedIndex = 0;

            if (preview.Image != null)
            {
                preview.Image.Dispose();
                preview.Image = null;
            }
            if (previewBitmap != null)
            {
                preview.Image = null;
                previewBitmap.Dispose();
            }
            previewBitmap = new Bitmap((int)resR.Value, (int)resG.Value);
            preview.Image = new Bitmap(preview.Width, preview.Height);

            if(null == colorLutTools) colorLutTools = new AfxCppCli.ColorLutTools();

            if(!colorLutTools.New(
                (uint)resR.Value,
                (uint)resG.Value,
                (uint)resB.Value,
                (uint)resA.Value
            ))
            {
                MessageBox.Show(this,
                    L10n._("Error when creating / re-sizing color map."),
                    L10n._("Error"),
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);

                colorLutTools.Dispose();
                colorLutTools = null;
            }

            if (null != colorLutTools && !colorLutTools.IteratePut(Iterate))
            {
                MessageBox.Show(this,
                    L10n._("Iteration aborted."),
                    L10n._("Error"),
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);

                colorLutTools.Dispose();
                colorLutTools = null;
            }

            trackZ.Value = 255;
            trackW.Value = 255;

            suspendPreview = false;

            UpdatePreview();

            if (null != colorLutTools && colorLutTools.IsValid()) SaveToFile();
        }

        private void AfxRgbaLutVoronoiGenerator_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (null != colorLutTools)
            {
                colorLutTools.Dispose();
                colorLutTools = null;
            }
        }

        private void SetPreviewEnabled(bool value)
        {
            comboX.Enabled = value;
            comboY.Enabled = value;
            trackW.Enabled = value;
            trackZ.Enabled = value;
            drawMode.Enabled = value;
            buttonSave.Enabled = value;
        }

        private int[] UpdateAxis()
        {
            string[] axis = { "R", "G", "B", "A" };
            int[] orgAxisVal = { 0, 1, 2, 3 };
            int[] axisVal = { 0, 1, 2, 3 };

            int j = 0;
            int i = 0;
            for (; i < axis.Length; ++i)
            {
                if (comboX.Text.Equals(axis[i]))
                {
                    axisVal[0] = orgAxisVal[i];
                }
                else if (comboY.Text.Equals(axis[i]))
                {
                    axisVal[1] = orgAxisVal[i];
                }
                else if (j == 0)
                {
                    labelZ.Text = axis[i];
                    axisVal[2] = orgAxisVal[i];
                    ++j;
                }
                else if (j == 1)
                {
                    labelW.Text = axis[i];
                    axisVal[3] = orgAxisVal[i];
                    ++j;
                }
            }

            return axisVal;
        }

        private void UpdatePreview()
        {
            if (suspendPreview || preview.Width <= 0 || preview.Height <= 0) return;

            int[] axisVal = UpdateAxis();

            bool hasLut = null != colorLutTools && colorLutTools.IsValid();

            UpdatePictureBoxBg(preview, 320 / 4, hasLut ? System.Drawing.Color.FromArgb(255, 128, 128, 0)  : System.Drawing.Color.FromArgb(255, 128, 0, 0));
            SetPreviewEnabled(hasLut);
            preview.Refresh();

            if(null != colorLutTools)
            {
                if (null == previewBitmap || previewBitmap.Width != preview.Width || previewBitmap.Height != preview.Height)
                {
                   if(null != previewBitmap) previewBitmap.Dispose();
                   previewBitmap = new Bitmap(preview.Width, preview.Height);
                }
                if (null == preview.Image || preview.Image != previewBitmap)
                {
                    if (null != preview.Image) preview.Image.Dispose();
                    preview.Image = previewBitmap;
                }

                for(int x = 0; x < preview.Width; ++x)
                {
                    for(int y = 0; y < preview.Height; ++y)
                    {
                        float[] vals = new float[]{ 0, 0, 0, 0 };

                        vals[axisVal[0]] = (float)x / (preview.Width - 1);
                        vals[axisVal[1]] = (float)y / (preview.Height - 1);
                        vals[axisVal[2]] = (float)trackZ.Value / trackZ.Maximum;
                        vals[axisVal[3]] = (float)trackW.Value / trackW.Maximum;

                        float newR = vals[0];
                        float newG = vals[1];
                        float newB = vals[2];
                        float newA = vals[3];

                        if (0 == drawMode.SelectedIndex || 2 == drawMode.SelectedIndex)
                        {
                            if (colorLutTools.Query(vals[0], vals[1], vals[2], vals[3], out newR, out newG, out newB, out newA))
                            {
                                if(2 == drawMode.SelectedIndex)
                                {
                                    newR = Math.Abs(newR - vals[0]);
                                    newG = Math.Abs(newG - vals[1]);
                                    newB = Math.Abs(newB - vals[2]);
                                }
                            }
                        }

                        previewBitmap.SetPixel(x, y, new HlaeColorUc(new HlaeColor(newR, newG, newB, newA)).ToColor());

                        int curTick = System.Environment.TickCount;

                        if (1000 <= Math.Abs(curTick - lastTick))
                        {
                            lastTick = curTick;
                            preview.Refresh();
                        }
                    }
                }

                UpdatePictureBoxBg(preview, 320 / 4, System.Drawing.Color.FromArgb(255, 128, 128, 128));
                preview.Refresh();
            }
        }

        private void comboX_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (comboY.SelectedIndex == comboX.SelectedIndex)
            {
                comboY.SelectedIndex = (comboY.SelectedIndex + 1) % comboY.Items.Count;
            }
            else UpdatePreview();
        }

        private void comboY_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (comboX.SelectedIndex == comboY.SelectedIndex)
            {
                comboX.SelectedIndex = (comboX.SelectedIndex + 1) % comboX.Items.Count;
            }
            else UpdatePreview();
        }

        private void preview_Resize(object sender, EventArgs e)
        {
            UpdatePreview();
        }

        private void AfxRgbaLutVoronoiGenerator_ResizeEnd(object sender, EventArgs e)
        {
            suspendPreview = false;
        }

        private void preview_SizeChanged(object sender, EventArgs e)
        {
            UpdatePreview();
        }

        private void AfxRgbaLutVoronoiGenerator_ResizeBegin(object sender, EventArgs e)
        {
            suspendPreview = true;
        }

        private void SaveToFile()
        {
            if (colorLutTools != null && colorLutTools.IsValid())
            {
                if (DialogResult.OK == saveFileDialog.ShowDialog(this))
                {
                    if (!colorLutTools.SaveToFile(saveFileDialog.FileName))
                    {
                        MessageBox.Show(this,
                            L10n._("Saving to \"{0}\" failed.", saveFileDialog.FileName),
                            L10n._("Error"),
                            MessageBoxButtons.OK,
                            MessageBoxIcon.Error);
                    }
                }
            }
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            SaveToFile();
        }

             private void track_ValueChanged(object sender, EventArgs e)
        {
            UpdatePreview();
        }

        private void track_MouseDown(object sender, MouseEventArgs e)
        {
            suspendPreview = true;
        }

        private void track_MouseUp(object sender, MouseEventArgs e)
        {
            suspendPreview = false;
            UpdatePreview();
        }

        private void drawMode_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdatePreview();
        }
    }
}
