#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "area_concom.h"

#define CCOMP_UNINITIALIZED 0
#define CCOMP_INITIALIZED 10
#define CCOMP_LABELS_GENERATED 20
#define CCOMP_BLOBS_COMPUTED 30


/*
* ===================================================================
  merge()

  PURPOSE
	Combines two equivalence chains a & b in ``equiv_tab'' 
        into a single chain.
*/


static void merge(unsigned int a, unsigned int b,
                  unsigned int *equiv_tab,  int label_val)
{
  unsigned int dum;
  char errstr[100];

  if (a == b)
    return;
  if (a > b) {
    dum = a; a = b; b = dum;
  }
  while ((equiv_tab[a] != 0) && (equiv_tab[a] < b)) {
    a = equiv_tab[a];
    if ( a >=  (unsigned int) label_val) {
      sprintf(errstr,"merge: a=%d\n",a);
      printf(errstr, __LINE__, __FILE__);
	  return;
    }
  }
  if (equiv_tab[a] == 0) {
    equiv_tab[a] = b;
    return;
  }
  if (equiv_tab[a] == b)
    return;

  merge(b, equiv_tab[a], equiv_tab, label_val);
  equiv_tab[a] = b;
}


static void generate_labels(IplImage *in, IplImage *out,
                            int x0, int y0, int w, int h,
                            unsigned int max_labels,
                            unsigned int *equiv_tab,
                            unsigned int *label_val)
{
	int i,j;
	short *out_b, *out_a;
	unsigned char *row_b, *row_a;
	short *out_b1, *out_a1;
	unsigned char *row_b1, *row_a1;
	int max_j = y0 + h - 1;
	int max_i = x0 + w - 1;

	short *outptr = 0;
	unsigned char *inptr = 0;
	int inStep, outStep;

	cvGetRawData(in, (unsigned char **) &inptr, &inStep);
	cvGetRawData(out, (unsigned char **) &outptr, &outStep);
	outStep /= sizeof(short);

	out_b = (short *) (outptr + y0*outStep + x0);
	row_b = (unsigned char *) (inptr + y0*inStep + x0);

	out_a = (short *) (outptr + (y0-1)*outStep+x0);
	row_a = (unsigned char *) (inptr + (y0-1)*inStep + x0);; 
	out_a1 = (short *) (out_a - 1);
	out_b1 = (short *) (out_b - 1);
	row_a1 = (unsigned char *) (row_a - 1);
	row_b1 = (unsigned char *) (row_b - 1);
	
	for (j = y0; j <= max_j; j++) 
	{		
		out_b = (short *) (outptr + j*outStep+x0);
		row_b = (unsigned char *) (inptr + j*inStep+x0);
		out_b1 = out_b - 1;
		row_b1 = row_b - 1;

		for (i= x0; i <= max_i; i++, out_a++, out_b++, row_a++, row_b++) 
		{			
			if(*out_b = *row_b)
			{
				if(*row_b1)
				{
					if(!(*row_a1) && *row_a)
					{
						*out_b = *out_a;
						/* merge two labels */
						merge((*out_b1), (*out_a), equiv_tab, *label_val); 
					}
					else
						*out_b = *out_b1;
				}
				else if(*row_a)
				{
					*out_b = *out_a;
				}
				else if(*row_a1)
				{
					*out_b = *out_a1;
				}
				else if(*(row_a+1))
				{
					*out_b = *(out_a+1);
				}
				else 
				{	/* generate new label */
					if ((*label_val) >=  max_labels) {
						*label_val = max_labels - 1;
					}

					equiv_tab[*label_val]=0;
					*out_b = (short) (*label_val)++;
				}
			}

			row_b1 = row_b; out_b1 = out_b;
			row_a1 = row_a; out_a1 = out_a; 
		}

		out_a = (short *) (outptr + j*outStep+x0);
		row_a = (unsigned char *) (inptr + j*inStep+x0);
		out_a1 = out_a - 1;
		row_a1 = row_a - 1;

	}
}



bool ccomp_alloc(CCOMP_STATE *state, int w, int h, int max_labels)
{
	state->scr = cvCreateImage(cvSize(w, h), IPL_DEPTH_16S, 1);

   state->max_labels = max_labels;   

   /* Allocate equivalence table */
   state->equiv_tab = (unsigned int *)
                 malloc(((size_t)state->max_labels) *sizeof(unsigned int));
   if (state->equiv_tab == NULL) {
     return(false);
   }

   state->map_tab = (unsigned int *)
                    malloc(((size_t)state->max_labels) * sizeof(unsigned int));
   if (state->map_tab == NULL) {
     return(false);
   }


   state->nz = (int *) malloc(((size_t)h) * sizeof(int));
   if (state->nz == NULL) {
     return(false);
   }

   state->h = h;
   state->w = w;
   
   state->blob = (CCOMP_BLOB *) calloc(state->max_labels+1, sizeof(CCOMP_BLOB));

   state->status = CCOMP_INITIALIZED;

   return(true);
}

