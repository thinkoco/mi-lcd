#include "terasic_includes.h"
#include "vip_fr.h"


//#define ENABLE_DMA

#ifdef ENABLE_DMA
    #include <altera_avalon_sgdma.h>;
    #include <altera_avalon_sgdma_descriptor.h>
    #include <altera_avalon_sgdma_regs.h>
    void my_memcpy(void *pDes, void *pSrc, int nSize);
#endif    

void FrameReader_SetFrame0(alt_u32 VipBase, alt_u32 FrameBase, alt_u32 words, alt_u32 cycle, alt_u32 width, alt_u32 height, alt_u32 interlace);
void FrameReader_SetFrame1(alt_u32 VipBase, alt_u32 FrameBase, alt_u32 words, alt_u32 cycle, alt_u32 width, alt_u32 height, alt_u32 interlace);
void FrameReader_SelectFrame(alt_u32 VipBase, alt_u8 FrameIndex);
void FrameReader_Go(alt_u32 VipBase, bool bGO);



VIP_FRAME_READER* VIPFR_Init(alt_u32 VipBase, void* Frame0_Base, void* Frame1_Base, alt_u32 Frame_Width, alt_u32 Frame_Height){

    VIP_FRAME_READER *p;
    
    p = malloc(sizeof(VIP_FRAME_READER));
    p->VipBase = VipBase;
    p->Frame0_Base = Frame0_Base;
    p->Frame1_Base = Frame1_Base;
    p->DisplayFrame = 0;
     
    p->bytes_per_pixel = 4;
    p->color_depth = 32;
    p->interlace = 0;
    
    FrameReader_Go(VipBase, FALSE); // stop for config
    
    VIPFR_SetFrameSize(p, Frame_Width, Frame_Height);
    
    FrameReader_SelectFrame(VipBase, p->DisplayFrame);
    //
    FrameReader_Go(VipBase, TRUE); // go
    
    return p;
}

void VIPFR_UnInit(VIP_FRAME_READER* p){
    if (p)
        free(p);
}

void VIPFR_SetFrameSize(VIP_FRAME_READER* p, int width, int height){
    alt_u32 words, cycle;
    words = width*height;
    cycle = width*height;
    
  //  p->Frame_Width = width;
  //  p->Frame_Height = height;
    //
    p->width = width;
    p->height = height;
    //
    FrameReader_SetFrame0(p->VipBase, (alt_u32)p->Frame0_Base, words, cycle, p->width, p->height, p->interlace);
    FrameReader_SetFrame1(p->VipBase, (alt_u32)p->Frame1_Base, words, cycle, p->width, p->height, p->interlace);
    
}

void VIPFR_Go(VIP_FRAME_READER* p, bool bGo){
    IOWR(p->VipBase, 0x00, bGo?0x01:0x00);
}

void* VIPFR_GetDrawFrame(VIP_FRAME_READER* p){
        if (p->DisplayFrame == 0)
            return p->Frame1_Base;
        return p->Frame0_Base;
}
void VIPFR_ActiveDrawFrame(VIP_FRAME_READER* p){
     p->DisplayFrame =  (p->DisplayFrame+1)%2;
     FrameReader_SelectFrame(p->VipBase, p->DisplayFrame);
     alt_dcache_flush_all();
}

void DRAW_EraseScreen(VIP_FRAME_READER *p, alt_u32 Color){
    memset(VIPFR_GetDrawFrame(p), Color, p->width*p->height*p->bytes_per_pixel);
}

////////////////////////////////////////////////////////////////////
// internal function
////////////////////////////////////////////////////////////////////


void FrameReader_SetFrame0(alt_u32 VipBase, alt_u32 FrameBase, alt_u32 words, alt_u32 cycle, alt_u32 width, alt_u32 height, alt_u32 interlace){
    IOWR(VipBase, 4, FrameBase); // frame0 base address
    IOWR(VipBase, 5, words); // frame0 words
    IOWR(VipBase, 6, cycle); // frame0 single cylce color pattern
    IOWR(VipBase, 8, width); // frame0 width
    IOWR(VipBase, 9, height); // frame0 height
    IOWR(VipBase, 10, interlace); // frame0 interlaced
}

void FrameReader_SetFrame1(alt_u32 VipBase, alt_u32 FrameBase, alt_u32 words, alt_u32 cycle, alt_u32 width, alt_u32 height, alt_u32 interlace){
    IOWR(VipBase, 11, FrameBase); // frame0 base address
    IOWR(VipBase, 12, words); // frame0 words
    IOWR(VipBase, 13, cycle); // frame0 single cylce color pattern
    IOWR(VipBase, 15, width); // frame0 width
    IOWR(VipBase, 16, height); // frame0 height
    IOWR(VipBase, 17, interlace); // frame0 interlaced
}

void FrameReader_SelectFrame(alt_u32 VipBase, alt_u8 FrameIndex){
    IOWR(VipBase, 3, FrameIndex);
}        

void FrameReader_Go(alt_u32 VipBase, bool bGO){
    IOWR(VipBase, 0, bGO?0x01:0x00);
}



///////////////////////////////////////////////////////////////


void VIPFR_ReserveBackground(VIP_FRAME_READER* p){
    alt_u32 *pSrc, *pDes;
    int nSize;
    
    nSize = p->width * p->height * p->bytes_per_pixel;
        
    
    if (p->DisplayFrame == 0){
        pSrc = p->Frame0_Base;
        pDes = p->Frame1_Base;
    }else{
        pDes = p->Frame0_Base;
        pSrc = p->Frame1_Base;
    }
#ifdef ENABLE_DMA    
    my_memcpy(pDes, pSrc, nSize);
#else    
    memcpy(pDes, pSrc, nSize);
#endif //ENABLE_DMA    
}


#ifdef ENABLE_DMA

void my_memcpy(void *pDes, void *pSrc, int nSize){
    alt_sgdma_dev *dma;
    alt_sgdma_descriptor *desc;
    const int nMaxBlockSize = 32*1024; // 32KB
    int nBlockSize, nRemaindSize, nCnt;
    char *pDesBlock, *pSrcBlock;
    int nIndex = 0;

    // open dma
    dma = alt_avalon_sgdma_open(SGDMA_FRAME_NAME);
    if (!dma){
        printf("dma not found\r\n");
        return;
    }        

    // builder descriptor
    nCnt = (nSize+nMaxBlockSize-1)/nMaxBlockSize;
    desc = (alt_sgdma_descriptor *)malloc(sizeof(alt_sgdma_descriptor)*nCnt);
    
    nRemaindSize = nSize;
    pDesBlock = pDes;
    pSrcBlock = pSrc;
    while(nRemaindSize > 0){
        nBlockSize = nMaxBlockSize;
        if (nBlockSize > nRemaindSize)
            nBlockSize = nRemaindSize;
        nRemaindSize -= nBlockSize;
        alt_avalon_sgdma_construct_mem_to_mem_desc(desc+nIndex, (nRemaindSize ==0)?0:(desc+nIndex+1), 
                (alt_u32 *)pDesBlock, (alt_u32 *)pSrcBlock, nBlockSize, 0, 0);
        pDesBlock += nBlockSize;
        pSrcBlock += nBlockSize;
        nIndex++;
    }

    // start sync transfer. block until transfer is done    
    alt_avalon_sgdma_do_sync_transfer(dma, desc);
    
    // free resource
    free(desc);        
}

#endif // ENABLE_DMA

