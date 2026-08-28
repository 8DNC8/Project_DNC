#include "zf_device_color_track.h"

uint8 color_track_find_green (const uint16 image[TRACK_IMAGE_H][TRACK_IMAGE_W], color_blob_t *result)
{
    uint32 sum_x = 0, sum_y = 0, count = 0;
    uint16 x_min = TRACK_IMAGE_W, x_max = 0;
    uint16 y_min = TRACK_IMAGE_H, y_max = 0;

    for (uint16 y = 0; y < TRACK_IMAGE_H; y++)
    {
        for (uint16 x = 0; x < TRACK_IMAGE_W; x++)
        {
            /* 凌瞳(SCC8660)的 RGB565 在内存里是"高字节在前"（字节交换，与
             * ips200_show_rgb565_image(color_mode=1) 的显示字节序一致），
             * 先换回标准 RGB565 再提取通道，否则颜色分量会错位。 */
            uint16 px = image[y][x];
            uint16 std = (uint16)(((px & 0xFFu) << 8) | (px >> 8));
            uint8 r = (std >> 11) & 0x1F;
            uint8 g = (std >> 5)  & 0x3F;
            uint8 b =  std        & 0x1F;

            if (g > GREEN_G_MIN && g * 10 > r * GREEN_RG_RATIO && g * 10 > b * GREEN_BG_RATIO)
            {
                sum_x += x;
                sum_y += y;
                count++;
                if (x < x_min) x_min = x;
                if (x > x_max) x_max = x;
                if (y < y_min) y_min = y;
                if (y > y_max) y_max = y;
            }
        }
    }

    if (count < MIN_BLOB_PIXELS) return 0;

    result->cx          = (uint16)(sum_x / count);
    result->cy          = (uint16)(sum_y / count);
    result->x_min       = x_min;
    result->y_min       = y_min;
    result->x_max       = x_max;
    result->y_max       = y_max;
    result->pixel_count = count;

    return 1;
}
