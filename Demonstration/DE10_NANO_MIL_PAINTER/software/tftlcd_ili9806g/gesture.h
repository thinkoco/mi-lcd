#ifndef GESTURE_H_
#define GESTURE_H_

struct bmp_struct{
    unsigned int width;
    unsigned int height;
    unsigned int bytes_per_pixel; /* 3:RGB, 4:RGBA */
    unsigned char    pixel_data[60 * 1560 * 3 + 1];
};

#endif /*GESTURE_H_*/
