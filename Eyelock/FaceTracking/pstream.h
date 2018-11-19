
#define BUFLEN 1500
//#define IMAGE_SIZE 1152000
#define NPACK 10
// 8192 left camer
// 8193 right

#define min(a,b)((a<b)?(a):(b))
#define max(a,b)((a>b)?(a):(b))
#define WIDTH 1200
#define HEIGHT 960

#define PORT 8193

#define IMAGE_SIZE WIDTH*HEIGHT

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
	VideoStream(int port);
	~VideoStream();
	int m_port;
	volatile int running;
	int cam_id;
	RingBufferImageQueueF *m_pRingBuffer;

	RingBufferImageQueueF *m_FreeBuffer;
	RingBufferImageQueueF *m_ProcessBuffer;

	ImageQueueItemF m_ImageQueueItem;
	ImageQueueItemF m_current_process_queue_item;

	char buf[BUFLEN];
	char offset_image[IMAGE_SIZE];
	char offset_sub_enable;
	char offset_image_loaded;
	pthread_t Thread;
	static void *ThreadServer(void *arg);

	ImageQueueItemF GetFreeBuffer();
	void ReleaseProcessBuffer(ImageQueueItemF m);
	void PushProcessBuffer(ImageQueueItemF m);

	void flush(void);
	int get(int *win, int *hin, char *m_pImageBuffer, char get_last=0);
	//bool HandleReceiveImage(unsigned char *ptr, int length);
	int GetCamId(void);
	int length;

	int frameId;

};
