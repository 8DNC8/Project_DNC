#ifndef VEST_FOLLOW_H_
#define VEST_FOLLOW_H_

#include "zf_common_headfile.h"

/* 鑳屽績璺熼殢鍙傛暟 */
#define VEST_FOLLOW_MIN_AREA_PIXELS        (220U)    /* 鏈€灏忔湁鏁堝儚绱犻潰绉?*/
#define VEST_FOLLOW_MAX_AREA_PIXELS        (52000U)  /* 鏈€澶ф湁鏁堝儚绱犻潰绉?*/
#define VEST_FOLLOW_LOCK_FRAMES            (3U)      /* 杩炵画妫€娴嬪埌澶氬皯甯ф墠绠楅攣瀹?*/
#define VEST_FOLLOW_LOST_HOLD_FRAMES       (8U)      /* 杩炵画涓㈠け澶氬皯甯ф墠绠椾涪澶?*/

/* 鑳屽績璺熼殢鐩爣淇℃伅 */
typedef struct
{
    bool raw_found;          /* 褰撳墠甯ф槸鍚︽娴嬪埌鐩爣 */
    bool stable_found;       /* 鏄惁绋冲畾閿佸畾 */
    uint16_t x;              /* 鍖呭洿妗嗗乏涓婅 X */
    uint16_t y;              /* 鍖呭洿妗嗗乏涓婅 Y */
    uint16_t width;          /* 鍖呭洿妗嗗搴?*/
    uint16_t height;         /* 鍖呭洿妗嗛珮搴?*/
    uint16_t center_x;       /* 涓績 X */
    uint16_t center_y;       /* 涓績 Y */
    uint32_t area;           /* 闈㈢Н锛堝儚绱狅級 */
    uint8_t lock_count;      /* 閿佸畾璁℃暟 */
    uint8_t lost_count;      /* 涓㈠け璁℃暟 */
} vest_follow_target_t;

void vest_follow_init(void);
void vest_follow_update(const uint16 image[SCC8660_H][SCC8660_W]);
void vest_follow_get_target(vest_follow_target_t *target);
void vest_follow_draw_overlay(uint16_t display_width, uint16_t display_height);

#endif /* VEST_FOLLOW_H_ */