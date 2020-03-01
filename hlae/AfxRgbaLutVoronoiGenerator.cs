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

            m_Colors.Add(new HlaeCommentedColor(0.878431f, 0.686275f, 0.337255f, 0, L10n._p("terrorist", "T")), new HlaeColor(0.902f, 0.525f, 0.196f, 0));
            m_Colors.Add(new HlaeCommentedColor(0.878431f, 0.686275f, 0.337255f, 1, L10n._p("terrorist", "T")), new HlaeColor(0.902f, 0.525f, 0.196f, 1));
            m_Colors.Add(new HlaeCommentedColor(0.447059f, 0.607843f, 0.866667f, 0, L10n._p("counter-terrorist", "CT")), new HlaeColor(0.384f, 0.388f, 0.741f, 0));
            m_Colors.Add(new HlaeCommentedColor(0.447059f, 0.607843f, 0.866667f, 1, L10n._p("counter-terrorist", "CT")), new HlaeColor(0.384f, 0.388f, 0.741f, 1));
            //m_Colors.Add(new HlaeCommentedColor(0.662745f, 0.647059f, 0.601961f, 1, "T/CT primary neutral"), new HlaeColor(0.637258f, 0.637258f, 0.637258f));
            m_Colors.Add(new HlaeCommentedColor(0.894f, 0.486f, 0f, 0, L10n._p("terrorist", "T aim")), new HlaeColor(0.902f, 0.525f, 0.196f, 0));
            m_Colors.Add(new HlaeCommentedColor(0.894f, 0.486f, 0f, 1, L10n._p("terrorist", "T aim")), new HlaeColor(0.902f, 0.525f, 0.196f, 1));
            m_Colors.Add(new HlaeCommentedColor(0f, 0.447f, 0.922f, 0, L10n._p("counter-terrorist", "CT aim")), new HlaeColor(0.384f, 0.388f, 0.741f, 0));
            m_Colors.Add(new HlaeCommentedColor(0f, 0.447f, 0.922f, 1, L10n._p("counter-terrorist", "CT aim")), new HlaeColor(0.384f, 0.388f, 0.741f, 1));
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

            int idx = 0;

            foreach (KeyValuePair<HlaeCommentedColor, HlaeColor> kvp in m_Colors)
            {
                StyleRow(dataGridViewColors.Rows[dataGridViewColors.Rows.Add(
                    idx,
                    VavlueToByte(kvp.Key.Color.R),
                    VavlueToByte(kvp.Key.Color.G),
                    VavlueToByte(kvp.Key.Color.B),
                    VavlueToByte(kvp.Key.Color.A),
                    kvp.Key.Comment,
                    VavlueToByte(kvp.Value.R),
                    VavlueToByte(kvp.Value.G),
                    VavlueToByte(kvp.Value.B),
                    VavlueToByte(kvp.Value.A)
                )]);

                ++idx;
            }

            UpdateEstimate();

            UpdatePreview();
        }

        private AfxCppCli.ColorLutTools colorLutTools;
        private Bitmap previewBitmap;
        private bool suspendPreview = false;

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
            row.Cells[9].Style.SelectionForeColor = row.Cells[9].Style.ForeColor = ForeColorValue(row.Cells[8].Style.BackColor);
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

        private bool GetRowColor(DataGridViewRow row, out HlaeCommentedColor src, out HlaeColor dst)
        {
            bool bOk = true;
            string[] colNames = { "Index", "R1", "G1", "B1", "A1", "Comment", "R2", "G2", "B2", "A2" };
            int[] cols = { 1, 2, 3, 4, 6, 7, 8, 9 };
            byte[] vals = { 255, 255, 255, 255, 255, 255, 255, 255 };
            string comment = "ERROR";

            for (int i = 0; i < cols.Length && bOk; ++i)
            {
                bOk = bOk && null != row.Cells[cols[i]].Value;
                if (bOk) bOk = bOk && byte.TryParse(row.Cells[cols[i]].Value.ToString(), out vals[i]);
                if (!bOk) RowColumnError(row.Index, colNames[cols[i]]);
            }

            if (bOk && null == row.Cells[5].Value)
            {
                bOk = false;
                RowColumnError(0, colNames[cols[5]]);
            }
            else comment = row.Cells[5].Value.ToString();

            src = new HlaeCommentedColor(vals[0], vals[1], vals[2], vals[3], comment);
            dst = new HlaeColor(vals[4], vals[5], vals[6], vals[7]);

            return bOk;
        }

        private bool GetColors()
        {
            m_Colors.Clear();

            for (int i = 0; i < dataGridViewColors.Rows.Count; ++i)
            {
                if (dataGridViewColors.Rows[i].IsNewRow) continue;

                HlaeCommentedColor src;
                HlaeColor dst;
                if (!GetRowColor(dataGridViewColors.Rows[i], out src, out dst)) return false;

                if (m_Colors.ContainsKey(src))
                {
                    int idx = 0;
                    foreach (KeyValuePair<HlaeCommentedColor, HlaeColor> kvp in m_Colors)
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

            return true;
        }

        private void buttonCheck_Click(object sender, EventArgs e)
        {
            if (GetColors())
            {
                MessageBox.Show(
                    L10n._("No Errors"),
                    L10n._("Check passsed"),
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

        private bool Iterate(float r, float g, float b, float a, out float outR, out float outG, out float outB, out float outA)
        {
            outR = r;
            outG = g;
            outB = b;
            outA = a;

            if(r == 0 && g == 0)
            {
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
            }

            previewBitmap.SetPixel(Math.Max(Math.Min(previewBitmap.Width -1,(int)(r * (float)(resR.Value - 1))),0), Math.Max(Math.Min(previewBitmap.Height - 1, (int)(g * (float)(resG.Value - 1))), 0), new HlaeColorUc(new HlaeColor(outR, outG, outB, outA)).ToColor());

            trackZ.Value = Math.Max(Math.Min(trackZ.Maximum, (int)(b * (float)(trackZ.Maximum))), 0);
            trackW.Value = Math.Max(Math.Min(trackW.Maximum, (int)(a * (float)(trackW.Maximum))), 0);

            if (b == 1 && a == 1)
            {
                using (Graphics gfx = Graphics.FromImage(preview.Image))
                {
                    gfx.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.Half;
                    gfx.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
                    gfx.DrawImage(previewBitmap, 0, 0, preview.Width, preview.Height);
                }
                preview.Refresh();                
            }

            return true;
        }

        private void buttonGenerateLut_Click(object sender, EventArgs e)
        {
            if (!GetColors()) return;

            if (null != colorLutTools) colorLutTools.Dispose();

            colorLutTools = null;

            comboX.SelectedIndex = 0;
            comboY.SelectedIndex = 1;
            trackZ.Value = 255;
            trackW.Value = 255;

            suspendPreview = true;

            colorLutTools = new AfxCppCli.ColorLutTools();

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
                return;
            }

            if (!colorLutTools.IteratePut(Iterate))
            {
                MessageBox.Show(this,
                    L10n._("User aborted."),
                    L10n._("Error"),
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);
            }

            suspendPreview = false;

            UpdatePreview();
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
        } 

        private void UpdatePreview()
        {
            if (suspendPreview || preview.Width <= 0 || preview.Height <= 0) return;

            string[] axis = { "R", "G", "B", "A" };
            int[] orgAxisVal = { 0, 1, 2, 3 };
            int[] axisVal = { 0, 1, 2, 3 };

            int j = 0;
            int i = 0;
            for(; i < axis.Length; ++i)
            {
                if (comboX.Text.Equals(axis[i]))
                {
                    axisVal[0] = orgAxisVal[i];
                }
                else if(comboY.Text.Equals(axis[i]))
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

            bool hasLut = null != colorLutTools && colorLutTools.IsValid();

            UpdatePictureBoxBg(preview, 320 / 4, hasLut ? System.Drawing.Color.FromArgb(255, 255, 255, 0)  : System.Drawing.Color.FromArgb(255, 255, 0, 0));
            SetPreviewEnabled(hasLut);
            preview.Refresh();

            if(null != colorLutTools)
            {
                if (null != previewBitmap) previewBitmap.Dispose();
                if (null != preview.Image) preview.Image.Dispose();

                preview.Image = previewBitmap = new Bitmap(preview.Width, preview.Height);

                for(int x = 0; x < preview.Width; ++x)
                {
                    for(int y = 0; y < preview.Height; ++y)
                    {
                        float[] vals = new float[]{ 0, 0, 0, 0 };

                        vals[axisVal[0]] = (float)x / (preview.Width - 1);
                        vals[axisVal[1]] = (float)y / (preview.Height - 1);
                        vals[axisVal[2]] = (float)trackZ.Value / (trackZ.Maximum - 1);
                        vals[axisVal[3]] = (float)trackW.Value / (trackW.Maximum - 1);

                        float newR, newG, newB, newA;

                        if(colorLutTools.Query(vals[0], vals[1], vals[2], vals[3], out newR, out newG, out newB, out newA))
                        {
                            previewBitmap.SetPixel(x, y, new HlaeColorUc(new HlaeColor(newR, newG, newB, newA)).ToColor());
                        }

                        if (((x * y) & 65535) == 65535)
                        {
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

        private void trackZ_ValueChanged(object sender, EventArgs e)
        {
            UpdatePreview();
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
    }
}
