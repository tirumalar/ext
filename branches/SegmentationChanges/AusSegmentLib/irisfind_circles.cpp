#include "compiler.h"
#include "eyelid.h"
#include "helper.h"
#include "irisfind.h"

#include <math.h>  // for sqrtf()

#define STEP_TEST 1

uint8 make_xy_search_area(int16 width, int16 height, int16 xc, int16 yc,
                          int16 radius, int16 searchBox, int16* xmin,
                          int16* xmax, int16* ymin, int16* ymax) {
  // expand radius for safety margin
  radius++;
  // set limits
  int16 xLimitMin = radius;
  int16 yLimitMin = radius;
  int16 xLimitMax = width - radius - 1;
  int16 yLimitMax = height - radius - 1;
  int16 searchBoxRadius = (searchBox >> 1);
  // set range under test
  *xmin = xc - searchBoxRadius;
  *xmax = xc + searchBoxRadius;
  *ymin = yc - searchBoxRadius;
  *ymax = yc + searchBoxRadius;
  // detect violation and limit range
  uint8 withinBoundaries = 1;
  if (*xmin < xLimitMin) {
    *xmin = xLimitMin;
    withinBoundaries = 0;
  }
  if (*xmax > xLimitMax) {
    *xmax = xLimitMax;
    withinBoundaries = 0;
  }
  if (*ymin < yLimitMin) {
    *ymin = yLimitMin;
    withinBoundaries = 0;
  }
  if (*ymax > yLimitMax) {
    *ymax = yLimitMax;
    withinBoundaries = 0;
  }
  // return error if out of bounds
  // only the subpixel routines care if range was not allowed
  if (withinBoundaries == 0)
	  return 0;
  return 1;
}

