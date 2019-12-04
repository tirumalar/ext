#include "Encode.h"

Encode::Encode(int base_scale, size_t flat_iris_width, size_t flat_iris_height,
               size_t template_width, size_t template_height) {
  _base_scale = base_scale;
  _flat_iris_width = flat_iris_width;
  _flat_iris_height = flat_iris_height;
  _template_width = template_width;
  _template_height = template_height;
  _frequency_response_threshold = new float[template_height];

  _feature_filter_length = base_scale * 16;
  _wrap_image_width = flat_iris_width + _feature_filter_length;
  _wrap_image_height = flat_iris_height;
  _wrap_integral_width = _wrap_image_width + 1;
  _wrap_integral_height = _wrap_image_height + 1;

  _line_ptr_flat = new PLINE[flat_iris_height];
  _line_ptr_mask = new PLINE[flat_iris_height];
  _line_ptr_template_encode = new PLINE[template_height];
  _line_ptr_template_mask = new PLINE[template_height];

  _buf_wrap = new uint8_t[_wrap_image_width * _wrap_image_height];
  _buf_mask_wrap = new uint8_t[_wrap_image_width * _wrap_image_height];
  _line_ptr_buf_wrap = new PLINE[_wrap_image_height];
  _line_ptr_buf_mask_wrap = new PLINE[_wrap_image_height];

  _buf_integral_wrap =
      new uint32_t[_wrap_integral_width * _wrap_integral_height];
  _buf_mask_integral_wrap =
      new uint32_t[_wrap_integral_width * _wrap_integral_height];
  _line_ptr_buf_integral_wrap = new PLINE[_wrap_integral_height];
  _line_ptr_buf_mask_integral_wrap = new PLINE[_wrap_integral_height];

  get_lineptrs_8((uint8_t*)_buf_wrap, _wrap_image_width, _wrap_image_height,
                 _line_ptr_buf_wrap);
  get_lineptrs_8((uint8_t*)_buf_mask_wrap, _wrap_image_width,
                 _wrap_image_height, _line_ptr_buf_mask_wrap);

  get_lineptrs_32((uint32_t*)_buf_integral_wrap, _wrap_integral_width,
                  _wrap_integral_height, _line_ptr_buf_integral_wrap);
  get_lineptrs_32((uint32_t*)_buf_mask_integral_wrap, _wrap_integral_width,
                  _wrap_integral_height, _line_ptr_buf_mask_integral_wrap);

  float extra_factor = (float)(2.0 * (float)base_scale / 4.0f);
  extra_factor *= ENCODE_CORING_SCALE_FACTOR;

  _vertical_template_sample_size = 8;
  _horizontal_template_sample_size = 3;

  if (_base_scale == 10) {
    // legacy thresh
    _frequency_response_threshold[0] = 55;
    _frequency_response_threshold[1] = 70;
    _frequency_response_threshold[2] = 135;
    _frequency_response_threshold[3] = 165;
    _frequency_response_threshold[4] = 240;  // unused
    _frequency_response_threshold[5] = 325;  // unused
    _frequency_response_threshold[6] = 370;  // unused
    _frequency_response_threshold[7] = 430;  // unused
  } else if (_base_scale == 6) {
    // optimized
    _frequency_response_threshold[0] = 29;
    _frequency_response_threshold[1] = 44;
    _frequency_response_threshold[2] = 79;
    _frequency_response_threshold[3] = 101;
    _frequency_response_threshold[4] = 154;
    _frequency_response_threshold[5] = 219;
    _frequency_response_threshold[6] = 222;
    _frequency_response_threshold[7] = 258;

  } else {
    // legacy thresh equation
    _frequency_response_threshold[0] =
        (uint16)((11.0 * extra_factor) + 0.5);  // 6
    _frequency_response_threshold[1] = (uint16)((14.0 * extra_factor) + 0.5);
    _frequency_response_threshold[2] =
        (uint16)((27.0 * extra_factor) + 0.5);  // 12
    _frequency_response_threshold[3] = (uint16)((33.0 * extra_factor) + 0.5);
    _frequency_response_threshold[4] =
        (uint16)((48.0 * extra_factor) + 0.5);  // 18
    _frequency_response_threshold[5] = (uint16)((65.0 * extra_factor) + 0.5);
    _frequency_response_threshold[6] =
        (uint16)((74.0 * extra_factor) + 0.5);  // 24
    _frequency_response_threshold[7] = (uint16)((86.0 * extra_factor) + 0.5);
  }
}

