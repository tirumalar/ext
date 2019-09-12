#define AUDIO_STORE_SIZE 32000
#define NUM_AUD_STORES 4

#define DATA_STORE_IMAGE NUM_AUD_STORES
typedef struct
{
    char audio[AUDIO_STORE_SIZE];
    int len;
}AUDIO_STORE;

extern AUDIO_STORE a_store[NUM_AUD_STORES];

void data_store_set(int id);
char * data_store_info(int id, int *max_len);
extern int current_store_len;
int data_store_get_cur_len(void);
