/***********************************************************************
 *                                                                     *
 * File:     simple_graphics.c                                         *
 *                                                                     *
 * Purpose:  Contains some useful graphics routines                    *
 *                                                                     *
 * Author:   N. Knight                                                 *
 *           Altera Corporation                                        *
 *           Apr 2006                                                  *
 **********************************************************************/
#include "terasic_includes.h"
#include <string.h>
#include <io.h>
//#include "alt_video_display.h"
//#include "vip_fr.h"
#include "simple_graphics.h"
#include "sys/alt_alarm.h"
#include "sys/alt_cache.h"
#include "system.h"


// richard add
void vid_clean_screen(alt_video_display* display, int color){
    vid_paint_block(0 , 0,display->width, display->height, color, display);
}

/******************************************************************
*  Function: vid_draw_line
*
*  Purpose: Draws a line between two end points. First checks
*           to see if the line is horizontal.  If it is, it calls
*           vid_draw_horiz_line(), which is much faster than 
*           vid_draw_sloped_line.
*
******************************************************************/
__inline__ void vid_draw_line(int horiz_start, int vert_start, int horiz_end, int vert_end, int width, int color, alt_video_display* display)
{
 
  if( vert_start == vert_end )
  {
//    c2h_draw_horiz_line( (unsigned short)horiz_start, 
//                         (unsigned short)horiz_end,  
//                         (unsigned short)vert_start,
//                         color,
//                         display->buffer_ptrs[display->buffer_being_written]->buffer);

    vid_draw_horiz_line( (unsigned short)horiz_start, 
                         (unsigned short)horiz_end, 
                         (unsigned short)vert_start,
                         color,
                         display );
  }
  else
  {
    vid_draw_sloped_line( (unsigned short)horiz_start, 
                          (unsigned short)vert_start, 
                          (unsigned short)horiz_end, 
                          (unsigned short)vert_end, 
                          (unsigned short)width, 
                          color,
                          display );
  }
}



void vid_scroll_string_quit(vid_text_scroll_struct* scroll)
{
  free(scroll->string);
  free(scroll);
}

vid_text_scroll_struct* vid_scroll_string_init(int hbegin, int vbegin, int hend, int f_color, int b_color, char* font, int ms_delay, char *string)
{
  vid_text_scroll_struct* scroll;
  scroll = malloc(sizeof (vid_text_scroll_struct));
  
  scroll->hbegin = hbegin;
  scroll->vbegin = vbegin;
  scroll->hend = hend;
  scroll->f_color = f_color;
  scroll->b_color = b_color;
  scroll->string = malloc(strlen(string)+2);
  strcpy(scroll->string, string);
  scroll->font = font;
  scroll->ms_delay = ms_delay;
  scroll->ticks_at_last_move = alt_nticks();
  scroll->text_scroll_index = 0;
  scroll->text_scroll_started = 0;
  scroll->window_width = scroll->hend - scroll->hbegin;
  scroll->length_of_string = strlen(string);
  scroll->string_points = scroll->length_of_string * 8;
  scroll->scroll_points = (scroll->window_width + scroll->string_points);
 
  return(scroll);
  
}


int vid_scroll_string(vid_text_scroll_struct* scroll, alt_video_display* display)
{

  int x_start, x_end, x_index, string_x_index, string_char_index, char_row, char_column;
  char character, column_mask;
  char* font_char_ptr;
  char pixels_to_move_by = 1;

  // If it's time to move the scroll..
  if (alt_nticks() >= (scroll->ticks_at_last_move + ((alt_ticks_per_second() * (scroll->ms_delay)) / 1000))) {
    scroll->ticks_at_last_move = alt_nticks();
    
    // Track where we are in the scroll.
    if(scroll->text_scroll_started == 0) {
      scroll->text_scroll_index = 0;
      scroll->text_scroll_started = 1;
    } else if(scroll->text_scroll_index >= scroll->scroll_points)  {
      scroll->text_scroll_started = 0;
    } else {
      scroll->text_scroll_index += pixels_to_move_by;
    }
    
    //Find out where we start
    if (scroll->text_scroll_index < scroll->window_width) {
      x_start = scroll->hbegin + scroll->window_width - scroll->text_scroll_index;
    } else {
      x_start = scroll->hbegin;
    }
    //Find out where we end
    if (scroll->string_points > scroll->text_scroll_index) {
      x_end = scroll->hend;
    } else {
      x_end = (scroll->hend - scroll->text_scroll_index + scroll->string_points);
    }

    // Write the string segment a column (x) at a time
    for(x_index = x_start; x_index < x_end; x_index++) {
      // Find the x index we're at within the string
      // If first part of string hasnt yet reached left side of scroll window
      if (scroll->text_scroll_index < scroll->window_width) {
        string_x_index = (x_index - x_start);
      } else {
        string_x_index = scroll->text_scroll_index - scroll->window_width + x_index - x_start;
      }
      //Find the character we're supposed to be writing
      string_char_index = (string_x_index / 8);
      character = scroll->string[string_char_index];
      char_column = (string_x_index % 8);
      column_mask = (((unsigned int)0x80) >> char_column);
      font_char_ptr = (scroll->font + ((character - 0x20) * FONT_10PT_ROW));
      //We have all the data now, so let's write a column
      for(char_row = 0; char_row < 11; char_row++) {
        // If the font table says this pixel is on, then set it to the foreground color
        if (*(font_char_ptr + char_row) & column_mask) {
          vid_set_pixel(x_index, scroll->vbegin + char_row, scroll->f_color, display); 
        // Otherwise, set it to the background color.
        } else {
          vid_set_pixel(x_index, scroll->vbegin + char_row, scroll->b_color, display); //background color
        }
      }
    }
    // Erase the leftover column (x) of the last string we wrote.
    vid_draw_line(x_end, scroll->vbegin, x_end, scroll->vbegin + 10, 1, scroll->b_color, display);
    // Log what time we moved the scroll.
  }
  return(0);
}


