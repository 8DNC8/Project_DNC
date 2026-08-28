#include "camera_display.h"
#include "tof_follow.h"

#include "zf_common_headfile.h"
#include "voice_control.h"
#include "imu_angle_display.h"
#include "vest_follow.h"
#include <string.h>

/* 鎽勫儚澶?璺熼殢妯″紡锛? *
 *   CAMERA_MODE_VIEW锛堣闊?'5' 鎽勫儚澶存ā寮忚繘鍏ワ級锛? *     - IPS200 鏄剧ず SCC8660 鎽勫儚澶寸敾闈紙2 鍊嶆斁澶ч摵婊?320x240锛夛紝
 *     - 璇嗗埆鑽у厜缁胯儗蹇冨苟鐢绘锛坴est_follow 鍙瘑鍒笉璺熼殢锛夛紝
 *     - 鍏抽棴鑸垫満+鏃犲埛銆佷笉缁存寔骞宠　銆佷笉浜х敓閫熷害锛坆alance_control 妫€娴嬪埌
 *       camera_display_view_active() 鍚庢柇鐢甸潤缃級銆? *
 *   CAMERA_MODE_TOF_FOLLOW锛堣闊?'9' 璺熼殢妯″紡杩涘叆锛屼换鎰忕姸鎬佸彲鐩存帴瑙﹀彂锛夛細
 *     - 娓呯┖鐢婚潰锛屾樉绀?tof_follow 璺濈/鐘舵€侀〉锛? *     - 鎸?DL1B 娴嬭窛璺熼殢锛氬崐绫冲唴闈欐銆佸崐绫冲鐩寸嚎鍓嶈繘 400锛堣 tof_follow锛夈€? *
 * 涓ょ妯″紡鍙簰鐩稿垏鎹紙'5' <-> '9'锛夛紱鍏跺畠浠绘剰璇煶鎸囦护閫€鍑猴紝鎭㈠鍘熼〉闈€? * 杩愬姩杈撳嚭澶嶇敤 camera_follow_command_t 閫氶亾锛堢敾闈㈡ā寮?active=false锛夈€?*/

#define CAMERA_DISPLAY_SCALE        (2U)
#define CAMERA_DISPLAY_WIDTH        ((uint16_t)(SCC8660_W * CAMERA_DISPLAY_SCALE))
#define CAMERA_DISPLAY_HEIGHT       ((uint16_t)(SCC8660_H * CAMERA_DISPLAY_SCALE))
#define CAMERA_DISPLAY_INIT_RETRIES (3U)
#define CAMERA_DISPLAY_CENTER_X     (CAMERA_DISPLAY_WIDTH / 2U)
#define CAMERA_DISPLAY_INFO_HEIGHT  (48U)
#define CAMERA_DISPLAY_TOF_TICKS    (16U)   /* 16 * 5ms = 80ms锛歍OF 娴嬭窛杞锛堜粎淇℃伅鏉℃樉绀猴紝涓嶅奖鍝嶆帶鍒讹級 */
#define CAMERA_DISPLAY_REFRESH_TICKS (20U)   /* 20 * 5ms = 100ms锛歍OF 璺熼殢椤电姸鎬佽鍒锋柊 */
#define CAMERA_DISPLAY_IMAGE_DECIMATE (2U)   /* 鍏ㄥ睆鍒峰浘闅?2 甯т竴娆★紙绾?10-12fps锛夛細鎽勫儚澶寸敾闈㈡ā寮忎笅涓诲惊鐜棤骞宠　 PID 璐熻浇锛屽彲鎻愰珮鍒峰浘棰戠巼 */

typedef enum
{
    CAMERA_MODE_OFF = 0,        /* 涓嶅湪鎽勫儚澶?璺熼殢妯″紡锛堟甯搁〉闈級 */
    CAMERA_MODE_VIEW,           /* 鎽勫儚澶寸敾闈?+ 鑳屽績妗嗛€夛紝鍏冲姩鍔涢潤缃?*/
    CAMERA_MODE_TOF_FOLLOW,     /* TOF 娴嬭窛璺熼殢锛氬崐绫冲唴鍋溿€佸崐绫冲鐩磋 400 */
} camera_display_mode_t;

