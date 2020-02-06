using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Web.Script.Serialization;
using System.IO;
using System.Collections;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Drawing.Drawing2D;
using System.Threading;
using ImageLogViewer.UI.FormBaseClasses;

namespace ImageLogViewer
{
	public partial class MainWindow : GradientBaseForm
	{
		enum IMAGE_FORMAT
		{
			FORMAT_RAW = 2,
			FORMAT_J2K = 10,
			FORMAT_PNG = 14,
		};

		private Matrix transform = new Matrix();
		private float m_dZoomscale = 1.0f;
		private const float s_dScrollValue = 0.1f;
		private NodeSorter nodeSorter;
		private TreeNode m_SelectedNode = null;
		private string m_CurrentFilename = "";
		private int m_nFrameCount = 0;
		private int m_nCropCount = 0;
		private long m_lCurrentFileOffset = 0; // Holds the position of the last read object to virtualize loading of large image files...
        private MyTCPLogServer m_LogServer = null;
		private Thread m_LogServerThread = null;

		public MainWindow()
		{
			InitializeComponent();

			StartPosition = FormStartPosition.CenterScreen;

			nodeSorter = new NodeSorter();
			ImageLogTree.TreeViewNodeSorter = nodeSorter;
			tsSortCombo.SelectedIndex = 0;
		}


		// This object has already been extracted from the Network stream...
		// all we need to do is deserialize it into a JSON object and add it to the tree....	
		public void parseJSONTextToTree(string theJSON)
		{
			// We a JSON object, DeSerialize it and display it...
			ImageLogTree.BeginUpdate();
			JObject theObject = (JObject)JToken.Parse(theJSON);

			LoadObject(theObject);

			// Note, as the network stream comes in, it gets written to file on disk... with the garbage stripped...
			// When image display is called for, this
			m_lCurrentFileOffset += theJSON.Length;

			// done, draw all nodes now
			ImageLogTree.EndUpdate();
		}


		private void Deserialize(string Filename)
		{
			Cursor.Current = Cursors.WaitCursor;
			JObject theObject;

			ClearUI();

			// read JSON directly from a file
			using (StreamReader file = File.OpenText(Filename))
			using (JsonTextReader reader = new JsonTextReader(file))
			{
				reader.SupportMultipleContent = true;

				ImageLogTree.BeginUpdate();
				while (reader.Read())
				{
					theObject = (JObject)JToken.ReadFrom(reader);
					LoadObject(theObject);
					m_lCurrentFileOffset = reader.LinePosition;
				}

				// done, draw all nodes now
				ImageLogTree.EndUpdate();
			}

			Cursor.Current = Cursors.Default;
		}