/******************************************************************
*  Function: vid_move_block
*
*  Purpose: Moves a block around the screen, backfilling with 
*           the backfill_color parameter.
*
******************************************************************/

int vid_move_block(int xbegin, int ybegin, int xend, int yend, int x_distance, int y_distance, int backfill_color, alt_video_display* display)
{
  int read_x, read_y, write_x, write_y;
  short temp_pixel;
  
  if(x_distance <= 0 && y_distance <= 0) {  
    //Move by rows because they are contiguous in memory (could help speed if in SDRAM)
    for (read_y = ybegin; read_y < yend; read_y++) {
      write_y = read_y + y_distance;
      for(read_x = xbegin; read_x < xend; read_x++) {
        write_x = read_x + x_distance;
        temp_pixel = vid_get_pixel(read_x, read_y, display);
        vid_set_pixel(write_x, write_y, temp_pixel, display);
        if(read_x >= xend + x_distance || read_y >= yend + y_distance) 
        {
	        vid_set_pixel(read_x, read_y, backfill_color, display);
        }
      }
    }
  }
  return (0);
}

/******************************************************************
*  Function: vid_print_string
*
*  Purpose: Prints a string to the specified location of the screen
*           using the specified font and color.
*           Calls vid_print_char
*
******************************************************************/
int vid_print_string(int horiz_offset, int vert_offset, int color, char *font, alt_video_display* display, char string[])
{
  int i = 0;
  int original_horiz_offset;

  original_horiz_offset = horiz_offset;

  // Print until we hit the '\0' char.
  while (string[i]) {
    //Handle newline char here.
    if (string[i] == '\n') {
      horiz_offset = original_horiz_offset;
      vert_offset += 12;
      i++;
      continue;
    }
    // Lay down that character and increment our offsets.
    vid_print_char (horiz_offset, vert_offset, color, string[i], font, display);
    i++;
    horiz_offset += 8;
  }
  return (0);
}

/******************************************************************
*  Function: vid_draw_box
*
*  Purpose: Draws a box on the screen with the specified corner
*  points.  The fill parameter tells the function whether or not
*  to fill in the box.  1 = fill, 0 = do not fill.
*
******************************************************************/
int vid_draw_box (int horiz_start, int vert_start, int horiz_end, int vert_end, int color, int fill, alt_video_display* display)
{

  // If we want to fill in our box
  if (fill) {
     vid_paint_block (horiz_start, vert_start, horiz_end, vert_end, color, display);
  // If we're not filling in the box, just draw four lines.
  } else {
    vid_draw_line(horiz_start, vert_start, horiz_start, vert_end-1, 1, color, display);
    vid_draw_line(horiz_end-1, vert_start, horiz_end-1, vert_end-1, 1, color, display);
    vid_draw_line(horiz_start, vert_start, horiz_end-1, vert_start, 1, color, display);
    vid_draw_line(horiz_start, vert_end-1, horiz_end-1, vert_end-1, 1, color, display);
  }

  return (0);
}


/******************************************************************
*  Function: vid_print_char
*
*  Purpose: Prints a character to the specified location of the
*           screen using the specified font and color.
*
******************************************************************/

int vid_print_char (int horiz_offset, int vert_offset, int color, char character, char *font, alt_video_display* display)
{

  int i, j;
  
  char temp_char, char_row;

  // Convert the ASCII value to an array offset
  temp_char = (character - 0x20);

  //Each character is 8 pixels wide and 11 tall.
  for(i = 0; i < 11; i++) {
      char_row = *(font + (temp_char * FONT_10PT_ROW) + i);
    for (j = 0; j < 8; j++) {
      //If the font table says the pixel in this location is on for this character, then set it.
      if (char_row & (((unsigned char)0x80) >> j)) {
        vid_set_pixel((horiz_offset + j), (vert_offset + i), color, display); // plot the pixel
      }
    }
  }
  return(0);
}


/******************************************************************
*  Function: vid_set_pixel
*
*  Purpose: Sets the specified pixel to the specified color.
*           Sets one pixel although frame buffer consists of
*           two-pixel words.  Therefore this function is not
*           efficient when painting large areas of the screen.
*
******************************************************************/

void vid_set_pixel(int horiz, int vert, unsigned int color, alt_video_display* display)
{
  int addr;
  
  if( display->color_depth == 32 )
  {
//  	addr = ( ( (int)(display->buffer_ptrs[display->buffer_being_written]) )+ (vert * (display->width * 4)) + horiz * 4);
    addr = ( ( (int)(VIPFR_GetDrawFrame(display)) )+ (vert * (display->width * 4)) + horiz * 4);
		IOWR_32DIRECT( addr, 0, (unsigned int)(color));
  }
  
  else if( display->color_depth == 24 )
  {
  	addr = ( ( (int)(VIPFR_GetDrawFrame(display)) )+ (vert * (display->width * 3)) + horiz * 3);
		IOWR_8DIRECT( addr, 0, (unsigned char)(color));
 		IOWR_8DIRECT( addr+1, 0, (unsigned char)(color >> 8));
    IOWR_8DIRECT( addr+2, 0, (unsigned char)(color >> 16));
  }
  
  else if( display->color_depth == 16 )
  {
  	addr = ( ( (int)(VIPFR_GetDrawFrame(display)) )+ (vert * (display->width * 2)) + horiz * 2);
  	IOWR_16DIRECT( addr, 0, (int)(color));
	}


}

