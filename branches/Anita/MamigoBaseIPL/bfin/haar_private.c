#include "haar_private.h"


void set_roi_planar_ptr(int *img, int widthStep, MAMIGO_Rect rect, int *p)
{
	int step = widthStep >> 2;

	p[0] = (int) (img 	+ step*rect.y + rect.x);
	p[1] = (int) (img 	+ step*rect.y + rect.x + rect.width);
	p[2] = (int) (img 	+ step*(rect.y + rect.height) + rect.x);
	p[3] = (int) (img 	+ step*(rect.y + rect.height) + rect.x + rect.width);

}


void set_roi_ptr(int *img, int widthStep, MAMIGO_Rect rect, int *p)
{
	int step = widthStep >> 2;

	p[0] = (int) (img 	+ step*rect.y + (rect.x << 1));
	p[1] = (int) (img 	+ step*rect.y + ((rect.x + rect.width) << 1));
	p[2] = (int) (img 	+ step*(rect.y + rect.height) + (rect.x << 1));
	p[3] = (int) (img 	+ step*(rect.y + rect.height) + ((rect.x + rect.width) << 1));

}

void set_roi_ptr_type_0(int *img, int widthStep, int *p, int *op)
{
	int step = widthStep >> 2;

	op[0] = (int) (img 	+ step*p[0] + p[4]);
	op[1] = (int) (img  + step*p[1] + p[4]);
	op[2] = (int) (img  + step*p[2] + p[4]);
	op[3] = op[4] = op[5] = op[7] = 0;
	op[6] = p[6] - p[4];

}

void set_roi_ptr_type_1(int *img, int widthStep, int *p, int *op)
{
	int step = widthStep >> 2;

	op[0] = (int) (img 	+ step*p[0] + p[4]);
	op[1] = op[3] = 0;
	op[2] = (int) (img  + step*p[2] + p[4]);

	op[4] = op[7] = 0;
	op[5] = p[5] - p[4];
	op[6] = p[6] - p[4];
}

void set_roi_ptr_type_2(int *img, int widthStep, int *p, int *op)
{
	int step = widthStep >> 2;

	op[0] = (int) (img 	+ step*p[0] + p[4]);
	op[1] = (int) (img  + step*p[1] + p[4]);
	op[2] = (int) (img  + step*p[2] + p[4]);
	op[3] = (int) (img  + step*p[3] + p[4]);

	op[4] = op[5] = op[7] = 0;
	op[6] = p[6] - p[4];
}

void set_roi_ptr_type_3(int *img, int widthStep, int *p, int *op)
{
	int step = widthStep >> 2;

	op[0] = (int) (img 	+ step*p[0] + p[4]);
	op[1] = 0;
	op[2] = (int) (img  + step*p[2] + p[4]);
	op[3] = 0;

	op[4] = 0;
	op[5] = p[5] - p[4];
	op[6] = p[6] - p[4];
	op[7] = p[7] - p[4];
}

void set_roi_ptr_type_4(int *img, int widthStep, int *p, int *op)
{
	int step = widthStep >> 2;

	op[0] = (int) (img 	+ step*p[0] + p[4]);
	op[1] = (int) (img  + step*p[1] + p[4]);
	op[2] = (int) (img  + step*p[2] + p[4]);
	op[3] = 0;

	op[4] = 0;
	op[5] = p[5] - p[4];
	op[6] = p[6] - p[4];
	op[7] = 0;

}

