#ifndef NAG_NAVIGATION_H_
#define NAG_NAVIGATION_H_

#include "zf_common_headfile.h"

/*
 * 鎯妯″潡锛堢Щ妞嶈嚜 CYT4BB7 鍙傝€冨伐绋?Flash.c/h 鐨?Nag_System / N.Final_Out锛夈€? *
 * 鍔熻兘锛? *   褰曞埗锛氳溅琚帹琛屾椂锛屾瘡绱 NAG_SET_MILEAGE 閲岀▼璁颁竴娆″綋鍓嶅亸鑸锛坹aw锛夈€? *   澶嶇幇锛氳溅鍐嶆琚帹琛屾椂锛屾寜閲岀▼鎶婂綍鍒舵椂鐨勫亸鑸浣滀负鐩爣瑙掞紝瀹炴椂绠楀嚭
 *         N.Final_Out = angle_plan(褰撳墠 yaw - 鐩爣 yaw)锛屼氦缁欏钩琛℃帶鍒跺仛宸€熻浆鍚戯紝
 *         浠庤€屽鐜板綍鍒剁殑璺緞鏈濆悜銆? *
 * 瀛樺偍锛氬綍鍒剁偣鍏堝啓 RAM 瀛樺偍灞傦紙nag_flash锛夛紝鏂數涓㈠け锛涘悗缁帴鍏ヤ唬鐮?Flash 鍚庡彲鏂數淇濆瓨銆? */

typedef enum
{
    NAG_STATE_IDLE = 0,
    NAG_STATE_RECORDING,
    NAG_STATE_REPLAYING,
    NAG_STATE_STRAIGHT,
} nag_navigation_state_t;

extern float nag_navigation_turn_gain;    /* Final_Out -> 鍗犵┖姣斿樊閫熺殑澧炵泭锛岃繍琛屾椂鍙皟 */
extern float nag_navigation_turn_trim;    /* 鍥哄畾宸€熻ˉ鍋匡紝鐢ㄤ簬鎶垫秷杞︿綋澶╃劧宸?鍙冲亸 */
extern float nag_navigation_travel_speed; /* 澶嶇幇鏃跺墠杩涢€熷害鐩爣锛堟棤鍒烽€熷害鍙嶉鍗曚綅锛岄渶鏍囧畾锛?*/

void nag_navigation_init(void);
void nag_navigation_update(void);        /* 姣?5ms 璋冪敤涓€娆?*/

void nag_navigation_start_record(void);
void nag_navigation_stop_record(void);
void nag_navigation_start_replay(void);
void nag_navigation_start_straight(void);

float nag_navigation_get_final_out(void);
nag_navigation_state_t nag_navigation_get_state(void);
uint16_t nag_navigation_get_record_count(void);
float nag_navigation_get_mileage(void);
float nag_navigation_get_angle_run(void);

#endif /* NAG_NAVIGATION_H_ */
