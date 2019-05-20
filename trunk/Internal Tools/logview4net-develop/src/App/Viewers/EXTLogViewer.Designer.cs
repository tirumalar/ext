namespace logview4net.Viewers
{
	partial class EXTLogViewer
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
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.splitContainer2 = new System.Windows.Forms.SplitContainer();
			this.splitContainer3 = new System.Windows.Forms.SplitContainer();
			this.lSessionList = new System.Windows.Forms.ListView();
			this.columnHeaderSession = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.lEvents = new System.Windows.Forms.ListView();
			this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.propertyGrid1 = new System.Windows.Forms.PropertyGrid();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
			this.splitContainer2.Panel1.SuspendLayout();
			this.splitContainer2.Panel2.SuspendLayout();
			this.splitContainer2.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).BeginInit();
			this.splitContainer3.Panel1.SuspendLayout();
			this.splitContainer3.Panel2.SuspendLayout();
			this.splitContainer3.SuspendLayout();
			this.SuspendLayout();
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.Location = new System.Drawing.Point(0, 0);
			this.splitContainer1.Name = "splitContainer1";
			this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.splitContainer3);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
			this.splitContainer1.Size = new System.Drawing.Size(632, 390);
			this.splitContainer1.SplitterDistance = 168;
			this.splitContainer1.TabIndex = 0;
			// 
			// splitContainer2
			// 
			this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer2.Location = new System.Drawing.Point(0, 0);
			this.splitContainer2.Name = "splitContainer2";
			// 
			// splitContainer2.Panel1
			// 
			this.splitContainer2.Panel1.Controls.Add(this.label2);
			this.splitContainer2.Panel1.Controls.Add(this.propertyGrid1);
			// 
			// splitContainer2.Panel2
			// 
			this.splitContainer2.Panel2.Controls.Add(this.label1);
			this.splitContainer2.Size = new System.Drawing.Size(632, 218);
			this.splitContainer2.SplitterDistance = 256;
			this.splitContainer2.TabIndex = 0;
			// 
			// splitContainer3
			// 
			this.splitContainer3.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer3.Location = new System.Drawing.Point(0, 0);
			this.splitContainer3.Name = "splitContainer3";
			// 
			// splitContainer3.Panel1
			// 
			this.splitContainer3.Panel1.Controls.Add(this.lSessionList);
			// 
			// splitContainer3.Panel2
			// 
			this.splitContainer3.Panel2.Controls.Add(this.lEvents);
			this.splitContainer3.Size = new System.Drawing.Size(632, 168);
			this.splitContainer3.SplitterDistance = 210;
			this.splitContainer3.TabIndex = 0;
			// 
			// lSessionList
			// 
			this.lSessionList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderSession});
			this.lSessionList.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lSessionList.FullRowSelect = true;
			this.lSessionList.GridLines = true;
			this.lSessionList.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.lSessionList.Location = new System.Drawing.Point(0, 0);
			this.lSessionList.MultiSelect = false;
			this.lSessionList.Name = "lSessionList";
			this.lSessionList.Size = new System.Drawing.Size(210, 168);
			this.lSessionList.TabIndex = 0;
			this.lSessionList.UseCompatibleStateImageBehavior = false;
			this.lSessionList.View = System.Windows.Forms.View.Details;
			// 
			// columnHeaderSession
			// 
			this.columnHeaderSession.Text = "Capture Session";
			this.columnHeaderSession.Width = 205;
			// 
			// lEvents
			// 
			this.lEvents.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4});
			this.lEvents.Dock = System.Windows.Forms.DockStyle.Fill;
			this.lEvents.FullRowSelect = true;
			this.lEvents.GridLines = true;
			this.lEvents.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.lEvents.Location = new System.Drawing.Point(0, 0);
			this.lEvents.MultiSelect = false;
			this.lEvents.Name = "lEvents";
			this.lEvents.Size = new System.Drawing.Size(418, 168);
			this.lEvents.TabIndex = 0;
			this.lEvents.UseCompatibleStateImageBehavior = false;
			this.lEvents.View = System.Windows.Forms.View.Details;
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Type";
			this.columnHeader1.Width = 51;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Timestamp";
			this.columnHeader2.Width = 75;
			// 
			// columnHeader3
			// 
			this.columnHeader3.Text = "Time Offset";
			this.columnHeader3.Width = 73;
			// 
			// columnHeader4
			// 
			this.columnHeader4.Text = "Event Details";
			this.columnHeader4.Width = 214;
			// 
			// propertyGrid1
			// 
			this.propertyGrid1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.propertyGrid1.Location = new System.Drawing.Point(0, 0);
			this.propertyGrid1.Name = "propertyGrid1";
			this.propertyGrid1.Size = new System.Drawing.Size(256, 218);
			this.propertyGrid1.TabIndex = 0;
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(92, 64);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(182, 53);
			this.label1.TabIndex = 0;
			this.label1.Text = "Additional Event details, images, etc";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(88, 77);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(122, 58);
			this.label2.TabIndex = 0;
			this.label2.Text = "Property Grid of Event Details";
			// 
			// EXTLogViewer
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.splitContainer1);
			this.Name = "EXTLogViewer";
			this.Size = new System.Drawing.Size(632, 390);
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
			this.splitContainer1.ResumeLayout(false);
			this.splitContainer2.Panel1.ResumeLayout(false);
			this.splitContainer2.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
			this.splitContainer2.ResumeLayout(false);
			this.splitContainer3.Panel1.ResumeLayout(false);
			this.splitContainer3.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.splitContainer3)).EndInit();
			this.splitContainer3.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.SplitContainer splitContainer2;
		private System.Windows.Forms.SplitContainer splitContainer3;
		private System.Windows.Forms.ListView lSessionList;
		private System.Windows.Forms.ColumnHeader columnHeaderSession;
		private System.Windows.Forms.ListView lEvents;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.PropertyGrid propertyGrid1;
		private System.Windows.Forms.Label label1;
	}
}
