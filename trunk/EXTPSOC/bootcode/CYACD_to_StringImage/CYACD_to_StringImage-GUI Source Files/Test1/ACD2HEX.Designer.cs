namespace ACD2HexConvertor
{
    partial class ACD2HEX
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
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.FileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.SourceFile = new System.Windows.Forms.Label();
            this.FileNameTB = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.FolderName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.button3 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // folderBrowserDialog1
            // 
            this.folderBrowserDialog1.SelectedPath = "C:\\Documents and Settings\\anmd\\Desktop";
            this.folderBrowserDialog1.HelpRequest += new System.EventHandler(this.folderBrowserDialog1_HelpRequest);
            // 
            // FileDialog1
            // 
            this.FileDialog1.FileName = "*.cyacd";
            this.FileDialog1.FileOk += new System.ComponentModel.CancelEventHandler(this.openFileDialog1_FileOk);
            // 
            // SourceFile
            // 
            this.SourceFile.AutoSize = true;
            this.SourceFile.Location = new System.Drawing.Point(39, 59);
            this.SourceFile.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.SourceFile.Name = "SourceFile";
            this.SourceFile.Size = new System.Drawing.Size(57, 13);
            this.SourceFile.TabIndex = 0;
            this.SourceFile.Text = "SourceFile";
            // 
            // FileNameTB
            // 
            this.FileNameTB.Location = new System.Drawing.Point(100, 55);
            this.FileNameTB.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.FileNameTB.Name = "FileNameTB";
            this.FileNameTB.Size = new System.Drawing.Size(216, 20);
            this.FileNameTB.TabIndex = 1;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(352, 54);
            this.button1.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(56, 19);
            this.button1.TabIndex = 2;
            this.button1.Text = "Browse";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(352, 118);
            this.button2.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(56, 19);
            this.button2.TabIndex = 3;
            this.button2.Text = "Browse";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // FolderName
            // 
            this.FolderName.Location = new System.Drawing.Point(100, 118);
            this.FolderName.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.FolderName.Name = "FolderName";
            this.FolderName.Size = new System.Drawing.Size(216, 20);
            this.FolderName.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(41, 122);
            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Destination";
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(190, 188);
            this.button3.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(56, 19);
            this.button3.TabIndex = 6;
            this.button3.Text = "OK";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // ACD2HEX
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(426, 296);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.FolderName);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.FileNameTB);
            this.Controls.Add(this.SourceFile);
            this.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.Name = "ACD2HEX";
            this.Text = "CYACD_to_StringImage";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        private System.Windows.Forms.OpenFileDialog FileDialog1;
        private System.Windows.Forms.Label SourceFile;
        private System.Windows.Forms.TextBox FileNameTB;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.TextBox FolderName;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button button3;

    }
}

