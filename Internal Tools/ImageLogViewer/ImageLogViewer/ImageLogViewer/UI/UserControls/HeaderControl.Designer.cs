namespace EyEnterprise.UI.UserControls
{
    partial class HeaderControl
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
            this.CompanyName = new System.Windows.Forms.Label();
            this.MainTitle = new System.Windows.Forms.Label();
            this.edtSearch = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // CompanyName
            // 
            this.CompanyName.AutoSize = true;
            this.CompanyName.Font = new System.Drawing.Font("Arial", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.CompanyName.Location = new System.Drawing.Point(15, 10);
            this.CompanyName.Name = "CompanyName";
            this.CompanyName.Size = new System.Drawing.Size(76, 19);
            this.CompanyName.TabIndex = 0;
            this.CompanyName.Text = "EyeLock";
            // 
            // MainTitle
            // 
            this.MainTitle.AutoSize = true;
            this.MainTitle.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MainTitle.Location = new System.Drawing.Point(15, 62);
            this.MainTitle.Name = "MainTitle";
            this.MainTitle.Size = new System.Drawing.Size(163, 24);
            this.MainTitle.TabIndex = 0;
            this.MainTitle.Text = "Access Central";
            // 
            // edtSearch
            // 
            this.edtSearch.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.edtSearch.Font = new System.Drawing.Font("Arial", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.edtSearch.Location = new System.Drawing.Point(323, 62);
            this.edtSearch.Name = "edtSearch";
            this.edtSearch.Size = new System.Drawing.Size(162, 22);
            this.edtSearch.TabIndex = 1;
            this.edtSearch.Text = "Search";
            // 
            // HeaderControl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.edtSearch);
            this.Controls.Add(this.MainTitle);
            this.Controls.Add(this.CompanyName);
            this.Name = "HeaderControl";
            this.Size = new System.Drawing.Size(545, 98);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.HeaderControl_Paint);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.HeaderControl_MouseDown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label CompanyName;
        private System.Windows.Forms.Label MainTitle;
        private System.Windows.Forms.TextBox edtSearch;
    }
}