		public void LoadObject(JObject theObject)
		{
			try
			{
				// Ok, crops come in before their Frame... to handle this, we buffer up the crops until the frame comes in
				// then we make them all children of the frame...
				if (((int)theObject["ImageType"] == 0) || ((int)theObject["ImageType"] == 2)) // Crop
				{
					TreeNode theNodeFrame;

					// Is this part of a capture session?  If so... use or create the top level session and the parent frame nodes...
					string sCapSession = (string)theObject["CaptureSession"];

					// Do we already have a session Node?
					TreeNode[] theNodes = ImageLogTree.Nodes
							.Cast<TreeNode>()
							.Where(r => r.Text == string.Format("CaptureSession: {0}", sCapSession))
							.ToArray();
						
					TreeNode theCapSessionNode;

					// If not found, create it...
					if (theNodes.Count() == 0)
					{
						theCapSessionNode = new TreeNode(string.Format("CaptureSession: {0}", sCapSession));
						ImageLogTree.Nodes.Add(theCapSessionNode);
						theCapSessionNode.Name = string.Format("{0}", DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond);
						theCapSessionNode.ImageIndex = 0;
						theCapSessionNode.SelectedImageIndex = theCapSessionNode.ImageIndex;
					}
					else // Capture session already exists, which means that the parent frame also exists...
						theCapSessionNode = theNodes[0];

					// If our frame doesn't already exist in this session, we need to add it...
					//Since the node is new, we must also create our Frame Node...
					TreeNode[] theFrameNodes = theCapSessionNode.Nodes
						.Cast<TreeNode>()
						.Where(r => r.Name == string.Format("{0}", (long)theObject["FrameUUID"]))
						.ToArray();

					// If not found, create it...
					if (theFrameNodes.Count() == 0)
					{
						theNodeFrame = new TreeNode(string.Format("Frame Image: {0},{1}", (int)theObject["CameraID"], (int)theObject["FrameID"]));
						theCapSessionNode.Nodes.Add(theNodeFrame);
						theNodeFrame.Name = string.Format("{0}", (long)theObject["FrameUUID"]);
						theNodeFrame.ImageIndex = 4;
						theNodeFrame.SelectedImageIndex = theNodeFrame.ImageIndex;

						toolStripStatusFrameCount.Text = string.Format("{0}", ++m_nFrameCount);
					}
					else // Already there...
						theNodeFrame = theFrameNodes[0];

					// Add the Crop to the frame...
					ImageNode theCrop = new ImageNode(theObject);
					theCrop.Name = (string)theObject["TimeStampMS"]; //Read timestamp in MS as a string and put it into the Node Key value for default sorting by time
					theCrop.ImageIndex = (bool)theObject["Discarded"] ? 3 : 2;
					theCrop.SelectedImageIndex = theCrop.ImageIndex;

					// Replace the very large image data, with an offset to in the file to the Object, when we need to display the image, we load it...
					theObject["imageData"] = "";
					theObject.Add("objectOffset", m_lCurrentFileOffset);

					theNodeFrame.Nodes.Add(theCrop);
					toolStripStatusCropCount.Text = string.Format("{0}", ++m_nCropCount);
				}
				else if ((int)theObject["ImageType"] == 1) // Frame
				{
					TreeNode theNodeFrame;
					TreeNode theCaptureSession;

					// Is this part of a capture session?  If so... use or create the top level session and the parent frame nodes...
					string sCapSession = (string)theObject["CaptureSession"];

					//Lookup default capture session... NOTE all unassociated objects will get stuck here... regardless of time...
					TreeNode[] theNodes = ImageLogTree.Nodes
							.Cast<TreeNode>()
							.Where(r => r.Text == string.Format("CaptureSession: {0}", sCapSession))
							.ToArray();

					// If not found, create it...
					if (theNodes.Count() == 0)
					{
						theCaptureSession = new TreeNode(string.Format("CaptureSession: {0}", sCapSession));
						theCaptureSession.Name = string.Format("{0}", DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond);
						ImageLogTree.Nodes.Add(theCaptureSession);
						theCaptureSession.ImageIndex = 0;
						theCaptureSession.SelectedImageIndex = theCaptureSession.ImageIndex;
					}
					else
						theCaptureSession = theNodes[0];

					// We have the capture session for this Frame... see if the frame exists already or not...
					// If our node already exists, just find it and populate the metadata...
					TreeNode[] theFrameNodes = theCaptureSession.Nodes
						.Cast<TreeNode>()
						.Where(r => r.Name == string.Format("{0}", (long)theObject["FrameUUID"]))
						.ToArray();

					// If not found in the tree anywhere... and if the it's not the default capsession, we need to check the default for the frame and move it if it's there...
					if (theFrameNodes.Count() == 0) 
					{
						theNodeFrame = new TreeNode(string.Format("Frame Image: {0},{1}", (int)theObject["CameraID"], (int)theObject["FrameID"]));
						theCaptureSession.Nodes.Add(theNodeFrame); //Add the frame object
						theNodeFrame.Name = string.Format("{0}", (long)theObject["FrameUUID"]);
						theNodeFrame.ImageIndex = 4;
						theNodeFrame.SelectedImageIndex = theNodeFrame.ImageIndex;
						toolStripStatusFrameCount.Text = string.Format("{0}", ++m_nFrameCount);
					}
					else // It exists... (created earlier...) associate the object with the item...
					{
						theNodeFrame = theFrameNodes[0];

						// Update the text to show crop count...
						theNodeFrame.Text = string.Format("Frame Image: {0},{1} [{2} EyeCrop{3}]", (int)theObject["CameraID"], (int)theObject["FrameID"], theNodeFrame.Nodes.Count, (theNodeFrame.Nodes.Count > 1) ? "s" : "");
					}

					// Set some values now that we have our frame
					theNodeFrame.Name = (string)theObject["TimeStampMS"]; //Read timestamp in MS as a string and put it into the Node Key value for default sorting by time
					// Store the current
					theNodeFrame.Tag = theObject;
						
					// Replace the very large image data, with an offset to in the file to the Object, when we need to display the image, we load it...
					theObject["imageData"] = "";
					theObject.Add("objectOffset", m_lCurrentFileOffset);
				}
			}
			catch (Exception)
			{
				// no op
			}
		}



