
#define BUFLEN 1500
#define IMAGE_SIZE 1152000
#define NPACK 10
// 8192 left camer
// 8193 right

#define min(a,b)((a<b)?(a):(b))
#define max(a,b)((a>b)?(a):(b))
#define WIDTH 1200
#define HEIGHT 960

#define PORT 8193

#define IMAGE_SIZE WIDTH*HEIGHT
typedef struct ImageQueue {
	unsigned char m_ptr[IMAGE_SIZE];	// image size 1152000
	int m_ill0;
	int m_frameIndex;
	__int64_t m_startTime, m_endTime;
};

typedef RingBuffer<ImageQueue> RingBufferImageQueue;

class VideoStream {
public:
	VideoStream(int port);
	~VideoStream();
	int m_port;
	volatile int running;
	int cam_id;
	RingBufferImageQueue *m_pRingBuffer;
	char buf[BUFLEN];
	char offset_image[IMAGE_SIZE];
	char offset_sub_enable;
	char offset_image_loaded;
	pthread_t Thread;
	static void *ThreadServer(void *arg);
	void flush(void);
	int get(int *win, int *hin, char *m_pImageBuffer);
	bool HandleReceiveImage(unsigned char *ptr, int length);
	int GetCamId(void);
	int length;

};

