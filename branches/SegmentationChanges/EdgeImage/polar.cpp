#include <math.h>
#include "image.h"
#include <stdio.h>
#include <stdlib.h>
//#include "input.h"
#include "Input1.h"
#include "Input2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <list>
#include <map>

#include "EyeSegmentServer_fixed.h"

//#include <tut/tut.hpp>
//#include <tut/tut_reporter.hpp>

extern "C" {
#include "test_fw.h"
#include "file_manip.h"
#include "EdgeImage_private.h"
}

#define Q15 (1.0*(1<<15))
//#define Q31 (1.0*(1<<31))
#define Q14 (1.0*(1<<14))
#define M_PI 3.14159265358979323846
//#define MAX(a,b) (a>b?a:b)
//#define MIN(a,b) (a<b?a:b)
//segment ("sdram0")
unsigned char OutputImgASM[500*500];
unsigned char OutputImgC[500*500];
unsigned char *InputImg,*OutputImg;
unsigned int *sinCosTable;






void polar_warp_fix_q16(ucImage *img, Point3D_fix irc, Point3D_fix puc, ucImage *output)
{
	short radiusSampling =(short)((1.0f/(output->height))*Q14);
	short angleSampling = (short)(((2 * M_PI)/ (output->width))*Q14);
	unsigned char *inptr = img->data, *outptr = output->data;
	int inStep = img->widthstep, outStep = output->widthstep;

	unsigned int w = img->width;
	unsigned int h = img->height;
	int i,j;

#ifdef FILEIO
	FILE *fp = fopen("Out_Q16.C","w");
#endif
	for( i=0; i< output->height; i++)
	{

		short rad = (short)(i * radiusSampling); //Q14
#ifdef FILEIO

		fprintf(fp,"\nH = %3d  ",i);
		fprintf(fp," Radius sampling = %#08x \n",rad);
#endif
		unsigned char *outC1 = outptr + i * outStep;


		for(j=0; j<output->width; j++, outC1++)
		{

			unsigned int temp2 = sinCosTable[j];
			short ca = 0;//CosTable[j];//cos(angle);//Q14
			short sa = 0;//SinTable[j];//sin(angle);//Q14
			ca = ((int)temp2>>16);
			sa = (((int)(temp2<<16)>>16));

			//16Q16 = Q14*(Q0<<14 + Q0*Q14)) + Q14*(Q0<<14 + Q0*Q14)
			long long int temp = puc.z*ca; //Q14 = 16Q0*Q14
			temp +=(puc.x<<14); //16Q14
			temp = ((0x4000 - rad)*temp)>>12; //Q14*16Q14 = 16Q28 >>12 = Q16
			long long int temp1 = irc.z*ca; //Q14 = Q0*Q14
			temp1 +=(irc.x<<14); //Q14
			temp1 = (rad*temp1)>>12; //Q14*Q14 = Q28 >>12 = Q16
			int xx = temp+temp1;//Q16+Q16 = Q16

			//float xx = rad * (irc.x + irc.z * ca) + (1 - rad)*(puc.x + puc.z*ca);
			//float yy = rad * (irc.y + irc.z * sa) + (1 - rad)*(puc.y + puc.z*sa);

			temp = puc.z*sa; //Q14 = Q0*Q14
			temp +=(puc.y<<14); //Q14
			temp = ((0x4000 - rad)*temp)>>12; //Q14*Q14 = Q28 >>12 = Q16
			temp1 = irc.z*sa; //Q14 = Q0*Q14
			temp1 +=(irc.y<<14); //Q14
			temp1 = (rad*temp1)>>12; //Q14*Q14 = Q28 >>12 = Q16
			int yy = temp+temp1;//Q16+Q16 = Q16

#ifdef FILEIO
			fprintf(fp,"W = %3d ",j);
			fprintf(fp,"XX = %#08x ",xx);
			fprintf(fp,"YY = %#08x ",yy);
#endif
			// use bilinear interpolation
			short intx = (xx>>16);//Q16
			short inty = (yy>>16);//Q16
			unsigned short yfrac = yy&0xffff; //Q16
			unsigned char *fptr1= inptr + inty*inStep;
			unsigned char *fptr2= inptr + (inty+1)*inStep;
			if (((unsigned short) intx) < (w-1) && ((unsigned short) inty) < (h-1))
			{
				unsigned short xfrac = xx&0xffff;
				int y1val= (fptr1[intx]<<16) + xfrac*(fptr1[intx+1] - fptr1[intx]);
				int y2val= (fptr2[intx]<<16) + xfrac*(fptr2[intx+1] - fptr2[intx]);
				temp = ((y2val-y1val));
				temp = (yfrac*temp)>>16;

#ifdef FILEIO

				fprintf(fp,"A = %#02x ",fptr1[intx]);
				fprintf(fp,"B = %#02x ",fptr1[intx+1]);
				fprintf(fp,"A1= %#02x ",fptr2[intx]);
				fprintf(fp,"B1= %#02x ",fptr2[intx+1]);
				fprintf(fp,"Y1= %#08x ",y1val);
				fprintf(fp,"Y2= %#08x ",y2val);
#endif
				y1val = y1val+temp + 0x8000;
				*outC1 =  (y1val)>>16;
#ifdef FILEIO
				fprintf(fp,"Y= %#02x ",*outC1);
#endif
			}
#ifdef FILEIO
			fprintf(fp,"\n");
			fflush(fp);
#endif
		}
	}
#ifdef FILEIO
	fclose(fp);
#endif
}

