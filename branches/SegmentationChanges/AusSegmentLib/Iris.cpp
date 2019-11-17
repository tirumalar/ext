#include "Iris.h"

#define PI 3.14159265

AusIris::AusIris(size_t eyecrop_width, size_t eyecrop_height, size_t flat_iris_width,
           size_t flat_iris_height)

{
  _line_ptr_crop = new PLINE[eyecrop_height];
  _line_ptr_flat = new PLINE[flat_iris_height];
  _line_ptr_mask = new PLINE[flat_iris_height];
  _cos_table = new float[flat_iris_width];
  _sin_table = new float[flat_iris_width];

  _flat_iris_width = flat_iris_width;
  _flat_iris_height = flat_iris_height;

  _eyecrop_width = eyecrop_width;
  _eyecrop_height = eyecrop_height;

  _radius_sampling = (float)(1.0 / flat_iris_height);

  float angle_sampling = (float)((2.0 * PI) / flat_iris_width);
  float angle = 0.0f;
  for (int i = 0; i < flat_iris_width; i++) {
    _cos_table[i] = (float)cos(angle);
    _sin_table[i] = (float)sin(angle);
    angle += angle_sampling;
  }
}

AusIris ::~AusIris() {
  delete _line_ptr_crop;
  delete _line_ptr_flat;
  delete _line_ptr_mask;
  delete _cos_table;
  delete _sin_table;
}

void AusIris::AusIris_init() {
  ek_irisfind_session_init(_iris_find);
  ek_eyelid_session_init(_eyelid, true);
}

float AusIris::IrisFocus(uint8_t Iris[FLAT_IMAGE_SIZE],
                      uint8_t Mask[FLAT_IMAGE_SIZE]) {
  int Row, Column;
  int Offset, NextOffset;
  float Accumulator = 0;
  int Count = 0;

  /* For each row of the flat iris */
  for (Row = 0; Row < FLAT_IMAGE_HEIGHT; Row++) {
    /* For each column in the row */
    for (Column = 0; Column < FLAT_IMAGE_WIDTH; Column++) {
      /* Calculate the offset to the pixel */
      Offset = Row * FLAT_IMAGE_WIDTH + Column;

      /* If the column is the last column */
      if (Column == FLAT_IMAGE_WIDTH - 1)
        /* The next pixel is in the first column (wraparound) */
        NextOffset = Row * FLAT_IMAGE_WIDTH;
      else
        /* The next pixel is right after the pixel */
        NextOffset = Offset + 1;

      /* If the pixel and the next pixel are uncovered */
      if ((Mask[Offset] == UNCOVERED) && (Mask[NextOffset] == UNCOVERED)) {
        /* Accumulate the difference between the value of the pixel */
        /* and the value of the next pixel                          */
        if (Iris[Offset] > Iris[NextOffset])
          Accumulator += Iris[Offset] - Iris[NextOffset];
        else
          Accumulator += Iris[NextOffset] - Iris[Offset];

        /* Count the number of uncovered pixel pairs */
        Count++;
      }
    }
  }

  /* Return the average difference between all uncovered pixel pairs */
  return Accumulator / Count;
}

int AusIris::GenerateFlatIris(uint8_t* eyecrop, uint8_t* flat_iris,
                           uint8_t* partial_mask) {
  get_lineptrs_8(eyecrop, _eyecrop_width, _eyecrop_height, _line_ptr_crop);
  get_lineptrs_8(flat_iris, _flat_iris_width, _flat_iris_height,
                 _line_ptr_flat);
  get_lineptrs_8(partial_mask, _flat_iris_width, _flat_iris_height,
                 _line_ptr_mask);

  // Initialize the irisfind software
  ek_irisfind_init(_iris_find);

  // Initialize the eyelid software
  ek_eyelid_init(_eyelid);

  // Find the boundaries of the iris. Lock the mutex to the shared global
  // context so that threads do not trample over one another.
  bool flat_paper = true;

  // copy the IRLED side information into the LiveDetection structure
  // m_GlobalContext.LiveDetection.irled.firingSide = EyeCrop->IrledSide;

  // retrieve the value of the "Pipeline.LiveDetectionReq" flag via the
  // configuration interface in the support object and copy it into the
  // LiveDetecton structure
  // m_GlobalContext.LiveDetection.enabled =
  // Cfg->GetBool("Pipeline.LiveDetectionReq");

  // LiveDetection->enabled = true;

  // run the iris find algorithm

  ek_irisfind_main(_line_ptr_crop, _line_ptr_flat, _iris_find, _eyecrop_width,
                   _eyecrop_height, _flat_iris_width, _flat_iris_height);

  //
  // extract the "last human" flag from the "spoof detection" sub-struct
  // of the "live detection" struct
  //
  //   -- MJR 20180831
  //
  // flat_paper = !LiveDetection->spoofdet.last_human;

  // m_GlobalContext.AlgoGlobalMutex.unlock();

  // Find the boudaries of the eyelid

  ek_eyelid_main(_line_ptr_crop, _eyecrop_width, _eyecrop_height,
                 _flat_iris_width, _flat_iris_height, _iris_find, _eyelid,
                 _line_ptr_flat, _line_ptr_mask, _radius_sampling, _cos_table,
                 _sin_table);

  // Template->brightnessValid = _iris_find->brightnessValid;
  // Template->brightness = _iris_find->irisBrightness;

  // Fill in the fields of the template frame
  // Template->FrameNumber = EyeCrop->FrameNumber;
  // Template->Timestamp = EyeCrop->Timestamp;
  // Template->Dilated = false;
  // Template->QualityScore = 0;
  // EyeCrop->UserTooFar = eyefind->tooFar;
  // EyeCrop->UserTooClose = eyefind->tooClose;
  // Template->LiveDet_FlatPaper = flat_paper;

  return RESULT_SUCCESS;
  // return int8_t();
}
