namespace AfxGui.Tools
{
    partial class Calculator
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
            this.components = new System.ComponentModel.Container();
            this.errorProvider = new System.Windows.Forms.ErrorProvider(this.components);
            this.labelWidth = new System.Windows.Forms.Label();
            this.labelHeight = new System.Windows.Forms.Label();
            this.textWidth = new System.Windows.Forms.TextBox();
            this.textHeight = new System.Windows.Forms.TextBox();
            this.textFps = new System.Windows.Forms.TextBox();
            this.textMin = new System.Windows.Forms.TextBox();
            this.labelFPS = new System.Windows.Forms.Label();
            this.labelDuration = new System.Windows.Forms.Label();
            this.labelMin = new System.Windows.Forms.Label();
            this.textSec = new System.Windows.Forms.TextBox();
            this.labelSec = new System.Windows.Forms.Label();
            this.groupBoxEstimate = new System.Windows.Forms.GroupBox();
            this.checkHuffYuv = new System.Windows.Forms.CheckBox();
            this.textSize = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).BeginInit();
            this.groupBoxEstimate.SuspendLayout();
            this.SuspendLayout();
            // 
            // errorProvider
            // 
            this.errorProvider.ContainerControl = this;
            // 
            // labelWidth
            // 
            this.labelWidth.AutoSize = true;
            this.labelWidth.Location = new System.Drawing.Point(12, 9);
            this.labelWidth.Name = "labelWidth";
            this.labelWidth.Size = new System.Drawing.Size(65, 13);
            this.labelWidth.TabIndex = 0;
            this.labelWidth.Text = "L10n Width:";
            // 
            // labelHeight
            // 
            this.labelHeight.AutoSize = true;
            this.labelHeight.Location = new System.Drawing.Point(12, 35);
            this.labelHeight.Name = "labelHeight";
            this.labelHeight.Size = new System.Drawing.Size(68, 13);
            this.labelHeight.TabIndex = 2;
            this.labelHeight.Text = "L10n Height:";
            // 
            // textWidth
            // 
            this.textWidth.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textWidth.Location = new System.Drawing.Point(111, 6);
            this.textWidth.Name = "textWidth";
            this.textWidth.Size = new System.Drawing.Size(169, 20);
            this.textWidth.TabIndex = 1;
            // 
            // textHeight
            // 
            this.textHeight.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textHeight.Location = new System.Drawing.Point(111, 32);
            this.textHeight.Name = "textHeight";
            this.textHeight.Size = new System.Drawing.Size(169, 20);
            this.textHeight.TabIndex = 3;
            // 
            // textFps
            // 
            this.textFps.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textFps.Location = new System.Drawing.Point(111, 58);
            this.textFps.Name = "textFps";
            this.textFps.Size = new System.Drawing.Size(169, 20);
            this.textFps.TabIndex = 5;
            // 
            // textMin
            // 
            this.textMin.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textMin.Location = new System.Drawing.Point(111, 84);
            this.textMin.Name = "textMin";
            this.textMin.Size = new System.Drawing.Size(60, 20);
            this.textMin.TabIndex = 8;
            // 
            // labelFPS
            // 
            this.labelFPS.AutoSize = true;
            this.labelFPS.Location = new System.Drawing.Point(12, 61);
            this.labelFPS.Name = "labelFPS";
            this.labelFPS.Size = new System.Drawing.Size(57, 13);
            this.labelFPS.TabIndex = 4;
            this.labelFPS.Text = "L10n FPS:";
            // 
            // labelDuration
            // 
            this.labelDuration.AutoSize = true;
            this.labelDuration.Location = new System.Drawing.Point(12, 87);
            this.labelDuration.Name = "labelDuration";
            this.labelDuration.Size = new System.Drawing.Size(77, 13);
            this.labelDuration.TabIndex = 6;
            this.labelDuration.Text = "L10n Duration:";
            // 
            // labelMin
            // 
            this.labelMin.AutoSize = true;
            this.labelMin.Location = new System.Drawing.Point(177, 87);
            this.labelMin.Name = "labelMin";
            this.labelMin.Size = new System.Drawing.Size(50, 13);
            this.labelMin.TabIndex = 7;
            this.labelMin.Text = "L10n min";
            // 
            // textSec
            // 
            this.textSec.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textSec.Location = new System.Drawing.Point(220, 84);
            this.textSec.Name = "textSec";
            this.textSec.Size = new System.Drawing.Size(60, 20);
            this.textSec.TabIndex = 10;
            // 
            // labelSec
            // 
            this.labelSec.AutoSize = true;
            this.labelSec.Location = new System.Drawing.Point(286, 87);
            this.labelSec.Name = "labelSec";
            this.labelSec.Size = new System.Drawing.Size(51, 13);
            this.labelSec.TabIndex = 9;
            this.labelSec.Text = "L10n sec";
            // 
            // groupBoxEstimate
            // 
            this.groupBoxEstimate.Controls.Add(this.checkHuffYuv);
            this.groupBoxEstimate.Controls.Add(this.textSize);
            this.groupBoxEstimate.Location = new System.Drawing.Point(12, 110);
            this.groupBoxEstimate.Name = "groupBoxEstimate";
            this.groupBoxEstimate.Size = new System.Drawing.Size(330, 80);
            this.groupBoxEstimate.TabIndex = 11;
            this.groupBoxEstimate.TabStop = false;
            this.groupBoxEstimate.Text = "L10n Estimated Disk Usage";
            // 
            // checkHuffYuv
            // 
            this.checkHuffYuv.AutoSize = true;
            this.checkHuffYuv.Location = new System.Drawing.Point(6, 51);
            this.checkHuffYuv.Name = "checkHuffYuv";
            this.checkHuffYuv.Size = new System.Drawing.Size(170, 17);
            this.checkHuffYuv.TabIndex = 1;
            this.checkHuffYuv.Text = "L10n HuffYuv encoded output";
            this.checkHuffYuv.UseVisualStyleBackColor = true;
            this.checkHuffYuv.CheckedChanged += new System.EventHandler(this.text_TextChanged);
            // 
            // textSize
            // 
            this.textSize.BackColor = System.Drawing.SystemColors.Info;
            this.textSize.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textSize.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.textSize.Location = new System.Drawing.Point(6, 19);
            this.textSize.Name = "textSize";
            this.textSize.ReadOnly = true;
            this.textSize.Size = new System.Drawing.Size(315, 26);
            this.textSize.TabIndex = 0;
            // 
            // Calculator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(354, 203);
            this.Controls.Add(this.groupBoxEstimate);
            this.Controls.Add(this.labelSec);
            this.Controls.Add(this.textSec);
            this.Controls.Add(this.labelMin);
            this.Controls.Add(this.labelDuration);
            this.Controls.Add(this.labelFPS);
            this.Controls.Add(this.textMin);
            this.Controls.Add(this.textFps);
            this.Controls.Add(this.textHeight);
            this.Controls.Add(this.textWidth);
            this.Controls.Add(this.labelHeight);
            this.Controls.Add(this.labelWidth);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "Calculator";
            this.Text = "L10n File Size Calculator";
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).EndInit();
            this.groupBoxEstimate.ResumeLayout(false);
            this.groupBoxEstimate.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ErrorProvider errorProvider;
        private System.Windows.Forms.GroupBox groupBoxEstimate;
        private System.Windows.Forms.CheckBox checkHuffYuv;
        private System.Windows.Forms.TextBox textSize;
        private System.Windows.Forms.Label labelSec;
        private System.Windows.Forms.TextBox textSec;
        private System.Windows.Forms.Label labelMin;
        private System.Windows.Forms.Label labelDuration;
        private System.Windows.Forms.Label labelFPS;
        private System.Windows.Forms.TextBox textMin;
        private System.Windows.Forms.TextBox textFps;
        private System.Windows.Forms.TextBox textHeight;
        private System.Windows.Forms.TextBox textWidth;
        private System.Windows.Forms.Label labelHeight;
        private System.Windows.Forms.Label labelWidth;
    }
}