#ifndef _zf_device_color_track_h_
#define _zf_device_color_track_h_

#include "zf_common_typedef.h"

#define TRACK_IMAGE_W    160
#define TRACK_IMAGE_H    120

#define GREEN_G_MIN        60
#define GREEN_RG_RATIO     12
#define GREEN_BG_RATIO     12
#define MIN_BLOB_PIXELS    30

typedef struct
{
    uint16 cx, cy;
    uint16 x_min, y_min, x_max, y_max;
    uint32 pixel_count;
} color_blob_t;

uint8 color_track_find_green (const uint16 image[TRACK_IMAGE_H][TRACK_IMAGE_W], color_blob_t *result);

#endif