bool ccomp_free(CCOMP_STATE *state)
{

   cvReleaseImage(&state->scr);
	
   /* free equivalence table */
   free(state->equiv_tab);
   free(state->map_tab);
   free(state->nz);
   free(state->blob);
   memset((char *) state,0,sizeof(CCOMP_STATE));

   state->status = CCOMP_UNINITIALIZED;

   return(true);
}

bool ccomp_label_regions(CCOMP_STATE *state, IplImage *bimg, 
			   int x0, int y0, int w, int h)
{
   int i,j,k;

   /* Initialize the relevant fields in state. */
   state->label_val=1; state->flabel_val=1;
   (void) memset(state->equiv_tab, 0, 
		 ((size_t)state->max_labels) * sizeof(unsigned int));
	
   cvSetZero(state->scr);

   generate_labels(bimg, state->scr, x0, y0, w, h, state->max_labels,
                   state->equiv_tab, &(state->label_val));

   state->map_tab[0] = (unsigned int) 0;
   for (i=1; i < ((int)state->label_val); i++){ /* generate final region labels */
     if ((state->equiv_tab[i]) == 0)
       state->map_tab[i] = state->flabel_val++;
   }

   for (i=1; i < ((int)state->label_val); i++) { /* discover all equivalences */
     j = i;
     while ((k = ((int)state->equiv_tab[j])) != 0) {
       j = k;
     }
     state->map_tab[i] = state->map_tab[j];
   }

   state->status = CCOMP_LABELS_GENERATED;

   return(true);

//   return(((int)(state->flabel_val - 1)));
}

void ccomp_reset(CCOMP_STATE *state)
{
	state->status = CCOMP_INITIALIZED;
}

IplImage *ccomp_get_labels(CCOMP_STATE *state)
{
	if(state->status < CCOMP_LABELS_GENERATED)
	{
		return(NULL);
	}
	else if(state->status < CCOMP_BLOBS_COMPUTED)
	{
		ccomp_get_blobs(state);
	}

	return(state->scr);
}

int ccomp_get_num_blobs(CCOMP_STATE *state)
{
	return(state->flabel_val - 1);
}

CCOMP_BLOB *ccomp_get_blobs(CCOMP_STATE *state)
{
 
	if(state->status < CCOMP_LABELS_GENERATED)
		return(NULL);
	else if(state->status == CCOMP_BLOBS_COMPUTED) 
		return(state->blob);

	int s;
	int idx;
	IplImage *labels = state->scr;
	unsigned int *map = state->map_tab;
	int num_comps = (state->flabel_val - 1);

	int nx = labels->width;
	int ny = labels->height;
	CCOMP_BLOB *first_blob = state->blob, *blob;
	CvRect *roi;

	if(num_comps > state->max_labels)
		return(NULL);

	for(s=1, blob = first_blob+1;s<=num_comps;s++, blob++)
	{
		blob->num_pixels = 0;
		blob->id = s;
		blob->valid = true;
		roi = &(blob->roi);
		roi->x = state->w; roi->y = state->h; roi->width = -state->w; roi->height = -state->h;
		blob->Cx = blob->Cy = 0;
	}
 
	short *labelptr = 0;
	int labelStep = 0;

	cvGetRawData(labels, (unsigned char **) &labelptr, &labelStep);
	labelStep /= sizeof(short);

	for(int i=0;i<state->h;i++)
	{
		short *lptr = labelptr + labelStep*i;
		
		for(int j=0;j<state->w;j++,lptr++)
		{
			if(idx = map[*lptr])
			{
				*lptr = idx;
				blob = first_blob + idx;
				roi = &(blob->roi);
				blob->num_pixels++;
				if(roi->x > j) roi->x = j;
				if(roi->y > i) roi->y = i;
				if(roi->x + roi->width < j+1) roi->width = j+1-roi->x;
				if(roi->y + roi->height < i+1) roi->height = i+1-roi->y;
				blob->Cx += j;
				blob->Cy += i;
			}
		}
	}

	for(s=1, blob = first_blob+1;s<=num_comps;s++, blob++)
	{
		blob->Cx /= blob->num_pixels;
		blob->Cy /= blob->num_pixels;
	}

	state->status = CCOMP_BLOBS_COMPUTED;

	return(state->blob);

}
/* =================================================================== */
/* end conn_comp.c */
