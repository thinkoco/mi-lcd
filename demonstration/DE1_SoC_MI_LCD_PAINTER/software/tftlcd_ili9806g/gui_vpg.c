// --------------------------------------------------------------------
// Copyright (c) 2011 by Terasic Technologies Inc. 
// --------------------------------------------------------------------
//
// Permission:
//
//   Terasic grants permission to use and modify this code for use
//   in synthesis for all Terasic Development Boards and Altera Development 
//   Kits made by Terasic.  Other use of this code, including the selling 
//   ,duplication, or modification of any portion is strictly prohibited.
//
// Disclaimer:
//
//   This VHDL/Verilog or C/C++ source code is intended as a design reference
//   which illustrates how these types of functions can be implemented.
//   It is the user's responsibility to verify their design for
//   consistency and functionality through the use of formal
//   verification methods.  Terasic provides no warranty regarding the use 
//   or functionality of this code.
//
// --------------------------------------------------------------------
//           
//                     Terasic Technologies Inc
//                     356 Fu-Shin E. Rd Sec. 1. JhuBei City,
//                     HsinChu County, Taiwan
//                     302
//
//                     web: http://www.terasic.com/
//                     email: support@terasic.com
//
// --------------------------------------------------------------------

#include "terasic_includes.h"
#include "gui.h"
#include "vip_fr.h"
#include "multi_touch2.h"
#include "simple_graphics.h"
#include "geometry.h"
#include "gesture.h"
#include "system.h"


#define FRAME_WIDTH  854
#define FRAME_HEIGHT 480

#define FR_FRAME_0  (SDRAM_BASE + SDRAM_SPAN/2)
#define FR_FRAME_1  (SDRAM_BASE + SDRAM_SPAN/2 + FRAME_WIDTH*FRAME_HEIGHT*4)

void ShowInfo(VIP_FRAME_READER *pReader, char *pText){
    vid_print_string_alpha(1, 1, WHITE_24, BLACK_24, tahomabold_20, pReader, pText);    
}

void VPG_XLine(VIP_FRAME_READER *pReader){
    alt_u32 *p;
    
    p = VIPFR_GetDrawFrame(pReader);

    // erase background
     vid_clean_screen(pReader, BLACK_24);
    
    // draw line    
    vid_draw_line(0, 0, FRAME_HEIGHT-1, FRAME_HEIGHT-1, 1, WHITE_24, pReader);
    vid_draw_line(FRAME_HEIGHT-1, 0, 0, FRAME_HEIGHT-1, 1, WHITE_24, pReader);

    // show text
    ShowInfo(pReader, "X Line");
    
    //    
    VIPFR_ActiveDrawFrame(pReader);        
}

void VPG_ColorPanel(VIP_FRAME_READER *pReader, alt_u32 Color){
    alt_u32 *p;
    int i;
    char szText[32];
    
    
    p = VIPFR_GetDrawFrame(pReader);
    for(i=0;i<FRAME_HEIGHT*FRAME_WIDTH;i++)
        *p++ = Color;
        
    // show text
    sprintf(szText,"Panel: %08xh", Color);
    ShowInfo(pReader, szText);
        
    VIPFR_ActiveDrawFrame(pReader);        
    
}

void VPG_Grid(VIP_FRAME_READER *pReader, alt_u8 GridSize){
    int x,y;
    alt_u32 *p, Value32;
    char szText[32];
    
    p = VIPFR_GetDrawFrame(pReader);
    for(y=0;y<FRAME_HEIGHT;y++){
        for(x=0;x<FRAME_WIDTH;x++){
            if (x%GridSize == 0 || y%GridSize == 0)
                Value32 = WHITE_24;
            else
                Value32 = BLACK_24;
            *p++ = Value32;
        }            
        
    }
    // show text
    sprintf(szText,"Grid%d", GridSize);
    ShowInfo(pReader, szText);
    
    VIPFR_ActiveDrawFrame(pReader);
}

void VPG_VGrid(VIP_FRAME_READER *pReader, alt_u8 GridSize){
    int x,y;
    alt_u32 *p, Value32;
    char szText[32];
    
    p = VIPFR_GetDrawFrame(pReader);
    for(y=0;y<FRAME_HEIGHT;y++){
        for(x=0;x<FRAME_WIDTH;x++){
            if (x%GridSize == 0)
                Value32 = WHITE_24;
            else
                Value32 = BLACK_24;
            *p++ = Value32;
        }            
        
    }
    // show text
    sprintf(szText,"VGrid%d", GridSize);
    ShowInfo(pReader, szText);
    
    VIPFR_ActiveDrawFrame(pReader);
}

