#ifndef _IMAGE_H_
#define _IMAGE_H_


typedef struct usImage
{
	unsigned short *data;
	int widthstep;
}usImage;

typedef struct uiImage
{
	unsigned int *data;
	int widthstep;
}uiImage;

typedef struct Pointparam
{
	unsigned short C00, C01, C10, C11;
	int left_offset, right_offset;
	int sx, sy;
	unsigned char *inpimage;
	int widthstep;
}Pointparam;

typedef struct ucImage
{
	unsigned char *data;

	int widthstep;
	int width;
	int height;

}ucImage;


typedef struct Point3D_fix
{
	short x;
	short y;
	short z;

}Point3D_fix;

#endif

