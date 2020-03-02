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
            this.tabMap = new System.Windows.Forms.TabPage();
            this.buttonCheck = new System.Windows.Forms.Button();
            this.buttonPaste = new System.Windows.Forms.Button();
            this.buttonCopy = new System.Windows.Forms.Button();
            this.dataGridViewColors = new System.Windows.Forms.DataGridView();
            this.colIndex = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcR = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcG = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcB = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcA = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colComment = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstR = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstG = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstB = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstA = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.colDstWeight = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.tabGenerate = new System.Windows.Forms.TabPage();
            this.buttonSave = new System.Windows.Forms.Button();
            this.buttonStart = new System.Windows.Forms.Button();
            this.preview = new System.Windows.Forms.PictureBox();
            this.grpRes = new System.Windows.Forms.GroupBox();
            this.memEstimate = new System.Windows.Forms.TextBox();
            this.labelMemEstimate = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.resA = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.resB = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.resR = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.resG = new System.Windows.Forms.NumericUpDown();
            this.grpPreview = new System.Windows.Forms.GroupBox();
            this.drawMode = new System.Windows.Forms.ComboBox();
            this.labelVert = new System.Windows.Forms.Label();
            this.comboY = new System.Windows.Forms.ComboBox();
            this.labelHori = new System.Windows.Forms.Label();
            this.comboX = new System.Windows.Forms.ComboBox();
            this.labelZ = new System.Windows.Forms.Label();
            this.labelW = new System.Windows.Forms.Label();
            this.trackZ = new System.Windows.Forms.TrackBar();
            this.trackW = new System.Windows.Forms.TrackBar();
            this.tabControl1.SuspendLayout();
            this.tabMap.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).BeginInit();
            this.tabGenerate.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.preview)).BeginInit();
            this.grpRes.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.resA)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resB)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resR)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.resG)).BeginInit();
            this.grpPreview.SuspendLayout();
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
            this.tabControl1.Controls.Add(this.tabMap);
            this.tabControl1.Controls.Add(this.tabGenerate);
            this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(584, 361);
            this.tabControl1.TabIndex = 0;
            // 
            // tabMap
            // 
            this.tabMap.Controls.Add(this.buttonCheck);
            this.tabMap.Controls.Add(this.buttonPaste);
            this.tabMap.Controls.Add(this.buttonCopy);
            this.tabMap.Controls.Add(this.dataGridViewColors);
            this.tabMap.Location = new System.Drawing.Point(4, 22);
            this.tabMap.Name = "tabMap";
            this.tabMap.Padding = new System.Windows.Forms.Padding(3);
            this.tabMap.Size = new System.Drawing.Size(576, 335);
            this.tabMap.TabIndex = 0;
            this.tabMap.Text = "L10n Color map";
            this.tabMap.UseVisualStyleBackColor = true;
            // 
            // buttonCheck
            // 
            this.buttonCheck.Location = new System.Drawing.Point(6, 6);
            this.buttonCheck.Name = "buttonCheck";
            this.buttonCheck.Size = new System.Drawing.Size(562, 31);
            this.buttonCheck.TabIndex = 0;
            this.buttonCheck.Text = "L10n Check";
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
            this.buttonPaste.Click += new System.EventHandler(this.buttonPaste_Click);
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
            this.buttonCopy.Click += new System.EventHandler(this.buttonCopy_Click);
            // 
            // dataGridViewColors
            // 
            this.dataGridViewColors.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewColors.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewColors.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colIndex,
            this.SrcR,
            this.SrcG,
            this.SrcB,
            this.SrcA,
            this.colComment,
            this.DstR,
            this.DstG,
            this.DstB,
            this.DstA,
            this.colDstWeight});
            this.dataGridViewColors.Location = new System.Drawing.Point(6, 43);
            this.dataGridViewColors.Name = "dataGridViewColors";
            this.dataGridViewColors.Size = new System.Drawing.Size(562, 239);
            this.dataGridViewColors.TabIndex = 1;
            this.dataGridViewColors.CellValueChanged += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridViewColors_CellValueChanged);
            this.dataGridViewColors.RowsRemoved += new System.Windows.Forms.DataGridViewRowsRemovedEventHandler(this.dataGridViewColors_RowsRemoved);
            // 
            // colIndex
            // 
            this.colIndex.HeaderText = "L10n Index";
            this.colIndex.MinimumWidth = 60;
            this.colIndex.Name = "colIndex";
            this.colIndex.ReadOnly = true;
            this.colIndex.Width = 60;
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
            // colComment
            // 
            this.colComment.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.colComment.HeaderText = "L10n Comment";
            this.colComment.MinimumWidth = 60;
            this.colComment.Name = "colComment";
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
            // colDstWeight
            // 
            this.colDstWeight.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.ColumnHeader;
            this.colDstWeight.HeaderText = "L10n Weight";
            this.colDstWeight.MinimumWidth = 60;
            this.colDstWeight.Name = "colDstWeight";
            this.colDstWeight.Width = 86;
            // 
            // tabGenerate
            // 
            this.tabGenerate.Controls.Add(this.buttonSave);
            this.tabGenerate.Controls.Add(this.buttonStart);
            this.tabGenerate.Controls.Add(this.preview);
            this.tabGenerate.Controls.Add(this.grpRes);
            this.tabGenerate.Controls.Add(this.grpPreview);
            this.tabGenerate.Location = new System.Drawing.Point(4, 22);
            this.tabGenerate.Name = "tabGenerate";
            this.tabGenerate.Padding = new System.Windows.Forms.Padding(3);
            this.tabGenerate.Size = new System.Drawing.Size(576, 335);
            this.tabGenerate.TabIndex = 1;
            this.tabGenerate.Text = "L10n Generation";
            this.tabGenerate.UseVisualStyleBackColor = true;
            // 
            // buttonSave
            // 
            this.buttonSave.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSave.Location = new System.Drawing.Point(474, 303);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(96, 24);
            this.buttonSave.TabIndex = 2;
            this.buttonSave.Text = "L10n Save";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // buttonStart
            // 
            this.buttonStart.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonStart.Location = new System.Drawing.Point(336, 303);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(132, 24);
            this.buttonStart.TabIndex = 1;
            this.buttonStart.Text = "L10n Generate";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonGenerateLut_Click);
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
            // grpRes
            // 
            this.grpRes.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.grpRes.Controls.Add(this.memEstimate);
            this.grpRes.Controls.Add(this.labelMemEstimate);
            this.grpRes.Controls.Add(this.label4);
            this.grpRes.Controls.Add(this.resA);
            this.grpRes.Controls.Add(this.label3);
            this.grpRes.Controls.Add(this.resB);
            this.grpRes.Controls.Add(this.label1);
            this.grpRes.Controls.Add(this.resR);
            this.grpRes.Controls.Add(this.label2);
            this.grpRes.Controls.Add(this.resG);
            this.grpRes.Location = new System.Drawing.Point(336, 7);
            this.grpRes.Name = "grpRes";
            this.grpRes.Size = new System.Drawing.Size(234, 117);
            this.grpRes.TabIndex = 0;
            this.grpRes.TabStop = false;
            this.grpRes.Text = "L10n Resolution";
            // 
            // memEstimate
            // 
            this.memEstimate.Location = new System.Drawing.Point(10, 89);
            this.memEstimate.Name = "memEstimate";
            this.memEstimate.ReadOnly = true;
            this.memEstimate.Size = new System.Drawing.Size(218, 20);
            this.memEstimate.TabIndex = 9;
            // 
            // labelMemEstimate
            // 
            this.labelMemEstimate.AutoSize = true;
            this.labelMemEstimate.Location = new System.Drawing.Point(7, 72);
            this.labelMemEstimate.Name = "labelMemEstimate";
            this.labelMemEstimate.Size = new System.Drawing.Size(151, 13);
            this.labelMemEstimate.TabIndex = 8;
            this.labelMemEstimate.Text = "L10n Memory estimate (Bytes):";
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
            64,
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
            3,
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
            64,
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
            7,
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
            64,
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
            7,
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
            64,
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
            7,
            0,
            0,
            0});
            this.resG.ValueChanged += new System.EventHandler(this.ResValueChanged);
            // 
            // grpPreview
            // 
            this.grpPreview.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.grpPreview.Controls.Add(this.drawMode);
            this.grpPreview.Controls.Add(this.labelVert);
            this.grpPreview.Controls.Add(this.comboY);
            this.grpPreview.Controls.Add(this.labelHori);
            this.grpPreview.Controls.Add(this.comboX);
            this.grpPreview.Controls.Add(this.labelZ);
            this.grpPreview.Controls.Add(this.labelW);
            this.grpPreview.Controls.Add(this.trackZ);
            this.grpPreview.Controls.Add(this.trackW);
            this.grpPreview.Location = new System.Drawing.Point(336, 130);
            this.grpPreview.Name = "grpPreview";
            this.grpPreview.Size = new System.Drawing.Size(234, 167);
            this.grpPreview.TabIndex = 2;
            this.grpPreview.TabStop = false;
            this.grpPreview.Text = "L10n Preview";
            // 
            // drawMode
            // 
            this.drawMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.drawMode.FormattingEnabled = true;
            this.drawMode.Location = new System.Drawing.Point(11, 135);
            this.drawMode.Name = "drawMode";
            this.drawMode.Size = new System.Drawing.Size(219, 21);
            this.drawMode.TabIndex = 8;
            this.drawMode.SelectedIndexChanged += new System.EventHandler(this.drawMode_SelectedIndexChanged);
            // 
            // labelVert
            // 
            this.labelVert.AutoSize = true;
            this.labelVert.Location = new System.Drawing.Point(6, 49);
            this.labelVert.Name = "labelVert";
            this.labelVert.Size = new System.Drawing.Size(72, 13);
            this.labelVert.TabIndex = 2;
            this.labelVert.Text = "L10n Vertical:";
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
            // labelHori
            // 
            this.labelHori.AutoSize = true;
            this.labelHori.Location = new System.Drawing.Point(6, 22);
            this.labelHori.Name = "labelHori";
            this.labelHori.Size = new System.Drawing.Size(84, 13);
            this.labelHori.TabIndex = 0;
            this.labelHori.Text = "L10n Horizontal:";
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
            this.trackZ.ValueChanged += new System.EventHandler(this.track_ValueChanged);
            this.trackZ.MouseDown += new System.Windows.Forms.MouseEventHandler(this.track_MouseDown);
            this.trackZ.MouseUp += new System.Windows.Forms.MouseEventHandler(this.track_MouseUp);
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
            this.trackW.ValueChanged += new System.EventHandler(this.track_ValueChanged);
            this.trackW.MouseDown += new System.Windows.Forms.MouseEventHandler(this.track_MouseDown);
            this.trackW.MouseUp += new System.Windows.Forms.MouseEventHandler(this.track_MouseUp);
            // 
            // AfxRgbaLutVoronoiGenerator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.tabControl1);
            this.MinimumSize = new System.Drawing.Size(600, 400);
            this.Name = "AfxRgbaLutVoronoiGenerator";
            this.Text = "L10n Voronoi Color Lookup Table";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.AfxRgbaLutVoronoiGenerator_FormClosed);
            this.ResizeBegin += new System.EventHandler(this.AfxRgbaLutVoronoiGenerator_ResizeBegin);
            this.ResizeEnd += new System.EventHandler(this.AfxRgbaLutVoronoiGenerator_ResizeEnd);
            this.tabControl1.ResumeLayout(false);
            this.tabMap.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).EndInit();
            this.tabGenerate.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.preview)).EndInit();
            this.grpRes.ResumeLayout(false);
            this.grpRes.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.resA)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resB)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resR)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.resG)).EndInit();
            this.grpPreview.ResumeLayout(false);
            this.grpPreview.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.trackZ)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackW)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabMap;
        private System.Windows.Forms.DataGridView dataGridViewColors;
        private System.Windows.Forms.TabPage tabGenerate;
        private System.Windows.Forms.PictureBox preview;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.GroupBox grpRes;
        private System.Windows.Forms.Label labelMemEstimate;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown resA;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown resB;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown resR;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown resG;
        private System.Windows.Forms.GroupBox grpPreview;
        private System.Windows.Forms.Label labelVert;
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
        private System.Windows.Forms.Label labelHori;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.ComboBox drawMode;
        private System.Windows.Forms.DataGridViewTextBoxColumn colIndex;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcR;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcG;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcB;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcA;
        private System.Windows.Forms.DataGridViewTextBoxColumn colComment;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstR;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstG;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstB;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstA;
        private System.Windows.Forms.DataGridViewTextBoxColumn colDstWeight;
    }
}