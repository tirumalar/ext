#include "Iris.h"

#define PI 3.14159265

IrisSegmentation::IrisSegmentation(size_t eyecrop_width, size_t eyecrop_height, size_t flat_iris_width,
           size_t flat_iris_height, float gaze_radius_thresh, float PorportionOfIrisVisibleThreshold)
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

  _PIVThreshold = PorportionOfIrisVisibleThreshold;

  // printf("\n_PIVThreshold....%f gaze_radius_thresh...%f\n", _PIVThreshold, gaze_radius_thresh);

  float angle_sampling = (float)((2.0 * PI) / flat_iris_width);
  float angle = 0.0f;
  for (unsigned int i = 0; i < flat_iris_width; i++) {
    _cos_table[i] = (float)cos(angle);
    _sin_table[i] = (float)sin(angle);
    angle += angle_sampling;
  }
  m_iris_find = new Irisfind(eyecrop_width, eyecrop_height, gaze_radius_thresh);

  m_LogSegInfo = LoadEyelockConfigINIFile();
}

IrisSegmentation ::~IrisSegmentation()
{
	delete [] _line_ptr_crop;
	delete [] _line_ptr_flat;
	delete [] _line_ptr_mask;
	delete [] _cos_table;
	delete [] _sin_table;
	delete m_iris_find;
}

void IrisSegmentation::IrisSegmentation_init(size_t eyecrop_width, size_t eyecrop_height,
		unsigned short int MinIrisDiameter, unsigned short int MaxIrisDiameter,
		unsigned short int MinPupilDiameter, unsigned short int MaxPupilDiameter,
		unsigned short int MinSpecDiameter, unsigned short int MaxSpecDiameter)
{

	//Initialize Segmentation Parameters - Iris, Pupil and Spec
	m_iris_find->SetIrisfind_Iris_Diameter(MinIrisDiameter, MaxIrisDiameter);
	m_iris_find->SetIrisfind_Pupil_Diameter(MinPupilDiameter, MaxPupilDiameter);
	m_iris_find->SetIrisfind_Spec_Diameter( MinSpecDiameter, MaxSpecDiameter);

	m_iris_find->ek_irisfind_session_init(eyecrop_width, eyecrop_height);

	ek_eyelid_session_init(_eyelid, true);
}

float IrisSegmentation::IrisFocus(uint8_t Iris[FLAT_IMAGE_SIZE],
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

#define MAX_LINE_LEN1 256
int IrisSegmentation::LoadEyelockConfigINIFile(){
	static FILE *fp;
	char line[MAX_LINE_LEN1 + 1] ;
	char *token; char *Value;
	int LogSegInfo = 0;
	// char *saveseg = (char *)calloc(10,sizeof(char));
	fp = fopen("Eyelock.ini", "rb");
	if(fp == NULL) {
		printf("Can't open Eyelock.ini\n");
		//return -1;
	}else{
		while( fgets( line, MAX_LINE_LEN1, fp ) != NULL )
		{
			token = strtok( line, "\t =\n\r" ) ;
			if( token != NULL && token[0] != '#' )
			{
				Value = strtok( NULL, "\t =\n\r" ) ;
				if(strcmp(token,"Eyelock.AusLogSegmentResults") == 0){
					if(strcmp(Value,"true") == 0)
						LogSegInfo = 1;
					else
						LogSegInfo = 0;
				}
			}
		}
		fclose(fp);
	}
	return LogSegInfo;
}

int IrisSegmentation::GenerateFlatIris(uint8_t* eyecrop, uint8_t* flat_iris, uint8_t* partial_mask, size_t eyecrop_width, size_t eyecrop_height, IrisFindParameters& IrisPupilParams) {
  get_lineptrs_8(eyecrop, _eyecrop_width, _eyecrop_height, _line_ptr_crop);
  get_lineptrs_8(flat_iris, _flat_iris_width, _flat_iris_height,
                 _line_ptr_flat);
  get_lineptrs_8(partial_mask, _flat_iris_width, _flat_iris_height,
                 _line_ptr_mask);

  TemplatePipelineError eIrisError;
  // Initialize the irisfind software
  m_iris_find->ek_irisfind_init();

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
  eIrisError = m_iris_find->ek_irisfind_main(_line_ptr_crop, _line_ptr_flat, eyecrop_width, eyecrop_height, _flat_iris_width, _flat_iris_height, IrisPupilParams, m_LogSegInfo);

  if(m_LogSegInfo)
	  printf("eIrisError ek_irisfind_main:%d\n", eIrisError);
  //
  // extract the "last human" flag from the "spoof detection" sub-struct
  // of the "live detection" struct
  //
  //   -- MJR 20180831
  //
  // flat_paper = !LiveDetection->spoofdet.last_human;

  // m_GlobalContext.AlgoGlobalMutex.unlock();

  // Find the boudaries of the eyelid

  if(eIrisError == TemplatePipelineError::Segmentation_Successful){
	  eIrisError = ek_eyelid_main(_line_ptr_crop, _eyecrop_width, _eyecrop_height,
					 _flat_iris_width, _flat_iris_height, m_iris_find, _eyelid,
					 _line_ptr_flat, _line_ptr_mask, _radius_sampling, _cos_table,
					 _sin_table);

		  // printf("_eyelid.coverage....%f  %f _PIVThreshold %f\n", _eyelid.coverage[0], (1.0- _eyelid.coverage[0]), _PIVThreshold);
	 if(m_LogSegInfo)
		 printf("eIrisError ek_eyelid_main  :%d\n", eIrisError);

	  if((1.0 - _eyelid.coverage[0]) < _PIVThreshold){
		  eIrisError = TemplatePipelineError::Usable_Iris_Area_not_sufficient;
	  }

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

	  IrisPupilParams.eyelidCoverage = _eyelid.coverage[0];
	  IrisPupilParams.Usable_Iris_Area = (1.0 - _eyelid.coverage[0]);
	  IrisPupilParams.eIrisError = eIrisError;

	  if(eIrisError == TemplatePipelineError::Segmentation_Successful)
		  return true;
	  else
	  {
		  // std::error_code ec = make_error_code(eIrisError);
		  // std::cout << ec << std::endl;
		  printf("eIrisError......%d\n", eIrisError);
		  return false;
	  }
  }else{
	  return false;
  }



}
