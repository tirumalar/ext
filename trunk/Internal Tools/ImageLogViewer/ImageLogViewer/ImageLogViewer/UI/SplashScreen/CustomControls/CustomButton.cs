using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EyEnterprise.UI.CustomControls
{
    using System;
    using System.ComponentModel;
    using System.Drawing;
    using System.Drawing.Design;
    using System.Windows.Forms;
    using System.Windows.Forms.Design;
    using System.Security.Permissions;
	using System.Drawing.Drawing2D;

        [System.Flags]
        public enum Corners
        {
            None = 0,
            TopLeft = 1,
            TopRight = 2,
            BottomLeft = 4,
            BottomRight = 8,
            All = TopLeft | TopRight | BottomLeft | BottomRight
        }


        public enum CustomButtonState
        {
            Normal = 1,
            Hot,
            Pressed,
            HotPressed,
            Disabled,
            Focused
        }


		public enum LEDState
		{
			LED_ON = 0,
			LED_OFF
		}

		public enum LEDBaseIndex
		{
			LED_RED = 1,
			LED_GREEN = 3
		}


		public enum CustomButtonStyleEx
		{
			Standard = 0,
			CheckButton,
			ThreeState,
			ImageLabel // Custom to give us just a label with an imagelist in it... no button functionality
		}


        public class CustomButton : Control, IButtonControl
        {
			// If a CHECK or 3_STATE style button, determines if we are pushed.
            private bool m_bToggled = false; // this should just be another state... later...
			private bool m_bFlashing = false;
			private CustomButtonStyleEx m_StyleEx = CustomButtonStyleEx.Standard;


            public CustomButton() : base()
            {
                this.SetStyle(ControlStyles.Selectable | ControlStyles.StandardClick | ControlStyles.ResizeRedraw | ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint | ControlStyles.SupportsTransparentBackColor, true);
            }
            

            #region Private Instance Variables

            private DialogResult m_DialogResult;
            private bool m_IsDefault;

            private int m_CornerRadius = 8;
            private Corners m_RoundCorners;
            public CustomButtonState m_ButtonState = CustomButtonState.Normal;

            private ContentAlignment m_ImageAlign = ContentAlignment.MiddleCenter;
            private ContentAlignment m_TextAlign = ContentAlignment.MiddleCenter;
            private ImageList m_ImageList;
            private int m_ImageIndex = -1;
			private int m_BaseImageIndex = -1;

            private bool keyPressed;
            private Rectangle contentRect;

            #endregion

 #region IButtonControl Implementation

            [Category("Behavior"), DefaultValue(typeof(DialogResult), "None")]
            [Description("The dialog result produced in a modal form by clicking the button.")]
            public DialogResult DialogResult
            {
                get { return m_DialogResult; }
                set
                {
                    if (Enum.IsDefined(typeof(DialogResult), value))
                        m_DialogResult = value;
                }
            }


            public void NotifyDefault(bool value)
            {
                if (m_IsDefault != value)
                    m_IsDefault = value;
                this.Invalidate();
            }


            public void PerformClick()
            {
                if (this.CanSelect)
                    base.OnClick(EventArgs.Empty);
            }

			public void PerformOutsideClick()
			{
	//			if (this.CanSelect)
	//				this.OnClick(EventArgs.Empty);
				// Simulate a keydown, keyup event..
				KeyEventArgs ke = new KeyEventArgs(Keys.Space);
				OnKeyDown(ke);

				if (m_StyleEx == CustomButtonStyleEx.CheckButton)
					m_bToggled = !m_bToggled;

				OnKeyUp(ke);

				// This is a hack, OnKeyUp() calls PerformClick() above, but if CanSelect is false, OnClick won't be called
				// so I just call it here...
		//		OnClick(EventArgs.Empty);
				if (!this.CanSelect) // Prevent multiples...
					base.OnClick(EventArgs.Empty);
			}

            #endregion

            #region Properties

            //ButtonState
            [Browsable(false)]
            public CustomButtonState ButtonState
            {
                get { return m_ButtonState; }
            }


			[Browsable(false)]
			public bool Checked
			{
				get { return m_bToggled; }
				set 
				{
					m_bToggled = value;
					
					if (m_bToggled)
						m_ButtonState = CustomButtonState.Pressed;
					else
						m_ButtonState = CustomButtonState.Normal;
					
					Invalidate();
					Update();
				}
			}


			[Category("Behavior")]
			[DefaultValue(CustomButtonStyleEx.Standard)]
			[Description("Defines type of button")]
			public CustomButtonStyleEx ButtonStyle
			{
				get { return m_StyleEx; }
				set
				{
					m_StyleEx = value; 
				}
			}

            //CornerRadius
            [Category("Appearance")]
            [DefaultValue(0)]
            [Description("Defines the radius of the controls RoundedCorners.")]
            public int CornerRadius
            {
                get { return m_CornerRadius; }
                set
                {
                    if (m_CornerRadius == value)
                        return;
                    m_CornerRadius = value;
                    this.Invalidate();
                }
            }


            //DefaultSize
            protected override System.Drawing.Size DefaultSize
            {
                get { return new Size(75, 23); }
            }

            //IsDefault
            [Browsable(false)]
            public bool IsDefault
            {
                get { return m_IsDefault; }
            }

			
			//ImageList
            [Category("Appearance"), DefaultValue(typeof(ImageList), null)]
            [Description("The image list to get the image to display in the face of the control.")]
            public ImageList ImageList
            {
                get { return m_ImageList; }
                set
                {
                    m_ImageList = value;
                    this.Invalidate();
                }
            }


            //ImageIndex
            [Category("Appearance"), DefaultValue(-1)]
            [Description("The index of the image in the image list to display in the face of the control.")]
            [TypeConverter(typeof(ImageIndexConverter))]
            [Editor("System.Windows.Forms.Design.ImageIndexEditor, System.Design", typeof(System.Drawing.Design.UITypeEditor))]
            public int ImageIndex
            {
                get { return m_ImageIndex; }
                set
                {
                    m_ImageIndex = value;
                    this.Invalidate();
                }
            }


	
			[Category("Appearance"), DefaultValue(-1)]
			[Description("The base index of the image in the image list to display in the face of the control.")]
			public int BaseImageIndex
			{
				get { return m_BaseImageIndex; }
				set
				{
					m_BaseImageIndex = value;
					this.Invalidate();
				}
			}

            //ImageAlign
            [Category("Appearance"), DefaultValue(typeof(ContentAlignment), "MiddleCenter")]
            [Description("The alignment of the image that will be displayed in the face of the control.")]
            public ContentAlignment ImageAlign
            {
                get { return m_ImageAlign; }
                set
                {
                    if (!Enum.IsDefined(typeof(ContentAlignment), value))
                        throw new InvalidEnumArgumentException("value", (int)value, typeof(ContentAlignment));
                    if (m_ImageAlign == value)
                        return;
                    m_ImageAlign = value;
                    this.Invalidate();
                }
            }


            //RoundCorners
            [Category("Appearance")]
            [DefaultValue(typeof(Corners), "None")]
            [Description("Gets/sets the corners of the control to round.")]
            [Editor(typeof(RoundCornersEditor), typeof(UITypeEditor))]
            [DesignerSerializationVisibility(DesignerSerializationVisibility.Visible)]
            public Corners RoundCorners
            {
                get { return m_RoundCorners; }
                set
                {
                    if (m_RoundCorners == value)
                        return;
                    m_RoundCorners = value;
                    this.Invalidate();
                }
            }


			//TextAlign
            [Category("Appearance"), DefaultValue(typeof(ContentAlignment), "MiddleCenter")]
            [Description("The alignment of the text that will be displayed in the face of the control.")]
            public ContentAlignment TextAlign
            {
                get { return m_TextAlign; }
                set
                {
                    if (!Enum.IsDefined(typeof(ContentAlignment), value))
                        throw new InvalidEnumArgumentException("value", (int)value, typeof(ContentAlignment));
                    if (m_TextAlign == value)
                        return;
                    m_TextAlign = value;
                    this.Invalidate();
                }
            }

	
			[Browsable(false)]
			public bool Flashing
			{
				get { return m_bFlashing; }
				set
				{ 
					m_bFlashing = value;

					// If turning off, make sure LED is 'off'
					if (!m_bFlashing)
						FlashLED();
				}
			}

            #endregion

            #region Overriden Methods

            protected override void OnKeyDown(KeyEventArgs e)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnKeyDown(e);
                if (e.KeyCode == Keys.Space)
                {
                    keyPressed = true;
                    m_ButtonState = CustomButtonState.Pressed;
                }
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnKeyUp(KeyEventArgs e)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnKeyUp(e);
                if (e.KeyCode == Keys.Space)
                {
                    if (this.ButtonState == CustomButtonState.Pressed || this.ButtonState == CustomButtonState.HotPressed)
                        this.PerformClick();
                    keyPressed = false;

                    if (!m_bToggled)
                        m_ButtonState = CustomButtonState.Focused;
                    else
                        m_ButtonState = CustomButtonState.Pressed;
                }
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnMouseEnter(EventArgs e)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnMouseEnter(e);
                if (!keyPressed)
                    m_ButtonState = m_bToggled ? CustomButtonState.HotPressed : CustomButtonState.Hot;
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnMouseLeave(EventArgs e)
			{
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnMouseLeave(e);
                if (!keyPressed)
                    if (this.IsDefault)
                        m_ButtonState = m_bToggled ? CustomButtonState.Pressed : CustomButtonState.Focused;
                    else
                        m_ButtonState = m_bToggled ? CustomButtonState.Pressed : CustomButtonState.Normal;
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnMouseDown(MouseEventArgs e)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnMouseDown(e);
                if (e.Button == MouseButtons.Left)
                {
                    this.Focus();
                    m_ButtonState = CustomButtonState.Pressed;
                }
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnMouseUp(MouseEventArgs e)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnMouseUp(e);
       //         m_ButtonState = CustomButtonState.Focused;
                m_ButtonState = m_bToggled ? CustomButtonState.Pressed : CustomButtonState.Focused;
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnMouseMove(MouseEventArgs e)
			{
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnMouseMove(e);
                if (new Rectangle(Point.Empty, this.Size).Contains(e.X, e.Y))
                {
                    if (e.Button == MouseButtons.Left)
                        m_ButtonState = CustomButtonState.Pressed;
                    else
                        m_ButtonState = m_bToggled ? CustomButtonState.HotPressed : CustomButtonState.Hot;
                }
                else
                {
                    if (keyPressed)
                        return;
                    m_ButtonState = m_bToggled ? CustomButtonState.Pressed : CustomButtonState.Normal;
                }
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnGotFocus(EventArgs e)
			{
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnGotFocus(e);
            //    m_ButtonState = CustomButtonState.Focused;
                this.NotifyDefault(true);
            }


            protected override void OnLostFocus(EventArgs e)
			{
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                base.OnLostFocus(e);
                if (this.FindForm().Focused)
                    this.NotifyDefault(false);
				m_ButtonState = m_bToggled ? CustomButtonState.Pressed : CustomButtonState.Normal;
            }


            protected override void OnEnabledChanged(EventArgs e)
			{
                base.OnEnabledChanged(e);
                if (this.Enabled)
					m_ButtonState = m_bToggled ? CustomButtonState.Pressed : CustomButtonState.Normal;
                else
                    m_ButtonState = CustomButtonState.Disabled;
                OnStateChange(EventArgs.Empty);
            }


            protected override void OnClick(EventArgs e)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                //Click gets fired before MouseUp which is handy
                if (this.ButtonState == CustomButtonState.Pressed || this.ButtonState == CustomButtonState.HotPressed)
                {
                    this.Focus();
					if (m_StyleEx == CustomButtonStyleEx.CheckButton)
						m_bToggled = !m_bToggled;
                    this.PerformClick();
                }
            }


            protected override void OnDoubleClick(EventArgs e)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
					return;

                if (this.ButtonState == CustomButtonState.Pressed)
                {
                    this.Focus();
                    this.PerformClick();
                }
            }


            protected override bool ProcessMnemonic(char charCode)
			{
                if (IsMnemonic(charCode, this.Text))
                {
					if (CustomButtonStyleEx.ImageLabel != m_StyleEx)
	                    base.OnClick(EventArgs.Empty);
                    return true;
                }
                return base.ProcessMnemonic(charCode);
            }


            protected override void OnTextChanged(EventArgs e)
            {
                base.OnTextChanged(e);
                this.Invalidate();
            }


            protected override void OnPaintBackground(PaintEventArgs pevent)
            {
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
				{
					base.OnPaintBackground(pevent);
					contentRect = this.ClientRectangle;
					return;
				}

                //Simulate Transparency
                System.Drawing.Drawing2D.GraphicsContainer g = pevent.Graphics.BeginContainer();
                Rectangle translateRect = this.Bounds;
                pevent.Graphics.TranslateTransform(-this.Left, -this.Top);
                PaintEventArgs pe = new PaintEventArgs(pevent.Graphics, translateRect);
                this.InvokePaintBackground(this.Parent, pe);
                this.InvokePaint(this.Parent, pe);
                pevent.Graphics.ResetTransform();
                pevent.Graphics.EndContainer(g);

                pevent.Graphics.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;

                Color shadeColor, highlightColor; // For pens
				Color fillColorDark, fillColorLight; // For gradient


                if (this.ButtonState == CustomButtonState.Hot)
                {
					fillColorLight = this.ForeColor;
					fillColorDark = ControlPaint.Dark(fillColorLight, .25f);

//                    fillColorDark = Color.FromArgb(255, 32, 32, 32);
//					fillColorLight = Color.FromArgb(255, 45, 45, 45);
					shadeColor = Color.FromArgb(255, 32, 32, 32);
                    highlightColor = Color.FromArgb(255, 60, 60, 60);
                }
                else if (this.ButtonState == CustomButtonState.Pressed)
                {
					fillColorDark = Color.FromArgb(255, 16, 16, 16);
					fillColorLight = Color.FromArgb(255, 32, 32, 32);
				//	fillColorDark = Color.FromArgb(255, 128, 108, 0);
				//	fillColorLight = Color.FromArgb(255, 255, 216, 0);
					shadeColor = Color.FromArgb(255, 16, 16, 16);
					highlightColor = Color.FromArgb(255, 60, 60, 60);
                }
                else if (this.ButtonState == CustomButtonState.HotPressed)
                {
					fillColorDark = Color.FromArgb(255, 45, 45, 45);
					fillColorLight = Color.FromArgb(255, 60, 60, 60);
					shadeColor = Color.FromArgb(255, 32, 32, 32);
					highlightColor = Color.FromArgb(255, 60, 60, 60);
                }
				else
				{
                    fillColorLight = this.ForeColor;
                    fillColorDark = ControlPaint.Dark(fillColorLight, .25f);

                    shadeColor = Color.FromArgb(255, 32, 32, 32);
                    highlightColor = Color.FromArgb(255, 60, 60, 60);

#if null
					fillColorDark = Color.FromArgb(255, 45, 45, 45);
					fillColorLight = Color.FromArgb(255, 60, 60, 60);
					shadeColor = Color.FromArgb(255, 32, 32, 32);
					highlightColor = Color.FromArgb(255, 60, 60, 60);
#endif
				}

                Rectangle r = this.ClientRectangle;
                System.Drawing.Drawing2D.GraphicsPath path = RoundRectangle(r, this.CornerRadius, Corners.All);//this.RoundCorners);

                //System.Drawing.SolidBrush paintBrush = new SolidBrush(fillColor);
                System.Drawing.Drawing2D.LinearGradientBrush paintBrush = new System.Drawing.Drawing2D.LinearGradientBrush(r, fillColorLight, fillColorDark, System.Drawing.Drawing2D.LinearGradientMode.Vertical);

#if null
                //We want a sharp change in the colors so define a Blend for the brush
                System.Drawing.Drawing2D.Blend b = new System.Drawing.Drawing2D.Blend();
                b.Positions = new float[] { 0, .35f, .65f, 1 };
                b.Factors = new float[] { 0, 0, 1, 1 };
                paintBrush.Blend = b;
#endif

                //Draw the Button Background
                pevent.Graphics.FillPath(paintBrush, path);
                paintBrush.Dispose();

                // If hot, draw the highlight...
                if (this.ButtonState == CustomButtonState.Hot)
                {
					Rectangle bounds = this.ClientRectangle;
					using (var ellipsePath = new GraphicsPath())
					{
						ellipsePath.AddEllipse(bounds);
						using (var brush = new PathGradientBrush(ellipsePath))
						{
							float[] myFactors = { 1f, 0f, 0f, 0f, 0f, 0f };
							float[] myPositions = { 0.0f, .2f, .4f, .6f, .8f, 1.0f };
							Blend myBlend = new Blend();
							myBlend.Factors = myFactors;
							myBlend.Positions = myPositions;
							brush.Blend = myBlend;

							brush.CenterPoint = new PointF(bounds.Width/2f, bounds.Height/2f);
							brush.CenterColor = fillColorDark;
							brush.SurroundColors = new[] { Color.Transparent };
							brush.FocusScales = new PointF(0, 0);

							pevent.Graphics.FillRectangle(brush, bounds);
						}
					}
                }

				/*
                //...and border
                Pen drawingPen = new Pen(darkDarkColor);
                pevent.Graphics.DrawPath(drawingPen, path);
               drawingPen.Dispose();
                */

                // Draw a flat rectangle
        //        pevent.Graphics.FillRectangle(paintBrush, r);


#if null // round buttons
                // Draw the edges and borders...
                Pen drawingPen = new Pen(shadeColor);
                pevent.Graphics.DrawLine(drawingPen, r.Left, r.Bottom-1, r.Left, r.Top+1);
                pevent.Graphics.DrawLine(drawingPen, r.Left, r.Top, r.Right-1, r.Top);
                drawingPen.Dispose();
				if (this.ButtonState == CustomButtonState.HotPressed || this.ButtonState == CustomButtonState.Pressed)
					drawingPen = new Pen(shadeColor);
				else
					drawingPen = new Pen(highlightColor);
                pevent.Graphics.DrawLine(drawingPen, r.Left+1, r.Bottom-2, r.Left+1, r.Top+1);
                pevent.Graphics.DrawLine(drawingPen, r.Left+1, r.Top+1, r.Right-2, r.Top+1);
                drawingPen.Dispose();

                drawingPen = new Pen(highlightColor);
                pevent.Graphics.DrawLine(drawingPen, r.Left, r.Bottom-1, r.Right-1, r.Bottom-1);
                pevent.Graphics.DrawLine(drawingPen, r.Right-1, r.Bottom-1, r.Right-1, r.Top+1);
                drawingPen.Dispose();
                drawingPen = new Pen(shadeColor);
                pevent.Graphics.DrawLine(drawingPen, r.Left + 1, r.Bottom - 2, r.Right-2, r.Bottom-2);
                pevent.Graphics.DrawLine(drawingPen, r.Right-2, r.Bottom-2, r.Right -2, r.Top + 2);
                drawingPen.Dispose();

				// If button was already down, and we are over it...
				if (this.ButtonState == CustomButtonState.HotPressed)
                {
                    Rectangle rfade = r;

					rfade.Inflate(-2, -2);
					rfade.Width = 10;

                    System.Drawing.Drawing2D.LinearGradientBrush myBrush = new System.Drawing.Drawing2D.LinearGradientBrush(rfade, Color.FromArgb(128, 0, 0, 0), Color.FromArgb(0, 0, 0, 0), System.Drawing.Drawing2D.LinearGradientMode.Horizontal);
                    pevent.Graphics.FillRectangle(myBrush, rfade.Left, rfade.Top, rfade.Width-1, rfade.Height);
                    myBrush.Dispose();

                    rfade.Offset(r.Right - 3 - rfade.Width, 0);
					myBrush = new System.Drawing.Drawing2D.LinearGradientBrush(rfade, Color.FromArgb(0, 0, 0, 0), Color.FromArgb(128, 0, 0, 0), System.Drawing.Drawing2D.LinearGradientMode.Horizontal);
                    pevent.Graphics.FillRectangle(myBrush, rfade.Left, rfade.Top, rfade.Width, rfade.Height);
                    myBrush.Dispose();
                }
#endif				

                //Get the Rectangle to be used for Content
                bool inBounds = false;
                //We could use some Math to get this from the radius but I'm 
                //not great at Math so for the example this hack will suffice.
                while (!inBounds && r.Width >= 1 && r.Height >= 1)
                {
                    inBounds = path.IsVisible(r.Left, r.Top) &&
                                path.IsVisible(r.Right, r.Top) &&
                                path.IsVisible(r.Left, r.Bottom) &&
                                path.IsVisible(r.Right, r.Bottom);
                    r.Inflate(-1, -1);
                }

                contentRect = r;
            }


			// We use an external timer (from the parent) and we have it flash
			// us when necessary... this allows us to keep our LEDs all in sync...
			// easily
			public void FlashLED()
			{
				if (m_bFlashing)
					ImageIndex = (ImageIndex == (int)LEDState.LED_OFF ? (int)LEDState.LED_ON : (int)LEDState.LED_OFF);
				else
					ImageIndex = (int)LEDState.LED_OFF;

				Update();
			}

            protected override void OnPaint(PaintEventArgs e)
            {
                DrawImage(e.Graphics);
                DrawText(e.Graphics);
				if (CustomButtonStyleEx.ImageLabel != m_StyleEx)
	                DrawFocus(e.Graphics);
                base.OnPaint(e);
            }


            protected override void OnParentBackColorChanged(EventArgs e)
            {
                base.OnParentBackColorChanged(e);
                this.Invalidate();
            }


            protected override void OnParentBackgroundImageChanged(EventArgs e)
            {
                base.OnParentBackgroundImageChanged(e);
                this.Invalidate();
            }


            #endregion

            #region Internal Draw Methods

            private void DrawImage(Graphics g)
            {
                if (this.ImageList == null || this.ImageIndex == -1)
                    return;
                if (this.ImageIndex < 0 || this.ImageIndex >= this.ImageList.Images.Count)
                    return;

                Image _Image = this.ImageList.Images[this.ImageIndex];

                Point pt = Point.Empty;
				int nImageOffset = 1;

                switch (this.ImageAlign)
                {
                    case ContentAlignment.TopLeft:
                        pt.X = contentRect.Left;
                        pt.Y = contentRect.Top;
						pt.Offset(nImageOffset, nImageOffset);
                        break;

                    case ContentAlignment.TopCenter:
                        pt.X = (Width - _Image.Width) / 2;
                        pt.Y = contentRect.Top;
						pt.Offset(0, nImageOffset);
                        break;

                    case ContentAlignment.TopRight:
                        pt.X = contentRect.Right - _Image.Width;
                        pt.Y = contentRect.Top;
						pt.Offset(-nImageOffset, nImageOffset);
                        break;

                    case ContentAlignment.MiddleLeft:
                        pt.X = contentRect.Left;
                        pt.Y = (Height - _Image.Height) / 2;
						pt.Offset(nImageOffset, 0);
                        break;

                    case ContentAlignment.MiddleCenter:
                        pt.X = (Width - _Image.Width) / 2;
                        pt.Y = (Height - _Image.Height) / 2;
                        break;

                    case ContentAlignment.MiddleRight:
                        pt.X = contentRect.Right - _Image.Width;
                        pt.Y = (Height - _Image.Height) / 2;
						pt.Offset(-nImageOffset, 0);
                        break;

                    case ContentAlignment.BottomLeft:
                        pt.X = contentRect.Left;
                        pt.Y = contentRect.Bottom - _Image.Height;
						pt.Offset(-nImageOffset, -nImageOffset);
                        break;

                    case ContentAlignment.BottomCenter:
                        pt.X = (Width - _Image.Width) / 2;
                        pt.Y = contentRect.Bottom - _Image.Height;
						pt.Offset(0, -nImageOffset);
                        break;

                    case ContentAlignment.BottomRight:
                        pt.X = contentRect.Right - _Image.Width;
                        pt.Y = contentRect.Bottom - _Image.Height;
						pt.Offset(-nImageOffset, -nImageOffset);
                        break;
                }

				if (this.ButtonState == CustomButtonState.Pressed || this.ButtonState == CustomButtonState.HotPressed)
                    pt.Offset(1, 1);

             //   if (this.Enabled)
                    this.ImageList.Draw(g, pt, this.ImageIndex);
            //    else
            //        ControlPaint.DrawImageDisabled(g, _Image, pt.X, pt.Y, this.BackColor);

				// Give the rect a nudge so our left aligned text is evenly spaced from our image...
				// Non-left aligned text will land where it lands based on the original rect size
				if (CustomButtonStyleEx.ImageLabel == m_StyleEx)
				{
					if (ContentAlignment.TopLeft == this.TextAlign ||
						ContentAlignment.MiddleLeft == this.TextAlign ||
						ContentAlignment.BottomLeft == this.TextAlign)
					{
						contentRect.X = nImageOffset*3 + _Image.Width;
					}
				}
            }


            private void DrawText(Graphics g)
            {
                SolidBrush TextBrush = new SolidBrush(this.ForeColor);

                RectangleF R = (RectangleF)contentRect;

         //       if (!this.Enabled)
         //           TextBrush.Color = Color.Black;

                StringFormat sf = new StringFormat(StringFormatFlags.NoWrap | StringFormatFlags.NoClip);

                if (ShowKeyboardCues)
                    sf.HotkeyPrefix = System.Drawing.Text.HotkeyPrefix.Show;
                else
                    sf.HotkeyPrefix = System.Drawing.Text.HotkeyPrefix.Hide;

                switch (this.TextAlign)
                {
                    case ContentAlignment.TopLeft:
                        sf.Alignment = StringAlignment.Near;
                        sf.LineAlignment = StringAlignment.Near;
                        break;

                    case ContentAlignment.TopCenter:
                        sf.Alignment = StringAlignment.Center;
                        sf.LineAlignment = StringAlignment.Near;
                        break;

                    case ContentAlignment.TopRight:
                        sf.Alignment = StringAlignment.Far;
                        sf.LineAlignment = StringAlignment.Near;
                        break;

                    case ContentAlignment.MiddleLeft:
                        sf.Alignment = StringAlignment.Near;
                        sf.LineAlignment = StringAlignment.Center;
                        break;

                    case ContentAlignment.MiddleCenter:
                        sf.Alignment = StringAlignment.Center;
                        sf.LineAlignment = StringAlignment.Center;
                        break;

                    case ContentAlignment.MiddleRight:
                        sf.Alignment = StringAlignment.Far;
                        sf.LineAlignment = StringAlignment.Center;
                        break;

                    case ContentAlignment.BottomLeft:
                        sf.Alignment = StringAlignment.Near;
                        sf.LineAlignment = StringAlignment.Far;
                        break;

                    case ContentAlignment.BottomCenter:
                        sf.Alignment = StringAlignment.Center;
                        sf.LineAlignment = StringAlignment.Far;
                        break;

                    case ContentAlignment.BottomRight:
                        sf.Alignment = StringAlignment.Far;
                        sf.LineAlignment = StringAlignment.Far;
                        break;
                }

                if (this.ButtonState == CustomButtonState.Pressed || this.ButtonState == CustomButtonState.HotPressed)
                    R.Offset(1, 1);

				if (this.Enabled)
					g.DrawString(this.Text, this.Font, TextBrush, R, sf);
				else
				{
					Color fillColorDark = Color.FromArgb(255, 32, 32, 32);
					Color fillColorLight = Color.FromArgb(255, 64, 64, 64);

					//		ControlPaint.DrawStringDisabled(g, this.Text, this.Font, this.BackColor, R, sf);
					TextBrush.Color = fillColorLight;
					R.Offset(1, 1);
					g.DrawString(this.Text, this.Font, TextBrush, R, sf);
					TextBrush.Color = fillColorDark;
					R.Offset(-1, -1);
					g.DrawString(this.Text, this.Font, TextBrush, R, sf);
					TextBrush.Color = Color.FromArgb(80, 255, 128, 0);
					g.DrawString(this.Text, this.Font, TextBrush, R, sf);
				}
            }


            private void DrawFocus(Graphics g)
            {
                Rectangle r = contentRect;
                r.Inflate(1, 1);
                if (this.Focused && this.ShowFocusCues && this.TabStop)
                    ControlPaint.DrawFocusRectangle(g, r, this.ForeColor, this.BackColor);
            }


            #endregion

            #region Helper Methods

            private System.Drawing.Drawing2D.GraphicsPath RoundRectangle(Rectangle r, int radius, Corners corners)
            {
                //Make sure the Path fits inside the rectangle
                r.Width -= 1;
                r.Height -= 1;

                //Scale the radius if it's too large to fit.
                if (radius > (r.Width))
                    radius = r.Width;
                if (radius > (r.Height))
                    radius = r.Height;

                System.Drawing.Drawing2D.GraphicsPath path = new System.Drawing.Drawing2D.GraphicsPath();

                if (radius <= 0)
                    path.AddRectangle(r);
                else
                    if ((corners & Corners.TopLeft) == Corners.TopLeft)
                        path.AddArc(r.Left, r.Top, radius, radius, 180, 90);
                    else
                        path.AddLine(r.Left, r.Top, r.Left, r.Top);

                if ((corners & Corners.TopRight) == Corners.TopRight)
                    path.AddArc(r.Right - radius, r.Top, radius, radius, 270, 90);
                else
                    path.AddLine(r.Right, r.Top, r.Right, r.Top);

                if ((corners & Corners.BottomRight) == Corners.BottomRight)
                    path.AddArc(r.Right - radius, r.Bottom - radius, radius, radius, 0, 90);
                else
                    path.AddLine(r.Right, r.Bottom, r.Right, r.Bottom);

                if ((corners & Corners.BottomLeft) == Corners.BottomLeft)
                    path.AddArc(r.Left, r.Bottom - radius, radius, radius, 90, 90);
                else
                    path.AddLine(r.Left, r.Bottom, r.Left, r.Bottom);

                path.CloseFigure();

                return path;
            }


            //The ControlPaint Class has methods to Lighten and Darken Colors, but they return a Solid Color.
            //The Following 2 methods return a modified color with original Alpha.
            private Color DarkenColor(Color colorIn, int percent)
            {
                //This method returns Black if you Darken by 100%

                if (percent < 0 || percent > 100)
                    throw new ArgumentOutOfRangeException("percent");

                int a, r, g, b;

                a = colorIn.A;
                r = colorIn.R - (int)((colorIn.R / 100f) * percent);
                g = colorIn.G - (int)((colorIn.G / 100f) * percent);
                b = colorIn.B - (int)((colorIn.B / 100f) * percent);

                return Color.FromArgb(a, r, g, b);
            }


            private Color LightenColor(Color colorIn, int percent)
            {
                //This method returns White if you lighten by 100%

                if (percent < 0 || percent > 100)
                    throw new ArgumentOutOfRangeException("percent");

                int a, r, g, b;

                a = colorIn.A;
                r = colorIn.R + (int)(((255f - colorIn.R) / 100f) * percent);
                g = colorIn.G + (int)(((255f - colorIn.G) / 100f) * percent);
                b = colorIn.B + (int)(((255f - colorIn.B) / 100f) * percent);

                return Color.FromArgb(a, r, g, b);
            }


            #endregion

            private CustomButtonState currentState;
            private void OnStateChange(EventArgs e)
            {
                //Repaint the button only if the state has actually changed
                if (this.ButtonState == currentState)
                    return;
                currentState = this.ButtonState;
                this.Invalidate();
            }


        }

	#region Custom TypeEditor for RoundCorners property

        [PermissionSetAttribute(SecurityAction.LinkDemand, Unrestricted = true)]
        [PermissionSetAttribute(SecurityAction.InheritanceDemand, Unrestricted = true)]
        public class RoundCornersEditor : UITypeEditor
        {
            public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
            {
                return UITypeEditorEditStyle.DropDown;
            }

            public override Object EditValue(ITypeDescriptorContext context, IServiceProvider provider, Object value)
            {
                if ((System.Type)value != typeof(Corners) || provider == null)
                    return value;

                IWindowsFormsEditorService edSvc = (IWindowsFormsEditorService)provider.GetService(typeof(IWindowsFormsEditorService));
                if (edSvc != null)
                {
                    CheckedListBox lb = new CheckedListBox();
                    lb.BorderStyle = BorderStyle.None;
                    lb.CheckOnClick = true;

                    lb.Items.Add("TopLeft", (((CustomButton)context.Instance).RoundCorners & Corners.TopLeft) == Corners.TopLeft);
                    lb.Items.Add("TopRight", (((CustomButton)context.Instance).RoundCorners & Corners.TopRight) == Corners.TopRight);
                    lb.Items.Add("BottomLeft", (((CustomButton)context.Instance).RoundCorners & Corners.BottomLeft) == Corners.BottomLeft);
                    lb.Items.Add("BottomRight", (((CustomButton)context.Instance).RoundCorners & Corners.BottomRight) == Corners.BottomRight);

                    edSvc.DropDownControl(lb);
                    Corners cornerFlags = Corners.None;
                    foreach (object o in lb.CheckedItems)
                    {
                        cornerFlags = cornerFlags | (Corners)Enum.Parse(typeof(Corners), o.ToString());
                    }
                    lb.Dispose();
                    edSvc.CloseDropDown();
                    return cornerFlags;
                }
                return value;
            }

        }

        #endregion
}