uint16 collect_circumference_gradients_dark(PLINE* dLptr, int16 x, int16 y,
                                            struct Circle_struct* circleR,
                                            uint32* scoreptr, uint8 stepSize) {
  int32 i;
  int16 xr, yr, xx, yy;
  uint8 /*grad,*/ mask, dark;
  uint16 lutSize;
  PLINE* gptr = dLptr;
  struct PointXY_signed* clut;
  // dark mask
  // uses the dark area as a keepout
  // while gptr uses the dark area as a keep-in
  uint16 maskcnt = 0;
  //
  int16 maskStandoff = 2;  // distance from circle edge
  int16 darkStandoff = 2;
  //
  uint32 score = 0;
  clut = circleR->vTopRight;
  lutSize = circleR->vTopRightSize;
  for (i = 0; i < lutSize; i += stepSize) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    // grad = *((uint8*)gptr[yy] + xx);
    mask = *((uint8*)gptr[yy - maskStandoff] + xx);
    dark = *((uint8*)gptr[yy + darkStandoff] + xx);
    if (mask)
      maskcnt += 2;
    else if (dark) {
      score += 1;
      // if(grad) score += 1;
    }
    xx = x - xr;
    // grad = *((uint8*)gptr[yy] + xx);
    mask = *((uint8*)gptr[yy - maskStandoff] + xx);
    dark = *((uint8*)gptr[yy + darkStandoff] + xx);
    if (mask)
      maskcnt += 2;
    else if (dark) {
      score += 1;
      // if(grad) score += 1;
    }
  }
  clut = circleR->hRight;
  lutSize = circleR->hRightSize;
  for (i = 0; i < lutSize; i += stepSize) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    // grad = *((uint8*)gptr[yy] + xx);
    mask = *((uint8*)gptr[yy] + xx + maskStandoff);
    dark = *((uint8*)gptr[yy] + xx - darkStandoff);
    if (mask)
      maskcnt += 2;
    else if (dark) {
      score += 1;
      // if(grad) score += 1;
    }
    xx = x - xr;
    // grad = *((uint8*)gptr[yy] + xx);
    mask = *((uint8*)gptr[yy] + xx - maskStandoff);
    dark = *((uint8*)gptr[yy] + xx + darkStandoff);
    if (mask)
      maskcnt += 2;
    else if (dark) {
      score += 1;
      // if(grad) score += 1;
    }
  }
  clut = circleR->vBottomRight;
  lutSize = circleR->vBottomRightSize;
  for (i = 0; i < lutSize; i += stepSize) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    // grad = *((uint8*)gptr[yy] + xx);
    mask = *((uint8*)gptr[yy + maskStandoff] + xx);
    dark = *((uint8*)gptr[yy - darkStandoff] + xx);
    if (mask)
      maskcnt += 2;
    else if (dark) {
      score += 1;
      // if(grad) score += 1;
    }
    xx = x - xr;
    // grad = *((uint8*)gptr[yy] + xx);
    mask = *((uint8*)gptr[yy + maskStandoff] + xx);
    dark = *((uint8*)gptr[yy - darkStandoff] + xx);
    if (mask)
      maskcnt += 2;
    else if (dark) {
      score += 1;
      // if(grad) score += 1;
    }
  }
  //
  *scoreptr = score;
  return maskcnt;
}
//
//
// accumulate gradients on spec circle circumference
//
// Gradient threshold polarity is flipped compared to pupil and iris
// so we can detect a brighter area instead of a darker area.
//
uint16 collect_circumference_gradients_spec(PLINE* hGradLptr, PLINE* vGradLptr,
                                            int8 minGrad, int8 maxGrad, int16 x,
                                            int16 y,
                                            struct Circle_struct* circleR,
                                            uint32* scoreptr) {
  int32 i;
  int16 xr, yr, xx, yy;
  int8 grad;
  uint16 lutSize;
  PLINE* gptr;
  struct PointXY_signed* clut;
  uint32 score = 0;
  //
  clut = circleR->vTopRight;
  lutSize = circleR->vTopRightSize;
  gptr = vGradLptr;
  for (i = 0; i < lutSize; i++) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    grad = *((int8*)gptr[yy] + xx);
    if (grad <= -minGrad) {
      if (grad < -maxGrad) grad = -maxGrad;
      score += -grad;
    }
    xx = x - xr;
    grad = *((int8*)gptr[yy] + xx);
    if (grad <= -minGrad) {
      if (grad < -maxGrad) grad = -maxGrad;
      score += -grad;
    }
  }
  clut = circleR->hRight;
  lutSize = circleR->hRightSize;
  gptr = hGradLptr;
  for (i = 0; i < lutSize; i++) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    grad = *((int8*)gptr[yy] + xx);
    if (grad >= minGrad) {
      if (grad > maxGrad) grad = maxGrad;
      score += grad;
    }
    xx = x - xr;
    grad = *((int8*)gptr[yy] + xx);
    if (grad <= -minGrad) {
      if (grad < -maxGrad) grad = -maxGrad;
      score += -grad;
    }
  }
  clut = circleR->vBottomRight;
  lutSize = circleR->vBottomRightSize;
  gptr = vGradLptr;
  for (i = 0; i < lutSize; i++) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    grad = *((int8*)gptr[yy] + xx);
    if (grad >= minGrad) {
      if (grad > maxGrad) grad = maxGrad;
      score += grad;
    }
    xx = x - xr;
    grad = *((int8*)gptr[yy] + xx);
    if (grad >= minGrad) {
      if (grad > maxGrad) grad = maxGrad;
      score += grad;
    }
  }
  //
  *scoreptr = score;
  return 0;  // no maskcnt to return
}
//
//
// accumulate gradients on pupil and iris circle circumference
// with spec mask
//
// do not include masked edges in score
//
uint16 collect_circumference_gradients_pupil(PLINE* hGradLptr, PLINE* vGradLptr,
                                             PLINE* mLptr, int8 minGrad,
                                             int8 maxGrad, int16 x, int16 y,
                                             struct Circle_struct* circleR,
                                             uint32* scoreptr) {
  int32 i;
  int16 xr, yr, xx, yy;
  int8 grad;
  uint16 lutSize;
  PLINE* gptr;
  struct PointXY_signed* clut;
  // spec mask
  PLINE* mptr = mLptr;
  uint8 m;
  uint16 maskcnt = 0;
  uint32 score = 0;
  //
  clut = circleR->vTopRight;
  lutSize = circleR->vTopRightSize;
  gptr = vGradLptr;
  for (i = 0; i < lutSize; i++) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad >= minGrad) {
      if (grad > maxGrad) grad = maxGrad;
      score += grad;
    }
    xx = x - xr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad >= minGrad) {
      if (grad > maxGrad) grad = maxGrad;
      score += grad;
    }
  }
  clut = circleR->hRight;
  lutSize = circleR->hRightSize;
  gptr = hGradLptr;
  for (i = 0; i < lutSize; i++) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad <= -minGrad) {
      if (grad < -maxGrad) grad = -maxGrad;
      score += -grad;
    }
    xx = x - xr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad >= minGrad) {
      if (grad > maxGrad) grad = maxGrad;
      score += grad;
    }
  }
  clut = circleR->vBottomRight;
  lutSize = circleR->vBottomRightSize;
  gptr = vGradLptr;
  for (i = 0; i < lutSize; i++) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad <= -minGrad) {
      if (grad < -maxGrad) grad = -maxGrad;
      score += -grad;
    }
    xx = x - xr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad <= -minGrad) {
      if (grad < -maxGrad) grad = -maxGrad;
      score += -grad;
    }
  }
  //
  *scoreptr = score;
  return maskcnt;
}
//
// accumulate gradients on circle circumference
// with spec mask
//
// do not include masked edges in score
//
uint16 collect_circumference_gradients_iris(PLINE* hGradLptr,
                                            PLINE* /*vGradLptr*/, PLINE* mLptr,
                                            int8 minGrad, int8 maxGrad, int16 x,
                                            int16 y,
                                            struct Circle_struct* circleR,
                                            uint32* scoreptr) {
  int32 i;
  int16 xr, yr, xx, yy;
  int8 grad;
  uint16 lutSize;
  PLINE* gptr;
  struct PointXY_signed* clut;
  // spec mask
  PLINE* mptr = mLptr;
  uint8 m;
  uint16 maskcnt = 0;
  uint32 score = 0;
  //
  clut = circleR->hRight;
  lutSize = circleR->hRightSize;
  gptr = hGradLptr;
  for (i = 0; i < lutSize; i++) {
    xr = clut[i].x;
    yr = clut[i].y;
    xx = x + xr;
    yy = y + yr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad <= -minGrad) {
      if (grad < -maxGrad) grad = -maxGrad;
      score += -grad;  // * 8;
    }
    xx = x - xr;
    grad = *((int8*)gptr[yy] + xx);
    m = *((uint8*)mptr[yy] + xx);
    if (m) {
      maskcnt++;
    } else if (grad >= minGrad) {
      if (grad > maxGrad) grad = maxGrad;
      score += grad;  // * 8;
    }
  }
  //
  *scoreptr = score;
  return maskcnt;
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// find best fit in xy search areas for a circle of one radius
//
void match_circles(PLINE* hGradLptr, PLINE* vGradLptr, PLINE* mLptr,
                   PLINE* dLptr, int8 minGrad, int8 maxGrad,
                   struct Circle_struct* circleR, uint16 radius, uint16 xmin,
                   uint16 xmax, uint16 ymin, uint16 ymax,
                   struct ScoreXYZS* winner, bool dark, bool spec, bool pupil,
                   bool iris, uint8 stepSize) {
  int16 x, y;
  uint16 winnerx = xmax, winnery = ymax,
         winnerEdgecnt; /* QHS: init winnerx and winnery */
  uint32 winnerscore;
  uint16 edgecnt_oneside = (circleR->vTopRightSize + circleR->hRightSize +
                            circleR->vBottomRightSize);
  uint16 edgecnt = edgecnt_oneside << 1;
  // init search loops
  winnerscore = 0;
  winnerEdgecnt = 0;
  for (y = ymin; y <= ymax; y += stepSize) {
    for (x = xmin; x <= xmax; x += stepSize) {
      //
      // accumulate gradients on circle circumference
      // return gradients in gLeft[] and gRight[]
      //
      // filter the edges
      // return lengths of continuous edges
      //
      uint16 maskcnt = 0;
      uint32 score = 0;
      if (dark) {
        maskcnt = collect_circumference_gradients_dark(dLptr, x, y, circleR,
                                                       &score, stepSize);
        if (score > maskcnt)
          score = score - maskcnt;
        else
          score = 0;
        maskcnt = 0;  // no spec mask required for dark find
        // If stepSize == 2, there are half as many samples, score is reduced by
        // half. Multiply score by stepSize to make all dark scores compatible
        // (comparable).
        score = score * stepSize;
      } else if (spec) {
        collect_circumference_gradients_spec(hGradLptr, vGradLptr, minGrad,
                                             maxGrad, x, y, circleR, &score);
        maskcnt = 0;  // no spec mask in spec find
      } else if (pupil) {
        maskcnt = collect_circumference_gradients_pupil(hGradLptr, vGradLptr,
                                                        mLptr, minGrad, maxGrad,
                                                        x, y, circleR, &score);
      } else if (iris) {
        maskcnt = collect_circumference_gradients_iris(hGradLptr, vGradLptr,
                                                       mLptr, minGrad, maxGrad,
                                                       x, y, circleR, &score);
      }
      //
      // sort for highest score from the xy search
      //
      if (score > winnerscore) {
        winnerx = x;
        winnery = y;
        winnerscore = score;
        // subtract the masked edges from the edge count
        winnerEdgecnt = edgecnt - maskcnt;
      }
      //-------------------------------------
    }  // end of x loop
  }    // end of y loop

  //
  // normalize score for circle size
  //

  uint32 normScore = 0;
  if (winnerEdgecnt > 0) normScore = (winnerscore * 1000) / winnerEdgecnt;

  //
  // output
  //
  winner->x = winnerx;
  winner->y = winnery;
  winner->z = radius;
  winner->s = normScore;

  // debug
  // if(dark){
  // Log("*radius: " + IntToStr(radius) + " lcm: " + IntToStr(winnerlongchain));
  // Log("*edgecnt: " + IntToStr(winnerEdgecnt));
  //}
  /*
  if(winnerEdgecnt > 0) winnerlongchain = (winnerlongchain *
  1000)/winnerEdgecnt; Log("*radius: " + IntToStr(radius) + " lcm: " +
  IntToStr(winnerlongchain));
  //Log("*radius: " + IntToStr(radius) + " s: " + IntToStr(normScore));
  //Log("*maxmaskcnt: " + IntToStr(maxmaskcnt));
  */
}
//
//-----------------------------------
//
void match_circles_subpixel(PLINE* hGradLptr, PLINE* vGradLptr, PLINE* mLptr,
                            PLINE* dLptr, int8 minGrad, int8 maxGrad,
                            struct Circle_struct* circleR, uint16 xc, uint16 yc,
                            bool dark, bool spec, bool pupil, bool iris,
                            struct ScoreXYZS (*sCube)[3][3][3], uint16 radius,
                            uint8 radiusLevel) {
  int16 x, y;
  const int16 edgecnt_oneside = (circleR->vTopRightSize + circleR->hRightSize +
                                 circleR->vBottomRightSize);
  const int16 edgecnt = edgecnt_oneside << 1;
  const uint8 stepSize = 1;  // always 1 for score cube
  uint8 iy = 0;              // scoreCube index
  for (y = yc - 1; y <= yc + 1; y++) {
    uint8 ix = 0;  // scoreCube index
    for (x = xc - 1; x <= xc + 1; x++) {
      //
      // accumulate gradients on circle circumference
      // return gradients in gLeft[] and gRight[]
      //
      // filter the edges
      // return lengths of continuous edges
      //
      uint16 maskcnt = 0;
      uint32 score = 0;
      if (dark) {
        maskcnt = collect_circumference_gradients_dark(dLptr, x, y, circleR,
                                                       &score, stepSize);
        if (score > maskcnt)
          score = score - maskcnt;
        else
          score = 0;
        maskcnt = 0;  // no spec mask required for dark find
        // If stepSize == 2, there are half as many samples, score is reduced by
        // half. Multiply score by stepSize to make all dark scores compatible
        // (comparable).
        score = score * stepSize;
      } else if (spec) {
        collect_circumference_gradients_spec(hGradLptr, vGradLptr, minGrad,
                                             maxGrad, x, y, circleR, &score);
        maskcnt = 0;  // no spec mask in spec find
      } else if (pupil) {
        maskcnt = collect_circumference_gradients_pupil(hGradLptr, vGradLptr,
                                                        mLptr, minGrad, maxGrad,
                                                        x, y, circleR, &score);
      } else if (iris) {
        maskcnt = collect_circumference_gradients_iris(hGradLptr, vGradLptr,
                                                       mLptr, minGrad, maxGrad,
                                                       x, y, circleR, &score);
      }
      //
      //-------------------------------------
      //
      // subpixel score cube
      //

      uint32 normScore = 0;
      const int16 winnerEdgecnt = edgecnt - maskcnt;
      if (winnerEdgecnt > 0) normScore = (score * 1000) / winnerEdgecnt;
      (*sCube)[ix][iy][radiusLevel].x = x;
      (*sCube)[ix][iy][radiusLevel].y = y;
      (*sCube)[ix][iy][radiusLevel].z = radius;
      (*sCube)[ix][iy][radiusLevel].s = normScore;

      //
      ix++;  // scoreCube index
             //
    }        // end of x loop
    //
    iy++;  // scoreCube index
           //
  }        // end of y loop
}
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//
// find best fit for circles over (x, y, radius) search area
//
void search_for_circles(PLINE* hGradLptr, PLINE* vGradLptr, PLINE* mLptr,
                        PLINE* dLptr, int8 minGrad, int8 maxGrad,
                        struct Circle_struct* circleR, uint16 minRadius,
                        uint16 maxRadius, int16 width, int16 height, int16 xc,
                        int16 yc, int16 searchBox, struct PointXYZ_float* wpos,
                        bool dark, bool ccl_dark, bool spec, bool pupil,
                        bool iris, uint16 Irisfind_max_pupil_radius) {
  struct ScoreXYZS winner;
  uint16 radius;
  int16 xmin, xmax, ymin, ymax;
  uint32 maxScore = 0;
  // int circleSize; // for debug

  wpos->z = 0;  // return radius = 0 for failed search result

  // minRadius = 43;
  // maxRadius = 43;

  // set search box size
  // if(!iris) make_xy_search_area(width, height, xc, yc, searchBox, &xmin,
  // &xmax, &ymin, &ymax);
  // The iris searchbox depends on offset of iris center from pupil center.
  // offset = 0 at (iris size == pupil size) which is at the smallest iris
  // radius So make the offset increase with iris size. Therefore the iris
  // searchbox goes from 0 to max as iris radius goes from minRadius to
  // maxRadius.
  float32 sb_rad_ratio = 0.0;
  float32 searchBoxMin = 0;
  if (iris) {
    float32 radRange = (float32)(maxRadius - minRadius + 1);
    float32 searchBoxMax = searchBox;
    searchBoxMin = 0;
    float32 searchBoxRange = searchBoxMax - searchBoxMin;
    sb_rad_ratio = searchBoxRange / radRange;
  }

  // if(pupil) Log("searchBox: " + IntToStr(searchBox));

  // (stepSize > 1) improves dark speed, some accuracy loss
  uint8 stepSize = 1;
  // if(dark && STEP_TEST) stepSize = 2;  // improves dark speed, retains
  // accuracy

  // search over (radius, x, y)
  for (radius = minRadius; radius <= maxRadius; radius += stepSize) {
    if (dark && STEP_TEST) {
      if (radius > (Irisfind_max_pupil_radius >> 1)) {
        stepSize = 2;
        // circleSize = circleR[radius].vTopRightSize * 4;
      }
      // Log("rad: " + IntToStr(radius) + " circ" + IntToStr(circleSize));
    }

    //
    // set search box size
    // Set searchBox once or (optional) for each radius.
    //

    // using dark area to set pupil searchbox
    if (pupil) {  // pupil searchbox depends on dark area (location (xc, yc) and
                  // radius)
      uint16 sb = searchBox;
      // Log(IntToStr(radius) + "  " + FloatToStr(sb,2));
      make_xy_search_area(width, height, xc, yc, radius, sb, &xmin, &xmax,
                          &ymin, &ymax);
    }
    /*
    // using spec to set pupil searchbox
    if(pupil){ // pupil searchbox depends on pupil radius and spec (location
    (xc, yc) and radius)
        // searchBox contains spec radius
        // multiply by 2 because searchbox is the full width of search
        uint16 sb = (2 * radius) + searchBox;
        //uint16 sb = (1 * (radius + searchBox));
        //Log(IntToStr(radius) + "  " + FloatToStr(sb,2));
        make_xy_search_area(width, height, xc, yc, radius, sb, &xmin, &xmax,
    &ymin, &ymax);
    }
    */
    else if (iris) {  // iris searchbox depends on pupil offset inside iris
      // multiply by 2 because searchbox is the full width of search
      // sb is already x2 because radius used are double the required searchbox
      uint16 sb = (uint16)(((float32)(radius - minRadius) * sb_rad_ratio) +
                           searchBoxMin + 0.5);
      // or add increase with iris size
      // uint16 sb = ((float32)(radius * 0.1)) * (((float32)(radius - minRadius)
      // * sb_rad_ratio) + searchBoxMin + 0.5); Log(IntToStr(radius) + "  " +
      // FloatToStr(sb,2));
      make_xy_search_area(width, height, xc, yc, radius, sb, &xmin, &xmax,
                          &ymin, &ymax);
    } else if (spec) {
      make_xy_search_area(width, height, xc, yc, radius, searchBox, &xmin,
                          &xmax, &ymin, &ymax);
    } else if (ccl_dark) {  // dark (found with CCL) pupil area searchbox
                            // depends on CCL results (location (xc, yc) and
                            // radius)
      uint16 sb = searchBox;
      make_xy_search_area(width, height, xc, yc, radius, sb, &xmin, &xmax,
                          &ymin, &ymax);
      dark = true;      // for the match_circles() call
    } else if (dark) {  // dark pupil area searchbox depends on pupil radius and
                        // spec (location (xc, yc) and radius)
      // searchBox contains spec radius
      // multiply by 2 because searchbox is the full width of search
      // uint16 sb = (2 * (radius + searchBox));
      uint16 sb = (2 * radius) + searchBox;
      make_xy_search_area(width, height, xc, yc, radius, sb, &xmin, &xmax,
                          &ymin, &ymax);
    } else {
      ASSERT(0);
      return;
    }

    match_circles(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                  &circleR[radius], radius, xmin, xmax, ymin, ymax, &winner,
                  dark, spec, pupil, iris, stepSize);

    // results of search on one radius
    uint16 x = (uint16_t)winner.x;
    uint16 y = (uint16_t)winner.y;
    uint32 score = winner.s;

    // debug
    // if(dark) Log("radius: " + IntToStr(radius) + " s: " + IntToStr(score));
    // if(!spec){
    // Log("radius: " + IntToStr(radius) + " s: " + IntToStr(score));
    // Log("    x: " + IntToStr(x) + " y: " + IntToStr(y));
    // Log("    s: " + IntToStr(score));
    //}

    // sort for highest score
    if (score >= maxScore) {  // favors larger radius for ties
      maxScore = score;
      // save winner
      wpos->x = x;
      wpos->y = y;
      wpos->z = radius;
      wpos->s = score;
    }

  }  // end of radius search loop

  // Log("");
  // if(dark) { Log("");Log("score: " + IntToStr(maxScore));Log("");}
  // if(dark) { Log(""); Log("winner radius: " + IntToStr(winnerRadius)); }
  // Log("xy: " + IntToStr(history->pos[winnerRadius].x) + ", " +
  // IntToStr(history->pos[winnerRadius].y)); Log("");

  // if(pupil)
  // Log("radius: " + IntToStr(winnerRadius) + " s: " + IntToStr(maxScore));
}
//
//-----------------------------------
//
void search_for_circles_subpixel(PLINE* hGradLptr, PLINE* vGradLptr,
                                 PLINE* mLptr, PLINE* dLptr, int8 minGrad,
                                 int8 maxGrad, struct Circle_struct* circleR,
                                 uint16 centerRadius, int16 xc, int16 yc,
                                 bool dark, bool spec, bool pupil, bool iris,
                                 struct ScoreXYZS (*sCube)[3][3][3]) {
  uint16 radius;
  uint8 radiusLevel = 0;

  //
  // (x, y, r) limits are protected in the calling function
  //

  // search over (radius, x, y)
  for (radius = centerRadius - 1; radius <= centerRadius + 1; radius++) {
    //

    match_circles_subpixel(hGradLptr, vGradLptr, mLptr, dLptr, minGrad, maxGrad,
                           &circleR[radius], xc, yc, dark, spec, pupil, iris,
                           sCube, radius, radiusLevel);

    radiusLevel++;  // index for scoreCube radius level

  }  // end of radius search loop
}


