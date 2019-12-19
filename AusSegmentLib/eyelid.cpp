#include "eyelid.h"

void ek_eyelid_session_init(Eyelid_struct& eyelid, bool enable_eyelid_detect) {
  // test flags

  eyelid.test_useSpecMask = true;
  eyelid.test_useEyelidMask = true;

  // set up buffers

  // eyelid buffers are the same size as flat iris image
  eyelid.width = FLAT_IMAGE_WIDTH;
  eyelid.height = FLAT_IMAGE_HEIGHT;

  // flat iris mask
  get_lineptrs_8((uint8*)eyelid.bufMask[0], eyelid.width, eyelid.height,
                 eyelid.lineptrMask[0]);
  get_lineptrs_8((uint8*)eyelid.bufMask[1], eyelid.width, eyelid.height,
                 eyelid.lineptrMask[1]);

  eyelid.enable_eyelid_detect = enable_eyelid_detect;
}
//
void ek_eyelid_init(Eyelid_struct& eyelid) {
  // init variables

  // settings

  // TBD: adjust with contrast and lighting

  // TBD: adjust with zoom

  // eyelid mask, fixed template
  int16 lowerEyelidHeight = EYELID_LOWER_EYELID_HEIGHT_DEFAULT;
  int16 upperEyelidHeight = EYELID_UPPER_EYELID_HEIGHT_DEFAULT;
  draw_two_eyelid_circles(eyelid.lineptrMask[0], eyelid.width, eyelid.height,
                          lowerEyelidHeight, upperEyelidHeight);
  draw_two_eyelid_circles(eyelid.lineptrMask[1], eyelid.width, eyelid.height,
                          lowerEyelidHeight, upperEyelidHeight);

  //
  // eyelid mask test
  //

  eyelid.coverage[0] = 0;
  eyelid.coverage[1] = 0;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//
//
void flattenIris(PLINE* lineptrEyeCrop, uint16 eyewidth, uint16 eyeheight,
                 struct PointXYZ_float* iris, struct PointXYZ_float* pupil,
                 PLINE* lineptrFlat, uint16 flatwidth, uint16 flatheight,
                 float radiusSampling, float* cosTable, float* sinTable) {
  uint16 x, y;
  uint8 val8;
  uint8* flat_pptr;

  for (y = 0; y < flatheight; y++) {
    float rad = y * radiusSampling;
    float offx = (rad * iris->x) + ((1.0f - rad) * pupil->x);
    float offy = (rad * iris->y) + ((1.0f - rad) * pupil->y);
    float iprad = (rad * iris->z) + ((1.0f - rad) * pupil->z);
    //
    flat_pptr = (uint8*)lineptrFlat[y];
    //
    for (x = 0; x < flatwidth; x++) {
      // float angle = j * angleSampling;
      float xx = offx + (iprad * cosTable[x]);
      float yy = offy + (iprad * sinTable[x]);

      // use bilinear interpolation
      int intx = (int)floor(xx);
      int inty = (int)floor(yy);
      float yfrac = yy - (float)inty;
      unsigned char* fptr1 = (uint8*)lineptrEyeCrop[inty];
      unsigned char* fptr2 = (uint8*)lineptrEyeCrop[inty + 1];

      // decreased w and h limits by -1 to account for
      // [intx + 1] and [inty + 1]
      if (((unsigned int)intx < (unsigned int)(eyewidth - 1)) &&
          ((unsigned int)inty < (unsigned int)(eyeheight - 1))) {
        float xfrac = xx - (float)intx;
        float y1val =
            (float)fptr1[intx] + (xfrac * (fptr1[intx + 1] - fptr1[intx]));
        float y2val =
            (float)fptr2[intx] + (xfrac * (fptr2[intx + 1] - fptr2[intx]));
        //*outC1 = (unsigned char)(round(y1val + yfrac*(y2val-y1val)));
        // round() for positive numbers = floor(i + 0.5)
        // val8 = (unsigned char)floor((y1val + yfrac*(y2val - y1val)) + 0.5);
        val8 = (unsigned char)(y1val + (yfrac * (y2val - y1val)) + 0.5);
        flat_pptr[x] = val8;
      }
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//
// draw_horizontal_line(lineptr, width, height, x1, x2, y0);
bool draw_horizontal_line(PLINE* lineptr, int16 width, int16 height, int16 x1,
                          int16 x2, int16 y) {
  if ((x2 >= x1) && (x2 < width) && (x1 >= 0) && (y < height) && (y >= 0)) {
    uint8* pptr = (uint8*)lineptr[y];
    for (uint16 x = x1; x <= x2; x++) {
      pptr[x] = 0xff;
    }
    return 1;
  }
  return 0;
}
//
uint16 circlePoints(uint16 x, uint16 y, struct PointXY_signed* pos) {
  uint16 i = 0;
  if (x == 0) {
    pos[i].x = y;
    pos[i].y = 0;
    i++;
  } else if (x == y) {
    pos[i].x = x;
    pos[i].y = -y;
    i++;
  } else if (x < y) {
    pos[i].x = x;
    pos[i].y = -y;
    i++;
    pos[i].x = y;
    pos[i].y = -x;
    i++;
  }
  return i;
}
//
void draw_one_eyelid_circle(PLINE* lineptr, int16 width, int16 height, int16 xc,
                            int16 eyelidHeight) {
  uint32 radius = 140;
  // int16 xc = 150;
  int16 yc = static_cast<int16_t>(64 + radius - eyelidHeight);
  struct PointXY_signed pos[4];
  int16 x1, x2, y0;
  uint16 x = 0;
  uint16 y = (uint16_t)radius;
  float32 p = (float32)((5.0 - ((float32)radius * 4.0)) / 4.0);
  uint16 cnt = 0;
  uint16 i = 0;
  cnt = circlePoints(x, y, pos);
  // Log("cnt" + IntToStr(cnt);
  for (i = 0; i < cnt; i++) {
    x1 = -pos[i].x + xc;
    x2 = pos[i].x + xc;
    y0 = pos[i].y + yc;
    // Log(IntToStr(x1) +", " + IntToStr(y0) + "    " + IntToStr(x2) +", " +
    // IntToStr(y0));
    draw_horizontal_line(lineptr, width, height, x1, x2, y0);
  }
  while (x < y) {
    x++;
    if (p < 0.0)
      p += (float32)((2.0 * (float32)x) + 1.0);
    else {
      y--;
      p += (float32)(2.0 * ((float32)x - (float32)y) + 1.0);
    }
    cnt = circlePoints(x, y, pos);
    // Log("");
    for (i = 0; i < cnt; i++) {
      x1 = -pos[i].x + xc;
      x2 = pos[i].x + xc;
      y0 = pos[i].y + yc;
      // Log(IntToStr(x1) +", " + IntToStr(y0) + "    " + IntToStr(x2) +", " +
      // IntToStr(y0));
      draw_horizontal_line(lineptr, width, height, x1, x2, y0);
    }
  }
}
//
void draw_two_eyelid_circles(PLINE* lineptr, int16 width, int16 height,
                             int16 heightLowerEyelid, int16 heightUpperEyelid) {
  if (heightLowerEyelid >= height) heightLowerEyelid = height - 1;
  if (heightUpperEyelid >= height) heightUpperEyelid = height - 1;
  if (heightLowerEyelid < EYELID_LOWER_EYELID_HEIGHT_MIN)
    heightLowerEyelid = EYELID_LOWER_EYELID_HEIGHT_MIN;
  if (heightUpperEyelid < EYELID_UPPER_EYELID_HEIGHT_MIN)
    heightUpperEyelid = EYELID_UPPER_EYELID_HEIGHT_MIN;

  // clear the mask
  memset((void*)lineptr[0], 0, (width * height));

  // xc, horizontal center of lower eyelid circle
  int16 xc = (width >> 2);
  draw_one_eyelid_circle(lineptr, width, height, xc, heightLowerEyelid);

  // xc, horizontal center of upper eyelid circle
  xc = xc + (width >> 1);
  draw_one_eyelid_circle(lineptr, width, height, xc, heightUpperEyelid);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// add threshold of image to mask
//
float32 addThreshMask(PLINE* lineptrImage, uint16 width, uint16 height,
                      uint8 thresh, PLINE* lineptrMask) {
  float32 percent_over_thresh = 0.0;
  uint32 spec_pcnt = 0;
  uint16 x, y;
  uint8 val8, maskval;
  uint8* image_pptr;
  uint8* mask_pptr;
  // bool setmask;
  for (y = 0; y < height; y++) {
    image_pptr = (uint8*)lineptrImage[y];
    mask_pptr = (uint8*)lineptrMask[y];
    for (x = 0; x < width; x++) {
      maskval = mask_pptr[x];
      // after eyelid has been added to the mask
      // ignore eyelid pixels to get a spec pixel count
      if (maskval == 0) {
        val8 = image_pptr[x];
        // add pixels over thresh to existing mask
        if (val8 > thresh) {
          mask_pptr[x] = 0xff;
          spec_pcnt++;
        }
      }
      /*
      // write to every mask pixel for a new (uninitialized) mask
      setmask = 0;
      if(val8 > thresh) setmask = 1;
      mask_pptr[x] = setmask ? 0xff : 0x00;
      */
    }
  }

  uint32 pcnt = width * height;
  if (pcnt > 0)
    percent_over_thresh = (float)(100.0 * ((float32)spec_pcnt / (float32)pcnt));

  return percent_over_thresh;
}
//
//
// add eyelid mask to iris mask
//
void addEyelidMask(PLINE* lineptrEyelidMask, uint16 width, uint16 height,
                   PLINE* lineptrIrisMask) {
  uint16 x, y;
  uint8 val8;
  uint8* eyelidMask_pptr;
  uint8* mask_pptr;
  for (y = 0; y < height; y++) {
    eyelidMask_pptr = (uint8*)lineptrEyelidMask[y];
    mask_pptr = (uint8*)lineptrIrisMask[y];
    for (x = 0; x < width; x++) {
      val8 = eyelidMask_pptr[x];
      //
      // add thresh to existing mask
      if (val8 > 0) mask_pptr[x] = 0xff;
      //
    }
  }
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//
//
//
float createEyelidMask(Eyelid_struct& eyelid, PLINE* lineptrIris, uint16 width,
                       uint16 height, PLINE* lineptrIrisMask) {
  double coverage[3];

  if (eyelid.enable_eyelid_detect) {
    make_eyelid_mask(lineptrIris[0], width, height, lineptrIrisMask[0],
                     coverage);
  } else {
    copy_buf(eyelid.lineptrMask[0], width, height, lineptrIrisMask);
  }

  return (float)coverage[0]; 
}
//
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//
//
//
uint8 iris_brightness(PLINE* lineptrIris, PLINE* lineptrIrisMask, uint16 width,
                      uint16 height) {
  uint16 x, y;
  uint8 avg = 0;
  uint32 sum = 0;
  uint32 pcnt = 0;
  for (y = 0; y < height / 2; y++) {  // optional, use top half of flat iris
    uint8* image_pptr = (uint8*)lineptrIris[y];
    uint8* mask_pptr = (uint8*)lineptrIrisMask[y];
    for (x = 0; x < width; x++) {
      if (mask_pptr[x] == 0) {
        sum += image_pptr[x];
        pcnt++;
      }
    }
  }
  if (pcnt > 0) avg = (uint8)(((float)sum / (float)pcnt) + 0.5f);
  return avg;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
//
//
/*

        Input:
        eyefind.lineptrCrop[i], eyefind.cropWidth, eyefind.cropHeight,

        eyefind.cropWidth = irisfind.width
        eyefind.cropHeight = irisfind.height


           irisfind.result[eyeCropIndex].irisFound;
           irisfind.result[eyeCropIndex].irisPos.x;
           irisfind.result[eyeCropIndex].irisPos.y;
           irisfind.result[eyeCropIndex].irisPos.z; // radius

           irisfind.result[eyeCropIndex].pupilFound;
           irisfind.result[eyeCropIndex].pupilPos.x;
           irisfind.result[eyeCropIndex].pupilPos.y;
           irisfind.result[eyeCropIndex].pupilPos.z; // radius


*/
//

TemplatePipelineError ek_eyelid_main(PLINE* lineptrCrop, size_t eyecrop_width,
                    size_t eyecrop_height, size_t flat_iris_width,
                    size_t flat_iris_height, Irisfind* irisfind,
                    Eyelid_struct& eyelid, PLINE* lineptrFlat,
                    PLINE* lineptrMask, float radiusSampling, float* cosTable,
                    float* sinTable) {

  if (irisfind->irisFound[0]) {
    flattenIris(lineptrCrop, eyecrop_width, eyecrop_height,
                &irisfind->irisPos[0], &irisfind->pupilPos[0], lineptrFlat,
                flat_iris_width, flat_iris_height, radiusSampling, cosTable,
                sinTable);

    eyelid.coverage[0] = createEyelidMask(eyelid, lineptrFlat, flat_iris_width,
                                          flat_iris_height, lineptrMask);


  }else{
	  return TemplatePipelineError::Iris_Sclera_Boundary_Not_Found;
  }

  //
  // iris brightness
  //

  if (irisfind->irisFound[0]) {
    irisfind->brightnessValid = true;

    irisfind->irisBrightness = iris_brightness(
        lineptrFlat, lineptrMask, flat_iris_width, flat_iris_height);
    irisfind->pupilBrightness = irisfind->pupilBrightnessLR[0];
    irisfind->pupilEdge = irisfind->pupilScore[0];
  }else{
	  return TemplatePipelineError::Iris_Sclera_Boundary_Not_Found;
  }
  //

  //
  // eye focus estimate
  //

  if (irisfind->brightnessValid) {
    float irisPupilStepSize =
        (float)irisfind->irisBrightness - (float)irisfind->pupilBrightness;
    float irisPupilEdge_normalized =
        (float)irisfind->pupilEdge / irisPupilStepSize;

    irisfind->focus_unfiltered = irisPupilEdge_normalized / 5.0f;
  }

  return TemplatePipelineError::Segmentation_Successful;
}