static camera_display_mode_t g_camera_mode = CAMERA_MODE_OFF;
static bool g_camera_ready = false;
static bool g_camera_wait_message_drawn = false;
static uint32_t g_camera_refresh_tick = 0U;
static uint32_t g_camera_tof_tick = 0U;
static uint32_t g_camera_frame_count = 0U;   /* 甯ц鏁板櫒锛氬叏灞忓埛鍥炬寜甯ф暟闄嶉 */
static uint32_t g_diag_tick = 0U;
static camera_follow_command_t g_camera_follow_command;

bool camera_display_mode_active(void)
{
    return (CAMERA_MODE_OFF != g_camera_mode);
}

bool camera_display_view_active(void)
{
    return (CAMERA_MODE_VIEW == g_camera_mode);
}

void camera_display_get_follow_command(camera_follow_command_t *command)
{
    if(NULL != command)
    {
        *command = g_camera_follow_command;
    }
}

bool camera_display_init(void)
{
    /* 鏈夐檺閲嶈瘯鍒濆鍖栨憚鍍忓ご锛涙憚鍍忓ご娌℃帴/澶辫触涓嶅奖鍝嶅皬杞︽甯稿姛鑳?*/
    for(uint32_t i = 0; i < CAMERA_DISPLAY_INIT_RETRIES; i++)
    {
        if(!scc8660_init())
        {
            g_camera_ready = true;
            printf("SCC8660 init success.\r\n");
            return true;
        }
        printf("SCC8660 init failed, retry %lu.\r\n", (unsigned long)(i + 1U));
        R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    }
    printf("SCC8660 init failed.\r\n");
    g_camera_ready = false;
    return false;
}

static fsp_err_t camera_display_start_capture_if_idle(void)
{
    capture_status_t status;
    fsp_err_t err = g_ceu0.p_api->statusGet(g_ceu0.p_ctrl, &status);
    if(FSP_SUCCESS != err)
    {
        return err;
    }
    if(CAPTURE_STATE_IDLE != status.state)
    {
        return FSP_ERR_IN_USE;
    }

    return g_ceu0.p_api->captureStart(g_ceu0.p_ctrl, (uint8_t *)scc8660_image);
}

/* ---- 杩涘叆鎽勫儚澶寸敾闈㈡ā寮忥紙'5'锛夛細鏄剧ず鐢婚潰 + 鑳屽績妗嗭紝鍏抽棴鑸垫満+鏃犲埛闈欑疆 ---- */
static void camera_enter_view(void)
{
    g_camera_mode = CAMERA_MODE_VIEW;
    g_camera_wait_message_drawn = false;
    tof_follow_stop();   /* 浠庤窡闅忔ā寮忓垏鍥炵敾闈㈡椂鍋滄帀 TOF 璺熼殢鐘舵€佹満 */
    memset(&g_camera_follow_command, 0, sizeof(g_camera_follow_command));   /* active=false锛氫笉浜х敓杩愬姩 */
    g_camera_follow_command.active = false;

    /* 淇濈暀宸插畬鎴愮殑棣栧抚锛涙病鏈夊彲鐢ㄥ抚涓?CEU 绌洪棽鏃舵墠閲嶆柊姝﹁ */
    if(g_camera_ready && !scc8660_finish_flag)
    {
        fsp_err_t ceu_err = camera_display_start_capture_if_idle();
        if((FSP_SUCCESS != ceu_err) && (FSP_ERR_IN_USE != ceu_err))
        {
            printf("[CAM] enter captureStart err=%d\r\n", (int)ceu_err);
        }
    }
    vest_follow_init();
    ips200_clear();
    printf("[CAM] view mode, camera_ready=%d\r\n", (int)g_camera_ready);
    if(!g_camera_ready)
    {
        ips200_set_color(RGB565_RED, RGB565_BLACK);
        ips200_show_string(80, 110, "CAMERA INIT FAIL");
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    }
    else
    {
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        ips200_show_string(104, 110, "CAM WAIT");
        g_camera_wait_message_drawn = true;
    }
}

/* ---- 杩涘叆 TOF 璺熼殢锛堣闊?'9' 璺熼殢妯″紡锛夛細娓呭睆鏄剧ず璺濈/鐘舵€侀〉骞跺紑濮嬭窡闅?---- */
static void camera_enter_tof_follow(void)
{
    g_camera_mode = CAMERA_MODE_TOF_FOLLOW;
    g_camera_refresh_tick = 0U;
    memset(&g_camera_follow_command, 0, sizeof(g_camera_follow_command));
    g_camera_follow_command.active = true;

    ips200_clear();
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);

    tof_follow_start();
    tof_follow_draw_page();
    printf("[CAM] follow -> TOF follow mode.\r\n");
}