/******************************************************************
*  Function: vid_get_pixel
*
*  Purpose: Reads the color of the pixel at the given coordinates
*
******************************************************************/

short vid_get_pixel(int horiz, int vert, alt_video_display* display)
{
  int addr;
  
  addr = ( ( (int)(VIPFR_GetDrawFrame(display)) )+ (vert * (display->width * 2)) + horiz * 2);
  return(IORD_16DIRECT(addr, 0));

}


/******************************************************************
*  Function: vid_paint_block
*
*  Purpose: Paints a block of the screen the specified color.
*           Note: works with two pixels at a time to maintain high
*           bandwidth.  Therefore, corner points must be even
*           numbered coordinates.  Use vid_draw_solid_box() for
*           areas with odd-numbered corner points.
*           The color parameter must contain two pixel's worth
*           (32 bits).
*
******************************************************************/
void vid_paint_block (int Hstart,int Vstart, int Hend, int Vend, int color, alt_video_display* display)
{

  int i;
  unsigned int addr; // richard add unsigned
  int bytes_per_line, bytes_per_pixel;
  char* line;
  
  bytes_per_pixel = (display->color_depth / 8);
  bytes_per_line = ((Hend - Hstart) * bytes_per_pixel);

  line = malloc(bytes_per_line + 12);

	if(display->color_depth == 16)
	{
    for (i = 0; i < bytes_per_line; i+=2) 
    {
      *(line + i) = (unsigned char)color;
      *(line + i + 1) = (unsigned char)(color >> 8);
    }
  }
  else if(display->color_depth == 24)
  { 
    for (i = 0; i < bytes_per_line; i+=3) 
    {
      *(line + i) = (unsigned char)color;
      *(line + i + 1) = (unsigned char)(color >> 8);
      *(line + i + 2) = (unsigned char)(color >> 16);
    }
  }
  else if(display->color_depth == 32)
  { 
    for (i = 0; i < bytes_per_line; i+=4) 
    {
      // Does the right hand side of this assignment determine the size?
      *(int*)(line + i) = (unsigned int)color;
    }
  }
  
  /* Initial Address */
  addr = (int)(VIPFR_GetDrawFrame(display)) + ((Vstart * (display->width * bytes_per_pixel)) + (Hstart * bytes_per_pixel));
  
  for (i = Vstart; i < Vend; i++)
  {
    memcpy( (void*)addr, line, bytes_per_line );
    addr += (display->width * bytes_per_pixel);
  }
  free (line);
}


/******************************************************************
*  Function: vid_draw_horiz_line
*
*  Purpose: Draws a horizontal line on the screen quickly.
*           Good for filling stuff.
*
******************************************************************/
void vid_draw_horiz_line (short Hstart, short Hend, int V, int color, alt_video_display* display)
{

  int i;
  int addr;
  int bytes_per_line;

  char *fast_buffer = malloc(1024 * 3);

  if( Hstart > Hend )
  {
    short temp = Hstart;
    Hstart = Hend;
    Hend = temp;
  }
  
  if(display->color_depth == 32)
  { 
    addr = (int)(VIPFR_GetDrawFrame(display)) + ((V * (display->width * 4)) + (Hstart * 4));
    bytes_per_line = ((Hend - Hstart) * 4);
    for (i = 0; i < bytes_per_line; i+=4) 
    {
      // Does the right hand side of this assignment determine the size?
      *(int*)(fast_buffer + i) = (unsigned int)color;
    }
    memcpy( (void*)addr, fast_buffer, bytes_per_line );
  }
  if(display->color_depth == 24)
  { 
    addr = (int)(VIPFR_GetDrawFrame(display)) + ((V * (display->width * 3)) + (Hstart * 3));
    bytes_per_line = ((Hend - Hstart) * 3);
    for (i = 0; i < bytes_per_line; i+=3) 
    {
      *(fast_buffer + i) = (unsigned char)color;
      *(fast_buffer + i + 1) = (unsigned char)(color >> 8);
      *(fast_buffer + i + 2) = (unsigned char)(color >> 16);
    }
    memcpy( (void*)addr, fast_buffer, bytes_per_line );
  }
  else if(display->color_depth == 16)
  {
    addr = (int)(VIPFR_GetDrawFrame(display)) + ((V * (display->width * 2)) + (Hstart * 2));
    bytes_per_line = ((Hend - Hstart) * 2);
    for (i = 0; i < bytes_per_line; i+=2) 
    {
      *(fast_buffer + i) = (unsigned char)color;
      *(fast_buffer + i + 1) = (unsigned char)(color >> 8);
    }
    memcpy( (void*)addr, fast_buffer, bytes_per_line );
  }
  free(fast_buffer);
}



/******************************************************************
*  Function: vid_merge_colors
*
*  Purpose: Takes 5-bit color values for each red, green, and blue
*           and merges them into one 16-bit color value.
*
******************************************************************/
int vid_merge_colors(int red, int green, int blue)
{
  // Green actually has 6-bits, but we'll make it's LSB 1 to be consistent.
  return ((blue) | (((green << 1) | 0x1) << 5) | (red << 11));
}

/******************************************************************
*  Function: vid_color_convert24_16
*
*  Purpose: Takes a pointer to a 24-bit (3-byte) 24-bit RGB color 
*           sample and converts it to 16-bits, displayable by the 
*           VGA controller in 16-bit mode
*
******************************************************************/
unsigned short vid_color_convert24_16(char* color24)
{
	unsigned char red, green, blue;
	unsigned short output;
	
	red = *(color24 + 0) & 0xF8;
	green = *(color24 + 1) & 0xFC; // green is actualy 6 bits
	blue = *(color24 + 2) & 0xF8;

	output = ((blue >> 3) | (green << 3) | (red << 8));  
	return output;
}