Encode::~Encode() {
	if(_frequency_response_threshold)
		delete [] _frequency_response_threshold;
	if(_line_ptr_flat)
		delete [] _line_ptr_flat;
	if(_line_ptr_mask)
		delete [] _line_ptr_mask;
	if(_line_ptr_template_encode)
		delete [] _line_ptr_template_encode;
	if(_line_ptr_template_mask)
		delete [] _line_ptr_template_mask;

	if(_buf_wrap)
		delete [] _buf_wrap;
	if(_buf_integral_wrap)
		delete [] _buf_integral_wrap;
	if(_buf_mask_wrap)
		delete [] _buf_mask_wrap;
	if(_buf_mask_integral_wrap)
		delete [] _buf_mask_integral_wrap;

	if(_line_ptr_buf_wrap)
		delete [] _line_ptr_buf_wrap;
	if(_line_ptr_buf_integral_wrap)
		delete [] _line_ptr_buf_integral_wrap;
	if(_line_ptr_buf_mask_wrap)
		delete [] _line_ptr_buf_mask_wrap;
	if(_line_ptr_buf_mask_integral_wrap)
		delete [] _line_ptr_buf_mask_integral_wrap;
}

// Add image data to beginning and end of image to make >360 deg flat iris.
// So that long filters only use pixel data and do not run over the ends of the
// original image.
//
void Encode::HorizontalBorderWrap(PLINE* linePtrIn, uint16 width, uint16 height,
                                  uint8 filterLength, PLINE* linePtrOut) {
  uint8 borderSize = filterLength >> 1;  // len = 48
  uint16 y;
  for (y = 0; y < height; y++) {
    uint8* iptr = (uint8*)linePtrIn[y];
    uint8* optr = (uint8*)linePtrOut[y];
    memcpy(optr, iptr + width - borderSize, borderSize);
    memcpy(optr + borderSize, iptr, width);
    memcpy(optr + width + borderSize, iptr, borderSize);
  }
}