void polar_warp(ucImage *img, Point3D_fix irc, Point3D_fix puc, ucImage *output)
{
	float radiusSampling = 1.0f/(output->height);
	float angleSampling = (float) (2 * M_PI)/ (output->width);
	unsigned char *inptr = img->data, *outptr = output->data;
	int inStep = img->widthstep, outStep = output->widthstep;

	unsigned int w = img->width;
	unsigned int h = img->height;
	int i,j;
	for(i=0; i< output->height; i++)
	{
		float rad = i * radiusSampling;
		unsigned char *outC1 = outptr + i * outStep;
		for(j=0; j<output->width; j++, outC1++)
		{
			unsigned int temp2 = sinCosTable[j];
			float ca = 0;//CosTable[j];//cos(angle);//Q14
			float sa = 0;//SinTable[j];//sin(angle);//Q14
			ca = ((int)temp2>>16)/(1.0f*(1<<14));
			sa = (((int)(temp2<<16)>>16))/(1.0f*(1<<14));

			float xx = rad * (irc.x + irc.z * ca) + (1 - rad)*(puc.x + puc.z*ca);
			float yy = rad * (irc.y + irc.z * sa) + (1 - rad)*(puc.y + puc.z*sa);
			// use bilinear interpolation
			int intx = floor(xx);
			int inty = floor(yy);
			float yfrac = yy - (float) inty;
			unsigned char *fptr1= inptr + inty*inStep;
			unsigned char *fptr2= inptr + (inty+1)*inStep;
			if (((unsigned int) intx) < (w-1) && ((unsigned int) inty) < (h-1))
			{
				float xfrac = xx - (float)intx;
				float y1val= ((float)fptr1[intx]) + xfrac*(fptr1[intx+1] - fptr1[intx]);
				float y2val= ((float)fptr2[intx]) + xfrac*(fptr2[intx+1] - fptr2[intx]);
				*outC1 = (unsigned char) floor(y1val + 0.5 + yfrac*(y2val-y1val));
			}
		}
	}
}

void GenerateLookupTable(int w)
{	int i;
	unsigned int temp;
	int s = 0x2000,c = 0x2000;
	for(i=0;i<w;i++)
	{
		temp = 0;
		c = ((int)(cos(((2.0*M_PI)/w)*i)*0x4000))&0xFFFF;
		s = ((int)(sin(((2.0*M_PI)/w)*i)*0x4000))&0xFFFF;
		temp = s|(c<<16);
		sinCosTable[i] = temp;
	}

}


void write_out(char *filename,unsigned char *ptr, int w,int h,int step)
{

	FILE *fp = fopen(filename,"w");
	int i,j;

	for( i=0; i< h; i++)
	{
		for( j=0; j< w; j++)
		{
			fprintf(fp,"%#2x ",*(ptr +i*step+j));

		}
		fprintf(fp,"\n");
	}
	fclose(fp);
}



