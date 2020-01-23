namespace ImageLogViewer
{
	partial class MainWindow
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainWindow));
			this.MainSplitContainer = new System.Windows.Forms.SplitContainer();
			this.TopSplitterContainer = new System.Windows.Forms.SplitContainer();
			this.ImageLogTree = new System.Windows.Forms.TreeView();
			this.ilTreeView = new System.Windows.Forms.ImageList(this.components);
			this.tsTreeView = new System.Windows.Forms.ToolStrip();
			this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
			this.tsSortCombo = new System.Windows.Forms.ToolStripComboBox();
			this.tabImageData = new System.Windows.Forms.TabControl();
			this.tabPage3 = new System.Windows.Forms.TabPage();
			this.pbImage = new System.Windows.Forms.PictureBox();
			this.toolStrip1 = new System.Windows.Forms.ToolStrip();
			this.imageToolStripToggleData = new System.Windows.Forms.ToolStripButton();
			this.imageToolStripSaveAs = new System.Windows.Forms.ToolStripButton();
			this.tabPage4 = new System.Windows.Forms.TabPage();
			this.pgImageDetails = new System.Windows.Forms.PropertyGrid();
			this.tabControl1 = new System.Windows.Forms.TabControl();
			this.tabPage1 = new System.Windows.Forms.TabPage();
			this.rtbRawText = new System.Windows.Forms.RichTextBox();
			this.tabPage2 = new System.Windows.Forms.TabPage();
			this.tabPage6 = new System.Windows.Forms.TabPage();
			this.notifyIcon1 = new System.Windows.Forms.NotifyIcon(this.components);
			this.menuStrip = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.btnPanel = new System.Windows.Forms.Panel();
			this.btnQuit = new System.Windows.Forms.Button();
			this.tsMain = new System.Windows.Forms.ToolStrip();
			this.btnOpen = new System.Windows.Forms.ToolStripButton();
			this.tsLogWindow = new System.Windows.Forms.ToolStrip();
			this.btnOpenLogFile = new System.Windows.Forms.ToolStripButton();
			this.statusStripTreeView = new System.Windows.Forms.StatusStrip();
			this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusFrameCount = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusLabel2 = new System.Windows.Forms.ToolStripStatusLabel();
			this.toolStripStatusCropCount = new System.Windows.Forms.ToolStripStatusLabel();
			((System.ComponentModel.ISupportInitialize)(this.MainSplitContainer)).BeginInit();
			this.MainSplitContainer.Panel1.SuspendLayout();
			this.MainSplitContainer.Panel2.SuspendLayout();
			this.MainSplitContainer.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.TopSplitterContainer)).BeginInit();
			this.TopSplitterContainer.Panel1.SuspendLayout();
			this.TopSplitterContainer.Panel2.SuspendLayout();
			this.TopSplitterContainer.SuspendLayout();
			this.tsTreeView.SuspendLayout();
			this.tabImageData.SuspendLayout();
			this.tabPage3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.pbImage)).BeginInit();
			this.toolStrip1.SuspendLayout();
			this.tabPage4.SuspendLayout();
			this.tabControl1.SuspendLayout();
			this.tabPage1.SuspendLayout();
			this.menuStrip.SuspendLayout();
			this.btnPanel.SuspendLayout();
			this.tsMain.SuspendLayout();
			this.tsLogWindow.SuspendLayout();
			this.statusStripTreeView.SuspendLayout();
			this.SuspendLayout();
			// 
			// MainSplitContainer
			// 
			this.MainSplitContainer.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.MainSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.MainSplitContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.MainSplitContainer.Location = new System.Drawing.Point(0, 49);
			this.MainSplitContainer.Name = "MainSplitContainer";
			this.MainSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// MainSplitContainer.Panel1
			// 
			this.MainSplitContainer.Panel1.Controls.Add(this.TopSplitterContainer);
			// 
			// MainSplitContainer.Panel2
			// 
			this.MainSplitContainer.Panel2.Controls.Add(this.tabControl1);
			this.MainSplitContainer.Panel2.Controls.Add(this.tsLogWindow);
			this.MainSplitContainer.Size = new System.Drawing.Size(852, 463);
			this.MainSplitContainer.SplitterDistance = 305;
			this.MainSplitContainer.TabIndex = 0;
			// 
			// TopSplitterContainer
			// 
			this.TopSplitterContainer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.TopSplitterContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
			this.TopSplitterContainer.Location = new System.Drawing.Point(0, 0);
			this.TopSplitterContainer.Name = "TopSplitterContainer";
			// 
			// TopSplitterContainer.Panel1
			// 
			this.TopSplitterContainer.Panel1.Controls.Add(this.statusStripTreeView);
			this.TopSplitterContainer.Panel1.Controls.Add(this.ImageLogTree);
			this.TopSplitterContainer.Panel1.Controls.Add(this.tsTreeView);
			// 
			// TopSplitterContainer.Panel2
			// 
			this.TopSplitterContainer.Panel2.Controls.Add(this.tabImageData);
			this.TopSplitterContainer.Size = new System.Drawing.Size(848, 301);
			this.TopSplitterContainer.SplitterDistance = 250;
			this.TopSplitterContainer.TabIndex = 0;
			// 
			// ImageLogTree
			// 
			this.ImageLogTree.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.ImageLogTree.Dock = System.Windows.Forms.DockStyle.Fill;
			this.ImageLogTree.ImageIndex = 0;
			this.ImageLogTree.ImageList = this.ilTreeView;
			this.ImageLogTree.Location = new System.Drawing.Point(0, 25);
			this.ImageLogTree.Name = "ImageLogTree";
			this.ImageLogTree.SelectedImageIndex = 0;
			this.ImageLogTree.Size = new System.Drawing.Size(250, 276);
			this.ImageLogTree.TabIndex = 0;
			this.ImageLogTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.ImageLogTree_AfterSelect);
			// 
			// ilTreeView
			// 
			this.ilTreeView.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("ilTreeView.ImageStream")));
			this.ilTreeView.TransparentColor = System.Drawing.Color.Transparent;
			this.ilTreeView.Images.SetKeyName(0, "CaptureFolder-24x24.png");
			this.ilTreeView.Images.SetKeyName(1, "CaptureFolderNoEyes-24x24.png");
			this.ilTreeView.Images.SetKeyName(2, "Crop-24.24.png");
			this.ilTreeView.Images.SetKeyName(3, "Crop_Discarded.png");
			this.ilTreeView.Images.SetKeyName(4, "Frame.png");
			// 
			// tsTreeView
			// 
			this.tsTreeView.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.tsSortCombo});
			this.tsTreeView.Location = new System.Drawing.Point(0, 0);
			this.tsTreeView.Name = "tsTreeView";
			this.tsTreeView.Size = new System.Drawing.Size(250, 25);
			this.tsTreeView.TabIndex = 1;
			this.tsTreeView.Text = "toolStrip2";
			// 
			// toolStripLabel1
			// 
			this.toolStripLabel1.Name = "toolStripLabel1";
			this.toolStripLabel1.Size = new System.Drawing.Size(47, 22);
			this.toolStripLabel1.Text = "Sort By:";
			// 
			// tsSortCombo
			// 
			this.tsSortCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.tsSortCombo.Items.AddRange(new object[] {
            "Time",
            "Camera ID",
            "Image Type"});
			this.tsSortCombo.Name = "tsSortCombo";
			this.tsSortCombo.Size = new System.Drawing.Size(121, 25);
			this.tsSortCombo.SelectedIndexChanged += new System.EventHandler(this.tsSortCombo_SelectedIndexChanged);
			// 
			// tabImageData
			// 
			this.tabImageData.Controls.Add(this.tabPage3);
			this.tabImageData.Controls.Add(this.tabPage4);
			this.tabImageData.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabImageData.Location = new System.Drawing.Point(0, 0);
			this.tabImageData.Name = "tabImageData";
			this.tabImageData.SelectedIndex = 0;
			this.tabImageData.Size = new System.Drawing.Size(594, 301);
			this.tabImageData.TabIndex = 0;
			// 
			// tabPage3
			// 
			this.tabPage3.Controls.Add(this.pbImage);
			this.tabPage3.Controls.Add(this.toolStrip1);
			this.tabPage3.Location = new System.Drawing.Point(4, 22);
			this.tabPage3.Name = "tabPage3";
			this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage3.Size = new System.Drawing.Size(586, 275);
			this.tabPage3.TabIndex = 0;
			this.tabPage3.Text = "Image";
			this.tabPage3.UseVisualStyleBackColor = true;
			// 
			// pbImage
			// 
			this.pbImage.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
			this.pbImage.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pbImage.Location = new System.Drawing.Point(3, 28);
			this.pbImage.Name = "pbImage";
			this.pbImage.Size = new System.Drawing.Size(580, 244);
			this.pbImage.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
			this.pbImage.TabIndex = 0;
			this.pbImage.TabStop = false;
			this.pbImage.Paint += new System.Windows.Forms.PaintEventHandler(this.pbImage_Paint);
			// 
			// toolStrip1
			// 
			this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.imageToolStripToggleData,
            this.imageToolStripSaveAs});
			this.toolStrip1.Location = new System.Drawing.Point(3, 3);
			this.toolStrip1.Name = "toolStrip1";
			this.toolStrip1.Size = new System.Drawing.Size(580, 25);
			this.toolStrip1.TabIndex = 0;
			this.toolStrip1.Text = "toolStrip1";
			// 
			// imageToolStripToggleData
			// 
			this.imageToolStripToggleData.CheckOnClick = true;
			this.imageToolStripToggleData.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.imageToolStripToggleData.Image = global::ImageLogViewer.Properties.Resources.Details;
			this.imageToolStripToggleData.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.imageToolStripToggleData.Name = "imageToolStripToggleData";
			this.imageToolStripToggleData.Size = new System.Drawing.Size(23, 22);
			this.imageToolStripToggleData.Text = "Toggle Details";
			this.imageToolStripToggleData.CheckedChanged += new System.EventHandler(this.imageToolStripToggleData_CheckedChanged);
			// 
			// imageToolStripSaveAs
			// 
			this.imageToolStripSaveAs.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.imageToolStripSaveAs.Image = global::ImageLogViewer.Properties.Resources.Disk;
			this.imageToolStripSaveAs.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.imageToolStripSaveAs.Name = "imageToolStripSaveAs";
			this.imageToolStripSaveAs.Size = new System.Drawing.Size(23, 22);
			this.imageToolStripSaveAs.Text = "toolStripButton1";
			this.imageToolStripSaveAs.ToolTipText = "Save As...";
			this.imageToolStripSaveAs.Click += new System.EventHandler(this.imageToolStripSaveAs_Click);
			// 
			// tabPage4
			// 
			this.tabPage4.Controls.Add(this.pgImageDetails);
			this.tabPage4.Location = new System.Drawing.Point(4, 22);
			this.tabPage4.Name = "tabPage4";
			this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage4.Size = new System.Drawing.Size(379, 272);
			this.tabPage4.TabIndex = 1;
			this.tabPage4.Text = "Details";
			this.tabPage4.UseVisualStyleBackColor = true;
			// 
			// pgImageDetails
			// 
			this.pgImageDetails.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pgImageDetails.Location = new System.Drawing.Point(3, 3);
			this.pgImageDetails.Name = "pgImageDetails";
			this.pgImageDetails.Size = new System.Drawing.Size(373, 266);
			this.pgImageDetails.TabIndex = 0;
			// 
			// tabControl1
			// 
			this.tabControl1.Controls.Add(this.tabPage1);
			this.tabControl1.Controls.Add(this.tabPage2);
			this.tabControl1.Controls.Add(this.tabPage6);
			this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tabControl1.Location = new System.Drawing.Point(0, 25);
			this.tabControl1.Name = "tabControl1";
			this.tabControl1.SelectedIndex = 0;
			this.tabControl1.Size = new System.Drawing.Size(848, 125);
			this.tabControl1.TabIndex = 0;
			// 
			// tabPage1
			// 
			this.tabPage1.Controls.Add(this.rtbRawText);
			this.tabPage1.Location = new System.Drawing.Point(4, 22);
			this.tabPage1.Name = "tabPage1";
			this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage1.Size = new System.Drawing.Size(840, 99);
			this.tabPage1.TabIndex = 0;
			this.tabPage1.Text = "NXT Log";
			this.tabPage1.UseVisualStyleBackColor = true;
			// 
			// rtbRawText
			// 
			this.rtbRawText.Dock = System.Windows.Forms.DockStyle.Fill;
			this.rtbRawText.Location = new System.Drawing.Point(3, 3);
			this.rtbRawText.Name = "rtbRawText";
			this.rtbRawText.Size = new System.Drawing.Size(834, 93);
			this.rtbRawText.TabIndex = 0;
			this.rtbRawText.Text = "";
			// 
			// tabPage2
			// 
			this.tabPage2.Location = new System.Drawing.Point(4, 22);
			this.tabPage2.Name = "tabPage2";
			this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage2.Size = new System.Drawing.Size(751, 103);
			this.tabPage2.TabIndex = 1;
			this.tabPage2.Text = "FaceTracking Log";
			this.tabPage2.UseVisualStyleBackColor = true;
			// 
			// tabPage6
			// 
			this.tabPage6.Location = new System.Drawing.Point(4, 22);
			this.tabPage6.Name = "tabPage6";
			this.tabPage6.Padding = new System.Windows.Forms.Padding(3);
			this.tabPage6.Size = new System.Drawing.Size(751, 103);
			this.tabPage6.TabIndex = 3;
			this.tabPage6.Text = "Motor Log";
			this.tabPage6.UseVisualStyleBackColor = true;
			// 
			// notifyIcon1
			// 
			this.notifyIcon1.Text = "notifyIcon1";
			this.notifyIcon1.Visible = true;
			// 
			// menuStrip
			// 
			this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
			this.menuStrip.Location = new System.Drawing.Point(0, 0);
			this.menuStrip.Name = "menuStrip";
			this.menuStrip.Size = new System.Drawing.Size(852, 24);
			this.menuStrip.TabIndex = 0;
			this.menuStrip.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.fileToolStripMenuItem.Text = "File";
			// 
			// openToolStripMenuItem
			// 
			this.openToolStripMenuItem.Name = "openToolStripMenuItem";
			this.openToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
			this.openToolStripMenuItem.Text = "Open";
			this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click_1);
			// 
			// btnPanel
			// 
			this.btnPanel.Controls.Add(this.btnQuit);
			this.btnPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.btnPanel.Location = new System.Drawing.Point(0, 512);
			this.btnPanel.Name = "btnPanel";
			this.btnPanel.Size = new System.Drawing.Size(852, 42);
			this.btnPanel.TabIndex = 1;
			// 
			// btnQuit
			// 
			this.btnQuit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.btnQuit.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.btnQuit.Location = new System.Drawing.Point(765, 8);
			this.btnQuit.Name = "btnQuit";
			this.btnQuit.Size = new System.Drawing.Size(75, 23);
			this.btnQuit.TabIndex = 0;
			this.btnQuit.Text = "Quit";
			this.btnQuit.UseVisualStyleBackColor = true;
			this.btnQuit.Click += new System.EventHandler(this.btnQuit_Click);
			// 
			// tsMain
			// 
			this.tsMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.btnOpen});
			this.tsMain.Location = new System.Drawing.Point(0, 24);
			this.tsMain.Name = "tsMain";
			this.tsMain.Size = new System.Drawing.Size(852, 25);
			this.tsMain.TabIndex = 2;
			this.tsMain.Text = "toolStrip2";
			// 
			// btnOpen
			// 
			this.btnOpen.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.btnOpen.Image = global::ImageLogViewer.Properties.Resources.CaptureFolder_24x24;
			this.btnOpen.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.btnOpen.Name = "btnOpen";
			this.btnOpen.Size = new System.Drawing.Size(23, 22);
			this.btnOpen.Text = "toolStripButton1";
			this.btnOpen.Click += new System.EventHandler(this.btnOpen_Click);
			// 
			// tsLogWindow
			// 
			this.tsLogWindow.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.btnOpenLogFile});
			this.tsLogWindow.Location = new System.Drawing.Point(0, 0);
			this.tsLogWindow.Name = "tsLogWindow";
			this.tsLogWindow.Size = new System.Drawing.Size(848, 25);
			this.tsLogWindow.TabIndex = 1;
			this.tsLogWindow.Text = "toolStrip2";
			// 
			// btnOpenLogFile
			// 
			this.btnOpenLogFile.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
			this.btnOpenLogFile.Image = global::ImageLogViewer.Properties.Resources.CaptureFolder_24x24;
			this.btnOpenLogFile.ImageTransparentColor = System.Drawing.Color.Magenta;
			this.btnOpenLogFile.Name = "btnOpenLogFile";
			this.btnOpenLogFile.Size = new System.Drawing.Size(23, 22);
			this.btnOpenLogFile.Text = "toolStripButton1";
			// 
			// statusStripTreeView
			// 
			this.statusStripTreeView.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.toolStripStatusFrameCount,
            this.toolStripStatusLabel2,
            this.toolStripStatusCropCount});
			this.statusStripTreeView.Location = new System.Drawing.Point(0, 279);
			this.statusStripTreeView.Name = "statusStripTreeView";
			this.statusStripTreeView.Size = new System.Drawing.Size(250, 22);
			this.statusStripTreeView.SizingGrip = false;
			this.statusStripTreeView.TabIndex = 2;
			this.statusStripTreeView.Text = "statusStrip1";
			// 
			// toolStripStatusLabel1
			// 
			this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
			this.toolStripStatusLabel1.Size = new System.Drawing.Size(79, 17);
			this.toolStripStatusLabel1.Text = "Frame Count:";
			// 
			// toolStripStatusFrameCount
			// 
			this.toolStripStatusFrameCount.Name = "toolStripStatusFrameCount";
			this.toolStripStatusFrameCount.Size = new System.Drawing.Size(13, 17);
			this.toolStripStatusFrameCount.Text = "0";
			// 
			// toolStripStatusLabel2
			// 
			this.toolStripStatusLabel2.Name = "toolStripStatusLabel2";
			this.toolStripStatusLabel2.Size = new System.Drawing.Size(72, 17);
			this.toolStripStatusLabel2.Text = "Crop Count:";
			// 
			// toolStripStatusCropCount
			// 
			this.toolStripStatusCropCount.Name = "toolStripStatusCropCount";
			this.toolStripStatusCropCount.Size = new System.Drawing.Size(13, 17);
			this.toolStripStatusCropCount.Text = "0";
			// 
			// MainWindow
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = global::ImageLogViewer.Properties.Settings.Default.Size;
			this.Controls.Add(this.MainSplitContainer);
			this.Controls.Add(this.btnPanel);
			this.Controls.Add(this.tsMain);
			this.Controls.Add(this.menuStrip);
			this.DataBindings.Add(new System.Windows.Forms.Binding("Location", global::ImageLogViewer.Properties.Settings.Default, "Location", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
			this.DataBindings.Add(new System.Windows.Forms.Binding("ClientSize", global::ImageLogViewer.Properties.Settings.Default, "Size", true, System.Windows.Forms.DataSourceUpdateMode.OnPropertyChanged));
			this.Location = global::ImageLogViewer.Properties.Settings.Default.Location;
			this.MainMenuStrip = this.menuStrip;
			this.Name = "MainWindow";
			this.Text = "EyeLock Image Log Viewer";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainWindow_FormClosing);
			this.Load += new System.EventHandler(this.MainWindow_Load);
			this.MainSplitContainer.Panel1.ResumeLayout(false);
			this.MainSplitContainer.Panel2.ResumeLayout(false);
			this.MainSplitContainer.Panel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.MainSplitContainer)).EndInit();
			this.MainSplitContainer.ResumeLayout(false);
			this.TopSplitterContainer.Panel1.ResumeLayout(false);
			this.TopSplitterContainer.Panel1.PerformLayout();
			this.TopSplitterContainer.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.TopSplitterContainer)).EndInit();
			this.TopSplitterContainer.ResumeLayout(false);
			this.tsTreeView.ResumeLayout(false);
			this.tsTreeView.PerformLayout();
			this.tabImageData.ResumeLayout(false);
			this.tabPage3.ResumeLayout(false);
			this.tabPage3.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.pbImage)).EndInit();
			this.toolStrip1.ResumeLayout(false);
			this.toolStrip1.PerformLayout();
			this.tabPage4.ResumeLayout(false);
			this.tabControl1.ResumeLayout(false);
			this.tabPage1.ResumeLayout(false);
			this.menuStrip.ResumeLayout(false);
			this.menuStrip.PerformLayout();
			this.btnPanel.ResumeLayout(false);
			this.tsMain.ResumeLayout(false);
			this.tsMain.PerformLayout();
			this.tsLogWindow.ResumeLayout(false);
			this.tsLogWindow.PerformLayout();
			this.statusStripTreeView.ResumeLayout(false);
			this.statusStripTreeView.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.SplitContainer MainSplitContainer;
		private System.Windows.Forms.SplitContainer TopSplitterContainer;
		private System.Windows.Forms.TreeView ImageLogTree;
		private System.Windows.Forms.NotifyIcon notifyIcon1;
		private System.Windows.Forms.MenuStrip menuStrip;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
		private System.Windows.Forms.Panel btnPanel;
		private System.Windows.Forms.Button btnQuit;
		private System.Windows.Forms.PictureBox pbImage;
		private System.Windows.Forms.ImageList ilTreeView;
		private System.Windows.Forms.TabControl tabImageData;
		private System.Windows.Forms.TabPage tabPage3;
		private System.Windows.Forms.ToolStrip toolStrip1;
		private System.Windows.Forms.TabPage tabPage4;
		private System.Windows.Forms.PropertyGrid pgImageDetails;
		private System.Windows.Forms.ToolStripButton imageToolStripToggleData;
		private System.Windows.Forms.TabControl tabControl1;
		private System.Windows.Forms.TabPage tabPage1;
		private System.Windows.Forms.RichTextBox rtbRawText;
		private System.Windows.Forms.TabPage tabPage2;
		private System.Windows.Forms.TabPage tabPage6;
		private System.Windows.Forms.ToolStrip tsTreeView;
		private System.Windows.Forms.ToolStripLabel toolStripLabel1;
		private System.Windows.Forms.ToolStripComboBox tsSortCombo;
		private System.Windows.Forms.ToolStripButton imageToolStripSaveAs;
		private System.Windows.Forms.ToolStrip tsMain;
		private System.Windows.Forms.ToolStripButton btnOpen;
		private System.Windows.Forms.ToolStrip tsLogWindow;
		private System.Windows.Forms.ToolStripButton btnOpenLogFile;
		private System.Windows.Forms.StatusStrip statusStripTreeView;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusFrameCount;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel2;
		private System.Windows.Forms.ToolStripStatusLabel toolStripStatusCropCount;
	}
}

