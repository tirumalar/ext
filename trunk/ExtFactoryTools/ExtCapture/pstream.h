#include "logging.h"
#include "FileConfiguration.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#define BUFLEN 1500
//#define IMAGE_SIZE 1152000
#define NPACK 10
// 8192 left camer
// 8193 right

#define min(a,b)((a<b)?(a):(b))
#define max(a,b)((a>b)?(a):(b))

//#define WIDTH 1200
//#define HEIGHT 960

#define PORT 8193

//#define IMAGE_SIZE WIDTH*HEIGHT

struct ImageQueueItemF {
	unsigned char *m_ptr;	// image size 1152000
	int m_ill0;
	int m_frameIndex;
	__int64_t m_startTime, m_endTime;
	int item_id;
};

typedef RingBuffer<ImageQueueItemF> RingBufferImageQueueF;

class VideoStream {
public:
	VideoStream(int port, bool ImageAuthFlag);
	~VideoStream();
	int m_port;
	bool m_UseImageAuthentication;

	int rgbLEDBrightness;

	volatile int running;
	int cam_id;
	RingBufferImageQueueF *m_pRingBuffer;

	RingBufferImageQueueF *m_FreeBuffer;
	RingBufferImageQueueF *m_ProcessBuffer;

	ImageQueueItemF m_ImageQueueItem;
	ImageQueueItemF m_current_process_queue_item;

	char buf[BUFLEN];
	char offset_sub_enable;
	char offset_image_loaded;
	pthread_t Thread;
	static void *ThreadServer(void *arg);

	ImageQueueItemF GetFreeBuffer();
	void ReleaseProcessBuffer(ImageQueueItemF m);
	void PushProcessBuffer(ImageQueueItemF m);

	void flush(void);
	int get(int *win, int *hin, char *m_pImageBuffer, bool bDebugFlag, char get_last=0);
	//bool HandleReceiveImage(unsigned char *ptr, int length);
	int GetCamId(void);
	int m_ImageSize;
	int m_ImageWidth;
	int m_ImageHeight;

	int frameId;

	unsigned short calc_syndrome(unsigned short syndrome, unsigned short p);
	unsigned short syndrome;
	unsigned short seed;
	void SetSeed(unsigned short sd);
};

