#include "helper.h"

float32 faceAngle_degrees(float32 x1, float32 y1, float32 x2, float32 y2) {
  float32 deltaX = x2 - x1;
  float32 deltaY = y2 - y1;
  if ((deltaX == 0) && (deltaY == 0)) return 0.0;  // atan2 undefined
  float32 rad = (float32)atan2((double)deltaY, (double)deltaX);  // In radians
  float32 deg = rad * (180.0f / 3.141593f);                      // in degrees
  //
  // rotate so that horizontal is 0 degrees
  // convert to +- 90 degree range
  // left eye up is positive
  if (deg > 0)
    deg = deg - 180.0f;
  else
    deg = deg + 180.0f;
  //
  return deg;
}

#define IRISFIND_DARK_RATIO_MIN 0.020
#define IRISFIND_DARK_RATIO_MAX 0.200

//
// TBD: testing this method, not used yet
//
uint32 mark_low_values_on_image_dense(PLINE* lineptr, int16 imagewidth,
                                      int16 imageheight, int16 xc, int16 yc,
                                      int16 roi, float32 /*darkRatio*/,
                                      PLINE* lineptrDark) {
  const uint16 HIST_SIZE = 256;
  uint32 hist[HIST_SIZE];
  int16 searchBoxRadius = roi >> 1;
  int16 minx = xc - searchBoxRadius;
  int16 maxx = minx + roi;
  int16 miny = yc - searchBoxRadius;
  int16 maxy = miny + roi;
  uint16 pwidth = maxx - minx;
  uint16 pheight = maxy - miny;
  uint32 pixcnt = pwidth * pheight;

  //-----------------------------------------------

  // test for out-of-bounds coord xy
  if (minx < 0) return 0;
  if (miny < 0) return 0;
  if (maxx > imagewidth) return 0;
  if (maxy > imageheight) return 0;
  //-----------------------------------------------
  // find histogram for area
  // clear histogram
  for (int i = 0; i < HIST_SIZE; i++) hist[i] = 0;
  // clear output image
  for (int y = 0; y < imageheight; y++) {
    uint8* pptrDark = (uint8*)lineptrDark[y];
    for (int x = 0; x < imagewidth; x++) {
      pptrDark[x] = 0;
    }
  }
  //-----------------------------------------------
  // collect histogram values
  for (int y = miny; y < maxy; y++) {
    uint8* pptr = (uint8*)lineptr[y];
    for (int x = minx; x < maxx; x++) {
      uint8 val8 = pptr[x];
      hist[val8]++;
    }
  }
  //-----------------------------------------------

  uint32 minlowvalpixels = (uint32)((float32)pixcnt * IRISFIND_DARK_RATIO_MIN);
  uint32 maxlowvalpixels = (uint32)((float32)pixcnt * IRISFIND_DARK_RATIO_MAX);

  //-----------------------------------------------
  // find threshold for dark pixel value
  // Log(IntToStr(maxlowvalpixels));

  uint8 low_thresh = 0;
  uint8 high_thresh = 200;  // arbitrary safety limit
  uint32 sum = 0;
  for (int i = 1; i < HIST_SIZE - 1; i++) {
    sum += (uint8)hist[i];
    if ((low_thresh == 0) && (sum > minlowvalpixels)) {
      low_thresh = (uint8_t)i;
    }
    if ((low_thresh != 0) && (sum > maxlowvalpixels)) {
      high_thresh = (uint8_t)i + 1;
      break;
    }
  }

  uint8 thresh = low_thresh;  // arbitrary safety limit
  for (thresh = low_thresh; thresh <= high_thresh; thresh++) {
    //        int xmin = 0;
    //        int ymin = 0;
    //        int xmax= 0;
    //        int ymax = 0;
    for (int y = miny; y < maxy; y++) {
      uint8* pptr = (uint8*)lineptr[y];
      for (int x = minx; x < maxx; x++) {
        uint8 val8 = pptr[x];
        if (val8 < thresh) {
        }
      }
    }
  }

  //-----------------------------------------------
  // mark location of input image dark pixels onto the output image
  uint32 lowcnt = 0;
  for (int y = miny; y < maxy; y++) {
    uint8* pptr = (uint8*)lineptr[y];
    uint8* pptrDark = (uint8*)lineptrDark[y];
    for (int x = minx; x < maxx; x++) {
      uint8 val8 = pptr[x];
      if (val8 < thresh) {
        pptrDark[x] = 0xff;
        lowcnt++;
      } else
        pptrDark[x] = 0;
    }
  }
  // Log(IntToStr(lowcnt));
  // delete noisy pixels
  lowcnt = 0;
  for (int y = miny; y < maxy; y++) {
    uint8* pptrDark = (uint8*)lineptrDark[y];
    uint8* pptrDark_below = (uint8*)lineptrDark[y + 1];
    for (int x = minx; x < maxx; x++) {
      uint8 val8 = pptrDark[x];
      uint8 val8_right = pptrDark[x + 1];
      uint8 val8_below = pptrDark_below[x];
      if (val8) {
        if (val8_right && val8_below) {
          lowcnt++;
        } else
          pptrDark[x] = 0;
      }
    }
  }
  //----------------------------------------------
  // Log(IntToStr(lowcnt));
  //
  return lowcnt;
}
//
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// use histogram to mark low values
//
// Input eye crop image.
// Detect dark pixels in an ROI of eye crop image.
// Output eyecrop sized buffer marked at pixel locations
// corresponding to dark pixels in the input eye crop image ROI.

