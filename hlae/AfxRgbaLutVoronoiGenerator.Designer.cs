namespace AfxGui
{
    partial class AfxRgbaLutVoronoiGenerator
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.buttonCheck = new System.Windows.Forms.Button();
            this.buttonPaste = new System.Windows.Forms.Button();
            this.buttonCopy = new System.Windows.Forms.Button();
            this.dataGridViewColors = new System.Windows.Forms.DataGridView();
            this.Index = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcR = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcG = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcB = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcA = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.TargetComment = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstR = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstG = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstB = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstA = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.buttonGenerateLut = new System.Windows.Forms.Button();
            this.preview = new System.Windows.Forms.PictureBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.memEstimate = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.resA = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.resB = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.resR = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.resG = new System.Windows.Forms.NumericUpDown();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label6 = new System.Windows.Forms.Label();
            this.comboY = new System.Windows.Forms.ComboBox();
            this.label5 = new System.Windows.Forms.Label();
            this.comboX = new System.Windows.Forms.ComboBox();
            this.labelZ = new System.Windows.Forms.Label();
            this.labelW = new System.Windows.Forms.Label();
            this.trackZ = new System.Windows.Forms.TrackBar();
            this.trackW = new System.Windows.Forms.TrackBar();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).BeginInit();
            this.tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.preview)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.resA)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resB)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resR)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resG)).BeginInit();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackZ)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackW)).BeginInit();
            this.SuspendLayout();
            // 
            // saveFileDialog
            // 
            this.saveFileDialog.DefaultExt = "afxlut";
            this.saveFileDialog.Filter = "AfxRgbaLut files|*.afxlut";
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(584, 361);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.buttonCheck);
            this.tabPage1.Controls.Add(this.buttonPaste);
            this.tabPage1.Controls.Add(this.buttonCopy);
            this.tabPage1.Controls.Add(this.dataGridViewColors);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(576, 335);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "L10n Color map";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // buttonCheck
            // 
            this.buttonCheck.Location = new System.Drawing.Point(6, 6);
            this.buttonCheck.Name = "buttonCheck";
            this.buttonCheck.Size = new System.Drawing.Size(562, 31);
            this.buttonCheck.TabIndex = 0;
            this.buttonCheck.Text = "L10n Check for errors";
            this.buttonCheck.UseVisualStyleBackColor = true;
            this.buttonCheck.Click += new System.EventHandler(this.buttonCheck_Click);
            // 
            // buttonPaste
            // 
            this.buttonPaste.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonPaste.Location = new System.Drawing.Point(293, 306);
            this.buttonPaste.Name = "buttonPaste";
            this.buttonPaste.Size = new System.Drawing.Size(275, 23);
            this.buttonPaste.TabIndex = 3;
            this.buttonPaste.Text = "L10n Paste all form clipboard";
            this.buttonPaste.UseVisualStyleBackColor = true;
            // 
            // buttonCopy
            // 
            this.buttonCopy.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonCopy.Location = new System.Drawing.Point(8, 306);
            this.buttonCopy.Name = "buttonCopy";
            this.buttonCopy.Size = new System.Drawing.Size(275, 23);
            this.buttonCopy.TabIndex = 2;
            this.buttonCopy.Text = "L10n Copy all to clipboard";
            this.buttonCopy.UseVisualStyleBackColor = true;
            // 
            // dataGridViewColors
            // 
            this.dataGridViewColors.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewColors.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewColors.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Index,
            this.SrcR,
            this.SrcG,
            this.SrcB,
            this.SrcA,
            this.TargetComment,
            this.DstR,
            this.DstG,
            this.DstB,
            this.DstA});
            this.dataGridViewColors.Location = new System.Drawing.Point(6, 43);
            this.dataGridViewColors.Name = "dataGridViewColors";
            this.dataGridViewColors.Size = new System.Drawing.Size(562, 239);
            this.dataGridViewColors.TabIndex = 1;
            this.dataGridViewColors.CellValueChanged += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridViewColors_CellValueChanged);
            this.dataGridViewColors.RowsRemoved += new System.Windows.Forms.DataGridViewRowsRemovedEventHandler(this.dataGridViewColors_RowsRemoved);
            // 
            // Index
            // 
            this.Index.HeaderText = "L10n Index";
            this.Index.MinimumWidth = 60;
            this.Index.Name = "Index";
            this.Index.ReadOnly = true;
            this.Index.Width = 60;
            // 
            // SrcR
            // 
            this.SrcR.HeaderText = "R";
            this.SrcR.Name = "SrcR";
            this.SrcR.Width = 40;
            // 
            // SrcG
            // 
            this.SrcG.HeaderText = "G";
            this.SrcG.Name = "SrcG";
            this.SrcG.Width = 40;
            // 
            // SrcB
            // 
            this.SrcB.HeaderText = "B";
            this.SrcB.Name = "SrcB";
            this.SrcB.Width = 40;
            // 
            // SrcA
            // 
            this.SrcA.HeaderText = "A";
            this.SrcA.Name = "SrcA";
            this.SrcA.Width = 40;
            // 
            // TargetComment
            // 
            this.TargetComment.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.TargetComment.HeaderText = "Comment";
            this.TargetComment.MinimumWidth = 60;
            this.TargetComment.Name = "TargetComment";
            // 
            // DstR
            // 
            this.DstR.HeaderText = "R";
            this.DstR.Name = "DstR";
            this.DstR.Width = 40;
            // 
            // DstG
            // 
            this.DstG.HeaderText = "G";
            this.DstG.Name = "DstG";
            this.DstG.Width = 40;
            // 
            // DstB
            // 
            this.DstB.HeaderText = "B";
            this.DstB.Name = "DstB";
            this.DstB.Width = 40;
            // 
            // DstA
            // 
            this.DstA.HeaderText = "A";
            this.DstA.Name = "DstA";
            this.DstA.Width = 40;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.buttonGenerateLut);
            this.tabPage2.Controls.Add(this.preview);
            this.tabPage2.Controls.Add(this.groupBox1);
            this.tabPage2.Controls.Add(this.groupBox2);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(576, 335);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "L10n Generation";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // buttonGenerateLut
            // 
            this.buttonGenerateLut.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonGenerateLut.Location = new System.Drawing.Point(336, 274);
            this.buttonGenerateLut.Name = "buttonGenerateLut";
            this.buttonGenerateLut.Size = new System.Drawing.Size(234, 53);
            this.buttonGenerateLut.TabIndex = 1;
            this.buttonGenerateLut.Text = "L10n Start";
            this.buttonGenerateLut.UseVisualStyleBackColor = true;
            this.buttonGenerateLut.Click += new System.EventHandler(this.buttonGenerateLut_Click);
            // 
            // preview
            // 
            this.preview.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.preview.Location = new System.Drawing.Point(9, 7);
            this.preview.Name = "preview";
            this.preview.Size = new System.Drawing.Size(320, 320);
            this.preview.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.preview.TabIndex = 15;
            this.preview.TabStop = false;
            this.preview.SizeChanged += new System.EventHandler(this.preview_SizeChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.memEstimate);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.resA);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.resB);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.resR);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.resG);
            this.groupBox1.Location = new System.Drawing.Point(336, 7);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(234, 117);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "L10n Resolution";
            // 
            // memEstimate
            // 
            this.memEstimate.Location = new System.Drawing.Point(10, 89);
            this.memEstimate.Name = "memEstimate";
            this.memEstimate.ReadOnly = true;
            this.memEstimate.Size = new System.Drawing.Size(218, 20);
            this.memEstimate.TabIndex = 9;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(7, 72);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(151, 13);
            this.label7.TabIndex = 8;
            this.label7.Text = "L10n Memory estimate (Bytes):";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(93, 45);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(14, 13);
            this.label4.TabIndex = 6;
            this.label4.Text = "A";
            // 
            // resA
            // 
            this.resA.Location = new System.Drawing.Point(114, 43);
            this.resA.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.resA.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.resA.Name = "resA";
            this.resA.Size = new System.Drawing.Size(60, 20);
            this.resA.TabIndex = 7;
            this.resA.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.resA.ValueChanged += new System.EventHandler(this.ResValueChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 45);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(14, 13);
            this.label3.TabIndex = 4;
            this.label3.Text = "B";
            // 
            // resB
            // 
            this.resB.Location = new System.Drawing.Point(27, 43);
            this.resB.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.resB.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.resB.Name = "resB";
            this.resB.Size = new System.Drawing.Size(60, 20);
            this.resB.TabIndex = 5;
            this.resB.Value = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.resB.ValueChanged += new System.EventHandler(this.ResValueChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(15, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "R";
            // 
            // resR
            // 
            this.resR.Location = new System.Drawing.Point(27, 17);
            this.resR.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.resR.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.resR.Name = "resR";
            this.resR.Size = new System.Drawing.Size(60, 20);
            this.resR.TabIndex = 1;
            this.resR.Value = new decimal(new int[] {
            8,
            0,
            0,
            0});
            this.resR.ValueChanged += new System.EventHandler(this.ResValueChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(93, 19);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(15, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "G";
            // 
            // resG
            // 
            this.resG.Location = new System.Drawing.Point(114, 17);
            this.resG.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.resG.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.resG.Name = "resG";
            this.resG.Size = new System.Drawing.Size(60, 20);
            this.resG.TabIndex = 3;
            this.resG.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            this.resG.ValueChanged += new System.EventHandler(this.ResValueChanged);
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.comboY);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.comboX);
            this.groupBox2.Controls.Add(this.labelZ);
            this.groupBox2.Controls.Add(this.labelW);
            this.groupBox2.Controls.Add(this.trackZ);
            this.groupBox2.Controls.Add(this.trackW);
            this.groupBox2.Location = new System.Drawing.Point(336, 130);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(234, 138);
            this.groupBox2.TabIndex = 2;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "L10n Preview";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(6, 49);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(72, 13);
            this.label6.TabIndex = 2;
            this.label6.Text = "L10n Vertical:";
            // 
            // comboY
            // 
            this.comboY.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboY.FormattingEnabled = true;
            this.comboY.Items.AddRange(new object[] {
            "R",
            "G",
            "B",
            "A"});
            this.comboY.Location = new System.Drawing.Point(160, 46);
            this.comboY.Name = "comboY";
            this.comboY.Size = new System.Drawing.Size(68, 21);
            this.comboY.TabIndex = 3;
            this.comboY.SelectedIndexChanged += new System.EventHandler(this.comboY_SelectedIndexChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(6, 22);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(84, 13);
            this.label5.TabIndex = 0;
            this.label5.Text = "L10n Horizontal:";
            // 
            // comboX
            // 
            this.comboX.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboX.FormattingEnabled = true;
            this.comboX.Items.AddRange(new object[] {
            "R",
            "G",
            "B",
            "A"});
            this.comboX.Location = new System.Drawing.Point(160, 19);
            this.comboX.Name = "comboX";
            this.comboX.Size = new System.Drawing.Size(68, 21);
            this.comboX.TabIndex = 1;
            this.comboX.SelectedIndexChanged += new System.EventHandler(this.comboX_SelectedIndexChanged);
            // 
            // labelZ
            // 
            this.labelZ.AutoSize = true;
            this.labelZ.Location = new System.Drawing.Point(8, 73);
            this.labelZ.Name = "labelZ";
            this.labelZ.Size = new System.Drawing.Size(14, 13);
            this.labelZ.TabIndex = 4;
            this.labelZ.Text = "Z";
            // 
            // labelW
            // 
            this.labelW.AutoSize = true;
            this.labelW.Location = new System.Drawing.Point(6, 105);
            this.labelW.Name = "labelW";
            this.labelW.Size = new System.Drawing.Size(18, 13);
            this.labelW.TabIndex = 6;
            this.labelW.Text = "W";
            // 
            // trackZ
            // 
            this.trackZ.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.trackZ.AutoSize = false;
            this.trackZ.LargeChange = 16;
            this.trackZ.Location = new System.Drawing.Point(27, 73);
            this.trackZ.Maximum = 255;
            this.trackZ.Name = "trackZ";
            this.trackZ.Size = new System.Drawing.Size(203, 24);
            this.trackZ.TabIndex = 5;
            this.trackZ.TickFrequency = 16;
            this.trackZ.TickStyle = System.Windows.Forms.TickStyle.TopLeft;
            this.trackZ.ValueChanged += new System.EventHandler(this.trackZ_ValueChanged);
            // 
            // trackW
            // 
            this.trackW.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.trackW.AutoSize = false;
            this.trackW.LargeChange = 16;
            this.trackW.Location = new System.Drawing.Point(27, 105);
            this.trackW.Maximum = 255;
            this.trackW.Name = "trackW";
            this.trackW.Size = new System.Drawing.Size(203, 24);
            this.trackW.TabIndex = 7;
            this.trackW.TickFrequency = 16;
            this.trackW.TickStyle = System.Windows.Forms.TickStyle.TopLeft;
            this.trackW.ValueChanged += new System.EventHandler(this.trackZ_ValueChanged);
            // 
            // AfxRgbaLutVoronoiGenerator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.tabControl1);
            this.MinimumSize = new System.Drawing.Size(600, 400);
            this.Name = "AfxRgbaLutVoronoiGenerator";
            this.Text = "L10n Natural Neighbour Interpolation - AfxRgbaLut";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.AfxRgbaLutVoronoiGenerator_FormClosed);
            this.ResizeBegin += new System.EventHandler(this.AfxRgbaLutVoronoiGenerator_ResizeBegin);
            this.ResizeEnd += new System.EventHandler(this.AfxRgbaLutVoronoiGenerator_ResizeEnd);
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).EndInit();
            this.tabPage2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.preview)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.resA)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resB)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resR)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resG)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackW)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.DataGridView dataGridViewColors;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.DataGridViewTextBoxColumn Index;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcR;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcG;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcB;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcA;
        private System.Windows.Forms.DataGridViewTextBoxColumn TargetComment;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstR;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstG;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstB;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstA;
        private System.Windows.Forms.PictureBox preview;
        private System.Windows.Forms.Button buttonGenerateLut;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown resA;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown resB;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown resR;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown resG;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.ComboBox comboY;
        private System.Windows.Forms.ComboBox comboX;
        private System.Windows.Forms.Label labelZ;
        private System.Windows.Forms.Label labelW;
        private System.Windows.Forms.TrackBar trackZ;
        private System.Windows.Forms.TrackBar trackW;
        private System.Windows.Forms.TextBox memEstimate;
        private System.Windows.Forms.Button buttonCheck;
        private System.Windows.Forms.Button buttonPaste;
        private System.Windows.Forms.Button buttonCopy;
        private System.Windows.Forms.Label label5;
    }
}