		public Image DecodeImageFromData(JObject theObject)
		{
			Image theImage = null;
			byte [] eyeImageData = Convert.FromBase64String((string)theObject["imageData"]);
			System.Drawing.Imaging.ImageFormat theImageFormat = System.Drawing.Imaging.ImageFormat.Bmp;

			if (eyeImageData.Length == 0)
				return theImage;

			switch ((IMAGE_FORMAT)(int)theObject["ImageFormat"])
			{
				case IMAGE_FORMAT.FORMAT_RAW:
				{
					theImage = GenerateBitmap(eyeImageData, (int)theObject["ImageWidth"], (int)theObject["ImageHeight"]);
					break;
				}

				case IMAGE_FORMAT.FORMAT_PNG:
				{
					Stream stream = new MemoryStream(eyeImageData);
					theImage =  Image.FromStream(stream);
					theImageFormat = System.Drawing.Imaging.ImageFormat.Png;
					break;
				}

				case IMAGE_FORMAT.FORMAT_J2K:
				{
					MessageBox.Show("Unable to decode J2K image file.  Not implemented yet!");
					break;
				}
			}

			return theImage;
		}

		public Bitmap GenerateBitmap(byte []eyeImage, int ImageWidth, int ImageHeight)
		{
			var bmp = new Bitmap(ImageWidth, ImageHeight, PixelFormat.Format8bppIndexed);
			{
				BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0,
																bmp.Width,
																bmp.Height),
												  ImageLockMode.WriteOnly,
												  bmp.PixelFormat);

				IntPtr pNative = bmpData.Scan0;
				Marshal.Copy(eyeImage, 0, pNative, eyeImage.Length);

				bmp.UnlockBits(bmpData);

				//Force the palette to grayscale
				ColorPalette _palette = bmp.Palette;
				Color[] _entries = _palette.Entries;
				for (int i = 0; i < 256; i++)
				{
					Color b = new Color();
					b = Color.FromArgb((byte)i, (byte)i, (byte)i);
					_entries[i] = b;
				}
				bmp.Palette = _palette;

				var bmpNonIndexed = new Bitmap(ImageWidth, ImageHeight);

				// Now convert it into a non-indexed bmp for easier handing...
				using(Graphics graphics = Graphics.FromImage(bmpNonIndexed))
				{
					// Draw the original bitmap onto the graphics of the new bitmap
					graphics.DrawImage(bmp, 0, 0);
				}

