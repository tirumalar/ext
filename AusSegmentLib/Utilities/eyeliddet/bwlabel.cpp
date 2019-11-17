/*
 * bwlabel.cpp
 *
 * Code generation for function 'bwlabel'
 *
 */

/* Include files */
#include "bwlabel.h"
#include "EESM_Coverage.h"
#include "EESM_emxutil.h"
#include "EESMask6.h"
#include "rt_nonfinite.h"

/* Function Definitions */
void bwlabel(const boolean_T varargin_1[7680], double L[7680]) {
  int numRuns;
  int firstRunOnPreviousColumn;
  emxArray_int32_T *startRow;
  int offset;
  emxArray_int32_T *endRow;
  emxArray_int32_T *startCol;
  emxArray_int32_T *labelForEachRun;
  int k;
  int runCounter;
  int row;
  int firstRunOnThisColumn;
  emxArray_int32_T *labelsRenumbered;
  double numComponents;
  int p;
  int root_k;
  int root_p;
  numRuns = 0;
  for (firstRunOnPreviousColumn = 0; firstRunOnPreviousColumn < 240;
       firstRunOnPreviousColumn++) {
    offset = firstRunOnPreviousColumn << 5;
    if (varargin_1[offset]) {
      numRuns++;
    }

    for (k = 0; k < 31; k++) {
      runCounter = offset + k;
      if (varargin_1[runCounter + 1] && (!varargin_1[runCounter])) {
        numRuns++;
      }
    }
  }

  emxInit_int32_T(&startRow, 1);
  emxInit_int32_T(&endRow, 1);
  emxInit_int32_T(&startCol, 1);
  emxInit_int32_T(&labelForEachRun, 1);
  if (numRuns == 0) {
    runCounter = startRow->size[0];
    startRow->size[0] = 0;
    emxEnsureCapacity_int32_T(startRow, runCounter);
    runCounter = endRow->size[0];
    endRow->size[0] = 0;
    emxEnsureCapacity_int32_T(endRow, runCounter);
    runCounter = startCol->size[0];
    startCol->size[0] = 0;
    emxEnsureCapacity_int32_T(startCol, runCounter);
    runCounter = labelForEachRun->size[0];
    labelForEachRun->size[0] = 0;
    emxEnsureCapacity_int32_T(labelForEachRun, runCounter);
    memset(&L[0], 0, 7680U * sizeof(double));
  } else {
    runCounter = startRow->size[0];
    startRow->size[0] = numRuns;
    emxEnsureCapacity_int32_T(startRow, runCounter);
    runCounter = endRow->size[0];
    endRow->size[0] = numRuns;
    emxEnsureCapacity_int32_T(endRow, runCounter);
    runCounter = startCol->size[0];
    startCol->size[0] = numRuns;
    emxEnsureCapacity_int32_T(startCol, runCounter);
    runCounter = 0;
    for (firstRunOnPreviousColumn = 0; firstRunOnPreviousColumn < 240;
         firstRunOnPreviousColumn++) {
      offset = (firstRunOnPreviousColumn << 5) - 1;
      row = 1;
      while (row <= 32) {
        while ((row <= 32) && (!varargin_1[row + offset])) {
          row++;
        }

        if ((row <= 32) && varargin_1[row + offset]) {
          startCol->data[runCounter] = firstRunOnPreviousColumn + 1;
          startRow->data[runCounter] = row;
          while ((row <= 32) && varargin_1[row + offset]) {
            row++;
          }

          endRow->data[runCounter] = row - 1;
          runCounter++;
        }
      }
    }

    runCounter = labelForEachRun->size[0];
    labelForEachRun->size[0] = numRuns;
    emxEnsureCapacity_int32_T(labelForEachRun, runCounter);
    for (runCounter = 0; runCounter < numRuns; runCounter++) {
      labelForEachRun->data[runCounter] = 0;
    }

    k = 0;
    runCounter = 1;
    row = 1;
    firstRunOnPreviousColumn = -1;
    offset = -1;
    firstRunOnThisColumn = 0;
    while (k + 1 <= numRuns) {
      if (startCol->data[k] == runCounter + 1) {
        firstRunOnPreviousColumn = firstRunOnThisColumn + 1;
        firstRunOnThisColumn = k;
        offset = k;
        runCounter = startCol->data[k];
      } else {
        if (startCol->data[k] > runCounter + 1) {
          firstRunOnPreviousColumn = -1;
          offset = -1;
          firstRunOnThisColumn = k;
          runCounter = startCol->data[k];
        }
      }

      if (firstRunOnPreviousColumn >= 0) {
        for (p = firstRunOnPreviousColumn - 1; p + 1 <= offset; p++) {
          if ((endRow->data[k] >= startRow->data[p]) &&
              (startRow->data[k] <= endRow->data[p])) {
            if (labelForEachRun->data[k] == 0) {
              labelForEachRun->data[k] = labelForEachRun->data[p];
              row++;
            } else {
              if (labelForEachRun->data[k] != labelForEachRun->data[p]) {
                for (root_k = k; root_k + 1 != labelForEachRun->data[root_k];
                     root_k = labelForEachRun->data[root_k] - 1) {
                  labelForEachRun->data[root_k] =
                      labelForEachRun->data[labelForEachRun->data[root_k] - 1];
                }

                for (root_p = p; root_p + 1 != labelForEachRun->data[root_p];
                     root_p = labelForEachRun->data[root_p] - 1) {
                  labelForEachRun->data[root_p] =
                      labelForEachRun->data[labelForEachRun->data[root_p] - 1];
                }

                if (root_k + 1 != root_p + 1) {
                  if (root_p + 1 < root_k + 1) {
                    labelForEachRun->data[root_k] = root_p + 1;
                    labelForEachRun->data[k] = root_p + 1;
                  } else {
                    labelForEachRun->data[root_p] = root_k + 1;
                    labelForEachRun->data[p] = root_k + 1;
                  }
                }
              }
            }
          }
        }
      }

      if (labelForEachRun->data[k] == 0) {
        labelForEachRun->data[k] = row;
        row++;
      }

      k++;
    }

    emxInit_int32_T(&labelsRenumbered, 1);
    runCounter = labelsRenumbered->size[0];
    labelsRenumbered->size[0] = labelForEachRun->size[0];
    emxEnsureCapacity_int32_T(labelsRenumbered, runCounter);
    numComponents = 0.0;
    memset(&L[0], 0, 7680U * sizeof(double));
    for (k = 0; k + 1 <= numRuns; k++) {
      if (labelForEachRun->data[k] == k + 1) {
        numComponents++;
        labelsRenumbered->data[k] = (int)numComponents;
      }

      labelsRenumbered->data[k] =
          labelsRenumbered->data[labelForEachRun->data[k] - 1];
      runCounter = (startCol->data[k] - 1) << 5;
      for (row = startRow->data[k]; row <= endRow->data[k]; row++) {
        L[(row + runCounter) - 1] = labelsRenumbered->data[k];
      }
    }

    emxFree_int32_T(&labelsRenumbered);
  }

  emxFree_int32_T(&labelForEachRun);
  emxFree_int32_T(&startCol);
  emxFree_int32_T(&endRow);
  emxFree_int32_T(&startRow);
}

/* End of code generation (bwlabel.cpp) */
