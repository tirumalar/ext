#ifndef _AREA_CONCOM_H

#define _AREA_CONCOM_H

#include <cxcore.h>
//#include "motion_common.h"

typedef struct _MAMIGO_MOTION_CCOMP_BLOB
{
	int num_pixels;
	CvRect roi;
	bool valid;
	int id;
	double Cx, Cy;
	char status;
}
CCOMP_BLOB;

typedef struct
{
	IplImage *scr;
	unsigned int *equiv_tab;
	unsigned int *map_tab;
	int *nz;
	int h, w;
	int max_labels;
	unsigned int label_val;
	unsigned int flabel_val;
	CCOMP_BLOB *blob;
	int max_num_blobs;
	int num_blobs;

	int status;

} 	CCOMP_STATE;

typedef struct
{
	CCOMP_BLOB *blob;
	size_t num_blobs;
	IplImage *mask;

} CCOMP_BLOB_OUTPUT;

bool ccomp_alloc(CCOMP_STATE *state, int w, int h, int max_labels);

bool ccomp_free(CCOMP_STATE *state);

bool ccomp_label_regions(CCOMP_STATE *state, IplImage *bimg, 
			   int x0, int y0, int w, int h);

CCOMP_BLOB *ccomp_get_blobs(CCOMP_STATE *state);

int ccomp_get_num_blobs(CCOMP_STATE *state);

IplImage *ccomp_get_labels(CCOMP_STATE *state);

void ccomp_reset(CCOMP_STATE *state);

#endif
