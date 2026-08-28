#ifndef VOICE_CONTROL_H_
#define VOICE_CONTROL_H_

#include "hal_data.h"

/* 澶╅棶(璇煶)鍗曠墖鏈轰覆鍙ｅ弬鏁帮細9600 娉㈢壒鐜囷紝8N1銆?
 * 澶╅棶鐨?serial_println 浼氬彂閫?鍛戒护瀛楃 + 鍥炶溅鎹㈣銆?*/
#define VOICE_CONTROL_BAUDRATE     (9600U)
#define VOICE_CONTROL_RX_PIN       (BSP_IO_PORT_08_PIN_02)   /* P802 / SCI2_RXD2 <- 澶╅棶 TX */

/* 澶╅棶璇煶鍛戒护瀵瑰簲鐨勫瓧绗︿覆锛堜笌 璇煶鎺у埗灏忚溅.hd 鐨?serial_println 涓€鑷达級锛? *   "0" = 鍓嶈繘  "1" = 鍚庨€€  "2" = 宸﹁浆  "3" = 鍙宠浆  "4" = 鍋滄
 *   "5" = 鎽勫儚澶存ā寮? "6" = 鎯妯″紡  "7" = 璺宠穬妯″紡  "8" = 閬ユ帶妯″紡
 *   "9" = 璺熼殢妯″紡锛堜粎鍦ㄦ憚鍍忓ご妯″紡涓嬬敓鏁堬紝瑙﹀彂鑳屽績璺熼殢鍚仠锛? *   "10" = 璇煶妯″紡锛堝彧鏈夎繘鍏ヨ妯″紡鍚庯紝"0"~"4" 鎵嶆墽琛岃繍鍔ㄥ懡浠わ級 */
typedef enum
{
    VOICE_COMMAND_NONE = 0,     /* 灏氭湭鏀跺埌鏈夋晥鍛戒护锛堜笂鐢甸粯璁ゆ樉绀?balance锛?*/
    VOICE_COMMAND_FORWARD,      /* '0' 鍓嶈繘 */
    VOICE_COMMAND_BACKWARD,     /* '1' 鍚庨€€ */
    VOICE_COMMAND_LEFT,         /* '2' 宸﹁浆 */
    VOICE_COMMAND_RIGHT,        /* '3' 鍙宠浆 */
    VOICE_COMMAND_STOP,         /* '4' 鍋滄 */
    VOICE_COMMAND_CAMERA,       /* '5' 鎽勫儚澶存ā寮?*/
    VOICE_COMMAND_INS,          /* '6' 鎯妯″紡 */
    VOICE_COMMAND_JUMP,         /* '7' 璺宠穬妯″紡 */
    VOICE_COMMAND_REMOTE,       /* '8' 閬ユ帶妯″紡 */
    VOICE_COMMAND_FOLLOW,       /* '9' 璺熼殢妯″紡锛堟憚鍍忓ご妯″紡涓嬭Е鍙戣窡闅忥級 */
    VOICE_COMMAND_VOICE_MODE,   /* "10" 璇煶妯″紡 */
} voice_command_t;

fsp_err_t voice_control_init(void);
void voice_control_process(void);
voice_command_t voice_control_get_command(void);
uint32_t voice_control_get_command_sequence(void);
bool voice_control_mode_active(void);
const char * voice_control_get_command_text(void);
void voice_control_display(void);
void voice_control_uart_callback(uart_callback_args_t * p_args);

#endif /* VOICE_CONTROL_H_ */