//
// build from the center (y = 0) out
uint32 circlePoints_horizontal(uint32 x, uint32 y, struct PointXY_signed* pos,
                               uint32 i) {
  if (x == 0) {
    pos[i].x = -(int16)y;
    pos[i].y = 0;
    i++;
  } else if (x < y) {
    pos[i].x = -(int16)y;
    pos[i].y = -(int16)x;
    i++;
  }
  return i;
}
//
// build from the center (x = 0) out
uint32 circlePoints_vertical(uint32 x, uint32 y, struct PointXY_signed* pos,
                             uint32 i) {
  if (x == 0) {
    pos[i].x = 0;
    pos[i].y = -(int16)y;
    i++;
  } else if (x < y) {
    pos[i].x = -(int16)x;
    pos[i].y = -(int16)y;
    i++;
  }
  return i;
}
//
void reverse_order_lut(uint32 lutSize, struct PointXY_signed* pos,
                       uint16* tempbuf) {
  uint32 i;
  for (i = 0; i < lutSize; i++) {
    tempbuf[i] = pos[i].x;
  }
  for (i = 0; i < lutSize; i++) {
    pos[i].x = tempbuf[lutSize - i - 1];
  }
  for (i = 0; i < lutSize; i++) {
    tempbuf[i] = pos[i].y;
  }
  for (i = 0; i < lutSize; i++) {
    pos[i].y = tempbuf[lutSize - i - 1];
  }
}





