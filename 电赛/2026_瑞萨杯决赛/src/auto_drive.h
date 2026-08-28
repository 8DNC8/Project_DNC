#ifndef AUTO_DRIVE_H_
#define AUTO_DRIVE_H_

#include "hal_data.h"

/* 鎽勫儚澶存ā寮忎笅鐨勫畾鏃惰椹跺姩浣滐紙璇煶 '5' 杩涘叆鎽勫儚澶存ā寮忔椂鑷姩鍚姩锛夛細
 *   绗竴闃舵锛氫互鍥哄畾閫熷害 SPD 琛岄┒ T1 绉掞紱
 *   绗簩闃舵锛氬仠姝㈢淮鎸?T2 绉掞紱
 *   绗笁闃舵锛氭寔缁互 SPD 琛岄┒锛岀洿鍒版敹鍒板叾瀹冭闊虫寚浠ら€€鍑烘憚鍍忓ご妯″紡銆? *
 * 涓変釜鍙傛暟 SPD / T1 / T2 鍦ㄦ憚鍍忓ご妯″紡涓嬬敤鎸夐敭閫夋嫨鍜岃皟鑺傦細
 *   K3 鍒囨崲閫変腑椤? K2 鍔? K1 鍑? K4 閲嶆柊寮€濮嬪姩浣滃簭鍒椼€? * IPS200 鏄剧ず鍙傛暟璋冭妭椤碉紙涓嶆樉绀烘憚鍍忓ご鐢婚潰锛夈€? *
 * 杈撳嚭鍛戒护锛坅ctive/speed_target/turn锛夌敱 camera_display 杞彂缁?balance_control锛? * 澶嶇敤鑳屽績璺熼殢鐨勮繍鍔ㄨ緭鍑洪€氶亾锛宐alance_control 鏃犻渶鏀瑰姩銆? */

typedef enum
{
    AUTO_DRIVE_STATE_IDLE = 0,      /* 鏈繘鍏ユ憚鍍忓ご妯″紡 */
    AUTO_DRIVE_STATE_DRIVE_1,       /* 绗竴闃舵锛氫互 SPD 琛岄┒ T1 */
    AUTO_DRIVE_STATE_HOLD,          /* 鍋滄缁存寔 T2 */
    AUTO_DRIVE_STATE_DRIVE_CONT,    /* 鎸佺画浠?SPD 琛岄┒ */
} auto_drive_state_t;

typedef enum
{
    AUTO_DRIVE_ITEM_SPEED = 0,      /* SPD 鍥哄畾閫熷害 */
    AUTO_DRIVE_ITEM_DRIVE_TIME,     /* T1 琛岄┒鏃堕棿锛堢锛?*/
    AUTO_DRIVE_ITEM_HOLD_TIME,      /* T2 缁存寔鏃堕棿锛堢锛?*/
    AUTO_DRIVE_ITEM_MAX,
} auto_drive_item_t;

typedef struct
{
    bool active;
    float speed_target;
    int16_t turn;
} auto_drive_command_t;

void auto_drive_init(void);
void auto_drive_start(void);                 /* 杩涘叆鎽勫儚澶存ā寮忔椂鍚姩鍔ㄤ綔搴忓垪 */
void auto_drive_stop(void);                  /* 閫€鍑烘憚鍍忓ご妯″紡鏃跺仠姝紝杈撳嚭娓呴浂 */
void auto_drive_update(void);                /* 姣?5ms 璋冪敤涓€娆★細鎺ㄨ繘鐘舵€佹満骞跺埛鏂拌緭鍑?*/
void auto_drive_get_command(auto_drive_command_t *command);

auto_drive_state_t auto_drive_get_state(void);
bool auto_drive_is_active(void);
const char * auto_drive_get_state_text(void);

/* ---- 鎸夐敭璋冭妭 ---- */
void auto_drive_key_cycle_item(void);        /* 鍒囨崲閫変腑椤?SPD -> T1 -> T2 -> SPD */
void auto_drive_key_adjust_steps(int32_t steps);  /* 閫変腑椤规寜姝ラ暱澧炲噺锛?1/-1 琛ㄧず涓€姝ワ級 */
void auto_drive_key_restart(void);           /* 閲嶆柊浠庣涓€闃舵寮€濮嬫墽琛?*/
auto_drive_item_t auto_drive_get_selected_item(void);
bool auto_drive_item_is_selected(auto_drive_item_t item);
const char * auto_drive_get_item_name(auto_drive_item_t item);
float auto_drive_get_item_value(auto_drive_item_t item);
float auto_drive_get_item_step(auto_drive_item_t item);

/* ---- 鏄剧ず ---- */
void auto_drive_draw_page(void);             /* 鏁撮〉閲嶇粯锛堣繘鍏ユā寮?鍙傛暟鍙樺寲鏃惰皟鐢級 */
void auto_drive_update_display(void);        /* 鍛ㄦ湡鍒锋柊锛堝彧鍒风姸鎬佽锛?00ms 涓€娆★級 */

#endif /* AUTO_DRIVE_H_ */
