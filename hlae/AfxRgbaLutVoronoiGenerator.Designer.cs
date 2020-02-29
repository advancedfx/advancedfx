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
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.label4 = new System.Windows.Forms.Label();
            this.numericUpDown3 = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.labelResolution = new System.Windows.Forms.Label();
            this.numR = new System.Windows.Forms.NumericUpDown();
            this.buttonGenerateLut = new System.Windows.Forms.Button();
            this.buttonCopy = new System.Windows.Forms.Button();
            this.buttonPaste = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numR)).BeginInit();
            this.SuspendLayout();
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
            this.dataGridViewColors.Location = new System.Drawing.Point(8, 41);
            this.dataGridViewColors.Name = "dataGridViewColors";
            this.dataGridViewColors.Size = new System.Drawing.Size(564, 188);
            this.dataGridViewColors.TabIndex = 7;
            this.dataGridViewColors.CellValueChanged += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridViewColors_CellValueChanged);
            this.dataGridViewColors.RowsRemoved += new System.Windows.Forms.DataGridViewRowsRemovedEventHandler(this.dataGridViewColors_RowsRemoved);
            // 
            // Index
            // 
            this.Index.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.DisplayedCellsExceptHeader;
            this.Index.HeaderText = "Index";
            this.Index.MinimumWidth = 40;
            this.Index.Name = "Index";
            this.Index.ReadOnly = true;
            this.Index.Width = 40;
            // 
            // SrcR
            // 
            this.SrcR.HeaderText = "R";
            this.SrcR.Name = "SrcR";
            this.SrcR.Width = 50;
            // 
            // SrcG
            // 
            this.SrcG.HeaderText = "G";
            this.SrcG.Name = "SrcG";
            this.SrcG.Width = 50;
            // 
            // SrcB
            // 
            this.SrcB.HeaderText = "B";
            this.SrcB.Name = "SrcB";
            this.SrcB.Width = 50;
            // 
            // SrcA
            // 
            this.SrcA.HeaderText = "A";
            this.SrcA.Name = "SrcA";
            this.SrcA.Width = 50;
            // 
            // TargetComment
            // 
            this.TargetComment.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.TargetComment.HeaderText = "Comment";
            this.TargetComment.MinimumWidth = 50;
            this.TargetComment.Name = "TargetComment";
            // 
            // DstR
            // 
            this.DstR.HeaderText = "R";
            this.DstR.Name = "DstR";
            this.DstR.Width = 50;
            // 
            // DstG
            // 
            this.DstG.HeaderText = "G";
            this.DstG.Name = "DstG";
            this.DstG.Width = 50;
            // 
            // DstB
            // 
            this.DstB.HeaderText = "B";
            this.DstB.Name = "DstB";
            this.DstB.Width = 50;
            // 
            // DstA
            // 
            this.DstA.HeaderText = "A";
            this.DstA.Name = "DstA";
            this.DstA.Width = 50;
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.progressBar1);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.numericUpDown3);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.numericUpDown2);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.numericUpDown1);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.labelResolution);
            this.groupBox1.Controls.Add(this.numR);
            this.groupBox1.Controls.Add(this.buttonGenerateLut);
            this.groupBox1.Location = new System.Drawing.Point(8, 235);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(564, 114);
            this.groupBox1.TabIndex = 9;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "groupBox1";
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(7, 83);
            this.progressBar1.Maximum = 16777216;
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(547, 23);
            this.progressBar1.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
            this.progressBar1.TabIndex = 19;
            // 
            // label4
            // 
            this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(478, 21);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(14, 13);
            this.label4.TabIndex = 18;
            this.label4.Text = "A";
            // 
            // numericUpDown3
            // 
            this.numericUpDown3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.numericUpDown3.Location = new System.Drawing.Point(499, 19);
            this.numericUpDown3.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.numericUpDown3.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDown3.Name = "numericUpDown3";
            this.numericUpDown3.Size = new System.Drawing.Size(60, 20);
            this.numericUpDown3.TabIndex = 17;
            this.numericUpDown3.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(391, 21);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(14, 13);
            this.label3.TabIndex = 16;
            this.label3.Text = "B";
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.numericUpDown2.Location = new System.Drawing.Point(412, 19);
            this.numericUpDown2.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.numericUpDown2.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(60, 20);
            this.numericUpDown2.TabIndex = 15;
            this.numericUpDown2.Value = new decimal(new int[] {
            32,
            0,
            0,
            0});
            // 
            // label2
            // 
            this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(304, 21);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(15, 13);
            this.label2.TabIndex = 14;
            this.label2.Text = "G";
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.numericUpDown1.Location = new System.Drawing.Point(325, 19);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.numericUpDown1.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(60, 20);
            this.numericUpDown1.TabIndex = 13;
            this.numericUpDown1.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(217, 21);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(15, 13);
            this.label1.TabIndex = 12;
            this.label1.Text = "R";
            // 
            // labelResolution
            // 
            this.labelResolution.AutoSize = true;
            this.labelResolution.Location = new System.Drawing.Point(7, 21);
            this.labelResolution.Name = "labelResolution";
            this.labelResolution.Size = new System.Drawing.Size(87, 13);
            this.labelResolution.TabIndex = 11;
            this.labelResolution.Text = "L10n Resolution:";
            // 
            // numR
            // 
            this.numR.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.numR.Location = new System.Drawing.Point(238, 19);
            this.numR.Maximum = new decimal(new int[] {
            32,
            0,
            0,
            0});
            this.numR.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numR.Name = "numR";
            this.numR.Size = new System.Drawing.Size(60, 20);
            this.numR.TabIndex = 10;
            this.numR.Value = new decimal(new int[] {
            8,
            0,
            0,
            0});
            // 
            // buttonGenerateLut
            // 
            this.buttonGenerateLut.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonGenerateLut.Location = new System.Drawing.Point(6, 45);
            this.buttonGenerateLut.Name = "buttonGenerateLut";
            this.buttonGenerateLut.Size = new System.Drawing.Size(553, 32);
            this.buttonGenerateLut.TabIndex = 9;
            this.buttonGenerateLut.Text = "L10n Start";
            this.buttonGenerateLut.UseVisualStyleBackColor = true;
            // 
            // buttonCopy
            // 
            this.buttonCopy.Location = new System.Drawing.Point(8, 12);
            this.buttonCopy.Name = "buttonCopy";
            this.buttonCopy.Size = new System.Drawing.Size(275, 23);
            this.buttonCopy.TabIndex = 10;
            this.buttonCopy.Text = "L10n Copy to clipboard";
            this.buttonCopy.UseVisualStyleBackColor = true;
            this.buttonCopy.Click += new System.EventHandler(this.buttonCopy_Click);
            // 
            // buttonPaste
            // 
            this.buttonPaste.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonPaste.Location = new System.Drawing.Point(297, 12);
            this.buttonPaste.Name = "buttonPaste";
            this.buttonPaste.Size = new System.Drawing.Size(275, 23);
            this.buttonPaste.TabIndex = 11;
            this.buttonPaste.Text = "L10n Paste form clipboard";
            this.buttonPaste.UseVisualStyleBackColor = true;
            this.buttonPaste.Click += new System.EventHandler(this.buttonPaste_Click);
            // 
            // AfxRgbaLutVoronoiGenerator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.buttonPaste);
            this.Controls.Add(this.buttonCopy);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.dataGridViewColors);
            this.MinimumSize = new System.Drawing.Size(600, 400);
            this.Name = "AfxRgbaLutVoronoiGenerator";
            this.Text = "L10n Natural Neighbour Interpolation HLAE Lookup Tree Generator";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown3)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numR)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.DataGridView dataGridViewColors;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown numericUpDown3;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label labelResolution;
        private System.Windows.Forms.NumericUpDown numR;
        private System.Windows.Forms.Button buttonGenerateLut;
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
        private System.Windows.Forms.Button buttonCopy;
        private System.Windows.Forms.Button buttonPaste;
    }
}