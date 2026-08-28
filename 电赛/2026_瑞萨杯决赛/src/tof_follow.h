#ifndef TOF_FOLLOW_H_
#define TOF_FOLLOW_H_

#include "hal_data.h"

/* TOF锛圖L1B 婵€鍏夋祴璺濓級璺熼殢妯″紡锛堣闊?'9' 杩涘叆锛夛細
 *   鍓嶆柟鍗婄背鍐呮湁浜?-> 缁濆闈欐锛涘墠鏂瑰崐绫冲鏈変汉 -> 缁濆鎵ц鍓嶈繘锛堥€熷害 400锛夈€? *   - 涓ユ牸 500mm 闃堝€硷紝鏃犲洖宸細<=500 鍋?/ >500 璧帮紝鍝嶅簲"绔嬮┈璺熻繃鏉?锛? *   - GO鈫掑崐绫冲唴鏃跺垏 BRAKE 鐘舵€侊紙绱ф€ュ埗鍔?100ms锛夛紝杈撳嚭 speed=0 瑙﹀彂 cam_hold 涓诲姩鍒跺姩锛? *   - 鍒跺姩鏈熼棿涓嶅垏 GO锛岄槻姝复鐣屾姈鎸紱鍒跺姩缁撴潫鍚庡垏 STOP锛岃窛绂?500 绔嬪嵆鎭㈠ GO锛? *   - 娴嬩笉鍒颁汉锛堣秴閲忕▼ >4m 鎴栨暟鎹棤鏁堬級-> 瀹夊叏鍋滆溅锛? *   - 鑸悜淇濇寔锛氶娆¤繘鍏ヨ窡闅忔ā寮忔椂閿佸畾 yaw 骞舵案涔呬繚鎸侊紙涓嶅洜閲嶆柊杩涘叆鑰岄噸閿侊級锛? *   - "涓嶈鍚庨€€澶"锛歜alance_control 鐨?cam_hold 鐢ㄥ甫绗﹀彿杞€熷弽棣堬紝鍊掓簻鏃惰嚜鍔ㄧ籂鍋忋€? * 杈撳嚭鍛戒护锛坅ctive/speed_target/turn锛夌敱 camera_display 杞彂缁?balance_control锛? * 澶嶇敤鑳屽績璺熼殢/瀹氭椂琛岄┒鐨勮繍鍔ㄨ緭鍑洪€氶亾銆?*/

typedef struct
{
    bool active;
    float speed_target;
    int16_t turn;
} tof_follow_command_t;

void tof_follow_init(void);
void tof_follow_start(void);                 /* 杩涘叆璺熼殢妯″紡锛氶攣瀹氳埅鍚戯紝鍏堥潤姝紙鍗婄背鍐呬笉鍓嶈繘锛?*/
void tof_follow_stop(void);                  /* 閫€鍑鸿窡闅忔ā寮忥細杈撳嚭娓呴浂 */
void tof_follow_update(void);                /* 姣?5ms 璋冪敤涓€娆★細杞 TOF + 鐘舵€佹満 + 鍒锋柊杈撳嚭 */
void tof_follow_get_command(tof_follow_command_t *command);

void tof_follow_draw_page(void);             /* 鏁撮〉閲嶇粯锛堣繘鍏ユā寮忔椂璋冪敤锛屾竻灞忕敱 camera_display 鍋氾級 */
void tof_follow_update_display(void);        /* 鍛ㄦ湡鍒锋柊璺濈/鐘舵€佽锛?00ms 涓€娆★級 */

#endif /* TOF_FOLLOW_H_ */