void Encode::ExtractFeatures(PLINE* lineptrWrapInt, PLINE* lineptrMaskWrapInt,
                             size_t wrapWidth, size_t filterLength,
                             PLINE* lineptrEncoded, PLINE* lineptrTag,
                             size_t encodeWidth, size_t encodeHeight,
                             uint32_t& validBitsInTemplate,
                             uint32_t& unmaskedBitsInTemplate,
                             float& validBitRatioInTemplate) {
  uint8 borderSize = filterLength >> 1;  // len = 48

  uint32 validBits = 0;
  uint32 unmaskedBits = 0;

  // point to buffers
  // TBD convert extractFeatures() to lineptrs instead of buffer math
  uint32* imgIntPtr = (uint32*)lineptrWrapInt[0];
  uint32* maskIntPtr = (uint32*)lineptrMaskWrapInt[0];

  int inStep = wrapWidth + 1;  // integral image has an extra column and row

  int ency = 0;

  int filty = 0;

  // test
  float sum[8];  // test result
  // int filter_area[8];
  // int8* pptr;
  // int8* tptr;

  for (int i = 0; i < 8; i++) {
    sum[i] = 0.0;
  }

  // TBD confirm this EyeLock legacy value for maskPerCent
  int maskPerCent = 0x800;  // EyeLock legacy code, maskPerCent = 8 * 0xff;

  for (unsigned int rr = 0; rr < encodeHeight; rr++) {
    uint8* encpptr = (uint8*)lineptrEncoded[ency];
    uint8* tagpptr = (uint8*)lineptrTag[ency];
    ency++;

    int encx = 0;

    // int filtx = 0;

    uint32* iptr =
        imgIntPtr + (_vertical_template_sample_size * rr * inStep) + borderSize;
    uint32* mptr = maskIntPtr + (_vertical_template_sample_size * rr * inStep) +
                   borderSize;

    // on final pass,
    // iptr8 points to below last line of image,
    // but integral image has an extra row,
    // so access is within integral buffer
    uint32* iptr8 = iptr + (_vertical_template_sample_size * inStep);
    uint32* mptr8 = mptr + (_vertical_template_sample_size * inStep);

    // increment all of the pointers ****
    for (unsigned int jj = 0; jj < encodeWidth; jj++,
             iptr += _horizontal_template_sample_size,
             iptr8 += _horizontal_template_sample_size,
             mptr += _horizontal_template_sample_size,
             mptr8 += _horizontal_template_sample_size) {
      unsigned char enc_feat = 0;
      unsigned char enc_mask = 0;

      char a1 = 0x1;
      char a2 = 0x2;

      int filterCnt = 0;

      for (int i = 0; i < 4; i++, a1 <<= 2, a2 <<= 2) {
        int32 D, M;
        int sc = _base_scale * (i + 1);
        int sc2 = sc << 1;

        float fd;

        // filter_area[i] =  4 * sc * 8; // to normalize filter result for debug

        //
        // read the mask in the same area as the filter
        //
        M = mptr[-sc2] - mptr[sc2] - mptr8[-sc2] + mptr8[sc2];

        //---------------------------------------------------
        // Even Filter
        /*
        //Even Filter
        D = 3*(iptr[-sc] - 2*iptr[0] + iptr[sc] - iptr8[-sc] + 2*iptr8[0] -
        iptr8[sc]); D += (iptr8[sc2] + iptr[sc] - iptr8[sc] - iptr[sc2]) -
        (iptr[-sc2] + iptr8[-sc] - iptr8[-sc2] - iptr[-sc]); D = D >> 1; if(D >
        0) enc_feat |= a1;

        even, sine, looking for gradient and edge, 2 cycles
                |    -1    |    +3    |    -3    |     +1    |

        */
        D = 3 * (iptr[-sc] - 2 * iptr[0] + iptr[sc] - iptr8[-sc] +
                 2 * iptr8[0] - iptr8[sc]);
        D += (iptr8[sc2] + iptr[sc] - iptr8[sc] - iptr[sc2]) -
             (iptr[-sc2] + iptr8[-sc] - iptr8[-sc2] - iptr[-sc]);
        D = D >> 1;

        if (D > 0) enc_feat |= a1;

        uint32 validBit = 1;
        uint32 unmaskedBit = 1;

        if (M > maskPerCent) {
          enc_mask |= a1;  // maskPerCent = ~0x800
          validBit = 0;
          unmaskedBit = 0;
        }

        // under threshold
        if (abs(D) < _frequency_response_threshold[i * 2]) {
          enc_mask |= a1;
          validBit = 0;
          fd = 0.0;

          // tptr = (int8*)encode.lineptrFilteredTag[filterCnt][filty];
          // tptr[encx] = (uint8)0;
        }
        // over threshold
        else {
          fd = (float)D;

          // tptr = (int8*)encode.lineptrFilteredTag[filterCnt][filty];
          // tptr[encx] = (uint8)0; // init
          // if (!(M > maskPerCent)) tptr[encx] = (uint8)255;
          // tptr[encx] = (uint8)255;
        }

        validBits += validBit;
        unmaskedBits += unmaskedBit;

        // sample middle row
        if (rr == 4) {
          if (fd < 0)
            sum[filterCnt] += -fd;
          else
            sum[filterCnt] += fd;
        }
        //
        // filter display
        // linear output
        if (fd > 127) fd = 127;
        if (fd < -128) fd = -128;
        // three color output
        // if(fd > 0) fd = 127;
        // if(fd < 0) fd = -128;
        // pptr = (int8*)encode.lineptrFiltered[filterCnt][filty];
        // pptr[encx] = (int8)fd;
        // scale
        // encode.filterScale[filterCnt] = (uint8_t)sc;
        filterCnt++;  // *****

        //---------------------------------------------------
        // Odd Filter
        /*
        // ODD Filter
        D = -iptr[-sc2] + 2*iptr[-sc] - 2*iptr[sc] + iptr[sc2] + iptr8[-sc2] -
        2*iptr8[-sc] + 2*iptr8[sc] - iptr8[sc2]; if(D > 0)enc_feat |= a2 ;

        odd, cosine, looking for pulses, 1 cycle
                |    -1    |    +1    |    +1    |     -1    |

        */
        D = -(int32)iptr[-sc2] + 2 * iptr[-sc] - 2 * iptr[sc] + iptr[sc2] +
            iptr8[-sc2] - 2 * iptr8[-sc] + 2 * iptr8[sc] - iptr8[sc2];

        if (D > 0) enc_feat |= a2;

        validBit = 1;
        unmaskedBit = 1;

        if (M > maskPerCent) {
          enc_mask |= a2;  // maskPerCent = ~0x800
          validBit = 0;
          unmaskedBit = 0;
        }

        // under threshold
        if (abs(D) < _frequency_response_threshold[i * 2 + 1]) {
          enc_mask |= a2;
          validBit = 0;
          fd = 0.0;
          // fd = (float)D; // no thresh

          // tptr = (int8*)encode.lineptrFilteredTag[filterCnt][filty];
          // tptr[encx] = (uint8)0;
        }
        // over threshold
        else {
          fd = (float)D;

          // tptr = (int8*)encode.lineptrFilteredTag[filterCnt][filty];
          // tptr[encx] = (uint8)0; // init
          // if (!(M > maskPerCent)) tptr[encx] = (uint8)255;
          // tptr[encx] = (uint8)255;
        }

        validBits += validBit;
        unmaskedBits += unmaskedBit;

        // sample middle row
        if (rr == 4) {
          if (fd < 0)
            sum[filterCnt] += -fd;
          else
            sum[filterCnt] += fd;
        }
        //
        // filter display
        // linear output
        if (fd > 127) fd = 127;
        if (fd < -128) fd = -128;
        // three color output
        // if(fd > 0) fd = 127;
        // if(fd < 0) fd = -128;
        // pptr = (int8*)encode.lineptrFiltered[filterCnt][filty];
        // pptr[encx] = (int8)fd;
        // scale
        // encode.filterScale[filterCnt] = (uint8_t)sc;
        filterCnt++;  // *****
                      //---------------------------------------------------
      }

      encpptr[encx] = enc_feat;
      tagpptr[encx] = (uint8)(~enc_mask);

      encx++;

      // filtx++;
    }

    filty++;
  }

  //
  // return vaild bits in template
  //

  validBitsInTemplate =
      validBits;  // not masked by eyelid, spec, and not under filter threshold
  unmaskedBitsInTemplate = unmaskedBits;  // not masked by eyelid or spec
  if (unmaskedBits > 0) {
    validBitRatioInTemplate =
        (float)validBits /
        (float)unmaskedBits;  // strength of signal, contrast of iris
  }
}