/* ---- 閫€鍑烘憚鍍忓ご/璺熼殢妯″紡锛氭仮澶嶆甯搁〉闈?---- */
static void camera_exit(void)
{
    g_camera_mode = CAMERA_MODE_OFF;
    tof_follow_stop();
    memset(&g_camera_follow_command, 0, sizeof(g_camera_follow_command));
    ips200_clear();                      /* 鏁村睆娓呮帀鐢婚潰/鍙傛暟椤碉紝閬垮厤娈嬬暀 */
    imu_angle_display_force_refresh();   /* 閫€鍑哄悗寮哄埗閲嶇粯姝ｅ父椤甸潰 */
    printf("[CAM] exit.\r\n");
}

/* ---- 鐢婚潰妯″紡锛氭瘡甯ф樉绀烘憚鍍忓ご + 鑳屽績妗嗭紙涓嶄骇鐢熻繍鍔級 ---- */
static void camera_update_view(void)
{
    if(!g_camera_ready)
    {
        return;
    }

    g_diag_tick++;

    /* 鍛ㄦ湡杞 TOF 娴嬭窛锛堜粎鐢ㄤ簬淇℃伅鏉℃樉绀鸿窛绂伙紝涓嶅奖鍝嶆帶鍒讹級 */
    g_camera_tof_tick++;
    if(g_camera_tof_tick >= CAMERA_DISPLAY_TOF_TICKS)
    {
        g_camera_tof_tick = 0U;
        dl1b_get_distance();
    }

    if(scc8660_finish_flag)
    {
        /* CEU 宸插畬鎴愪竴甯т笖涓嶅啀鑷姩閲嶅惎锛宻cc8660_image 瀹屾暣绋冲畾锛屽彲鐩存帴璇诲彇 */
        scc8660_finish_flag = false;
        g_camera_wait_message_drawn = false;

        /* 璇嗗埆鑽у厜缁胯儗蹇冿紙BFS 杩為€氬煙 + EMA 骞虫粦锛夛紝鍙瘑鍒笉璺熼殢 */
        vest_follow_update(scc8660_image);

        /* 鍏ㄥ睆鍒峰浘闄嶉锛氬彧鏈?full_frame 鎵嶅仛 40ms 绾х殑澶ч樆濉炰紶杈擄紙鍒峰浘+鍙傝€冪嚎+鐢绘锛夛紝
         * 涓棿甯у彧鏇存柊淇℃伅鏉★紙杞婚噺锛夛紝鎶婁富寰幆/骞宠　鎺у埗鍛ㄦ湡鎷夊洖姝ｅ父銆?*/
        g_camera_frame_count++;
        bool full_frame = ((g_camera_frame_count % CAMERA_DISPLAY_IMAGE_DECIMATE) == 0U);

        if(full_frame)
        {
            /* 鏄剧ず鎽勫儚澶寸敾闈紙鏈€杩戦偦 2 鍊嶆斁澶э級 */
            ips200_displayimage8660(scc8660_image[0], CAMERA_DISPLAY_WIDTH, CAMERA_DISPLAY_HEIGHT);

            /* 涓績姘村钩浣嶇疆鍙傝€冪嚎锛氭爣瀹氭椂璁╄儗蹇冧腑蹇冮潬杩戠敾闈腑蹇?*/
            ips200_draw_line(CAMERA_DISPLAY_CENTER_X,
                             CAMERA_DISPLAY_INFO_HEIGHT,
                             CAMERA_DISPLAY_CENTER_X,
                             (uint16_t)(CAMERA_DISPLAY_HEIGHT - 1U),
                             RGB565_YELLOW);

            /* 鑳屽績鍖呭洿妗嗭紙閿佸畾=缁胯壊鍙屾锛屾湭閿佸畾=绾㈣壊鍗曟锛?*/
            vest_follow_draw_overlay(CAMERA_DISPLAY_WIDTH, CAMERA_DISPLAY_HEIGHT);
        }

        /* 淇℃伅鏉★紙鍥哄畾瀹藉害锛岄伩鍏嶆暟鍊煎彉鐭畫鐣欐棫瀛楃锛?*/
        vest_follow_target_t vest_target;
        vest_follow_get_target(&vest_target);
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        ips200_show_string(0, 0, vest_target.stable_found ? "LOCK A:" : "LOST A:");
        ips200_show_uint(56, 0, vest_target.area, 5);
        ips200_show_string(0, 16, "X:");
        ips200_show_uint(16, 16, vest_target.center_x, 3);
        ips200_show_string(64, 16, "H:");
        ips200_show_uint(80, 16, vest_target.height, 3);
        if(!dl1b_init_flag)
        {
            ips200_show_string(0, 32, "D:INIT  ");
        }
        else if(dl1b_finsh_flag && (dl1b_distance_mm <= 4000U))
        {
            ips200_show_string(0, 32, "D:");
            ips200_show_uint(16, 32, dl1b_distance_mm, 4);
            ips200_show_string(48, 32, "mm");
        }
        else
        {
            ips200_show_string(0, 32, "D:WAIT  ");
        }

        /* 澶勭悊瀹屽綋鍓嶅抚鍚庡啀姝﹁涓嬩竴甯ч噰闆嗭紙CEU 绛変笅涓€涓?VSYNC 鎵嶅紑濮嬪啓锛?*/
        fsp_err_t ceu_err = camera_display_start_capture_if_idle();
        if(FSP_SUCCESS != ceu_err)
        {
            printf("[CAM] captureStart err=%d, retry next loop\r\n", (int)ceu_err);
        }
    }
    else
    {
        if(!g_camera_wait_message_drawn)
        {
            ips200_set_color(RGB565_WHITE, RGB565_BLACK);
            ips200_show_string(104, 110, "CAM WAIT");
            g_camera_wait_message_drawn = true;
        }

        if((g_diag_tick % 20U) == 0U)
        {
            fsp_err_t ceu_err = camera_display_start_capture_if_idle();
            if((FSP_SUCCESS != ceu_err) && (FSP_ERR_IN_USE != ceu_err))
            {
                printf("[CAM] retry captureStart err=%d\r\n", (int)ceu_err);
            }
        }
    }
}

