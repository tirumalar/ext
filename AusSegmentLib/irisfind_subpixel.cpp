#include "compiler.h"
#include "eyelid.h"
#include "helper.h"
#include "irisfind.h"

#include <math.h>  // for sqrtf()

bool sort_score_3(uint32* score, uint8* phigh, uint8* pmid, uint8* plow) {
  bool centerOK = true;
  // sort for lowest score
  uint8 temp;
  uint8 low = 0;   // index for sorted score
  uint8 high = 1;  // index for sorted score
  uint8 mid = 2;   // index for sorted score
  if (score[mid] < score[low]) {
    temp = mid;
    mid = low;
    low = temp;
  }
  // ??? bad situation if center plane has the lowest score,
  // can't adjust coordinate using outside planes ???
  // or maybe it actually helps in noisy seg score situations
  if (score[high] < score[low]) {
    // centerOK = false; // use integer result instead of subpixel result
    // Log("*** error: center plane lowest seg score ***");
    // return 0; // abort with error
    // or keep going and see what happens....
    temp = high;
    high = low;
    low = temp;
  }
  // sort for highest score to use in checking score variance
  if (score[high] < score[mid]) {
    temp = high;
    high = mid;
    mid = temp;
  }
  *phigh = high;
  *pmid = mid;
  *plow = low;
  return centerOK;
}
//
float weighted_coordinate(uint16 cval, uint16* coord, uint32* score,
                          uint8* phigh, uint8* pmid, uint8* plow,
                          uint32 minScoreVariance) {
  // sort score high to low
  // test if the center plane has a low score
  bool centerOK = sort_score_3(score, phigh, pmid, plow);
  // remove offset (lowest score)
  score[*pmid] -= score[*plow];
  score[*phigh] -= score[*plow];
  // integer result in case score variance is too low
  float subz = cval;
  // float result = weighted avg of higher scoring coordinates
  // But first test that there is enough score variation to safely choose a max
  // score and subtract min score. In practice, this threshold makes no
  // difference to Novetta 7k dataset tests.
  if (centerOK && (score[*phigh] > minScoreVariance)) {
    subz = (float)((float)((score[*pmid] * coord[*pmid]) +
                           (score[*phigh] * coord[*phigh])) /
                   (float)(score[*pmid] + score[*phigh]));
  }
  return subz;
}
//
// weighted subpixel avg for each plane
// subpixel_from_scoreCube() extends in x, y, z (radius) dimensions
// prevent accesses outside allocated radius
// TBD: return error code? score variance too low? (irisfind.minScoreVariance =
// MIN_SCORE_VARIANCE)
uint8 subpixel_from_scoreCube(struct ScoreXYZS (*sCube)[3][3][3], uint16 xc,
                              uint16 yc, uint16 zc, uint32 minScoreVariance,
                              float32* psx, float32* psy, float32* psz) {
  // weighted subpixel avg for each plane
  //---------------------------------------
  // subpixel z (radius)
  //
  // sum 9 scores for each of 3 "z" (radius) coordinates
  uint32 score[3];
  uint16 coord[3];
  uint8 low, mid, high;  // index for sorted score
  for (int zz = 0; zz < 3; zz++) {
    score[zz] = 0;
    coord[zz] = zc - 1 + zz;
    for (int yy = 0; yy < 3; yy++) {
      for (int xx = 0; xx < 3; xx++) {
        uint16 s = (*sCube)[xx][yy][zz].s;
        score[zz] += s;
      }
    }
  }
  // weighted avg of coordinate
  *psz = weighted_coordinate(zc, coord, score, &high, &mid, &low,
                             minScoreVariance);
  // for sub-x and sub-y, only use the two radii with the higher scores
  uint8 rad[2];
  rad[0] = mid;   // index for sorted radius (z) score
  rad[1] = high;  // index for sorted radius (z) score
  //---------------------------------------
  // subpixel y
  //
  // sum 6 scores for each of 3 "y" coordinates
  for (int yy = 0; yy < 3; yy++) {
    score[yy] = 0;
    // coord[yy] = scoreCube[0][yy][0].y;
    coord[yy] = yc - 1 + yy;
    // for sub-x and sub-y, only use the two radii with the higher scores
    for (int ri = 0; ri < 2; ri++) {
      int rr = rad[ri];  // use two higher radii
      for (int xx = 0; xx < 3; xx++) {
        uint16 s = (*sCube)[xx][yy][rr].s;
        score[yy] += s;
      }
    }
  }
  // weighted avg of coordinate
  *psy = weighted_coordinate(yc, coord, score, &high, &mid, &low,
                             minScoreVariance);
  //---------------------------------------
  // subpixel x
  //
  // sum 6 scores for each of 3 "x" coordinates
  for (int xx = 0; xx < 3; xx++) {
    score[xx] = 0;
    // coord[xx] = scoreCube[xx][0][0].x;
    coord[xx] = xc - 1 + xx;
    for (int ri = 0; ri < 2; ri++) {
      int rr = rad[ri];  // use two higher radii
      for (int yy = 0; yy < 3; yy++) {
        uint16 s = (*sCube)[xx][yy][rr].s;
        score[xx] += s;
      }
    }
  }
  // weighted avg of coordinate
  *psx = weighted_coordinate(xc, coord, score, &high, &mid, &low,
                             minScoreVariance);

  return 1;
}
