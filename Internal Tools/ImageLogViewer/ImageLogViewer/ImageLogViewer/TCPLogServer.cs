using System;
using System.Collections;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Linq;
using System.Windows.Forms;
using System.Text;

namespace ImageLogViewer
{
    public class TCPLogProcessor
	{
        private Stream inputStream;
        private static int MAX_MESSAGE_SIZE = 10 * 1024 * 1024; // 10MB

		static bool bQuitThread = false;

		private TcpClient socket;
		private TCPLogServer srv;

		string data = ""; // String for processing streambuffer...


        public TCPLogProcessor(TcpClient s, TCPLogServer srv) 
		{
            this.socket = s;
            this.srv = srv;                   
        }
        

        private string streamReadJSONObject(Stream inputStream) 
		{
			int nCount = 0;
			string tempdata = "";
			string theJSON = "";
			byte [] dataBuffer = new byte[1024*1024];

			// Here we skip all of the binary stuff that the sockeappender dumps in and we extract only a single JSON message..
			// This code does NOT support JSON with embedded JSON objects!  It will break if the logging subsystem is changed to support them...
			// This code is optimized and very dependent on the exact formatting that we know our application will produce...
            while (true)
			{
				try
				{
					// Read until we find an end marker, then extract the entire object... and return it...
					nCount = inputStream.Read(dataBuffer, 0, 1024*1024);

					if (nCount > 0)
					{
						tempdata = Encoding.UTF8.GetString(dataBuffer, 0, nCount);

						data += tempdata;

						// Do we have a full message?
						int nIndex = data.IndexOf("\"}");

						// didn't find our substring...
						if (nIndex <= 0) // Must read some more...
							continue;
						else // Found it, store the data...
						{
							nIndex += 2; // We want to include the trailing quote and bracket... we know it's there...

							int nStartIndex = data.IndexOf("{\"");

							if (nStartIndex >= 0)
							{
								theJSON = data.Substring(nStartIndex, nIndex - nStartIndex);

								// Hold onto the remainder of string... we'll need it for next time...
								data = data.Substring(nIndex +1);
								break;
							}
							else // were messed up on this one... drop it...
							{
								data = "";
								break;
							}
						}
					}
					else
						Thread.Sleep(1);
				}
				catch (Exception e)
				{
					Console.WriteLine(e.Message);
				}
            }            
            return theJSON;
        }


		public void kill()
		{
			bQuitThread = true;
		}

		//TCPLogProcessor Thread function
        public void process()
		{                        
            // we can't use a StreamReader for input, because it buffers up extra data on us inside it's
            // "processed" view of the world, and we want the data raw after the headers
            inputStream = new BufferedStream(socket.GetStream(), MAX_MESSAGE_SIZE);

			while (!bQuitThread)
			{
				try
				{
					int nAvailable = socket.Available;

					// Bytes waiting to be read...
					if (nAvailable > 0)
					{
						// Read only the JSON part of the message into
						srv.processJSON(parseLoggingMessage());
					}
					else
						Thread.Sleep(1); // give a timeslice back if nothing coming in...
				}
				catch (Exception e)
				{
					Console.WriteLine("Exception: " + e.ToString());
				}
			}

            inputStream = null;
            socket.Close();             
        }

        public string parseLoggingMessage()
		{
			// Testing 
			// Open our text file stream for writing...
//			var jsonStream = new FileStream(@"G:\AAAAAAAAA\json.txt", FileMode.Open);
//            String message = streamReadJSONObject(jsonStream);

            String message = streamReadJSONObject(inputStream);

			return message;
        }


		// Now that we have a good known full JSON object, read it with the JSON reader...
        public void processJSON(string theJSON)
		{
        }

    }

    public abstract class TCPLogServer
	{
        protected int port;
        TcpListener listener;
        bool is_active = true;

		private Thread tProcess;

        public TCPLogServer(int port)
		{
            this.port = port;
        }

		public void Kill(int timeout)
		{
			is_active = false;
			listener.Stop();
		}

        public void listen()
		{
			TCPLogProcessor processor = null;

            listener = new TcpListener(IPAddress.Any, port);
            listener.Start();
            while (is_active)
			{                
				try
				{
					TcpClient s = listener.AcceptTcpClient();
					processor = new TCPLogProcessor(s, this);
					tProcess = new Thread(new ThreadStart(processor.process));
					tProcess.Start();
					Thread.Sleep(1);
				}
				catch
				{
				}

				tProcess = null;
            }

			if (null != tProcess)
			{
				tProcess.Interrupt();
				tProcess.Join(1000);
			}

			if (null != processor)
				processor.kill();
        }

        public abstract void processJSON(string theJSON);
    }

    public class MyTCPLogServer : TCPLogServer
	{
		public MainWindow m_theDlg { get; set; }
		private FileStream m_tempFileStream;


        public MyTCPLogServer(int port, MainWindow theDlg) : base(port)
		{
			m_theDlg = theDlg;

			string theDir = Directory.GetCurrentDirectory();
			string thePath = Path.Combine(theDir,  Path.GetRandomFileName());

			// Open our text file stream for writing...
			m_tempFileStream = new FileStream(thePath, FileMode.Create, FileAccess.ReadWrite, FileShare.ReadWrite);
			m_theDlg.SetStreamFilename(thePath); // The the main dlg what file to use for looking up images...
        }

        public override void processJSON(string theJSON)
		{
			// Write the JSON to the temp diskfile...  this is needed because we don't hold image data in memory...
			m_tempFileStream.Write(System.Text.Encoding.UTF8.GetBytes(theJSON), 0, theJSON.Length);
			m_tempFileStream.Flush();

			// it gets referenced in the file on the fly...
			m_theDlg.Invoke((MethodInvoker)delegate {
				// Running on the UI thread
				m_theDlg.parseJSONTextToTree(theJSON);
			});
        }
    }
}