// Craig testing
//#include "mainwindow.h"
// extern MainWindow *mainWin; // for Log("text");

uint32 mark_low_values_on_image(PLINE* lineptr, int16 imagewidth,
                                int16 imageheight, int16 xc, int16 yc,
                                int16 roi, float32 darkRatio,
                                PLINE* lineptrDark, uint8* avgDarkValue) {
  const uint16 HIST_SIZE = 256;
  uint32 hist[HIST_SIZE];
  int16 searchBoxRadius = roi >> 1;
  int16 minx = xc - searchBoxRadius;
  int16 maxx = minx + roi;
  int16 miny = yc - searchBoxRadius;
  int16 maxy = miny + roi;
  uint16 pwidth = maxx - minx;
  uint16 pheight = maxy - miny;
  uint32 pixcnt = pwidth * pheight;
  //-----------------------------------------------

  // test for out-of-bounds coord xy
  if (minx < 0) return 0;
  if (miny < 0) return 0;
  if (maxx > imagewidth) return 0;
  if (maxy > imageheight) return 0;
  //-----------------------------------------------
  // find histogram for area
  // clear histogram
  for (int i = 0; i < HIST_SIZE; i++) hist[i] = 0;
  // clear output image
  for (int y = 0; y < imageheight; y++) {
    uint8* pptrDark = (uint8*)lineptrDark[y];
    for (int x = 0; x < imagewidth; x++) {
      pptrDark[x] = 0;
    }
  }
  //-----------------------------------------------
  // collect histogram values
  for (int y = miny; y < maxy; y++) {
    uint8* pptr = (uint8*)lineptr[y];
    for (int x = minx; x < maxx; x++) {
      uint8 val8 = pptr[x];
      hist[val8]++;
    }
  }

  // for(int i = 0;i < HIST_SIZE;i++) Log(IntToStr(i) + "  " +
  // IntToStr(hist[i]));

  //-----------------------------------------------

  uint32 lowvalpixels = (uint32)((float32)pixcnt * darkRatio);

  // Log("*** lowvalpixels " + IntToStr(lowvalpixels));

  //-----------------------------------------------
  // find threshold for dark pixel value
  // Log(IntToStr(maxlowvalpixels));
  uint8 thresh = 0;
  uint32 sum = 0;
  for (int i = 0; i < HIST_SIZE - 1; i++) {
    sum += (uint8)hist[i];
    if (sum > lowvalpixels) {
      thresh = (uint8_t)i + 1;
      break;
    }
  }
  // Log("*** thresh " + IntToStr(thresh));
  // mark location of input image dark pixels onto the output image
  uint32 lowcnt = 0;
  for (int y = miny; y < maxy; y++) {
    uint8* pptr = (uint8*)lineptr[y];
    uint8* pptrDark = (uint8*)lineptrDark[y];
    for (int x = minx; x < maxx; x++) {
      uint8 val8 = pptr[x];
      if (val8 < thresh) {
        pptrDark[x] = 0xff;
        lowcnt++;
      } else
        pptrDark[x] = 0;
    }
  }
  // Log("*** lowcnt " + IntToStr(lowcnt));

  //
  // delete noisy pixels
  // and avg dark pixel values
  //
  uint16 avgDark = 0;
  uint32 sumDark = 0;
  lowcnt = 0;
  for (int y = miny; y < maxy; y++) {
    uint8* pptr = (uint8*)lineptr[y];  // for avg pixel value
    uint8* pptrDark = (uint8*)lineptrDark[y];
    uint8* pptrDark_below = (uint8*)lineptrDark[y + 1];
    for (int x = minx; x < maxx; x++) {
      uint8 val8 = pptrDark[x];
      uint8 val8_right = pptrDark[x + 1];
      uint8 val8_below = pptrDark_below[x];
      if (val8) {
        if (val8_right && val8_below) {
          lowcnt++;
          sumDark += pptr[x];  // for avg pixel value
        } else
          pptrDark[x] = 0;
      }
    }
  }
  //----------------------------------------------
  // Log(IntToStr(sumDark));
  // Log(IntToStr(lowcnt));
  if (lowcnt > 0) avgDark = static_cast<uint16_t>(sumDark / lowcnt);
  if (avgDark > 255) avgDark = 255;
  *avgDarkValue = static_cast<uint8_t>(avgDark);
  return lowcnt;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// variance (brightness contrast in ROI)
//
// 8 bit image input
float32 pvar(PLINE* hptr8, int16 imagewidth, int16 imageheight, int16 xc,
             int16 yc, int16 searchBox) {
  uint8 thresh = 30;
  int16 searchBoxRadius = searchBox >> 1;
  int16 minx = xc - searchBoxRadius;
  int16 maxx = minx + searchBox;
  int16 miny = yc - searchBoxRadius;
  int16 maxy = miny + searchBox;
  int16 stepsize = 1;
  // test for out-of-bounds coord xy
  if (minx < 0) return 0.0;
  if (miny < 0) return 0.0;
  if (maxx > imagewidth) return 0.0;
  if (maxy > imageheight) return 0.0;
  //------------------------------------------------------------------
  // find std dev for area
  uint32 cnt;
  uint32 sum;
  uint8* pptr;
  uint16 x;
  uint16 y;
  float32 avg;
  // use 8 bit image for avg pixel value
  cnt = 0;
  sum = 0;
  for (y = miny; y < maxy; y += stepsize) {
    pptr = (uint8*)hptr8[y];
    for (x = minx; x < maxx; x += stepsize) {
      if (pptr[x] < thresh) {
        sum += pptr[x];
        cnt++;
      }
    }
  }
  avg = (float32)sum / (float32)cnt;
  // get sum diff^2
  int32 diff;
  int32 diff2;
  cnt = 0;
  sum = 0;
  for (y = miny; y < maxy; y += stepsize) {
    pptr = (uint8*)hptr8[y];
    for (x = minx; x < maxx; x += stepsize) {
      if (pptr[x] < thresh) {
        diff = (int32)(pptr[x] - avg);
        diff2 = diff * diff;
        sum += diff2;
        cnt++;
      }
    }
  }
  // get avg of sum diff^2
  float variance = (float32)sum / (float32)cnt;
  return variance;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//
//
void vert_gradient(PLINE* lineptr, uint16 width, uint16 height,
                   PLINE* lineptrGrad) {
  // roi
  uint16 xstart = 1;
  uint16 xend = width - 1;
  uint16 ystart = 1;
  uint16 yend = height - 1;
  //
  uint8* pptrAbove;
  uint8* pptrBelow;
  int8* pptrGrad;
  uint16 x, y;
  uint8 valAbove, valBelow;
  int32 grad;
  //
  for (y = ystart; y < yend; y++) {
    pptrAbove = (uint8*)lineptr[y - 1];
    pptrBelow = (uint8*)lineptr[y + 1];
    pptrGrad = (int8*)lineptrGrad[y];
    for (x = xstart; x < xend; x++) {
      // center pixel pptr[x] not used
      // grad = diff of 2 neighbors
      valAbove = (uint8)pptrAbove[x];
      valBelow = (uint8)pptrBelow[x];
      //
      grad = valAbove - valBelow;
      //
      // clip grad range
      //
      // clip to grad size = 8 bits
      // clip value to <= 0x7f if signed
      // clip value to <= 0xff if unsigned
      //
      // signed range: -128 to 127
      //
      if (grad > 127)
        grad = 127;
      else if (grad < -128)
        grad = -128;
      //
      pptrGrad[x] = (int8_t)grad;
      //
    }
  }
}
//
void hor_gradient(PLINE* lineptr, uint16 width, uint16 height,
                  PLINE* lineptrGrad) {
  // roi
  uint16 xstart = 1;
  uint16 xend = width - 1;
  uint16 ystart = 1;
  uint16 yend = height - 1;
  //
  uint8* pptr;
  int8* pptrGrad;
  uint16 x, y;
  uint8 valLeft, valRight;
  int32 grad;
  //
  for (y = ystart; y < yend; y++) {
    pptr = (uint8*)lineptr[y];
    pptrGrad = (int8*)lineptrGrad[y];
    for (x = xstart; x < xend; x++) {
      // center pixel pptr[x] not used
      // grad = diff of 2 neighbors
      valLeft = (uint8)pptr[x - 1];
      valRight = (uint8)pptr[x + 1];
      //
      grad = valLeft - valRight;
      //
      // clip grad range
      //
      // clip to grad size = 8 bits
      // clip value to <= 0x7f if signed
      // clip value to <= 0xff if unsigned
      //
      // signed range: -128 to 127
      //
      if (grad > 127)
        grad = 127;
      else if (grad < -128)
        grad = -128;
      //
      pptrGrad[x] = (int8_t)grad;
      //
    }
  }
}
//
// inverted gradient for finding the specularity
// using the pupil and iris circle matching routines
void vert_gradient_inverted(PLINE* lineptr, uint16 width, uint16 height,
                            PLINE* lineptrGrad) {
  // roi
  uint16 xstart = 1;
  uint16 xend = width - 1;
  uint16 ystart = 1;
  uint16 yend = height - 1;
  //
  uint8* pptrAbove;
  uint8* pptrBelow;
  int8* pptrGrad;
  uint16 x, y;
  uint8 valAbove, valBelow;
  int32 grad;
  //
  for (y = ystart; y < yend; y++) {
    pptrAbove = (uint8*)lineptr[y - 1];
    pptrBelow = (uint8*)lineptr[y + 1];
    pptrGrad = (int8*)lineptrGrad[y];
    for (x = xstart; x < xend; x++) {
      // center pixel pptr[x] not used
      // grad = diff of 2 neighbors
      valAbove = (uint8)pptrAbove[x];
      valBelow = (uint8)pptrBelow[x];
      //
      // inverted grad = valAbove - valBelow;
      grad = valBelow - valAbove;
      //
      // clip grad range
      //
      // clip to grad size = 8 bits
      // clip value to <= 0x7f if signed
      // clip value to <= 0xff if unsigned
      //
      // signed range: -128 to 127
      //
      if (grad > 127)
        grad = 127;
      else if (grad < -128)
        grad = -128;
      //
      pptrGrad[x] = (int8_t)grad;
      //
    }
  }
}
//
// inverted gradient for finding the specularity
// using the pupil and iris circle matching routines
void hor_gradient_inverted(PLINE* lineptr, uint16 width, uint16 height,
                           PLINE* lineptrGrad) {
  // roi
  uint16 xstart = 1;
  uint16 xend = width - 1;
  uint16 ystart = 1;
  uint16 yend = height - 1;
  //
  uint8* pptr;
  int8* pptrGrad;
  uint16 x, y;
  uint8 valLeft, valRight;
  int32 grad;
  //
  for (y = ystart; y < yend; y++) {
    pptr = (uint8*)lineptr[y];
    pptrGrad = (int8*)lineptrGrad[y];
    for (x = xstart; x < xend; x++) {
      // center pixel pptr[x] not used
      // grad = diff of 2 neighbors
      valLeft = (uint8)pptr[x - 1];
      valRight = (uint8)pptr[x + 1];
      //
      // inverted grad = valLeft - valRight;
      grad = valRight - valLeft;
      //
      // clip grad range
      //
      // clip to grad size = 8 bits
      // clip value to <= 0x7f if signed
      // clip value to <= 0xff if unsigned
      //
      // signed range: -128 to 127
      //
      if (grad > 127)
        grad = 127;
      else if (grad < -128)
        grad = -128;
      //
      pptrGrad[x] = (int8_t)grad;
      //
    }
  }
}

//
void hor_gradient_clipped(PLINE* lineptr, uint16 width, uint16 height,
                          int32 maxgrad, int32 mingrad, PLINE* lineptrGrad) {
  // roi
  uint16 xstart = 1;
  uint16 xend = width - 1;
  uint16 ystart = 1;
  uint16 yend = height - 1;
  //
  uint8* pptr;
  int8* pptrGrad;
  uint16 x, y;
  uint8 valLeft, valRight;
  int32 grad;
  //
  for (y = ystart; y < yend; y++) {
    pptr = (uint8*)lineptr[y];
    pptrGrad = (int8*)lineptrGrad[y];
    for (x = xstart; x < xend; x++) {
      // center pixel pptr[x] not used
      // grad = diff of 2 neighbors
      valLeft = (uint8)pptr[x - 1];
      valRight = (uint8)pptr[x + 1];
      //
      grad = valLeft - valRight;
      //
      // clip grad range
      //
      // 1. clip to limit Hough contribution of large grad
      // 2. clip to grad size = 8 bits
      // clip value to <= 0x7f if signed
      // clip value to <= 0xff if unsigned
      // 3. clip to make the grad image low values easier to view
      //
      if (grad > 0) {
        if (grad > maxgrad)
          grad = maxgrad;
        else if (grad < mingrad)
          grad = 0;
      } else {
        if (grad < -maxgrad)
          grad = -maxgrad;
        else if (grad > -mingrad)
          grad = 0;
      }
      //
      // if(grad > IRISFIND_MAX_GRADIENT) grad = IRISFIND_MAX_GRADIENT;
      // else if(grad < -IRISFIND_MAX_GRADIENT) grad = -IRISFIND_MAX_GRADIENT;
      //
      pptrGrad[x] = (int8_t)grad;
      //
    }
  }
}
//
void vert_gradient(PLINE* lineptr, uint16 width, uint16 height, int32 maxgrad,
                   int32 mingrad, PLINE* lineptrGrad) {
  // roi
  uint16 xstart = 1;
  uint16 xend = width - 1;
  uint16 ystart = 1;
  uint16 yend = height - 1;
  //
  uint8* pptrAbove;
  uint8* pptrBelow;
  int8* pptrGrad;
  uint16 x, y;
  uint8 valAbove, valBelow;
  int32 grad;
  //
  for (y = ystart; y < yend; y++) {
    pptrAbove = (uint8*)lineptr[y - 1];
    pptrBelow = (uint8*)lineptr[y + 1];
    pptrGrad = (int8*)lineptrGrad[y];
    for (x = xstart; x < xend; x++) {
      // center pixel pptr[x] not used
      // grad = diff of 2 neighbors
      valAbove = (uint8)pptrAbove[x];
      valBelow = (uint8)pptrBelow[x];
      //
      grad = valAbove - valBelow;
      //
      // clip grad range
      //
      // 1. clip to limit Hough contribution of large grad
      // 2. clip to grad size = 8 bits
      // clip value to <= 0x7f if signed
      // clip value to <= 0xff if unsigned
      // 3. clip to make the grad image low values easier to view
      //
      if (grad > 0) {
        if (grad > maxgrad)
          grad = maxgrad;
        else if (grad < mingrad)
          grad = 0;
      } else {
        if (grad < -maxgrad)
          grad = -maxgrad;
        else if (grad > -mingrad)
          grad = 0;
      }
      //
      // if(grad > IRISFIND_MAX_GRADIENT) grad = IRISFIND_MAX_GRADIENT;
      // else if(grad < -IRISFIND_MAX_GRADIENT) grad = -IRISFIND_MAX_GRADIENT;
      //
      pptrGrad[x] = (int8_t)grad;
      //
    }
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// copy one 8 bit buffer to another
// using line pointers in case one of the buffers is a crop-in-place
//
void copy_buf(PLINE* lineptrin, uint16 width, uint16 height,
              PLINE* lineptrout) {
  uint8* pptrin;
  uint8* pptrout;
  //  uint16 x;
  uint16 y;
  for (y = 0; y < height; y++) {
    pptrin = (uint8*)lineptrin[y];
    pptrout = (uint8*)lineptrout[y];
    //
    memcpy(pptrout, pptrin, width);
    //
    /*
    for(x = 0;x < width;x++){
            pptrout[x] = pptrin[x];
    }
    */
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
void test_buf_add_noise(uint8* buf, uint32 size, uint32 rmax, uint32 seed) {
  uint8 rmax2 = (uint8_t)rmax >> 1;
  srand(seed);  // initialize random seed
  for (uint32 i = 0; i < size; i++) {
    uint8 rval =
        (uint8)((rand() % rmax));  // generate random number between 0 and rmax
    int16 ival = rmax2 - rval;
    int16 val = (uint16)buf[i] + ival;
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    buf[i] = (uint8)val;
  }
}
//
void test_buf_add_noise_buf(uint8* buf1, uint8* bufnoise, uint32 size,
                            uint8* bufout) {
  for (uint32 i = 0; i < size; i++) {
    int16 val = ((uint16)buf1[i] + (uint16)bufnoise[i]) - 128;
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    bufout[i] = (uint8)val;
  }
}
//
void test_buf_subtract(uint8* buf1, uint8* buf2, uint32 size, uint8* bufout) {
  for (uint32 i = 0; i < size; i++) {
    uint16 val = (128 + (uint16)buf1[i]) - (uint16)buf2[i];
    bufout[i] = (uint8)val;
  }
}
//
void test_buf_val(PLINE* hptr, uint16 width, uint16 height, uint8 val) {
  uint8* pptr;
  uint16 x;
  uint16 y;
  for (y = 0; y < height; y++) {
    pptr = (uint8*)hptr[y];
    for (x = 0; x < width; x++) {
      pptr[x] = val;
    }
  }
}
//
void test_buf_16_val(PLINE* hptr, uint16 width, uint16 height, uint16 val) {
  uint16* pptr;
  uint16 x;
  uint16 y;
  for (y = 0; y < height; y++) {
    pptr = (uint16*)hptr[y];
    for (x = 0; x < width; x++) {
      pptr[x] = val;
    }
  }
}
//
void test_buf_random(PLINE* hptr, uint16 width, uint16 height, uint32 seed) {
  // srand(time(NULL)); // initialize random seed
  srand(seed);  // initialize random seed
  uint8* pptr;
  uint16 x;
  uint16 y;
  for (y = 0; y < height; y++) {
    pptr = (uint8*)hptr[y];
    for (x = 0; x < width; x++) {
      // pptr[x] = (uint8)(rand() % 255 + 0); // generate random number between
      // 0 and 255
      pptr[x] = (uint8)(rand());  // generate random number between 0 and 255
    }
  }
}
//
void test_buf_add_vert_stripe(PLINE* hptr, uint16 width, uint16 height) {
  uint8* pptr;
  uint16 x;
  uint16 y;
  for (y = 0; y < height; y++) {
    pptr = (uint8*)hptr[y];
    for (x = width / 2; x < width / 2 + 4; x++) {
      pptr[x] = 0x00;
    }
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//
/*

Note on converting to 32 bit buf[] locations:

        // 32 bit buf - eyefind.bufInt[], or eyefind.lineptrInt[y]
        // offset buf ptr to xy location
        uint32* data = (uint32*)(eyefind.lineptrInt[y] + (x << 2)); // 4*x for
32 bit word index uint32* data = (uint32*)&eyefind.bufInt[(y * width) + x]; //
offset buf ptr to xy location uint32* data = (uint32*)&eyefind.bufInt[0] + (y *
width) + x; // ? why does this work ?

*/
//
// create array of pointers to the buffer image lines
// byte buffer only
void get_lineptrs_8(uint8* buf, uint16 width, uint16 height, PLINE* lineptr) {
  PLINE nextptr = (PLINE)buf;
  uint16 i;
  for (i = 0; i < height; i++) {
    lineptr[i] = nextptr;
    nextptr += width;
  }
}
//
// create array of pointers to the buffer image lines
// 16 bit buffer only
void get_lineptrs_16(uint16* buf, uint16 width, uint16 height, PLINE* lineptr) {
  uint16 ystep = width << 1;
  PLINE nextptr = (PLINE)buf;
  uint16 i;
  for (i = 0; i < height; i++) {
    lineptr[i] = nextptr;
    nextptr += ystep;
  }
}
//
// create array of pointers to the buffer image lines
// 32 bit buffer only
void get_lineptrs_32(uint32* buf, uint16 width, uint16 height, PLINE* lineptr) {
  uint16 ystep = width << 2;
  PLINE nextptr = (PLINE)buf;
  uint16 i;
  for (i = 0; i < height; i++) {
    lineptr[i] = nextptr;
    nextptr += ystep;
  }
}
//
// create array of pointers to the buffer image lines in a cropped area
// byte buffer only
void get_lineptrs_crop(uint8* buf, uint16 bufwidth, uint16 x0, uint16 y0,
                       uint16 cropheight, PLINE* lineptr) {
  PLINE nextptr = (PLINE)buf + ((y0 * bufwidth) + x0);
  uint16 i;
  for (i = 0; i < cropheight; i++) {
    lineptr[i] = nextptr;
    nextptr += bufwidth;
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// 3rd level pramid copied directly from the top level image
//
void pyro4(PLINE* hptr, uint16 width, uint16 height, PLINE* phptr) {
  uint8* pptr;
  // uint8* fptr;
  uint16 x;
  uint16 y;
  uint8* pyrpptr;
  uint16 pyry;
  uint16 pyrx;
  // uint16 xx;
  // uint16 yy;
  // uint32 sum;
  pyry = 0;
  for (y = 2; y < height; y += 4) {
    pptr = (uint8*)hptr[y];
    pyrpptr = (uint8*)phptr[pyry];
    pyry++;
    pyrx = 0;
    for (x = 2; x < width; x += 4) {
      /*
      sum = 0;
      // using filter
      for(yy = y - 1;yy <= y + 1;yy++){
              fptr = (uint8*)hptr[yy];
              for(xx = x - 1;xx <= x + 1;xx++){
                      sum += fptr[xx]; // sum 9 pixels
              }
      }
      //
      // remove center pixel
      sum -= pptr[x]; // 9 - 1 = sum 8 neighbor pixels
      sum = sum >> 3; // avg 8 pixels
      //
      // filter 8
      // no center pixel
      pyrpptr[pyrx] = (uint8)sum; // avg 8 neighbor pixels
      //
      // filter 9
      // avg center pixel and 8 neighbor pixels
      //sum += pptr[x];
      //pyrpptr[pyrx] = (uint8)(sum >> 1);
      //
      */
      // no filter
      // center pixel only, no filter
      pyrpptr[pyrx] = pptr[x];
      //
      pyrx++;
      //
    }
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// create line pointers to a smaller area in a larger image
//
void crop_in_place(uint8* pSrcImg, uint16 width, uint16 height, uint16 xc,
                   uint16 yc, uint16 cropWidth, uint16 cropHeight,
                   PLINE* lineptrCrop) {
  uint16 i;
  uint16 x0, y0;
  uint16 halfwidth = cropWidth >> 1;
  uint16 halfheight = cropHeight >> 1;

  // test for out of bounds crop
  if (xc > halfwidth)
    x0 = xc - halfwidth;
  else
    x0 = 0;
  if ((width - xc) <= halfwidth) x0 = width - 1 - cropWidth;
  if (yc > halfheight)
    y0 = yc - halfheight;
  else
    y0 = 0;
  if ((height - yc) <= halfheight) y0 = height - 1 - cropHeight;

  // crop to lineptr array
  uint8* pptr = pSrcImg + (y0 * width) + x0;

  for (i = 0; i < cropHeight; i++) {
    lineptrCrop[i] = pptr;
    pptr += width;
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void crop_out(uint8* pSrcImg, uint16 width, uint16 height, uint16 xc, uint16 yc,
              uint16 cropWidth, uint16 cropHeight, uint8* pptrCrop,
              int Rotate) {
  int column, offset;
  int nextColumn;
  uint16 x0, y0;

  if (Rotate == 90 || Rotate == 270) {
    x0 = cropWidth;
    cropWidth = cropHeight;
    cropHeight = x0;
  }

  uint16 halfwidth = cropWidth >> 1;
  uint16 halfheight = cropHeight >> 1;
  uint16 rowStep = width - cropWidth;
  int cropSize = cropWidth * cropHeight;

  // test for out of bounds crop
  if (xc > halfwidth)
    x0 = xc - halfwidth;
  else
    x0 = 0;
  if ((width - xc) <= halfwidth) x0 = width - 1 - cropWidth;
  if (yc > halfheight)
    y0 = yc - halfheight;
  else
    y0 = 0;
  if ((height - yc) <= halfheight) y0 = height - 1 - cropHeight;

  const uint8* pptr = pSrcImg + (y0 * width) + x0;

  if (Rotate == 90) {
    for (column = cropHeight - 1; column >= 0; column--, pptr += rowStep)
      for (offset = column; offset < cropSize; offset += cropHeight)
        *(pptrCrop + offset) = *pptr++;
  } else if (Rotate == 180) {
    for (column = cropSize - 1; column > 0;
         column = nextColumn, pptr += rowStep) {
      nextColumn = column - cropWidth;
      for (offset = column; offset > nextColumn; offset--)
        *(pptrCrop + offset) = *pptr++;
    }
  } else if (Rotate == 270) {
    for (column = cropSize - cropHeight; column < cropSize;
         column++, pptr += rowStep)
      for (offset = column; offset >= 0; offset -= cropHeight)
        *(pptrCrop + offset) = *pptr++;
  } else /* Rotate == 0 */
  {
    for (column = 0; column < cropSize; column = nextColumn, pptr += rowStep) {
      nextColumn = column + cropWidth;
      for (offset = column; offset < nextColumn; offset++)
        *(pptrCrop + offset) = *pptr++;
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// threshold with greater-than and less-than values
//
void thresh16LtGt(PLINE* hptr, uint16 width, uint16 height, uint16 lowthresh,
                  uint16 lowthreshval, uint16 hithresh, uint16 hithreshval) {
  uint16* pptr;
  uint16 x;
  uint16 y;
  for (y = 0; y < height; y++) {
    pptr = (uint16*)hptr[y];
    for (x = 0; x < width; x++) {
      if (*pptr > hithresh)
        *pptr = hithreshval;
      else if (*pptr < lowthresh)
        *pptr = lowthreshval;
      *pptr = 0;
      pptr++;
    }
  }
}

void integralImage(PLINE* hptr, uint16 width, uint16 height, PLINE* ihptr) {
  // vars

  uint8* pptr;
  uint32* ipptr;
  uint32* ipptrAbove;
  uint16 x, y, prevx;
  uint32 sum;

  //
  // first integral row and column are = 0
  //
  // integral image has an extra column and row
  ipptr = (uint32*)ihptr[0];
  for (x = 0; x <= width; x++) {
    ipptr[x] = 0;
  }
  for (y = 0; y <= height; y++) {
    ipptr = (uint32*)ihptr[y];
    ipptr[0] = 0;
  }
  //
  // pixel at (0, 0)
  // integral image (x, y) is at (x + 1, y + 1)
  //
  x = 0;
  y = 0;
  pptr = (uint8*)hptr[y];
  ipptr = (uint32*)ihptr[y + 1];
  sum = (uint8)pptr[x];
  ipptr[x + 1] = (uint32)sum;
  // top row
  for (x = 1; x < width; x++) {
    sum += pptr[x];
    ipptr[x + 1] = sum;
  }
  // whole frame
  // extra row in integral image
  for (y = 1; y < height; y++) {
    pptr = (uint8*)hptr[y];
    ipptr = (uint32*)ihptr[y + 1];
    ipptrAbove = (uint32*)ihptr[y];  // row above
    // first column
    x = 0;
    sum = (uint8)pptr[x];
    sum += (uint32)ipptrAbove[x + 1];
    ipptr[x + 1] = (uint32)sum;
    prevx = 0;
    // extra column in integral image
    for (x = 1; x < width; x++) {
      sum += (uint8)pptr[x];
      sum += (uint32)ipptrAbove[x + 1];
      sum -= (uint32)ipptrAbove[prevx + 1];  // subtract last to keep sum
                                             // positive, or use int for sum
      prevx++;
      ipptr[x + 1] = (uint32)sum;
    }
  }
}
