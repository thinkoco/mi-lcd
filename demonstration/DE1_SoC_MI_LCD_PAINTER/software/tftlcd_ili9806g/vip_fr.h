#ifndef VIP_FR_H_
#define VIP_FR_H_

typedef struct{
    alt_u32 VipBase;
    void* Frame0_Base;
    void* Frame1_Base;
 //   alt_u32 Frame_Width;
  //  alt_u32 Frame_Height;
    alt_u8  DisplayFrame; // 0 or 1
    
    // for altera vip library
    int color_depth;
    int width;
    int height;
    int bytes_per_pixel;
    int interlace;
}VIP_FRAME_READER;


#define alt_video_display VIP_FRAME_READER

VIP_FRAME_READER* VIPFR_Init(alt_u32 VipBase, void* Frame0_Base, void* Frame1_Base, alt_u32 Frame_Width, alt_u32 Frame_Height);
void VIPFR_UnInit(VIP_FRAME_READER* p);
void VIPFR_Go(VIP_FRAME_READER* p, bool bGo);
void* VIPFR_GetDrawFrame(VIP_FRAME_READER* p);
void VIPFR_ActiveDrawFrame(VIP_FRAME_READER* p);
void VIPFR_ReserveBackground(VIP_FRAME_READER* p);
void VIPFR_SetFrameSize(VIP_FRAME_READER* p, int width, int height);
// 




#endif /*VIP_FR_H_*/
