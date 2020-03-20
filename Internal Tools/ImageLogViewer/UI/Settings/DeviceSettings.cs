using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ImageLogViewer.UI.FormBaseClasses;

namespace ImageLogViewer.UI.Settings
{
    public partial class DeviceSettings : GradientBaseForm
    {
        public DeviceSettings()
        {
            InitializeComponent();
        }

        private void btnSetLoggingOptions_Click(object sender, EventArgs e)
        {
            // Save all the new settings...
            Cursor.Current = Cursors.WaitCursor;

            var checkedButton = gbLogOptions.Controls.OfType<RadioButton>().FirstOrDefault(r => r.Checked);

            Properties.Settings.Default.ImageLoggingMode = checkedButton.Name;
            Properties.Settings.Default.SaveImagesToFile = cbxSaveImages.Checked;

            Properties.Settings.Default.NXTLogLevel = cbNXTLogLevel.GetItemText(cbNXTLogLevel.SelectedItem);
            Properties.Settings.Default.FaceTrackingLogLevel = cbFaceTrackingLevel.GetItemText(cbFaceTrackingLevel.SelectedItem);
            Properties.Settings.Default.MotorLogLevel = (string)cbMotorLogLevel.GetItemText(cbMotorLogLevel.SelectedItem);
            Properties.Settings.Default.EventLogLevel = (string)cbEventLogLevel.GetItemText(cbEventLogLevel.SelectedItem);

            Properties.Settings.Default.DeviceIPAddress = edtIPAddress.Text;
            Properties.Settings.Default.DevicePort = edtPort.Text;

            Properties.Settings.Default.IPWorkstation = edtWorkstationAddress.Text;

            Properties.Settings.Default.Save();

            // Create Client Connect to Device and set settings... one by one...
            // This MUST be done first because it causes a full reload of the environment...
            string theString = string.Format("REMOTEIP;{0};", Properties.Settings.Default.IPWorkstation);
            if (!SendMessageToDevice(theString))
            {
                Cursor.Current = Cursors.Default;
                return;
            }

            // LogFrameImages...
            if (checkedButton.Name == "radioDontLogImages")
            {
                string theString1 = string.Format("IMGLOGLEVEL;ERROR;{0};", Properties.Settings.Default.SaveImagesToFile ? "1" : "0");
                if (!SendMessageToDevice(theString1))
                {
                    Cursor.Current = Cursors.Default;
                    return;
                }
            }
            else if (checkedButton.Name == "radioLogCropsOnly")
            {
                string theString1 = string.Format("IMGLOGLEVEL;DEBUG;{0};", Properties.Settings.Default.SaveImagesToFile ? "1" : "0");
                if (!SendMessageToDevice(theString1))
                {
                    Cursor.Current = Cursors.Default;
                    return;
                }
            }
            else
            {
                string theString1 = string.Format("IMGLOGLEVEL;TRACE;{0};", Properties.Settings.Default.SaveImagesToFile ? "1" : "0");
                if (!SendMessageToDevice(theString1))
                {
                    Cursor.Current = Cursors.Default;
                    return;
                }
            }

            // set the current level for each logger...
            theString = string.Format("LOGGERLOGLEVEL;{0};{1};", "nxtlog", Properties.Settings.Default.NXTLogLevel);
            if (!SendMessageToDevice(theString))
            {
                Cursor.Current = Cursors.Default;
                return;
            }

            theString = string.Format("LOGGERLOGLEVEL;{0};{1};", "facetrackinglog", Properties.Settings.Default.FaceTrackingLogLevel);
            if (!SendMessageToDevice(theString))
            {
                Cursor.Current = Cursors.Default;
                return;
            }

            theString = string.Format("LOGGERLOGLEVEL;{0};{1};", "motorlog", Properties.Settings.Default.MotorLogLevel);
            if (!SendMessageToDevice(theString))
            {
                Cursor.Current = Cursors.Default;
                return;
            }

            theString = string.Format("LOGGERLOGLEVEL;{0};{1};", "eventlog", Properties.Settings.Default.EventLogLevel);
            if (!SendMessageToDevice(theString))
            {
                Cursor.Current = Cursors.Default;
                return;
            }
        }

        private bool SendMessageToDevice(string theMessage)
        {
            TcpClient client = null;
            bool bRetVal = false;

            //---create a TCPClient object at the IP and port no.---
            try
            {
                client = new TcpClient(edtIPAddress.Text, Int32.Parse(edtPort.Text));
                NetworkStream nwStream = client.GetStream();
                byte[] bytesToSend = ASCIIEncoding.ASCII.GetBytes(theMessage);

                //---send the text---
                //  Console.WriteLine("Sending : " + textToSend);
                nwStream.Write(bytesToSend, 0, bytesToSend.Length);
                bRetVal = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("Unable to send to Device: {0}", ex.Message));
            }
            finally
            {
                if (null != client)
                    client.Close();
            }

            return bRetVal;

#if null
            //---read back the text---
            byte[] bytesToRead = new byte[client.ReceiveBufferSize];
            int bytesRead = nwStream.Read(bytesToRead, 0, client.ReceiveBufferSize);
            Console.WriteLine("Received : " + Encoding.ASCII.GetString(bytesToRead, 0, bytesRead));
            Console.ReadLine();
#endif
        }

        private void DeviceSettings_Load(object sender, EventArgs e)
        {
            //Load all the controls here...
            if (Properties.Settings.Default.ImageLoggingMode == "radioDontLogImages")
                radioDontLogImages.Checked = true;
            else if (Properties.Settings.Default.ImageLoggingMode == "radioLogCropsOnly")
                radioLogCropsOnly.Checked = true;
            else
                radioLogFramesAndCrops.Checked = true;

            cbxSaveImages.Checked = Properties.Settings.Default.SaveImagesToFile;

            string theLevel = Properties.Settings.Default.NXTLogLevel;

            cbNXTLogLevel.SelectedItem = Properties.Settings.Default.NXTLogLevel;
            cbFaceTrackingLevel.SelectedItem = Properties.Settings.Default.FaceTrackingLogLevel;
            cbMotorLogLevel.SelectedItem = Properties.Settings.Default.MotorLogLevel;
            cbEventLogLevel.SelectedItem = Properties.Settings.Default.EventLogLevel;

            edtIPAddress.Text = Properties.Settings.Default.DeviceIPAddress;
            edtPort.Text = Properties.Settings.Default.DevicePort;

            if (Properties.Settings.Default.IPWorkstation == "0.0.0.0")
                edtWorkstationAddress.Text = GetLocalIPAddress();
            else
                edtWorkstationAddress.Text = Properties.Settings.Default.IPWorkstation;
        }


        public string GetLocalIPAddress()
        {
            var host = Dns.GetHostEntry(Dns.GetHostName());
            foreach (var ip in host.AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    return ip.ToString();
                }
            }
            throw new Exception("No network adapters with an IPv4 address in the system!");
        }


        private void btnRebootDevice_Click(object sender, EventArgs e)
        {
            if (System.Windows.Forms.DialogResult.Yes == MessageBox.Show("Are you sure you want to reboot the device now?", "REBOOT Device!", MessageBoxButtons.YesNo, MessageBoxIcon.Question))
                SendMessageToDevice("REBOOT;");
        }

        private void btnDefaultLogging_Click(object sender, EventArgs e)
        {
            SendMessageToDevice("REVERTCONFIG;");
        }
    }
}