/******************************************************************
*  Function: vid_color_convert24_16
*
*  Purpose: Takes a pointer to a 24-bit (3-byte) 24-bit RGB color 
*           sample and converts it to 16-bits, displayable by the 
*           VGA controller in 16-bit mode
*
******************************************************************/
int vid_color_convert16_24(unsigned short color16, char* color24)
{
	*(color24 + 0) = color16 >> 11;
	*(color24 + 1) = ((color16 & 0x3E) >> 5);
	*(color24 + 2) = (color16 & 0x1F);
	
	return (0);
}

/******************************************************************
*  Function: vid_copy_line_to_frame_buffer
*
*  Purpose: Copies a scanline from memory to the frame buffer at
*           the specified coordinates.  Converts color depth if
*           necessary.
*
******************************************************************/
int vid_copy_line_to_frame_buffer( int x, int y, char* buffer, int num_pixels, int source_color_depth, alt_video_display* display )
{
  unsigned short* temp_line;
  int index_24 = 0;
  int index_16 = 0;
  unsigned int dest_addr;
  unsigned int bytes_in_line;
  
  dest_addr = (int)(VIPFR_GetDrawFrame(display)) + 
    ((y * (display->width * (display->bytes_per_pixel))) + 
    (x * (display->bytes_per_pixel)));
  
  bytes_in_line = num_pixels * display->bytes_per_pixel;
  
  if(source_color_depth == 24)
  {
    if(display->color_depth == 16)
    {
      temp_line = malloc(bytes_in_line);
      while(index_24 < bytes_in_line)
      {
        *(temp_line + index_16) = vid_color_convert24_16_m((char*)(buffer + index_24));
        index_16++;
        index_24+=3;
      }
      memcpy( (void*)dest_addr, (void*)temp_line, bytes_in_line );
      free(temp_line);
    }
    else if(display->color_depth == 24)
    {
      memcpy( (void*)dest_addr, (void*)buffer, bytes_in_line );
    }
  }
  else if(source_color_depth == 16)
  {
    if(display->color_depth == 24)
    {
      temp_line = malloc(bytes_in_line);
      while(index_16 < num_pixels )
      {
        vid_color_convert16_24((short)*(buffer + index_16), (char*)(temp_line + index_24));
        index_16++;
        index_24+=3;
      }
      memcpy( (void*)dest_addr, (void*)temp_line, bytes_in_line );
      free(temp_line);
      
    }
    else if(display->color_depth == 16)
    {
      memcpy( (void*)dest_addr, (void*)buffer, bytes_in_line );
    }
  }
  return(0);
}

/******************************************************************
*  Function: vid_draw_sloped_line
*
*  Purpose: Draws a line between two end points using
*           Bresenham's line drawing algorithm.
*           width parameter is not used.  
*           It is reserved for future use.
*
******************************************************************/
void vid_draw_sloped_line( unsigned short horiz_start, 
                           unsigned short vert_start, 
                           unsigned short horiz_end, 
                           unsigned short vert_end, 
                           unsigned short width, 
                           int color, 
                           alt_video_display* display)
{
  // Find the vertical and horizontal distance between the two points
  int horiz_delta = abs(horiz_end-horiz_start);
  int vert_delta = abs(vert_end-vert_start);

  // Find out what direction we are going
  int horiz_incr, vert_incr;
  if (horiz_start > horiz_end) { horiz_incr=-1; } else { horiz_incr=1; }
  if (vert_start > vert_end) { vert_incr=-1; } else { vert_incr=1; }

  // Find out which axis is always incremented when drawing the line
  // If it's the horizontal axis
  if (horiz_delta >= vert_delta) {
    int dPr   = vert_delta<<1;
    int dPru  = dPr - (horiz_delta<<1);
    int P     = dPr - horiz_delta;

    // Process the line, one horizontal point at at time
    for (; horiz_delta >= 0; horiz_delta--) {
      // plot the pixel
      vid_set_pixel(horiz_start, vert_start, color, display);
      // If we're moving both up and right
      if (P > 0) {
        horiz_start+=horiz_incr;
        vert_start+=vert_incr;
        P+=dPru;
      } else {
        horiz_start+=horiz_incr;
        P+=dPr;
      }
    }
  // If it's the vertical axis
  } else {
    int dPr   = horiz_delta<<1;
    int dPru  = dPr - (vert_delta<<1);
    int P     = dPr - vert_delta;

    // Process the line, one vertical point at at time
    for (; vert_delta>=0; vert_delta--) {
      // plot the pixel
      vid_set_pixel(horiz_start, vert_start, color, display);
      // If we're moving both up and right
      if (P > 0) {
        horiz_start+=horiz_incr;
        vert_start+=vert_incr;
        P+=dPru;
      } else {
        vert_start+=vert_incr;
        P+=dPr;
      }
    }
  }
}


/******************************************************************
*  Function: vid_draw_circle
*
*  Purpose: Draws a circle on the screen with the specified center
*  and radius.  Draws symetric circles only.  The fill parameter
*  tells the function whether or not to fill in the box.  1 = fill,
*  0 = do not fill.
*
******************************************************************/
int vid_draw_circle(int Hcenter, int Vcenter, int radius, int color, char fill, alt_video_display* display)
{
  int x = 0;
  int y = radius;
  int p = (5 - radius*4)/4;

  // Start the circle with the top, bottom, left, and right pixels.
  vid_round_corner_points(Hcenter, Vcenter, x, y, 0, 0, color, fill, display);

  // Now start moving out from those points until the lines meet
  while (x < y) {
    x++;
    if (p < 0) {
      p += 2*x+1;
    } else {
      y--;
      p += 2*(x-y)+1;
    }
    vid_round_corner_points(Hcenter, Vcenter, x, y, 0, 0, color, fill, display);
  }
  return (0);
}