void test_compute_polar_warp()
{

	ucImage inimage;
	ucImage output;
	char *func1="Polar_warping_basic %d",func[30];
	unsigned int param[10];

	int inputStep,inputWidth,inputHeight;
	int outputStep,outputWidth,outputHeight;
	Point3D_fix *irc,*puc;

	int i= 0,j;

	for( j=0;j<8;j++)
	{
		switch(j)
		{
			case 0:
					InputImg = InputImg1;
					OutputImg = OutputImg1;
					inputStep = INPUT_STEP1;
					inputHeight = INPUT_HEIGHT1;
					inputWidth = INPUT_WIDTH1;
					outputStep = OUTPUT_STEP1;
					outputHeight = OUTPUT_HEIGHT1;
					outputWidth = OUTPUT_WIDTH1;
					sinCosTable = sinCosTable1;
					puc = &puc1;
					irc = &irc1;
					break;

			case 1:
					InputImg = InputImg2;
					OutputImg = OutputImg2;
					inputStep = INPUT_STEP2;
					inputHeight = INPUT_HEIGHT2;
					inputWidth = INPUT_WIDTH2;
					outputStep = OUTPUT_STEP2;
					outputHeight = OUTPUT_HEIGHT2;
					outputWidth = OUTPUT_WIDTH2;
					sinCosTable = sinCosTable2;
					puc = &puc2;
					irc = &irc2;
					break;
			case 2:
					InputImg = InputImg3;
					OutputImg = OutputImg3;
					inputStep = INPUT_STEP3;
					inputHeight = INPUT_HEIGHT3;
					inputWidth = INPUT_WIDTH3;
					outputStep = OUTPUT_STEP3;
					outputHeight = OUTPUT_HEIGHT3;
					outputWidth = OUTPUT_WIDTH3;
					sinCosTable = sinCosTable3;
					puc = &puc3;
					irc = &irc3;
					break;

			case 3:
					InputImg = InputImg4;
					OutputImg = OutputImg4;
					inputStep = INPUT_STEP4;
					inputHeight = INPUT_HEIGHT4;
					inputWidth = INPUT_WIDTH4;
					outputStep = OUTPUT_STEP4;
					outputHeight = OUTPUT_HEIGHT4;
					outputWidth = OUTPUT_WIDTH4;
					sinCosTable = sinCosTable4;
					puc = &puc4;
					irc = &irc4;
					break;
			case 4:
					InputImg = InputImg5;
					OutputImg = OutputImg5;
					inputStep = INPUT_STEP5;
					inputHeight = INPUT_HEIGHT5;
					inputWidth = INPUT_WIDTH5;
					outputStep = OUTPUT_STEP5;
					outputHeight = OUTPUT_HEIGHT5;
					outputWidth = OUTPUT_WIDTH5;
					sinCosTable = sinCosTable5;
					puc = &puc5;
					irc = &irc5;
					break;

			case 5:
					InputImg = InputImg6;
					OutputImg = OutputImg6;
					inputStep = INPUT_STEP6;
					inputHeight = INPUT_HEIGHT6;
					inputWidth = INPUT_WIDTH6;
					outputStep = OUTPUT_STEP6;
					outputHeight = OUTPUT_HEIGHT6;
					outputWidth = OUTPUT_WIDTH6;
					sinCosTable = sinCosTable6;
					puc = &puc6;
					irc = &irc6;
					break;
			case 6:
					InputImg = InputImg7;
					OutputImg = OutputImg7;
					inputStep = INPUT_STEP7;
					inputHeight = INPUT_HEIGHT7;
					inputWidth = INPUT_WIDTH7;
					outputStep = OUTPUT_STEP7;
					outputHeight = OUTPUT_HEIGHT7;
					outputWidth = OUTPUT_WIDTH7;
					sinCosTable = sinCosTable7;
					puc = &puc7;
					irc = &irc7;
					break;
			case 7:
					InputImg = InputImg8;
					OutputImg = OutputImg8;
					inputStep = INPUT_STEP8;
					inputHeight = INPUT_HEIGHT8;
					inputWidth = INPUT_WIDTH8;
					outputStep = OUTPUT_STEP8;
					outputHeight = OUTPUT_HEIGHT8;
					outputWidth = OUTPUT_WIDTH8;
					sinCosTable = sinCosTable8;
					puc = &puc8;
					irc = &irc8;
					break;

		}//case
		sprintf(func,func1,j+1);

		for(i=0;i<outputStep*outputHeight;i++)
		{
			OutputImgASM[i] = 0;
			OutputImgC[i] = 0;
		}

		inimage.data = InputImg;
		inimage.widthstep = inputStep;
		inimage.width = inputWidth;
		inimage.height = inputHeight;

		output.data = OutputImgASM;
		output.widthstep = outputStep;
		output.width = outputWidth;
		output.height = outputHeight;


		// PUC.Z,IRC.Z
		param[0] = (((int)puc->x)<<16)|irc->x;
		param[1] = (((int)puc->y)<<16)|irc->y;
		param[2] = (((int)puc->z)<<16)|irc->z;
		param[3] = (unsigned int)(0x80000000u/output.height);
		param[4] = (unsigned int)sinCosTable;
		compute_polar_warping(&inimage,&output,&param[0]);

//		output.data = OutputImgC;
//		polar_warp(&inimage,*irc,*puc,&output);// Actual float func..
		//compute_polar_warping_NN(&inimage,&output,&param[0]);
//		if(j == 0)
//		{
//			savefile_OfSize_asPGM(output.data,output.widthstep, output.height,"result.pgm");
//			savefile_OfSize_asPGM(inimage.data,inimage.widthstep, inimage.height,"input.pgm");
//		}
		//tut::ensure(ensure_results_byte_distance(func,OutputImg,OutputImgASM,outputWidth*outputHeight,3));

//		ensure_results(func,OutputImg,OutputImgC,outputWidth*outputHeight);

	//	write_out("ASM_fixQ6.txt",OutputImgASM,OUTPUT_WIDTH,OUTPUT_HEIGHT,OUTPUT_WIDTH);

	}

}


