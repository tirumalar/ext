/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

typedef enum msg_type
{
    MSG_NONE =0,
    CMD_IN,
    RSP_IN,
    
}MSG_TYPE;
#define MSG_MAX_LEN 64
typedef struct
{
    MSG_TYPE msg_type;
    uint8_t msg_len;
    uint8_t data[MSG_MAX_LEN];
}ICM_MSG;

#define CRITICAL_START 
#define CRITICAL_END

#define ICM_QUEUE_SIZE 10

typedef struct
{
    uint8_t front;
    uint8_t rear;
    uint8_t size;
    ICM_MSG q[ICM_QUEUE_SIZE];
}ICM_QUEUE;

void QUEUE_Init(ICM_QUEUE *q)
{
	q->front = 0;
	q->rear = -1;
	q->size = 0;
}
// Utility function to add an element x in the queue


uint8_t QUEUE_Push(ICM_QUEUE *pt, ICM_MSG *m)
{
    if (pt->size==ICM_QUEUE_SIZE)
       return 0;
    
    CRITICAL_START;
	pt->rear = (pt->rear + 1) % ICM_QUEUE_SIZE;	// circular queue
	pt->size++;
    CRITICAL_END;
	pt->q[pt->rear] = *m;
    return 1;
 
}

// Utility function to check if the queue is empty or not
int QUEUE_Empty(ICM_QUEUE *pt)
{
	return pt->size==0;
}

uint8_t QUEUE_Pop(ICM_QUEUE *pt, ICM_MSG *m)
{
	if (QUEUE_Empty(pt)) // front == rear
	{
        return 0;
	}
    *m=pt->q[pt->front];
    CRITICAL_START;
	pt->front = (pt->front + 1) % ICM_QUEUE_SIZE;	// circular queue
	pt->size--;
    CRITICAL_END;
    return 1;
}
   

