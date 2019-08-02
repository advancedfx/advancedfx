using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace HlaeCoreExtension
{
    public partial class ProgressDialog : Form
    {
        float[] mulss;
        float[] adds;
        bool userClosed;

        public ProgressDialog(float[] stepWeights)
        {
            InitializeComponent();

            float totalWeight = 0;

            foreach(float stepWight in stepWeights)
            {
                totalWeight += stepWight;
            }

            float add = 0;

            mulss = new float[stepWeights.Length];
            adds = new float[stepWeights.Length];

            for (int i = 0; i < stepWeights.Length; ++i)
            {
                float relStepWeight = stepWeights[i] / totalWeight;

                mulss[i] = relStepWeight;
                adds[i] = add;

                add += relStepWeight;
            }
        }

        public string Title
        {
            get { return this.Text;  }
            set { this.Text = value; }
        }

        public bool UserClosed
        {
            get {  return userClosed; }
        }

        public void SetProgress(int step, string description, float value)
        {
            labelDescription.Text = description;
            progressBar.Value = Math.Min(Math.Max(progressBar.Minimum, (int)Math.Round(progressBar.Minimum + (adds[step] + value * mulss[step]) * (progressBar.Maximum - progressBar.Minimum))), progressBar.Maximum);
        }

        private void ProgressDialog_FormClosed(object sender, FormClosedEventArgs e)
        {
            userClosed = e.CloseReason != CloseReason.None;
        }
    }
}