/******************************************************************
*  Function: vid_draw_round_corner_box
*
*  Purpose: Draws a box on the screen with the specified corner
*  points.  The fill parameter tells the function whether or not
*  to fill in the box.  1 = fill, 0 = do not fill.
*
******************************************************************/
int vid_draw_round_corner_box ( int horiz_start, int vert_start, int horiz_end, int vert_end, 
                                int radius, int color, int fill, alt_video_display* display)
{
  unsigned int x, y;
  int p;
  int diameter;
  int temp;
  unsigned int width, height, straight_width, straight_height;

  // Make sure the start point us up and left of the end point
  if( horiz_start > horiz_end )
  {
    temp = horiz_end;
    horiz_end = horiz_start;
    horiz_start = temp;
  }
  
  if( vert_start > vert_end )
  {
    temp = vert_end;
    vert_end = vert_start;
    vert_start = temp;
  }
  
  // These are the overall dimensions of the box
  width = horiz_end - horiz_start;
  height = vert_end - vert_start;

  // Make sure our radius isnt more than the shortest dimension 
  // of the box, or it'll screw us all up
  if( radius > ( width / 2 ))
    radius = width / 2;

  if( radius > ( height / 2 ))
    radius = height / 2;
  
  // We use the diameter for some calculations, so we'll pre calculate it here.
  diameter = ( radius * 2 );

  // These are the lengths of the straight portions of the box edges.
  straight_width = width - diameter;
  straight_height = height - diameter;

  x = 0;
  y = radius;
  p = (5 - radius*4)/4;
   
  // Start the corners with the top, bottom, left, and right pixels.
  vid_round_corner_points( horiz_start + radius, vert_start + radius, x, y, 
                           straight_width, straight_height, color, fill, display );
  
  // Now start moving out from those points until the lines meet
  while (x < y) {
    x++;
    if (p < 0) {
      p += 2*x+1;
    } else {
      y--;
      p += 2*(x-y)+1;
    }
    vid_round_corner_points( horiz_start + radius, vert_start + radius, x, y, 
                             straight_width, straight_height, color, fill, display);
  }

  // If we want to fill in our box
  if (fill) {
     vid_paint_block (horiz_start, vert_start + radius, horiz_end, vert_end - radius, color, display);
  // If we're not filling in the box, just draw four lines.
  } else {
    vid_draw_line(horiz_start, vert_start + radius, horiz_start, vert_end - radius , 1, color, display); //left
    vid_draw_line(horiz_end, vert_start + radius, horiz_end, vert_end - radius , 1, color, display); //right
    vid_draw_line(horiz_start + radius, vert_start, horiz_end - radius , vert_start, 1, color, display); //top
    vid_draw_line(horiz_start + radius, vert_end, horiz_end - radius , vert_end, 1, color, display); //bottom
  }

  return (0);
}


/******************************************************************
*  Function: vid_round_corner_points
*
*  Purpose: Called by vid_draw_round_corner_box() and 
*  vid_draw_circle() to plot the actual points of the round corners.
*  Draws horizontal lines to fill the shape.
*
******************************************************************/

void vid_round_corner_points( int cx, int cy, int x, int y, 
                              int straight_width, int straight_height, int color, 
                              char fill, alt_video_display* display)
{

    // If we're directly above, below, left and right of center (0 degrees), plot those 4 pixels
    if (x == 0) {
        // bottom
        vid_set_pixel(cx, cy + y + straight_height, color, display);
        vid_set_pixel(cx + straight_width, cy + y + straight_height, color, display);
        // top
        vid_set_pixel(cx, cy - y, color, display);
        vid_set_pixel(cx + straight_width, cy - y, color, display);

        if(fill) {
          vid_draw_line(cx - y, cy, cx + y + straight_width, cy, 1, color, display);
          vid_draw_line(cx - y, cy + straight_height, cx + y + straight_width, cy + straight_height, 1, color, display);
        } else {
          //right
          vid_set_pixel(cx + y + straight_width, cy, color, display);
          vid_set_pixel(cx + y + straight_width, cy + straight_height, color, display);
          //left
          vid_set_pixel(cx - y, cy, color, display);
          vid_set_pixel(cx - y, cy + straight_height, color, display);
        }

    } else
    // If we've reached the 45 degree points (x=y), plot those 4 pixels
    if (x == y) {
      if(fill) {
        vid_draw_line(cx - x, cy + y + straight_height, cx + x + straight_width, cy + y + straight_height, 1, color, display); // lower
        vid_draw_line(cx - x, cy - y, cx + x + straight_width, cy - y, 1, color, display); // upper
        
      } else {
        vid_set_pixel(cx + x + straight_width, cy + y + straight_height, color, display); // bottom right
        vid_set_pixel(cx - x, cy + y + straight_height, color, display); // bottom left
        vid_set_pixel(cx + x + straight_width, cy - y, color, display); // top right
        vid_set_pixel(cx - x, cy - y, color, display); // top left
      }
    } else
    // If we're between 0 and 45 degrees plot 8 pixels.
    if (x < y) {
        if(fill) {
          vid_draw_line(cx - x, cy + y + straight_height, cx + x + straight_width, cy + y + straight_height, 1, color, display);
          vid_draw_line(cx - y, cy + x + straight_height, cx + y + straight_width, cy + x + straight_height, 1, color, display);
          vid_draw_line(cx - y, cy - x, cx + y + straight_width, cy - x, 1, color, display);
          vid_draw_line(cx - x, cy - y, cx + x + straight_width, cy - y, 1, color, display);
        } else {
          vid_set_pixel(cx + x + straight_width, cy + y + straight_height, color, display);
          vid_set_pixel(cx - x, cy + y + straight_height, color, display);
          vid_set_pixel(cx + x + straight_width, cy - y, color, display);
          vid_set_pixel(cx - x, cy - y, color, display);
          vid_set_pixel(cx + y + straight_width, cy + x + straight_height, color, display);
          vid_set_pixel(cx - y, cy + x + straight_height, color, display);
          vid_set_pixel(cx + y + straight_width, cy - x, color, display);
          vid_set_pixel(cx - y, cy - x, color, display);
        }
    }
}


