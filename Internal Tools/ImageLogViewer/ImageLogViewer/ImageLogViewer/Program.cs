using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ImageLogViewer
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new MainWindow());


			#if null
			MainWindow theWindow = new MainWindow();

            HttpServer httpServer;
            httpServer = new MyHttpServer(8088, theWindow);

            Thread thread = new Thread(new ThreadStart(httpServer.listen));
            thread.Start();

			Application.Run(theWindow);

			httpServer.Kill(1000);
			thread.Interrupt();
			#endif
		}
	}
}
