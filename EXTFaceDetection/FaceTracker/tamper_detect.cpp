
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <chrono>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#include "log.h"
#include <vector>

#include "portcom.h"
static string fileName = "/home/root/data/calibration/tempOutput.csv";

const char logger[30] = "test";
float average(vector<float> vec){

	float sum  = 0.0;
	int n = 0;
	for(int i = 0; i < vec.size(); i++){
		sum = sum + vec[i];
		//printf("Data:::: %3.4f\n", vec[i]);
		n++;
	}

	float mean = sum/n;
	//printf("n:::::%d     MEan:::: %4.4f\n",n,mean);

	return mean;

}
vector<float> tempDataX,tempDataY,tempDataZ;
float xp =0,yp = 0,zp =0;
double difX,difY,difZ;

void DoTamper(void)
{
int len;
float x, y, z, a;
char temp[100];


		float t = clock();
		if ((len = port_com_send_return("accel", temp, 20)) > 0) {
			//printf("Inside tempering ------------------------------>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
			EyelockLog(logger, TRACE, "got data %d", len);
			temp[len] = 0;
			sscanf(temp, "%f %f %f %f", &x, &y, &z, &a);
			EyelockLog(logger, TRACE, "Buffer =>%s\n", temp);
			EyelockLog(logger, TRACE, "%3.3f %3.3f %3.3f %3.3f  readTime=%2.4f\n",x, y, z, a, (float) (clock() - t) / CLOCKS_PER_SEC );
			EyelockLog(logger, TRACE, "ACCEL reading ------------------------------------------------>>>>>>>>>>>>>>>>\n");
			EyelockLog(logger, TRACE, "x::: %3.3f y::: %3.3f z::: %3.3f a::: %3.3f\n", x,y,z,a);

			ofstream writeFile(fileName, std::ios_base::app);


/*			using std::chrono::system_clock;

			std::chrono::duration<int,std::ratio<60*60*24> > one_day (1);

			system_clock::time_point today = system_clock::now();

			std::time_t tt;

			tt = system_clock::to_time_t ( today );

			//writeFile <<  ctime(&tt) << ',';
			writeFile <<  x << ',';
			writeFile <<  y << ',';
			writeFile <<  z << ',';*/



			difX = fabs(x - xp);
			difY = fabs(y - yp);
			difZ = fabs(z - zp);
			xp = x; yp = y; zp = z;

/*			writeFile <<  difX << ',';
			writeFile <<  difY << ',';
			writeFile <<  difZ << ',';
			writeFile <<  a << '\n';*/

			int numData = 10;
			if(tempDataX.size()<=numData)
			{
				//printf("monitoring temparing-------------->>>>>>>>>! \n");
				tempDataX.push_back(difX);
				tempDataY.push_back(difY);
				tempDataZ.push_back(difZ);
			}
			//tempDataX.erase(0,1);


			if (tempDataX.size() > numData){

				tempDataX.erase(tempDataX.begin(),tempDataX.begin()+2);
				tempDataY.erase(tempDataY.begin(),tempDataY.begin()+2);
				tempDataZ.erase(tempDataZ.begin(),tempDataZ.begin()+2);

				float xAvg = average(tempDataX);
				float yAvg = average(tempDataY);
				float zAvg = average(tempDataZ);
				//printf("AVG val :::::: %3.3f, %3.3f, %3.3f -------------->>>>>>>>>! \n", xAvg,yAvg,zAvg);
				EyelockLog(logger, TRACE, "AVG val :::::: %3.3f, %3.3f, %3.3f -------------->>>>>>>>>! \n", xAvg,yAvg,zAvg);
				if (xAvg > 0.01 & yAvg > 0.01 & zAvg > 0.08){
					printf("TEMPER DETECTED!!!!!!!!!!!!!!!!!!!!!!------------------>>>>>>>>>>> \n");
					system("touch /home/root/OIMtamper");
					//exit(EXIT_FAILURE);;
				}
				//char s = cv::waitKey(1);
/*				if (s == 'q')
					break;
				else{}*/


				tempDataX.clear();
				tempDataY.clear();
				tempDataZ.clear();
			}


		}
}