/******************************************************************
*  Function: min3
*
*  Purpose:  Returns the minimum value of 3 parameters
*            Used for drawing filled shapes
*
******************************************************************/
static __inline__ int max3( int a, int b, int c )
{
  if( a < b )
    a = b;
  if( a < c )
    a = c;
  
  return a;
}

/******************************************************************
*  Function: min3
*
*  Purpose:  Returns the minimum value of 3 parameters
*            Used for drawing filled shapes.
*
******************************************************************/
static __inline__ int min3( int a, int b, int c )
{
  if( a > b )
    a = b;
  if( a > c )
    a = c;
  
  return a;
}

/******************************************************************
*  Function: max_diff3
*
*  Purpose:  Returns the positive max difference between 3 
*            parameters.  Used for drawing filled shapes
*
******************************************************************/
__inline__ int max_diff3(int a, int b, int c)
{
  int max, min;
    
  max = max3( a, b, c );
  min = min3( a, b, c );
  return (max - min);
}

/******************************************************************
*  Function: vid_put_pixel_in_span_map
*
*  Purpose:  This function places a pixel into either the max or
*            min value of an element that represents a span, 
*            essentially just a horizontal line used to fill shapes.
*
******************************************************************/
static inline void vid_put_pixel_in_span_map( int x, int y, int *span_array )
{
  if (span_array[y*2] == -1)
  {
    span_array[y*2] = x;
    span_array[(y*2)+1] = x;
  }
  else if( span_array[y*2] > x )
    span_array[y*2] = x;
  else if( span_array[(y*2)+1] < x )
    span_array[(y*2)+1] = x;
}

/******************************************************************
*  Function: vid_bres_scan_edges
*
*  Purpose:  This function uses Bresenham's algorithm to scan a line
*            and determine whether it's the left(min) or right(max)
*            edge of a horizontal span.  This is useful for drawing
*            filled shapes where you fill by drawing successive
*            horizontal lines.
*
******************************************************************/
void vid_bres_scan_edges( int x1, int y1, int x2, int y2, int *span_array) 
{

  int x_incr, y_incr;
  int y_delta, x_delta;

  // Assure we always draw left to right
  if( x1 > x2 )
  {
    int tempx = x2;
    x2 = x1;
    x1 = tempx;
    int tempy = y2;
    y2 = y1;
    y1 = tempy;
  }
  
  // Find the vertical and horizontal distance between the two points
  y_delta = abs(y1-y2);
  x_delta = (x2-x1);

  // Find out what direction we are going
  if (y1 > y2) { y_incr=-1; } else { y_incr=1; }
  x_incr=1;

  // Find out which axis is always incremented when drawing the line
  // If it's the horizontal axis
  if (x_delta >= y_delta) {
    int dPr   = y_delta<<1;
    int dPru  = dPr - (x_delta<<1);
    int P     = dPr - x_delta;

    // Process the line, one horizontal point at at time
    for (; x_delta >= 0; x_delta--) {
      // map the pixel
      vid_put_pixel_in_span_map(x1, y1, span_array);
      // If we're moving along both axis
      if (P > 0) {
        x1+=x_incr;
        y1+=y_incr;
        P+=dPru;
      } else {
        x1+=x_incr;
        P+=dPr;
      }
    }
  }
  else // If it's the vertical axis
  {
    int dPr   = x_delta<<1;
    int dPru  = dPr - (y_delta<<1);
    int P     = dPr - y_delta;

    // Process the line, one vertical point at at time
    for (; y_delta>=0; y_delta--) {
      // plot the pixel
      vid_put_pixel_in_span_map(x1, y1, span_array);
      // If we're moving along both axis
      if (P > 0) {
        x1+=x_incr;
        y1+=y_incr;
        P+=dPru;
      } else {
        y1+=y_incr;
        P+=dPr;
      }
    }
  }
}

