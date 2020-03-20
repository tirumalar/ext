using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace EyEnterprise.UI.UserControls
{
    public partial class HeaderControl : UserControl
    {
        public const int WM_NCLBUTTONDOWN = 0xA1;
        public const int HT_CAPTION = 0x2;

        public HeaderControl()
        {
            InitializeComponent();
        }

        private void HeaderControl_MouseDown(object sender, MouseEventArgs e)
        {
        //    Capture = false;
        //   Message msg = Message.Create(Handle, WM_NCLBUTTONDOWN, (IntPtr)HT_CAPTION, IntPtr.Zero);
        //    base.WndProc(ref msg);
        }

        // Draw a single line across the bottom as a separator...
        private void HeaderControl_Paint(object sender, PaintEventArgs e)
        {
            using (Pen thePen = new Pen(this.ForeColor))
                e.Graphics.DrawLine(thePen, (float)this.DisplayRectangle.Left, (float)this.DisplayRectangle.Bottom-1, (float)this.DisplayRectangle.Right, (float)this.DisplayRectangle.Bottom-1);
        }
    }
}