void test_compute_polar_warpNN()
{

	ucImage inimage;
	ucImage output;
	char *func1="Polar_warping_NN_basic %d",func[30];
	unsigned int param[10];

	int inputStep,inputWidth,inputHeight;
	int outputStep,outputWidth,outputHeight;
	Point3D_fix *irc,*puc;

	int i= 0,j;

	for( j=0;j<8;j++)
	{
		switch(j)
		{
			case 0:
					InputImg = InputImg1;
					OutputImg = OutputImgNN1;
					inputStep = INPUT_STEP1;
					inputHeight = INPUT_HEIGHT1;
					inputWidth = INPUT_WIDTH1;
					outputStep = OUTPUT_STEP1;
					outputHeight = OUTPUT_HEIGHT1;
					outputWidth = OUTPUT_WIDTH1;
					sinCosTable = sinCosTable1;
					puc = &puc1;
					irc = &irc1;
					break;

			case 1:
					InputImg = InputImg2;
					OutputImg = OutputImgNN2;
					inputStep = INPUT_STEP2;
					inputHeight = INPUT_HEIGHT2;
					inputWidth = INPUT_WIDTH2;
					outputStep = OUTPUT_STEP2;
					outputHeight = OUTPUT_HEIGHT2;
					outputWidth = OUTPUT_WIDTH2;
					sinCosTable = sinCosTable2;
					puc = &puc2;
					irc = &irc2;
					break;
			case 2:
					InputImg = InputImg3;
					OutputImg = OutputImgNN3;
					inputStep = INPUT_STEP3;
					inputHeight = INPUT_HEIGHT3;
					inputWidth = INPUT_WIDTH3;
					outputStep = OUTPUT_STEP3;
					outputHeight = OUTPUT_HEIGHT3;
					outputWidth = OUTPUT_WIDTH3;
					sinCosTable = sinCosTable3;
					puc = &puc3;
					irc = &irc3;
					break;

			case 3:
					InputImg = InputImg4;
					OutputImg = OutputImgNN4;
					inputStep = INPUT_STEP4;
					inputHeight = INPUT_HEIGHT4;
					inputWidth = INPUT_WIDTH4;
					outputStep = OUTPUT_STEP4;
					outputHeight = OUTPUT_HEIGHT4;
					outputWidth = OUTPUT_WIDTH4;
					sinCosTable = sinCosTable4;
					puc = &puc4;
					irc = &irc4;
					break;
			case 4:
					InputImg = InputImg5;
					OutputImg = OutputImgNN5;
					inputStep = INPUT_STEP5;
					inputHeight = INPUT_HEIGHT5;
					inputWidth = INPUT_WIDTH5;
					outputStep = OUTPUT_STEP5;
					outputHeight = OUTPUT_HEIGHT5;
					outputWidth = OUTPUT_WIDTH5;
					sinCosTable = sinCosTable5;
					puc = &puc5;
					irc = &irc5;
					break;

			case 5:
					InputImg = InputImg6;
					OutputImg = OutputImgNN6;
					inputStep = INPUT_STEP6;
					inputHeight = INPUT_HEIGHT6;
					inputWidth = INPUT_WIDTH6;
					outputStep = OUTPUT_STEP6;
					outputHeight = OUTPUT_HEIGHT6;
					outputWidth = OUTPUT_WIDTH6;
					sinCosTable = sinCosTable6;
					puc = &puc6;
					irc = &irc6;
					break;
			case 6:
					InputImg = InputImg7;
					OutputImg = OutputImgNN7;
					inputStep = INPUT_STEP7;
					inputHeight = INPUT_HEIGHT7;
					inputWidth = INPUT_WIDTH7;
					outputStep = OUTPUT_STEP7;
					outputHeight = OUTPUT_HEIGHT7;
					outputWidth = OUTPUT_WIDTH7;
					sinCosTable = sinCosTable7;
					puc = &puc7;
					irc = &irc7;
					break;
			case 7:
					InputImg = InputImg8;
					OutputImg = OutputImgNN8;
					inputStep = INPUT_STEP8;
					inputHeight = INPUT_HEIGHT8;
					inputWidth = INPUT_WIDTH8;
					outputStep = OUTPUT_STEP8;
					outputHeight = OUTPUT_HEIGHT8;
					outputWidth = OUTPUT_WIDTH8;
					sinCosTable = sinCosTable8;
					puc = &puc8;
					irc = &irc8;
					break;

		}//case
		sprintf(func,func1,j+1);

		for(i=0;i<outputStep*outputHeight;i++)
		{
			OutputImgASM[i] = 0;
			OutputImgC[i] = 0;
		}

		inimage.data = InputImg;
		inimage.widthstep = inputStep;
		inimage.width = inputWidth;
		inimage.height = inputHeight;

		output.data = OutputImgASM;
		output.widthstep = outputStep;
		output.width = outputWidth;
		output.height = outputHeight;


		// PUC.Z,IRC.Z
		param[0] = (((int)puc->x)<<16)|irc->x;
		param[1] = (((int)puc->y)<<16)|irc->y;
		param[2] = (((int)puc->z)<<16)|irc->z;
		param[3] = (unsigned int)(0x80000000u/output.height);
		param[4] = (unsigned int)sinCosTable;
		compute_polar_warping_NN(&inimage,&output,&param[0]);

//		char *buf = "OutputImgNN%d";
//		char buff[100];
//		sprintf(buff,buf,j+1);
//		FILE *fp = fopen("out.txt","a");
//		fprintf(fp,"unsigned char %s[] = {",buff);
//		for(int k=0;k<outputHeight;k++){
//			for(int i=0;i<outputStep;i++){
//				fprintf(fp,"%d ,",OutputImgASM[k*outputStep+i]);
//			}
//			fprintf(fp,"\n");
//		}
//		fprintf(fp,"};\n");
//		fclose(fp);

		//tut::ensure(ensure_results(func,OutputImg,OutputImgASM,outputWidth*outputHeight));
	}
}