				return bmpNonIndexed;
			}
		}

		private void openToolStripMenuItem_Click_1(object sender, EventArgs e)
		{
			OpenImageLogFile();
		}

		private void btnOpen_Click(object sender, EventArgs e)
		{
			OpenImageLogFile();
		}

		private void OpenImageLogFile()
		{
			OpenFileDialog ofDialog = new OpenFileDialog();
			if (DialogResult.OK == ofDialog.ShowDialog())
			{
				try
				{
					// If we are listening, turn off listening clean up any temp file...
					tsbutStartServer.Checked = false;

					// First if we have a temp file open, we need to close and delete it...
					m_CurrentFilename = ofDialog.FileName;

					// Reset
					m_nFrameCount = 0;
					m_nCropCount = 0;

					btnSave.Enabled = false;

					Deserialize(m_CurrentFilename);
				}
				catch (IOException ioEx)
				{
					MessageBox.Show("Unable to open the file. " + ioEx.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
				}
			}
		}

		private void ImageLogTree_AfterSelect(object sender, TreeViewEventArgs e)
		{
			string theString = e.Node.Text;

			
			//If it's not a level 0 node, get the parent
			JObject theObject = (JObject)e.Node.Tag;

			if ((theObject == null) && (e.Node.Parent != null))
				theObject = (JObject)e.Node.Parent.Tag;

			// Clear imagedata of previous item...
			if ((null != m_SelectedNode) && (null != m_SelectedNode.Tag))
			{
				// If the previous object had image data, clear it out...
				JObject selectedObject = (JObject)m_SelectedNode.Tag;

				if (null != selectedObject)
					selectedObject["imageData"] = ""; // clear it... 
			}


			if (theObject != null)
			{
				// Now, load the image for the newly selected object...
				LoadImageData(theObject);

				// Now we have the string convert it to an image and update out picturebox...
				Image theImage = DecodeImageFromData(theObject);

				AddImageDetails(theObject, theImage);
				pbImage.Image = theImage;

				// Now set the details in the property grid...
				pgImageDetails.SelectedObject = null; // Must set to null first for some reason...
				pgImageDetails.SelectedObject = theObject;
			}
			else
			{
				pbImage.Image = null;
				pgImageDetails.SelectedObject = null; // Must set to null first for some reason...
			}

			m_SelectedNode = e.Node; // The newly selected Node...
		}

		private void LoadImageData(JObject theObject)
		{
			// read JSON directly from a file
			if (m_CurrentFilename.Length > 0)
			{
				FileStream theStream = new FileStream(m_CurrentFilename, FileMode.Open, FileAccess.ReadWrite, FileShare.ReadWrite);
				using(StreamReader file = new StreamReader(theStream))
				{
					// Seek to the beginning of the object in the file... then read the object...
					file.BaseStream.Seek((long)theObject["objectOffset"], SeekOrigin.Begin);

					using (JsonTextReader reader = new JsonTextReader(file))
					{
						reader.SupportMultipleContent = true;

						if (reader.Read())
						{
							JObject tempObject;

							tempObject = (JObject)JToken.ReadFrom(reader);

							theObject["imageData"] = tempObject["imageData"];
						}
					}
				}
			}			
		}

		private void imageToolStripToggleData_CheckedChanged(object sender, EventArgs e)
		{
			JObject theObject;

			if (null == ImageLogTree.SelectedNode)
			{
				pbImage.Image = null;
				return;
			}

			if (ImageLogTree.SelectedNode.Level == 3)
				theObject = (JObject)ImageLogTree.SelectedNode.Parent.Tag;
			else
				theObject = (JObject)ImageLogTree.SelectedNode.Tag;

			if (null != theObject)
			{
				Image theImage = DecodeImageFromData(theObject);
				AddImageDetails(theObject, theImage);
				pbImage.Image = theImage;
			}
			else
				pbImage.Image = null;
		}


		public void AddImageDetails(JObject theObject, Image theImage)
		{
			if (theObject != null)
			{
				if ((theImage != null) && (imageToolStripToggleData.Checked))
				{
					int nLocationX = (int)theObject["ImageHeight"]/100;
					int nLocationY = nLocationX;

					// From this bitmap, the graphics can be obtained, because it has the right PixelFormat
					using(Graphics graphics = Graphics.FromImage(theImage))
					{
						// Need to calculate font height based on height of image to make it look similar regardless of resoution...
						using (Font arialFont =  new Font("Arial", ((int)theObject["ImageHeight"]*5)/100, FontStyle.Bold,  GraphicsUnit.Pixel))
						{
							string theText = string.Format("{0} {1}",  "Camera", (int)theObject["CameraID"]);

							var stringSize = graphics.MeasureString(theText, arialFont);
							var rectangle = new Rectangle(new Point(nLocationX, nLocationY), Size.Round(stringSize));

							graphics.FillRectangle(Brushes.Black, rectangle);
							graphics.DrawString(theText, arialFont, Brushes.GreenYellow, nLocationX, nLocationY);

							nLocationY += arialFont.Height;

							theText = string.Format("{0} {1}",  "FrameID", (int)theObject["FrameID"]);

							stringSize = graphics.MeasureString(theText, arialFont);
							rectangle = new Rectangle(new Point(nLocationX, nLocationY), Size.Round(stringSize));

							graphics.FillRectangle(Brushes.Black, rectangle);
							graphics.DrawString(theText, arialFont, Brushes.GreenYellow, nLocationX, nLocationY);


							// For each eye detection point add a crossll
							JArray a = (JArray)theObject["EyeDetectionPoints"];

							IList<DetectionPoint> detPoints = a.ToObject<IList<DetectionPoint>>();
							using (Pen thePen = new Pen(Brushes.Red, 2))
							{
								foreach (DetectionPoint point in detPoints)
								{
										graphics.DrawLine(thePen, point.PointX - 20, point.PointY, point.PointX + 20, point.PointY);
										graphics.DrawLine(thePen, point.PointX, point.PointY-20, point.PointX, point.PointY+20);
								}
							}

							// For each crop rectangle we found, add a rectangle...
							a = (JArray)theObject["EyeDetectionCropRects"];

							IList<CropRect> cropRects = a.ToObject<IList<CropRect>>();
							using (Pen thePenYellow = new Pen(Brushes.Blue, 2))
							{
								foreach (CropRect rect in cropRects)
									graphics.DrawRectangle(thePenYellow, rect.RectLeft,  rect.RectTop, rect.RectWidth, rect.RectHeight);
							}

							// Draw Pupil and Iris circles...
							using (Pen thePenGreen = new Pen(Brushes.GreenYellow, 2))
							{
								JToken t = (JToken)theObject["PupilCircle"];
								ImageCircle theCircle = t.ToObject<ImageCircle>();


								if (theCircle.Radius > 0)
								{
									graphics.DrawEllipse(thePenGreen, theCircle.PointX - theCircle.Radius, theCircle.PointY - theCircle.Radius,
													theCircle.Radius * 2, theCircle.Radius * 2);

									nLocationY += arialFont.Height;
									theText = string.Format("Pupil Dia: {0}px", theCircle.Radius*2);

									stringSize = graphics.MeasureString(theText, arialFont);
									rectangle = new Rectangle(new Point(nLocationX, nLocationY), Size.Round(stringSize));

									graphics.FillRectangle(Brushes.Black, rectangle);
									graphics.DrawString(theText, arialFont, Brushes.GreenYellow, nLocationX, nLocationY);

								}

								t = (JToken)theObject["IrisCircle"];
								theCircle = t.ToObject<ImageCircle>();

								if (theCircle.Radius > 0)
								{
									graphics.DrawEllipse(thePenGreen, theCircle.PointX - theCircle.Radius, theCircle.PointY - theCircle.Radius,
													theCircle.Radius * 2, theCircle.Radius * 2);

									nLocationY += arialFont.Height;
									theText = string.Format("Iris Dia: {0}px", theCircle.Radius * 2);

									stringSize = graphics.MeasureString(theText, arialFont);
									rectangle = new Rectangle(new Point(nLocationX, nLocationY), Size.Round(stringSize));

									graphics.FillRectangle(Brushes.Black, rectangle);
									graphics.DrawString(theText, arialFont, Brushes.GreenYellow, nLocationX, nLocationY);
								}
							}
						}
					}
				}
			}
		}

		private void pbImage_Paint(object sender, PaintEventArgs e)
		{
#if false
			Graphics g = e.Graphics;
			g.Transform = transform;
			Pen mypen = new Pen(Color.Red, 5);
			Rectangle rect = new Rectangle(10, 10, 30, 30);
			e.Graphics.DrawRectangle(mypen, rect);
#endif
		}

		protected override void OnMouseWheel(MouseEventArgs mea)
		{
			pbImage.Focus();
			if (pbImage.Focused == true && mea.Delta != 0)
			{
				// Map the Form-centric mouse location to the PictureBox client coordinate system
				Point pictureBoxPoint = pbImage.PointToClient(this.PointToScreen(mea.Location));
				ZoomScroll(pictureBoxPoint, mea.Delta > 0);
			}
		}

		private void ZoomScroll(Point location, bool zoomIn)
		{
			// Figure out what the new scale will be. Ensure the scale factor remains between
			// 1% and 1000%
			float newScale = Math.Min(Math.Max(m_dZoomscale + (zoomIn ? s_dScrollValue : -s_dScrollValue), 0.1f), 10);

			if (newScale != m_dZoomscale)
			{
				float adjust = newScale / m_dZoomscale;
				m_dZoomscale = newScale;

				// Translate mouse point to origin
				transform.Translate(-location.X, -location.Y, MatrixOrder.Append);

				// Scale view
				transform.Scale(adjust, adjust, MatrixOrder.Append);

				// Translate origin back to original mouse point.
				transform.Translate(location.X, location.Y, MatrixOrder.Append);

				pbImage.Invalidate();
			}
		}

		private void tsSortCombo_SelectedIndexChanged(object sender, EventArgs e)
		{
			nodeSorter.SortType = tsSortCombo.SelectedIndex;
			ImageLogTree.TreeViewNodeSorter = nodeSorter;

			ImageLogTree.Sort();
		}

		private void imageToolStripSaveAs_Click(object sender, EventArgs e)
		{
			if (pbImage != null)
			{
				SaveFileDialog sfDialog = new SaveFileDialog();

				sfDialog.Filter = "PNG Files (*.png)|*.png";
				sfDialog.FilterIndex = 0;

				if (DialogResult.OK == sfDialog.ShowDialog())
					pbImage.Image.Save(sfDialog.FileName);
			}
		}

		private void btnQuit_Click(object sender, EventArgs e)
		{
			 Close();
		}

		private void MainWindow_FormClosing(object sender, FormClosingEventArgs e)
		{
			// Clean up server listener
			if (null != m_LogServer)
			{
				m_LogServer.Kill(1000);
				m_LogServerThread.Interrupt();

				m_LogServer = null;
				m_LogServerThread = null;
			}

			remoteLogControlNXTLog.FormClosing();
			remoteLogFaceTrackingLog.FormClosing();
			remoteLogMotorLog.FormClosing();


			if (WindowState == FormWindowState.Maximized)
			{
				Properties.Settings.Default.Location = RestoreBounds.Location;
				Properties.Settings.Default.Size = RestoreBounds.Size;
			}
			else if (WindowState == FormWindowState.Normal)
			{
				Properties.Settings.Default.Location = Location;
				Properties.Settings.Default.Size = Size;
			}
			else
			{
				Properties.Settings.Default.Location = RestoreBounds.Location;
				Properties.Settings.Default.Size = RestoreBounds.Size;
			}
			Properties.Settings.Default.Save();
		}

		private void MainWindow_Load(object sender, EventArgs e)
		{
			LoadSettings();
		}


		private void LoadSettings()
		{
			Location = Properties.Settings.Default.Location;
			Size = Properties.Settings.Default.Size;

			lblRemotePort.Text = Properties.Settings.Default.ImageLogPort;
		}


		public void SetStreamFilename(string theFileName)
		{
			m_CurrentFilename = theFileName;
			// Enable the Save As button...
			btnSave.Enabled = true;
		}

		// If checked start our server, if unchecking, kill it if it exists...
		private void tsbutStartServer_Click(object sender, EventArgs e)
		{
	
		}


		private void ClearUI()
		{
			ImageLogTree.Nodes.Clear();
			m_lCurrentFileOffset = 0;
		}

		private void tsbutStartServer_CheckedChanged(object sender, EventArgs e)
		{
			if (tsbutStartServer.Checked)
			{
				int nPort;

				if (lblRemotePort.Text == "" || !Int32.TryParse(lblRemotePort.Text, out nPort))
					MessageBox.Show("Invalid Port specified!  Please enter a valid port number to listen on...", "Socket Error",  MessageBoxButtons.OK, MessageBoxIcon.Error);
				else
				{
					m_LogServer = new MyTCPLogServer(nPort, this, LogMode.MODE_JSON);
					SetStreamFilename(m_LogServer.tempFilename);
					m_LogServerThread = new Thread(new ThreadStart(m_LogServer.listen));
					m_LogServerThread.Start();

					Properties.Settings.Default.ImageLogPort = lblRemotePort.Text;
				}
			}
			else if (null != m_LogServer)
			{
				ClearUI();

				m_LogServer.Kill(1000);
				m_LogServerThread.Interrupt();

				m_LogServer = null;
				m_LogServerThread = null;
			}
		}

		private void btnSave_Click(object sender, EventArgs e)
		{
			// If clicked, give user chance to save the temporary file somewhere..
			SaveFileDialog sfDialog = new SaveFileDialog();

			sfDialog.FileName = "Untitled.json";
			sfDialog.Filter = "JSON Files (*.json)|*.json";
			sfDialog.FilterIndex = 0;

			if (DialogResult.OK == sfDialog.ShowDialog())
				File.Copy(m_CurrentFilename, sfDialog.FileName);
		}
	}


	public class NodeSorter : System.Collections.IComparer
	{
		public int SortType {get; set;}
		

		public NodeSorter() { }

		public int Compare(object x, object y)
		{
			long lValue1 = 0;
			long lValue2 = 0;

			TreeNode tx = x as TreeNode;
			TreeNode ty = y as TreeNode;

			JObject theObjectX, theObjectY;

			theObjectX = (JObject)tx.Tag;
			theObjectY = (JObject)ty.Tag;

			if (theObjectX == null || theObjectY == null)
				return 0;

			// Let ordering of Capture sessions remain by time (which is how they are intially loaded)...
			if ((tx.Level == 0) || (ty.Level == 0))
			{
				lValue1 = Convert.ToInt64(tx.Name);
				lValue2 = Convert.ToInt64(ty.Name);
			}

			switch (SortType)
			{
				default:
				case 0: // None
				{
					if (lValue1 < lValue2)
						return -1;
					else if (lValue1 == lValue2)
						return 0;
					else
						return 1;
				}

				case 1: // CamID
				{
					lValue1 = (int)theObjectX["CameraID"];
					lValue2 = (int)theObjectY["CameraID"];

					if (lValue1 < lValue2)
						return -1;
					else if (lValue1 == lValue2)
						return 0;
					else
						return 1;
				}

				case 2: // ImageID
				{
					lValue1 = (int)theObjectX["ImageType"];
					lValue2 = (int)theObjectY["ImageType"];

					if (lValue1 < lValue2)
						return 1;
					else if (lValue1 == lValue2)
						return 0;
					else
						return -1;
				}
			}
		}
	}


	public class CropRect
	{ 
		public CropRect() {}

		public int RectLeft;
		public int RectTop;
		public int RectWidth;
		public int RectHeight;
	}


	public class DetectionPoint
	{ 
		public DetectionPoint() {}

		public int PointX;
		public int PointY;
	}


	public class ImageCircle
	{ 
		public ImageCircle() {}

		public int PointX;
		public int PointY;
		public int Radius;
	}


	public class ImageNode : TreeNode
	{ 
		// takes an Image instance
		public ImageNode(JObject theObject)
		{
			// how do you want to represent it
			this.Text = string.Format("{0} - {1}", (int)theObject["ImageType"] == 0 ? "Face" : ((int)theObject["ImageType"] == 1 ? "Frame" : "Eye Crop"), DateTime.Now.ToString("yyyy’-‘MM’-‘dd’T’HH’:’mm’:’ss.fffffffK"));
			this.Tag = theObject;	// Hold onto it so we can reference the items...

			// If this object is a crop, we expect eventually that its frame will come through...  but it doesn't exist yet so we must create it...

			// and this class 'knows' how to handle its children
			Nodes.AddRange(theObject.Properties().Select(s => new ImageDetailNode(s)).ToArray());
		}
	}


	public class ImageDetailNode : TreeNode
	{
		public ImageDetailNode(JProperty theItem)
		{
			this.ImageIndex = 5;
			this.SelectedImageIndex = this.ImageIndex;

			this.Text = String.Format("{0} : {1}", theItem.Name, theItem.Value);   
		}

		public void Click()
		{
			MessageBox.Show(new String(this.Text.Reverse().ToArray()));
		}                
	}
}