/******************************************************************
*  Function: vid_draw_triangle
*
*  Purpose:  This function draws a triangle on the screen between
*            three points defined by the structure tri.
*
******************************************************************/
void vid_draw_triangle(triangle_struct* tri, alt_video_display* display)
{
  int i;
  
  // Outline it first
    vid_draw_line( tri->vertex_x[0], tri->vertex_y[0], 
                   tri->vertex_x[1], tri->vertex_y[1], 1, 
                   tri->col, display);
    vid_draw_line( tri->vertex_x[1], tri->vertex_y[1], 
                   tri->vertex_x[2], tri->vertex_y[2], 1, 
                   tri->col, display);
    vid_draw_line( tri->vertex_x[2], tri->vertex_y[2], 
                   tri->vertex_x[0], tri->vertex_y[0], 1, 
                   tri->col, display);
//  vid_draw_line(tri->bx, tri->by, tri->cx, tri->cy, 1, tri->col, display);
//  vid_draw_line(tri->cx, tri->cy, tri->ax, tri->ay, 1, tri->col, display);
  
  if(tri->fill == DO_FILL)
  {
    tri->top_y = min3(tri->vertex_y[0], tri->vertex_y[1], tri->vertex_y[2]);
    tri->bottom_y = max3(tri->vertex_y[0], tri->vertex_y[1], tri->vertex_y[2]);
    tri->spans_needed = max_diff3(tri->vertex_y[0], tri->vertex_y[1], tri->vertex_y[2]);
    tri->max_span = max_diff3(tri->vertex_x[0], tri->vertex_x[1], tri->vertex_x[2]);
    tri->span_array = malloc(display->height * 4 * 2);

    //init the span array
    for( i = tri->top_y; i <= tri->bottom_y; i++)
    {
      tri->span_array[i*2] = -1;
      tri->span_array[(i*2) + 1] = -1;
    }
    
    // Scan-convert the triangle  
    vid_bres_scan_edges( tri->vertex_x[0], tri->vertex_y[0], 
                         tri->vertex_x[1], tri->vertex_y[1], tri->span_array);
    vid_bres_scan_edges( tri->vertex_x[1], tri->vertex_y[1], 
                         tri->vertex_x[2], tri->vertex_y[2], tri->span_array);
    vid_bres_scan_edges( tri->vertex_x[2], tri->vertex_y[2], 
                         tri->vertex_x[0], tri->vertex_y[0], tri->span_array);
  
    // Render the polygon
    for( i = tri->top_y; i <= tri->bottom_y; i++ )
    {
//      vid_draw_horiz_line (tri->span_array[i*2], tri->span_array[(i*2)+1], i, tri->col, display);
      vid_draw_line (tri->span_array[i*2], i, tri->span_array[(i*2)+1], i, 1, tri->col, display);

    }
    free(tri->span_array);
  }
}

void CopyImage16( short * __restrict__ source_buffer, 
                  short * __restrict__ dest_buffer, 
                  short source_width, short source_height,
                  short dest_width, short dest_height )
{
  int y_src, y_dst;
  int y_src_offset = 0;
  int y_dst_offset = 0;
  
  for( y_src = 0, y_dst = 0; 
       y_src <= ( source_height  - 1 ); 
       y_src++, y_dst++, y_src_offset += source_width, y_dst_offset += dest_width )
  {
    memcpy(( dest_buffer + y_dst_offset ), 
           ( source_buffer + y_src_offset ), 
           ( source_width << 1 ));
  }
}

void CropImage16( short * __restrict__ source_buffer, 
                  short * __restrict__ dest_buffer, 
                  short source_width, short source_height,
                  short crop_left, short crop_right,
                  short crop_top, short crop_bottom,
                  short dest_width, short dest_height )
{
  int y_src, y_dst;
  int y_src_offset = (source_width * crop_top);
  int y_dst_offset = 0;
  
  for( y_src = crop_top, y_dst = 0; 
       y_src <= ( source_height - crop_bottom - 1 ); 
       y_src++, y_dst++, y_src_offset += source_width, y_dst_offset += dest_width )
  {
    memcpy(( dest_buffer + y_dst_offset ), 
           ( source_buffer + y_src_offset + crop_left ), 
           ((source_width - crop_left - crop_right) << 1));
  }
}


