#ifndef QUEUE_H_
#define QUEUE_H_


typedef struct{
    alt_u32 num;
    alt_u32 front;
    alt_u32 rear;
    alt_u32 data[0];
    alt_u32 mydata[2]; // custom data
}QUEUE_STRUCT;

QUEUE_STRUCT* QUEUE_New(int nQueueNum);
void QUEUE_Delete(QUEUE_STRUCT *pQueue);
bool QUEUE_IsEmpty(QUEUE_STRUCT *pQueue);
bool QUEUE_IsFull(QUEUE_STRUCT *pQueue);
bool QUEUE_Push(QUEUE_STRUCT *pQueue, alt_u32 data32);
alt_u32 QUEUE_Pop(QUEUE_STRUCT *pQueue);
void QUEUE_Empty(QUEUE_STRUCT *pQueue);

#endif /*QUEUE_H_*/
