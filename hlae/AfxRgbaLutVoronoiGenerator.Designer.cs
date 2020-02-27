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
            this.colorDialog1 = new System.Windows.Forms.ColorDialog();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.buttonGenerateLut = new System.Windows.Forms.Button();
            this.buttonLoadTemplate = new System.Windows.Forms.Button();
            this.buttonSaveTemplate = new System.Windows.Forms.Button();
            this.buttonAdd = new System.Windows.Forms.Button();
            this.buttonDelete = new System.Windows.Forms.Button();
            this.dataGridViewColors = new System.Windows.Forms.DataGridView();
            this.SrcR = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcG = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcB = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SrcA = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.TargetComment = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstR = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstG = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstB = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DstA = new System.Windows.Forms.DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).BeginInit();
            this.SuspendLayout();
            // 
            // buttonGenerateLut
            // 
            this.buttonGenerateLut.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonGenerateLut.Location = new System.Drawing.Point(12, 304);
            this.buttonGenerateLut.Name = "buttonGenerateLut";
            this.buttonGenerateLut.Size = new System.Drawing.Size(560, 45);
            this.buttonGenerateLut.TabIndex = 1;
            this.buttonGenerateLut.Text = "L10n Generate HLAELUT";
            this.buttonGenerateLut.UseVisualStyleBackColor = true;
            // 
            // buttonLoadTemplate
            // 
            this.buttonLoadTemplate.Location = new System.Drawing.Point(12, 12);
            this.buttonLoadTemplate.Name = "buttonLoadTemplate";
            this.buttonLoadTemplate.Size = new System.Drawing.Size(251, 23);
            this.buttonLoadTemplate.TabIndex = 2;
            this.buttonLoadTemplate.Text = "L10n Load Template";
            this.buttonLoadTemplate.UseVisualStyleBackColor = true;
            // 
            // buttonSaveTemplate
            // 
            this.buttonSaveTemplate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSaveTemplate.Location = new System.Drawing.Point(321, 12);
            this.buttonSaveTemplate.Name = "buttonSaveTemplate";
            this.buttonSaveTemplate.Size = new System.Drawing.Size(251, 23);
            this.buttonSaveTemplate.TabIndex = 3;
            this.buttonSaveTemplate.Text = "L10n Save Template";
            this.buttonSaveTemplate.UseVisualStyleBackColor = true;
            // 
            // buttonAdd
            // 
            this.buttonAdd.Location = new System.Drawing.Point(13, 44);
            this.buttonAdd.Name = "buttonAdd";
            this.buttonAdd.Size = new System.Drawing.Size(117, 23);
            this.buttonAdd.TabIndex = 5;
            this.buttonAdd.Text = "L10n Add";
            this.buttonAdd.UseVisualStyleBackColor = true;
            // 
            // buttonDelete
            // 
            this.buttonDelete.Location = new System.Drawing.Point(146, 44);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(117, 23);
            this.buttonDelete.TabIndex = 6;
            this.buttonDelete.Text = "L10n Delete";
            this.buttonDelete.UseVisualStyleBackColor = true;
            // 
            // dataGridViewColors
            // 
            this.dataGridViewColors.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewColors.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewColors.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.SrcR,
            this.SrcG,
            this.SrcB,
            this.SrcA,
            this.TargetComment,
            this.DstR,
            this.DstG,
            this.DstB,
            this.DstA});
            this.dataGridViewColors.Location = new System.Drawing.Point(13, 73);
            this.dataGridViewColors.Name = "dataGridViewColors";
            this.dataGridViewColors.Size = new System.Drawing.Size(559, 224);
            this.dataGridViewColors.TabIndex = 7;
            this.dataGridViewColors.CellValueChanged += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridViewColors_CellValueChanged);
            this.dataGridViewColors.RowsAdded += new System.Windows.Forms.DataGridViewRowsAddedEventHandler(this.dataGridViewColors_RowsAdded);
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
            // AfxRgbaLutVoronoiGenerator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 361);
            this.Controls.Add(this.dataGridViewColors);
            this.Controls.Add(this.buttonDelete);
            this.Controls.Add(this.buttonAdd);
            this.Controls.Add(this.buttonSaveTemplate);
            this.Controls.Add(this.buttonLoadTemplate);
            this.Controls.Add(this.buttonGenerateLut);
            this.MinimumSize = new System.Drawing.Size(600, 400);
            this.Name = "AfxRgbaLutVoronoiGenerator";
            this.Text = "L10n Natural Neighbour Interpolation HLAE Lookup Tree Generator";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewColors)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ColorDialog colorDialog1;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.Button buttonGenerateLut;
        private System.Windows.Forms.Button buttonLoadTemplate;
        private System.Windows.Forms.Button buttonSaveTemplate;
        private System.Windows.Forms.Button buttonAdd;
        private System.Windows.Forms.Button buttonDelete;
        private System.Windows.Forms.DataGridView dataGridViewColors;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcR;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcG;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcB;
        private System.Windows.Forms.DataGridViewTextBoxColumn SrcA;
        private System.Windows.Forms.DataGridViewTextBoxColumn TargetComment;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstR;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstG;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstB;
        private System.Windows.Forms.DataGridViewTextBoxColumn DstA;
    }
}