void VPG_HGrid(VIP_FRAME_READER *pReader, alt_u8 GridSize){
    int x,y;
    alt_u32 *p, Value32;
    char szText[32];
    
    p = VIPFR_GetDrawFrame(pReader);
    for(y=0;y<FRAME_HEIGHT;y++){
        for(x=0;x<FRAME_WIDTH;x++){
            if (y%GridSize == 0)
                Value32 = WHITE_24;
            else
                Value32 = BLACK_24;
            *p++ = Value32;
        }            
        
    }
    // show text
    sprintf(szText,"HGrid%d", GridSize);
    ShowInfo(pReader, szText);
    
    VIPFR_ActiveDrawFrame(pReader);
}




void VPG_ColorBar(VIP_FRAME_READER *pReader){
    int x,y;
    alt_u32 *p, Value32;
    
    p = VIPFR_GetDrawFrame(pReader);
    for(y=0;y<FRAME_HEIGHT;y++){
        for(x=0;x<FRAME_WIDTH;x++){
            if (x == 0 || y == 0 || x == FRAME_WIDTH-1 || y == FRAME_HEIGHT-1){
                Value32 = WHITE_24;
            }else if (y < FRAME_HEIGHT/4){
                // red
                Value32 = x & 0xFF;
            }else if (y < FRAME_HEIGHT/2){
                Value32 = (x & 0xFF) << 8;
            }else if (y < FRAME_HEIGHT*3/4){
                Value32 = (x & 0xFF) << 16;
            }else{
                Value32 = x & 0xFF;
                Value32 |= (x & 0xFF) << 8;
                Value32 |= (x & 0xFF) << 16;
            }
            *p++ = Value32;
        }            
        
    }
    
    VIPFR_ActiveDrawFrame(pReader);
}

void GUI_ShowPAT(VIP_FRAME_READER *pReader, int PatId){
                switch(PatId){
                    case 0: VPG_ColorBar(pReader); break;
                    case 1: VPG_ColorPanel(pReader, WHITE_24); break;
                    case 2: VPG_ColorPanel(pReader, RED_24); break;
                    case 3: VPG_ColorPanel(pReader, GREEN_24); break;
                    case 4: VPG_ColorPanel(pReader, BLUE_24); break;
                    case 5: VPG_XLine(pReader); break;
                    case 6: VPG_VGrid(pReader, 2); break;
                    case 7: VPG_VGrid(pReader, 4); break;
                    case 8: VPG_VGrid(pReader, 8); break;
                    case 9: VPG_HGrid(pReader, 2); break;
                    case 10: VPG_HGrid(pReader, 4); break;
                    case 11: VPG_HGrid(pReader, 8); break;
                    case 12: VPG_Grid(pReader, 2); break;
                    case 13: VPG_Grid(pReader, 4); break;
                    case 14: VPG_Grid(pReader, 8); break;
                }
    
}

void GUI_VPG(MTC2_INFO *pTouch){
    // video
    VIP_FRAME_READER *pReader;
    int X1, Y1, X2, Y2, X3, Y3, X4, Y4, X5, Y5;
    alt_u8 Event, TouchNum;    
    alt_u8 PatId = 0;
#ifdef ENALBE_TOUCH_FILTER    
    POINT LastPt1, LastPt2;
    alt_u32 LastTimePt1=0,LastTimePt2=0; 
    const alt_u32 ContinuedTime = alt_ticks_per_second()/50;
#endif    
    
    /////////////////////////////////////////////
    // frame reader
#ifdef DUAL_FRAME_BUFFER
    pReader =  VIPFR_Init(ALT_VIP_VFR_0_BASE, (void *)FR_FRAME_0, (void *)FR_FRAME_1, FRAME_WIDTH, FRAME_HEIGHT);
#else    
    pReader =  VIPFR_Init(ALT_VIP_VFR_0_BASE, (void *)FR_FRAME_0, (void *)FR_FRAME_0, FRAME_WIDTH, FRAME_HEIGHT);
#endif // DUAL_FRAME_BUFFER    
    VIPFR_Go(pReader, TRUE);
    
    
    // clean screen
    vid_clean_screen(pReader, BLACK_24);
    
   
    VIPFR_ActiveDrawFrame(pReader);
    
    VPG_ColorBar(pReader);

   GUI_ShowPAT(pReader, PatId);    
    
    while(1){
        if (MTC2_GetStatus(pTouch,&Event, &TouchNum, &X1, &Y1, &X2, &Y2, &X3, &Y3, &X4, &Y4, &X5, &Y5)){
           // printf(".");
            //
            if (TouchNum > 0){
                // change patter
                PatId++;
                if (PatId > 14)
                    PatId = 0;
                GUI_ShowPAT(pReader, PatId);
                usleep(300*1000); // debounce
                MTC2_ClearEvent(pTouch);
            }
        }
    }
    
}



