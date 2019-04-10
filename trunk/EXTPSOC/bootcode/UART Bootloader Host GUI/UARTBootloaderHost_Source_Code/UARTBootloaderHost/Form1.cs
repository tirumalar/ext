/* This file is the main Form implemeting the GUI. 
 * It contains the code to be executed on "Program" Button Click event. 
 * It also contains the code to be implemented on various selections in the Form
 * 
 * The file "Bootload_enums.cs" contains the definition for all the enum data types. It provides the status codes and return codes used during Bootloader operation
 * 
 * The file "Bootload_Utils_NativeCode.cs" contains code that delegates the UART communication functions to the unmanaged C functions
 * * The functions OpenConnection,CloseConnection,ReadData,WriteData called in the Bootloader Files provided by Cypress have to be defined by the user. 
 * * In our C# GUI, the methods OpenConnection_UART,CloseConnection_UART,ReadData_UART,WriteData_UART provide the defintions for above functions.
 * 
 * The file "delegated_functions.cs" contains the definitions for the methods, OpenConnection_UART(),CloseConnection_UART(),ReadData_UART(),WriteData_UART().
 *
 */
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.IO;
using System.IO.Ports;
using System.Threading;
using UARTBootloaderHost;

namespace UARTBootloaderHost
{
    public partial class UIForm : Form
    {
        string Chosen_File_Cyacd = "";
        bool Cyacd_found;
        bool ConnectionStatus;
        double progressBarProgress;
        double progressBarStepSize;
        delegate void SetTextCallback(TextBox tbox, string text); /* Delegate for cross-thread invoke on textbox control */

        int[] baud_rate = new int[] { 9600, 19200, 38400, 57600, 115200 };  //Supported baud rates
        Bootload_Utils.CyBtldr_CommunicationsData comm_data = new Bootload_Utils.CyBtldr_CommunicationsData();

        /// <summary>
        /// The main Form
        /// </summary>
        public UIForm()
        {
            InitializeComponent();

            serialPort.DataBits = 8;
            serialPort.StopBits = System.IO.Ports.StopBits.One;
            serialPort.Parity = System.IO.Ports.Parity.None;
            serialPort.Handshake = System.IO.Ports.Handshake.None;

            comm_data.OpenConnection = OpenConnection;
            comm_data.CloseConnection = CloseConnection;
            comm_data.ReadData = ReadData;
            comm_data.WriteData = WriteData;
            comm_data.MaxTransferSize = 64;
        }

        /// <summary>
        /// Method to select CYACD file on "Browse" button click
        /// </summary>
        private void browseButton_Click(object sender, EventArgs e)
        {
            openFileDialog1.Title = "Open CYACD File";
            openFileDialog1.Filter = "cyacd file|*.cyacd";

            if (openFileDialog1.ShowDialog() == DialogResult.None)
            {
                textBox_StatusLog.Text += " No file chosen\r\n";
                Cyacd_found = false;
            }
        }

        /// <summary>
        /// Method to start bootloading
        /// </summary>
        private void Bootload_Click(object sender, EventArgs e)
        {            
            if (comPortComboBox.SelectedIndex == -1 || baudComboBox.SelectedIndex == -1)
            {
                textBox_StatusLog.Text += " COM port or baud rate not selected\r\n";
            }
            else if (Cyacd_found == false)
            {
                textBox_StatusLog.Text += " No file chosen\r\n";
            }
            else
            {
                try
                {
                    comPortComboBox.Enabled = false;
                    baudComboBox.Enabled = false;
                    bootloadButton.Enabled = false;
                    browseButton.Enabled = false;
                    FileNameTB.Enabled = false;
                    serialPort.BaudRate = baud_rate[baudComboBox.SelectedIndex];
                    serialPort.PortName = comPortComboBox.Text;
                    textBox_StatusLog.Text += " Bootload Started at " + DateTime.Now.ToLongTimeString() + "\r\n";
                    this.Refresh();

                    if (backgroundWorker.IsBusy != true)
                    {
                        backgroundWorker.RunWorkerAsync();
                    }
                }
                catch (Exception exc)
                {
                    textBox_StatusLog.Text += " Error in bootloading\r\n " + exc.Message + "\r\n";
                }
            }            
        }

