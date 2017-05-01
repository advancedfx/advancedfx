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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.textWidth = new System.Windows.Forms.TextBox();
            this.textHeight = new System.Windows.Forms.TextBox();
            this.textFps = new System.Windows.Forms.TextBox();
            this.textMin = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.textSec = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.checkHuffYuv = new System.Windows.Forms.CheckBox();
            this.textSize = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // errorProvider
            // 
            this.errorProvider.ContainerControl = this;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(38, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Width:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 35);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(41, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Height:";
            // 
            // textWidth
            // 
            this.textWidth.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textWidth.Location = new System.Drawing.Point(111, 6);
            this.textWidth.Name = "textWidth";
            this.textWidth.Size = new System.Drawing.Size(169, 20);
            this.textWidth.TabIndex = 2;
            this.textWidth.Text = "960";
            this.textWidth.TextChanged += new System.EventHandler(this.text_TextChanged);
            // 
            // textHeight
            // 
            this.textHeight.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textHeight.Location = new System.Drawing.Point(111, 32);
            this.textHeight.Name = "textHeight";
            this.textHeight.Size = new System.Drawing.Size(169, 20);
            this.textHeight.TabIndex = 3;
            this.textHeight.Text = "540";
            this.textHeight.TextChanged += new System.EventHandler(this.text_TextChanged);
            // 
            // textFps
            // 
            this.textFps.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textFps.Location = new System.Drawing.Point(111, 58);
            this.textFps.Name = "textFps";
            this.textFps.Size = new System.Drawing.Size(169, 20);
            this.textFps.TabIndex = 4;
            this.textFps.Text = "30.0";
            this.textFps.TextChanged += new System.EventHandler(this.text_TextChanged);
            // 
            // textMin
            // 
            this.textMin.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textMin.Location = new System.Drawing.Point(111, 84);
            this.textMin.Name = "textMin";
            this.textMin.Size = new System.Drawing.Size(60, 20);
            this.textMin.TabIndex = 5;
            this.textMin.Text = "1";
            this.textMin.TextChanged += new System.EventHandler(this.text_TextChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 61);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(30, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "FPS:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 87);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(50, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "Duration:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(177, 87);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(23, 13);
            this.label5.TabIndex = 8;
            this.label5.Text = "min";
            // 
            // textSec
            // 
            this.textSec.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textSec.Location = new System.Drawing.Point(220, 84);
            this.textSec.Name = "textSec";
            this.textSec.Size = new System.Drawing.Size(60, 20);
            this.textSec.TabIndex = 9;
            this.textSec.Text = "0";
            this.textSec.TextChanged += new System.EventHandler(this.text_TextChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(286, 87);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(24, 13);
            this.label6.TabIndex = 10;
            this.label6.Text = "sec";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.checkHuffYuv);
            this.groupBox1.Controls.Add(this.textSize);
            this.groupBox1.Location = new System.Drawing.Point(12, 110);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(330, 80);
            this.groupBox1.TabIndex = 11;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Estimated Disk Usage";
            // 
            // checkHuffYuv
            // 
            this.checkHuffYuv.AutoSize = true;
            this.checkHuffYuv.Location = new System.Drawing.Point(6, 51);
            this.checkHuffYuv.Name = "checkHuffYuv";
            this.checkHuffYuv.Size = new System.Drawing.Size(143, 17);
            this.checkHuffYuv.TabIndex = 1;
            this.checkHuffYuv.Text = "HuffYuv encoded output";
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
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.textSec);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.textMin);
            this.Controls.Add(this.textFps);
            this.Controls.Add(this.textHeight);
            this.Controls.Add(this.textWidth);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "Calculator";
            this.Text = "File Size Calculator";
            ((System.ComponentModel.ISupportInitialize)(this.errorProvider)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ErrorProvider errorProvider;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.CheckBox checkHuffYuv;
        private System.Windows.Forms.TextBox textSize;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox textSec;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textMin;
        private System.Windows.Forms.TextBox textFps;
        private System.Windows.Forms.TextBox textHeight;
        private System.Windows.Forms.TextBox textWidth;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
    }
}