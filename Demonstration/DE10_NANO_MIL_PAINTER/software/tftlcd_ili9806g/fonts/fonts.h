#ifndef FONTS_H_
#define FONTS_H_


struct abc_font_struct {
  unsigned long extents_width;
  unsigned long extents_height;
  unsigned long extents_ascent;
  unsigned long extents_descent;
  unsigned long bounds_width;
  unsigned long bounds_height;
  unsigned char *char_alpha_map;
  unsigned long reserved;
};

/*
extern struct abc_font_struct arial_10[];
extern struct abc_font_struct arial_12[];
extern struct abc_font_struct arial_14[];
extern struct abc_font_struct arial_16[];
extern struct abc_font_struct arial_18[];
extern struct abc_font_struct arial_20[];
extern struct abc_font_struct arial_22[];
extern struct abc_font_struct arial_24[];
extern struct abc_font_struct courier_10[];
extern struct abc_font_struct courier_12[];
extern struct abc_font_struct courier_14[];
extern struct abc_font_struct courier_16[];
extern struct abc_font_struct courier_18[];
extern struct abc_font_struct courier_20[];
extern struct abc_font_struct courier_22[];
extern struct abc_font_struct courier_24[];
extern struct abc_font_struct timesnewroman_10[];
extern struct abc_font_struct timesnewroman_12[];
extern struct abc_font_struct timesnewroman_14[];
extern struct abc_font_struct timesnewroman_16[];
extern struct abc_font_struct timesnewroman_18[];
extern struct abc_font_struct timesnewroman_20[];
extern struct abc_font_struct timesnewroman_22[];
extern struct abc_font_struct timesnewroman_24[];*/

extern struct abc_font_struct tahomabold_32[];
extern struct abc_font_struct tahomabold_20[];

#endif /*FONTS_H_*/