        private void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            /* Exception occuring here will be passed as e.Error to _RunWorkerCompleted event handler */
            Bootload_Utils.CyBtldr_ProgressUpdate update = new Bootload_Utils.CyBtldr_ProgressUpdate(ProgressUpdate);
            e.Result = Bootload_Utils.CyBtldr_Program(Chosen_File_Cyacd, ref comm_data, update);            
        }

        private void backgroundWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            progressBarProgress += progressBarStepSize;
            progressBar1.Value = (int)progressBarProgress;
        }

        private void backgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            int bootloaderStatus;

            try
            {
                if (e.Error != null)
                {
                    textBox_StatusLog.Text += " Error in bootloading\r\n " + e.Error.Message + "\r\n";
                }
                else
                {
                    bootloaderStatus = (int)e.Result;
                    String timeNow = DateTime.Now.ToLongTimeString();

                    if (bootloaderStatus == (int)ReturnCodes.CYRET_SUCCESS)
                    {
                        textBox_StatusLog.Text += " Bootload Ended at " + timeNow + "\r\n Bootload is successful  !! \r\n";
                    }
                    else
                    {
                        textBox_StatusLog.Text += " Bootload Failed with Error Code: 0x" + bootloaderStatus.ToString("X5") + "\t" + timeNow + "\r\n";
                    }
                }
            }
            catch (Exception)
            {
            }
            finally
            {
                /* Serial port open & close is done by CyBtldr_Program() itself.
                 * CloseConnection() is called here to make sure the port is
                 * released if an exception occurs. The port is closed only if
                 * it is already open by checking the connection status boolean
                 * variable. 
                 */
                CloseConnection();
                progressBarProgress = 0;
                progressBar1.Value = 0;
                bootloadButton.Enabled = true;
                browseButton.Enabled = true;
                comPortComboBox.Enabled = true;
                baudComboBox.Enabled = true;
                FileNameTB.Enabled = true;
            }
        }

        /// <summary>
        /// Method to check the number of rows in the cyacd file and update the progress bar accordingly
        /// </summary>
        private void openFileDialog1_FileOk_1(object sender, CancelEventArgs e)
        {
            int lines;
            FileNameTB.ReadOnly = false;
            Chosen_File_Cyacd = openFileDialog1.FileName;
            FileNameTB.Text = Chosen_File_Cyacd;

            lines = File.ReadAllLines(Chosen_File_Cyacd).Length - 1; //Don't count header
            progressBarStepSize = 100.0 / lines;
            Cyacd_found = true;
        }
        
        /* This function loads the list of available serial ports into
         * comPortComboBox when mouse button is pressed down. 
         */ 
        private void comPortComboBox_MouseDown(object sender, MouseEventArgs e)
        {
            try
            {
                comPortComboBox.SelectedIndex = -1;
                comPortComboBox.Items.Clear();
                comPortComboBox.Items.AddRange(SerialPort.GetPortNames());
            }
            catch (Exception exc)
            {
                textBox_StatusLog.Text += " Error in acquiring list of serial ports\r\n " + exc.Message + "\r\n";
            }
        }

        private void UIForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                backgroundWorker.Dispose();
                CloseConnection();
            }
            catch (Exception)
            {
            }
        }

        /* This function sets the text in a thread-safe way. The textbox
         * is updated from the BackgroundWorker thread that does the 
         * bootloading whereas the text box is with the main UI thread.
         */
        private void SetText(TextBox tbox, string text)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (tbox.InvokeRequired)
            {
                SetTextCallback d = new SetTextCallback(SetText);
                this.Invoke(d, new object[] { tbox, text });
            }
            else
            {
                tbox.Text += text;
            }
        }

        #region DELEGATED_FUNCTIONS

        /// <summary>
        /// Method that updates the progress bar
        /// </summary>
        /// <param name="arrayID"></param>
        /// <param name="rowNum"></param>
        public void ProgressUpdate(byte arrayID, ushort rowNum)
        {
            /* Pass a dummy value of 0. This value is not used instead the
             * property progressBarStepSize is used inside the progress 
             * update function. Exception occuring here will be passed as
             * e.Error to _RunWorkerCompleted event handler. 
             */
            backgroundWorker.ReportProgress(0);
        }

        // <summary>
        /// Success 
        /// </summary>        
        internal const int ERR_SUCCESS = 0;
        /// <summary>
        /// Error opening communication channel
        /// </summary>
        internal const int ERR_OPEN = 1;
        /// <summary>
        /// Error closing communication channel
        /// </summary>
        internal const int ERR_CLOSE = 2;
        /// <summary>
        /// Error reading data
        /// </summary>
        internal const int ERR_READ = 3;
        /// <summary>
        /// Error writing data
        /// </summary>
        internal const int ERR_WRITE = 4;

        /// <summary>
        /// Method that opens the serial port connection
        /// Returns a success or failure
        /// </summary>
        public int OpenConnection()
        {
            int status = (int)ReturnCodes.CYRET_SUCCESS;

            if (ConnectionStatus == false)
            {
                try
                {
                    serialPort.Open();
                    ConnectionStatus = true;
                }
                catch (Exception exc)
                {
                    ConnectionStatus = false;
                    status = (int)ReturnCodes.CYRET_ERR_COMM_MASK;
                    SetText(textBox_StatusLog, " Error in opening serial port: " + exc.Message + "\r\n");

                    try
                    {
                        serialPort.Close();
                    }
                    catch
                    {
                    }
                }
            }
            return status;
        }

        /// <summary>
        /// Method that closes the serial  port connections
        /// </summary>
        public int CloseConnection()
        {
            if (ConnectionStatus == true)
            {
                try
                {
                    serialPort.Close();
                }
                catch (Exception)
                {
                }

                ConnectionStatus = false;
            }

            return (int)ReturnCodes.CYRET_SUCCESS;
        }

        /// <summary>
        /// Method that reads data from the UART terminal
        /// </summary>
        /// <param name="buffer"> Pointer to an array where data received from the bootloader needs to be stored </param>
        /// <param name="size"> Size of the Buffer, also indicates the number of bytes to be read from the bootloader </param>
        /// <returns></returns>
        public int ReadData(IntPtr buffer, int size)
        {
            int status = (int)ReturnCodes.CYRET_SUCCESS;
            int delayTime = 5; // Delay in milliseconds between successive reading until the 'size' number of bytes are read or timeout occurs
            int totalDelay = 100; //Timeout parameter in milliseconds to read 'size' number of bytes

            /* This will ensure the loop always runs one more time when totalDelay/delaytime
             * is not a whole number. e.g. If delayTime = 5 and totalDelay = 52, totalTries = 11.
             */
            int totalTries = (totalDelay + delayTime - 1) / delayTime;
            int tries = 0;
            int readCount = 0;

            try
            {
                byte[] data = new byte[size];

                /* Keep reading until size bytes are read or timeout occurs */
                while (readCount < size && tries++ <= totalTries)
                {
                    readCount += serialPort.Read(data, readCount, size - readCount);
                    System.Threading.Thread.Sleep(delayTime); //Delay between successive reading
                }

                /* If number of bytes read from serial port is not equal to size, return error */
                if (readCount != size)
                {
                    status = (int)ReturnCodes.CYRET_ERR_COMM_MASK;
                    SetText(textBox_StatusLog, " Error: Timeout occured while reading data from serial port\r\n");
                }

                /* Non-zero of data[1] indicates error. 
                 * Refer to the bootloader datasheet in PSoC Creator for the status codes. 
                 */
                if (data[1] != 0)
                {
                    SetText(textBox_StatusLog, " Error Code: " + data[1] + " received from PSoC\r\n");
                }

                Marshal.Copy(data, 0, buffer, size);
            }
            catch (Exception exc)
            {
                /* Return error if an exception occurs */
                SetText(textBox_StatusLog, " Error in reading data\r\n " + exc.Message + "\r\n");
                status = (int)ReturnCodes.CYRET_ERR_COMM_MASK;
            }

            return status;
        }

        /// <summary>
        /// Method that writes to the UART device
        /// </summary>
        /// <param name="buffer">Pointer to an array where data to be written to the bootloader is stored </param>
        /// <param name="size"> Size of the Buffer </param>
        /// <returns></returns>
        public int WriteData(IntPtr buffer, int size)
        {
            int status = (int)ReturnCodes.CYRET_SUCCESS;

            try
            {
                byte[] data = new byte[size];
                Marshal.Copy(buffer, data, 0, size);
                serialPort.Write(data, 0, size);
            }
            catch (Exception exc)
            {
                SetText(textBox_StatusLog, " Error in writing data\r\n " + exc.Message + "\r\n");
                status = (int)ReturnCodes.CYRET_ERR_COMM_MASK;
            }

            return status;
        }

        #endregion                
    }
}
