namespace EyEnterprise.UI.UserControls
{
    partial class AccessCentralButton
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lblAccessCentral = new System.Windows.Forms.Label();
            this.btnAccessCentral = new EyEnterprise.UI.CustomControls.CustomButton();
            this.SuspendLayout();
            // 
            // lblAccessCentral
            // 
            this.lblAccessCentral.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.lblAccessCentral.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblAccessCentral.Location = new System.Drawing.Point(0, 157);
            this.lblAccessCentral.Name = "lblAccessCentral";
            this.lblAccessCentral.Size = new System.Drawing.Size(150, 24);
            this.lblAccessCentral.TabIndex = 2;
            this.lblAccessCentral.Text = "text";
            this.lblAccessCentral.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnAccessCentral
            // 
            this.btnAccessCentral.Anchor = System.Windows.Forms.AnchorStyles.Top;
            this.btnAccessCentral.BackColor = System.Drawing.Color.Transparent;
            this.btnAccessCentral.Checked = false;
            this.btnAccessCentral.CornerRadius = 125;
            this.btnAccessCentral.Flashing = false;
            this.btnAccessCentral.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(170)))), ((int)(((byte)(0)))), ((int)(((byte)(255)))));
            this.btnAccessCentral.ImageIndex = 1;
            this.btnAccessCentral.Location = new System.Drawing.Point(12, 15);
            this.btnAccessCentral.Name = "btnAccessCentral";
            this.btnAccessCentral.RoundCorners = ((EyEnterprise.UI.CustomControls.Corners)((((EyEnterprise.UI.CustomControls.Corners.TopLeft | EyEnterprise.UI.CustomControls.Corners.TopRight)
                        | EyEnterprise.UI.CustomControls.Corners.BottomLeft)
                        | EyEnterprise.UI.CustomControls.Corners.BottomRight)));
            this.btnAccessCentral.Size = new System.Drawing.Size(127, 130);
            this.btnAccessCentral.TabIndex = 1;
            // 
            // AccessCentralButton
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.Transparent;
            this.Controls.Add(this.lblAccessCentral);
            this.Controls.Add(this.btnAccessCentral);
            this.Name = "AccessCentralButton";
            this.Size = new System.Drawing.Size(150, 195);
            this.Load += new System.EventHandler(this.AccessCentralButton_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private CustomControls.CustomButton btnAccessCentral;
        private System.Windows.Forms.Label lblAccessCentral;

    }
}
