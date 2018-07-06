#ifndef _HAAR_PRIVATE_H_
#define _HAAR_PRIVATE_H_

#ifndef _MAMIGO_RECT_
#define _MAMIGO_RECT_
typedef struct
{
    int x;
    int y;
    int width;
    int height;
}
MAMIGO_Rect;
#endif

// following functions work in pairs:
// set_roi_x helps the callee in preparing the params for compute_x and compute_x_skip

void set_roi_planar_ptr(int *img, int widthStep, MAMIGO_Rect rect, int *p);
void compute_haar_variance_scaled_nsquare_planar(int *params, int *varout);

void set_roi_ptr_type_0(int *img, int widthStep, int *p, int *op);
int compute_haar_feature_type_0(int *par, int *out);
int compute_haar_feature_with_skip_type_0(int *par, int *out);

void set_roi_ptr_type_1(int *img, int widthStep, int *p, int *op);
int compute_haar_feature_type_1(int *par, int *out);
int compute_haar_feature_with_skip_type_1(int *par, int *out);

void set_roi_ptr_type_2(int *img, int widthStep, int *p, int *op);
int compute_haar_feature_type_2(int *par, int *out);
int compute_haar_feature_with_skip_type_2(int *par, int *out);

void set_roi_ptr_type_3(int *img, int widthStep, int *p, int *op);
int compute_haar_feature_type_3(int *par, int *out);
int compute_haar_feature_with_skip_type_3(int *par, int *out);

void set_roi_ptr_type_4(int *img, int widthStep, int *p, int *op);
int compute_haar_feature_type_4(int *par, int *out);
int compute_haar_feature_with_skip_type_4(int *par, int *out);

/*
 * Finds the shift (exponent) needed to represent a feature, using the max value
 */
int getfeatureshift(int max_featureVal);

unsigned int HaarDecisionPositiveThreshold(int *feature_out, int *var_out, int *par);
unsigned int HaarDecisionNegativeThreshold(int *feature_out, int *var_out, int *par);

#endif //_HAAR_PRIVATE_H_