int Encode::EncodeFlatIris(uint8_t* flat_iris, uint8_t* partial_mask,
                           uint8_t* template_encode, uint8_t* template_mask) {
  uint32_t validBitsInTemplate;
  uint32_t unmaskedBitsInTemplate;
  float validBitRatioInTemplate;

  IRISERROR eIrisError = IRISERROR::Segmentation_Successful;

  get_lineptrs_8(flat_iris, _flat_iris_width, _flat_iris_height,
                 _line_ptr_flat);
  get_lineptrs_8(partial_mask, _flat_iris_width, _flat_iris_height,
                 _line_ptr_mask);
  get_lineptrs_8(template_encode, _template_width, _template_height,
                 _line_ptr_template_encode);
  get_lineptrs_8(template_mask, _template_width, _template_height,
                 _line_ptr_template_mask);

  // wrap the image to accommodate filter length
  HorizontalBorderWrap(_line_ptr_flat, _flat_iris_width, _flat_iris_height,
                       _feature_filter_length, _line_ptr_buf_wrap);
  HorizontalBorderWrap(_line_ptr_mask, _flat_iris_width, _flat_iris_height,
                       _feature_filter_length, _line_ptr_buf_mask_wrap);

  // make the integral image of the wrap image
  integralImage(_line_ptr_buf_wrap, _wrap_image_width, _wrap_image_height,
                _line_ptr_buf_integral_wrap);
  integralImage(_line_ptr_buf_mask_wrap, _wrap_image_width, _wrap_image_height,
                _line_ptr_buf_mask_integral_wrap);

  ExtractFeatures(_line_ptr_buf_integral_wrap, _line_ptr_buf_mask_integral_wrap,
                  _wrap_image_width, _feature_filter_length,
                  _line_ptr_template_encode, _line_ptr_template_mask,
                  _template_width, _template_height, validBitsInTemplate,
                  unmaskedBitsInTemplate, validBitRatioInTemplate);

  // TODO: Replace with Error Class
  if (validBitsInTemplate > TEMPLATE_MIN_VALID_BITS){
	  eIrisError =  IRISERROR::Segmentation_Successful;
  }else{
	  eIrisError =  IRISERROR::InValidbits_in_Template;
  }

  if(eIrisError == IRISERROR::Segmentation_Successful)
	  return true;
  else{
	  printf("Error   %d\n", eIrisError);
 	  return false;
  }
}