void test_EigenVal(void)
{
	int w= 20;
	int h= 20;
	int Param[10]={0};
	int *temp= (int*)calloc(5*64*25,4);
	float k = 65536.0*3.0f/(64.0f);
	int *C_Eigen = (int*)calloc(w*h,4);
	int ASM_Eigen[] =
	{
	-84387,-84387,-84387,224244,104573,366332,415163,755040,603000,921560,722540,1307060,89076,74638,74638,74638,
	779200,779200,779200,1731885,1759458,1762804,1434130,1796138,1465689,2522525,2536698,4020356,2053752,1955148,1955148,1955148,
	6419588,6419588,6419588,4867088,6648121,4232431,4806126,7987790,6530639,6052393,8160392,9158154,6237294,5315602,5315602,5315602,
	15589452,15589452,15589452,14022518,17158296,12606900,13366335,16675651,14208465,14029807,14750981,16352511,11457351,10541943,10541943,10541943,
	23773391,23773391,23773391,20395173,23611791,19170958,19074261,23471824,21666781,23796640,22962015,24071427,19770615,19111623,19111623,19111623,
	23861094,23861094,23861094,19233584,24675523,19324312,18757211,19582852,20508531,19795700,20092974,19780701,20650586,18574413,18574413,18574413,
	19355211,19355211,19355211,14399006,17731978,16590077,21361741,22623309,22360091,23136605,22424168,17891081,19818902,19154193,19154193,19154193,
	15228885,15228885,15228885,9314549,9589108,11136206,16158878,15126596,17247147,20361165,17805717,13662524,17280791,17167268,17167268,17167268,
	13724068,13724068,13724068,11002395,11018613,13052563,13356370,11849200,12539189,15205480,12857326,11598034,16169909,15853545,15853545,15853545,
	13410983,13410983,13410983,15534565,14338844,13902369,13233380,10485927,9698400,10665133,10518983,9694480,13226380,12316690,12316690,12316690,
	14360775,14360775,14360775,16514950,18558839,17392279,17631063,12844075,13887023,14112012,12301737,10096382,11935794,10484664,10484664,10484664,
	14685719,14685719,14685719,18348684,19982369,23817110,24504086,20605569,20611172,19009186,14464522,10916948,9313115,8008513,8008513,8008513,
	16183580,16183580,16183580,19676645,21632253,29628415,31713336,31383284,35730063,31984629,21987188,22156567,15197948,7569759,7569759,7569759,
	15028418,15028418,15028418,16314605,20096391,23900125,34000458,37931056,41133963,34646667,29411697,21504266,14635321,8755346,8755346,8755346,
	13782101,13782101,13782101,13838308,16609536,23317921,34125511,39433460,44252922,35831635,23514986,18554489,14385997,8583333,8583333,8583333,
	};
	int  t_maxMinEigValueC = -1;
	int  t_maxMinEigValueAsm = -1;

	char image[]={
	103 ,198 ,105 ,115 ,81 ,255 ,74 ,236 ,41 ,205 ,186 ,171 ,242 ,251 ,227 ,70 ,124 ,194 ,84 ,248 ,
	27 ,232 ,231 ,141 ,118 ,90 ,46 ,99 ,51 ,159 ,201 ,154 ,102 ,50 ,13 ,183 ,49 ,88 ,163 ,90 ,
	37 ,93 ,5 ,23 ,88 ,233 ,94 ,212 ,171 ,178 ,205 ,198 ,155 ,180 ,84 ,17 ,14 ,130 ,116 ,65 ,
	33 ,61 ,220 ,135 ,112 ,233 ,62 ,161 ,65 ,225 ,252 ,103 ,62 ,1 ,126 ,151 ,234 ,220 ,107 ,150 ,
	143 ,56 ,92 ,42 ,236 ,176 ,59 ,251 ,50 ,175 ,60 ,84 ,236 ,24 ,219 ,92 ,2 ,26 ,254 ,67 ,
	251 ,250 ,170 ,58 ,251 ,41 ,209 ,230 ,5 ,60 ,124 ,148 ,117 ,216 ,190 ,97 ,137 ,249 ,92 ,187 ,
	168 ,153 ,15 ,149 ,177 ,235 ,241 ,179 ,5 ,239 ,247 ,0 ,233 ,161 ,58 ,229 ,202 ,11 ,203 ,208 ,
	72 ,71 ,100 ,189 ,31 ,35 ,30 ,168 ,28 ,123 ,100 ,197 ,20 ,115 ,90 ,197 ,94 ,75 ,121 ,99 ,
	59 ,112 ,100 ,36 ,17 ,158 ,9 ,220 ,170 ,212 ,172 ,242 ,27 ,16 ,175 ,59 ,51 ,205 ,227 ,80 ,
	72 ,71 ,21 ,92 ,187 ,111 ,34 ,25 ,186 ,155 ,125 ,245 ,11 ,225 ,26 ,28 ,127 ,35 ,248 ,41 ,
	248 ,164 ,27 ,19 ,181 ,202 ,78 ,232 ,152 ,50 ,56 ,224 ,121 ,77 ,61 ,52 ,188 ,95 ,78 ,119 ,
	250 ,203 ,108 ,5 ,172 ,134 ,33 ,43 ,170 ,26 ,85 ,162 ,190 ,112 ,181 ,115 ,59 ,4 ,92 ,211 ,
	54 ,148 ,179 ,175 ,226 ,240 ,228 ,158 ,79 ,50 ,21 ,73 ,253 ,130 ,78 ,169 ,8 ,112 ,212 ,178 ,
	138 ,41 ,84 ,72 ,154 ,10 ,188 ,213 ,14 ,24 ,168 ,68 ,172 ,91 ,243 ,142 ,76 ,215 ,45 ,155 ,
	9 ,66 ,229 ,6 ,196 ,51 ,175 ,205 ,163 ,132 ,127 ,45 ,173 ,212 ,118 ,71 ,222 ,50 ,28 ,236 ,
	74 ,196 ,48 ,246 ,32 ,35 ,133 ,108 ,251 ,178 ,7 ,4 ,244 ,236 ,11 ,185 ,32 ,186 ,134 ,195 ,
	62 ,5 ,241 ,236 ,217 ,103 ,51 ,183 ,153 ,80 ,163 ,227 ,20 ,211 ,217 ,52 ,247 ,94 ,160 ,242 ,
	16 ,168 ,246 ,5 ,148 ,1 ,190 ,180 ,188 ,68 ,120 ,250 ,73 ,105 ,230 ,35 ,208 ,26 ,218 ,105 ,
	106 ,126 ,76 ,126 ,81 ,37 ,179 ,72 ,132 ,83 ,58 ,148 ,251 ,49 ,153 ,144 ,50 ,87 ,68 ,238 ,
	155 ,188 ,233 ,229 ,37 ,207 ,8 ,245 ,233 ,226 ,94 ,83 ,96 ,170 ,210 ,178 ,208 ,133 ,250 ,84
	};

	CvRect rect;
	rect.x = rect.y = 4;
	rect.width = rect.height =15;

	rect.x = MAX(4,(rect.x>>2)<<2);
	rect.width = (rect.width+3)&(~3);
	rect.y = MAX(1,rect.y);
	rect.height = MIN(h-1,rect.height);

	assert((rect.width*4)*25 <1000*5*4);
	memset(temp, 0, (rect.width*4)*25);

	Param[0] = rect.width;//input w
	Param[1] = rect.height;//input h
	Param[2] = w; //inputstep
	Param[3] = rect.width*4; //Actual outputstep for other int buffer //int buffers
	Param[4] = (int)temp; //tempbuffer
	Param[5] = (int)k; //constant K

	char* imagedata = image +(rect.y*w)+rect.x;
	t_maxMinEigValueC = compute_EigenVals((unsigned char*)imagedata,(int*)C_Eigen, Param);

	// ensure_results_int_allowrounding("EIGEN VALUES",C_Eigen,ASM_Eigen, Param[0]*Param[1]);

	unsigned char maskASM[]={
	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,000,000,000,000,000,255,255,255,000,000,000,000,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	};
	unsigned char *maskC= (unsigned char*)calloc(w*h,1);

	float threshold = 0.01*256*4*4*25;
	threshold *= threshold;

	int t_eigenTH = ( t_maxMinEigValueC > (int)(threshold+0.5) )? t_maxMinEigValueC/20:t_maxMinEigValueC/16;


	Param[0] = rect.width;
	Param[1] = rect.height;
	Param[2] = rect.width*4;
	Param[3] = w;
	Param[4] = t_eigenTH;


	unsigned char *data = maskC+(rect.y*w)+rect.x;
	compute_EigenMask(C_Eigen,data,Param);

	//tut::ensure(ensure_results_byte_distance("MASK EIGEN",maskASM,maskC,w*h,1));

}