void camera_display_process(void)
{
    voice_command_t command = voice_control_get_command();

    /* 妯″紡鍒囨崲锛?     *   '5' 鎽勫儚澶存ā寮忥細杩涘叆鐢婚潰妯″紡锛堣窡闅忔ā寮忎腑鍐嶈 '5' 鍥炲埌鐢婚潰锛夛紱
     *   '9' 璺熼殢妯″紡锛氫换鎰忕姸鎬佺洿鎺ユ墽琛?TOF 璺熼殢锛堝凡澶勪簬璺熼殢鍒欎繚鎸侊級锛?     *   鍏跺畠浠绘剰璇煶鎸囦护锛氶€€鍑恒€?*/
    if(VOICE_COMMAND_CAMERA == command)
    {
        if(CAMERA_MODE_OFF == g_camera_mode)
        {
            camera_enter_view();
        }
        else if(CAMERA_MODE_TOF_FOLLOW == g_camera_mode)
        {
            camera_enter_view();   /* 鍥炲埌鎽勫儚澶寸敾闈?*/
        }
        /* CAMERA_MODE_VIEW 涓?'5'锛氫繚鎸佺敾闈㈡ā寮?*/
    }
    else if(VOICE_COMMAND_FOLLOW == command)
    {
        if(CAMERA_MODE_TOF_FOLLOW != g_camera_mode)
        {
            camera_enter_tof_follow();   /* OFF/VIEW 鐘舵€佽 '9' 閮芥墽琛?TOF 璺熼殢 */
        }
    }
    else if(CAMERA_MODE_OFF != g_camera_mode)
    {
        camera_exit();
    }

    if(CAMERA_MODE_VIEW == g_camera_mode)
    {
        camera_update_view();
    }
    else if(CAMERA_MODE_TOF_FOLLOW == g_camera_mode)
    {
        /* 鎺ㄨ繘 TOF 璺熼殢鐘舵€佹満锛屽苟鎶婅緭鍑鸿浆鍙戠粰 balance_control 鐨勮繍鍔ㄩ€氶亾 */
        tof_follow_update();

        tof_follow_command_t cmd;
        tof_follow_get_command(&cmd);
        g_camera_follow_command.active = cmd.active;
        g_camera_follow_command.speed_target = cmd.speed_target;
        g_camera_follow_command.turn = cmd.turn;

        g_camera_refresh_tick++;
        if(g_camera_refresh_tick >= CAMERA_DISPLAY_REFRESH_TICKS)
        {
            g_camera_refresh_tick = 0U;
            tof_follow_update_display();
        }
    }
}