/******************************************************************
*  Data: cour10_font
*
*  Purpose: Data array that represents a 10-point courier font.
*
******************************************************************/
char cour10_font_array[95][11] =

 {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00},
 {0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x14, 0x14, 0x7E, 0x28, 0x28, 0x28, 0xFC, 0x50, 0x50, 0x00, 0x00},
 {0x10, 0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x10, 0x00, 0x00},
 {0x40, 0xA2, 0x44, 0x08, 0x10, 0x20, 0x44, 0x8A, 0x04, 0x00, 0x00},
 {0x30, 0x40, 0x40, 0x20, 0x60, 0x92, 0x94, 0x88, 0x76, 0x00, 0x00},
 {0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x10, 0x08},
 {0x20, 0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x20},
 {0x00, 0x00, 0x6C, 0x38, 0xFE, 0x38, 0x6C, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x10, 0x10, 0x10, 0xFE, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0x00},
 {0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00},
 {0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00},
 {0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00},
 {0x10, 0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00},
 {0x38, 0x44, 0x04, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7C, 0x00, 0x00},
 {0x38, 0x44, 0x04, 0x04, 0x18, 0x04, 0x04, 0x44, 0x38, 0x00, 0x00},
 {0x08, 0x18, 0x18, 0x28, 0x28, 0x48, 0x7C, 0x08, 0x1C, 0x00, 0x00},
 {0x7C, 0x40, 0x40, 0x40, 0x78, 0x04, 0x04, 0x44, 0x38, 0x00, 0x00},
 {0x18, 0x20, 0x40, 0x40, 0x78, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00},
 {0x7C, 0x44, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x00, 0x00},
 {0x38, 0x44, 0x44, 0x44, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00},
 {0x38, 0x44, 0x44, 0x44, 0x3C, 0x04, 0x04, 0x08, 0x30, 0x00, 0x00},
 {0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x20, 0x00, 0x00, 0x00},
 {0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x7C, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00, 0x00},
 {0x38, 0x44, 0x04, 0x04, 0x08, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00},
 {0x3C, 0x42, 0x9A, 0xAA, 0xAA, 0xAA, 0x9C, 0x40, 0x38, 0x00, 0x00},
 {0x30, 0x10, 0x10, 0x28, 0x28, 0x44, 0x7C, 0x44, 0xEE, 0x00, 0x00},
 {0xFC, 0x42, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x42, 0xFC, 0x00, 0x00},
 {0x3C, 0x42, 0x80, 0x80, 0x80, 0x80, 0x80, 0x42, 0x3C, 0x00, 0x00},
 {0xF8, 0x44, 0x42, 0x42, 0x42, 0x42, 0x42, 0x44, 0xF8, 0x00, 0x00},
 {0xFE, 0x42, 0x40, 0x48, 0x78, 0x48, 0x40, 0x42, 0xFE, 0x00, 0x00},
 {0xFE, 0x42, 0x40, 0x48, 0x78, 0x48, 0x40, 0x40, 0xF0, 0x00, 0x00},
 {0x3C, 0x42, 0x80, 0x80, 0x80, 0x8E, 0x82, 0x42, 0x3C, 0x00, 0x00},
 {0xEE, 0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0xEE, 0x00, 0x00},
 {0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00},
 {0x1E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00, 0x00},
 {0xE6, 0x44, 0x48, 0x48, 0x50, 0x70, 0x48, 0x44, 0xE6, 0x00, 0x00},
 {0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x22, 0xFE, 0x00, 0x00},
 {0xC6, 0x44, 0x6C, 0x6C, 0x54, 0x54, 0x44, 0x44, 0xEE, 0x00, 0x00},
 {0xCE, 0x44, 0x64, 0x64, 0x54, 0x4C, 0x4C, 0x44, 0xE4, 0x00, 0x00},
 {0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00},
 {0xFC, 0x42, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0xF0, 0x00, 0x00},
 {0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 0x44, 0x38, 0x36, 0x00},
 {0xFC, 0x42, 0x42, 0x42, 0x7C, 0x48, 0x48, 0x44, 0xE6, 0x00, 0x00},
 {0x7C, 0x82, 0x80, 0x80, 0x7C, 0x02, 0x02, 0x82, 0x7C, 0x00, 0x00},
 {0xFE, 0x92, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00},
 {0xEE, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00},
 {0xEE, 0x44, 0x44, 0x44, 0x28, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00},
 {0xEE, 0x44, 0x44, 0x44, 0x54, 0x54, 0x54, 0x28, 0x28, 0x00, 0x00},
 {0xEE, 0x44, 0x28, 0x28, 0x10, 0x28, 0x28, 0x44, 0xEE, 0x00, 0x00},
 {0xEE, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00},
 {0xFE, 0x84, 0x08, 0x08, 0x10, 0x20, 0x20, 0x42, 0xFE, 0x00, 0x00},
 {0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00},
 {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00},
 {0x1C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1C, 0x00, 0x00},
 {0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00},
 {0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x78, 0x04, 0x7C, 0x84, 0x84, 0x7A, 0x00, 0x00},
 {0xC0, 0x40, 0x40, 0x7C, 0x42, 0x42, 0x42, 0x42, 0xFC, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x7C, 0x82, 0x80, 0x80, 0x82, 0x7C, 0x00, 0x00},
 {0x0C, 0x04, 0x04, 0x7C, 0x84, 0x84, 0x84, 0x84, 0x7E, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x7C, 0x82, 0xFE, 0x80, 0x82, 0x7C, 0x00, 0x00},
 {0x30, 0x40, 0x40, 0xF0, 0x40, 0x40, 0x40, 0x40, 0xF0, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x7E, 0x84, 0x84, 0x84, 0x7C, 0x04, 0x04, 0x78},
 {0xC0, 0x40, 0x40, 0x58, 0x64, 0x44, 0x44, 0x44, 0xEE, 0x00, 0x00},
 {0x08, 0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00, 0x00},
 {0x08, 0x00, 0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x70},
 {0xC0, 0x40, 0x40, 0x4C, 0x48, 0x50, 0x70, 0x48, 0xC6, 0x00, 0x00},
 {0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xE8, 0x54, 0x54, 0x54, 0x54, 0xD6, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xD8, 0x64, 0x44, 0x44, 0x44, 0xEE, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x7C, 0x82, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xFC, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x40, 0xE0},
 {0x00, 0x00, 0x00, 0x7E, 0x84, 0x84, 0x84, 0x7C, 0x04, 0x0E, 0x00},
 {0x00, 0x00, 0x00, 0xEC, 0x32, 0x20, 0x20, 0x20, 0xF8, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0x7C, 0x82, 0x70, 0x0C, 0x82, 0x7C, 0x00, 0x00},
 {0x00, 0x20, 0x20, 0x78, 0x20, 0x20, 0x20, 0x24, 0x18, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xCC, 0x44, 0x44, 0x44, 0x4C, 0x36, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xEE, 0x44, 0x44, 0x28, 0x28, 0x10, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xEE, 0x44, 0x54, 0x54, 0x28, 0x28, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xEE, 0x44, 0x38, 0x38, 0x44, 0xEE, 0x00, 0x00},
 {0x00, 0x00, 0x00, 0xEE, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x60},
 {0x00, 0x00, 0x00, 0xFC, 0x88, 0x10, 0x20, 0x44, 0xFC, 0x00, 0x00},
 {0x0C, 0x10, 0x10, 0x10, 0x10, 0x60, 0x10, 0x10, 0x10, 0x10, 0x0C},
 {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
 {0x60, 0x10, 0x10, 0x10, 0x10, 0x0C, 0x10, 0x10, 0x10, 0x10, 0x60},
 {0x00, 0x00, 0x62, 0x92, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};

//Pointer to our font table
char* cour10_font = &cour10_font_array[0][0];

