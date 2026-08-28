#ifndef CAMERA_DISPLAY_H_
#define CAMERA_DISPLAY_H_

#include "hal_data.h"

typedef struct
{
    bool active;
    bool target_locked;
    bool distance_valid;
    bool moving;
    float speed_target;
    int16_t turn;
} camera_follow_command_t;

/* 鎽勫儚澶?璺熼殢妯″紡锛? *  璇煶 '5'锛堟憚鍍忓ご妯″紡锛夎繘鍏ョ敾闈㈡ā寮忥細IPS200 鏄剧ず SCC8660 鎽勫儚澶寸敾闈紝
 *    璇嗗埆鑽у厜缁胯儗蹇冨苟鐢绘锛屽悓鏃跺叧闂埖鏈?鏃犲埛銆佷笉缁存寔骞宠　銆佷笉浜х敓閫熷害锛堣溅闈欑疆锛夛紱
 *  璇煶 '9'锛堣窡闅忔ā寮忥級浠绘剰鐘舵€佺洿鎺ヨ繘鍏?TOF 璺熼殢锛氭竻绌虹敾闈紝鏄剧ず璺濈/鐘舵€侊紝
 *    鎸?DL1B 娴嬭窛璺熼殢锛堝崐绫冲唴闈欐銆佸崐绫冲鐩寸嚎鍓嶈繘 400锛岃 tof_follow锛夛紱
 *  涓ょ妯″紡鍙簰鐩稿垏鎹紙'5' <-> '9'锛夛紝鍏跺畠浠绘剰璇煶鎸囦护閫€鍑猴紝鎭㈠鍘熼〉闈€?*/
bool camera_display_init(void);             /* 涓婄數鍒濆鍖栵紙鏈夐檺閲嶈瘯锛屾憚鍍忓ご缂哄腑涓嶅奖鍝嶅皬杞︼級 */
void camera_display_process(void);          /* 涓诲惊鐜皟鐢細妯″紡鍒囨崲 + 鐢婚潰/璺熼殢鍒锋柊 */
bool camera_display_mode_active(void);      /* 褰撳墠鏄惁澶勪簬鎽勫儚澶寸敾闈?璺熼殢妯″紡 */
bool camera_display_view_active(void);      /* 褰撳墠鏄惁澶勪簬鎽勫儚澶寸敾闈㈠瓙妯″紡锛?5'锛屽叧鍔ㄥ姏浠呯湅鐢婚潰锛?*/
void camera_display_get_follow_command(camera_follow_command_t *command);

#endif /* CAMERA_DISPLAY_H_ */
