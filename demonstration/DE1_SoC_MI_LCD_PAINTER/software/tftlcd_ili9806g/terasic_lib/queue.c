#include "terasic_includes.h"
#include "queue.h"




QUEUE_STRUCT* QUEUE_New(int nQueueNum){
    int nSize;
    QUEUE_STRUCT *pQueue;
    nSize = sizeof(QUEUE_STRUCT)+nQueueNum*sizeof(alt_u32);
    pQueue = (QUEUE_STRUCT *)malloc(nSize);
    memset((void *)pQueue, 0, nSize);
    pQueue->num = nQueueNum;
    return pQueue;
}

void QUEUE_Delete(QUEUE_STRUCT *pQueue){
    free(pQueue);
}

bool QUEUE_IsEmpty(QUEUE_STRUCT *pQueue){
    if (pQueue->front == pQueue->rear)
        return TRUE;
    return FALSE;
}

bool QUEUE_IsFull(QUEUE_STRUCT *pQueue){
    if (((pQueue->front+1)%pQueue->num) == pQueue->rear)
        return TRUE;
    return FALSE;
}

bool QUEUE_Push(QUEUE_STRUCT *pQueue, alt_u32 data32){
    if (QUEUE_IsFull(pQueue))
        return FALSE;
    pQueue->data[pQueue->front] = data32;
    pQueue->front = (pQueue->front+1)%pQueue->num;        
    return TRUE;
}

alt_u32 QUEUE_Pop(QUEUE_STRUCT *pQueue){
    alt_u32 data32;
    if (QUEUE_IsEmpty(pQueue))
        return 0;
    data32 = pQueue->data[pQueue->rear];
    pQueue->rear = (pQueue->rear+1)%pQueue->num;        
    return data32;
}

void QUEUE_Empty(QUEUE_STRUCT *pQueue){
    pQueue->front = 0;
    pQueue->rear = 0;
}

