#include "voice_control.h"

#include "camera_display.h"
#include "zf_common_headfile.h"
#include <string.h>

/* ===== 鏈湴閰嶇疆 ===== */
#define VOICE_CTRL_RX_RING_SIZE             (64U)
#define VOICE_CTRL_RX_PROCESS_LIMIT         (16U)
#define VOICE_CTRL_TOKEN_SIZE               (3U)
#define VOICE_CTRL_DISPLAY_X                (10U)
#define VOICE_CTRL_DISPLAY_Y                (215U)  /* 浠呭嚭鐜板湪 RUN 椤垫渶鍚庝竴琛岋紙8x16 瀛椾綋锛?15..231 涓哄彲瑙佹渶鍚庝竴琛岋級 */
#define VOICE_CTRL_DISPLAY_REFRESH_TICKS    (20U)   /* 20 * 5ms = 100ms 寮哄埗鍒锋柊涓€娆★紝闃叉琚〉闈㈤噸缁樻摝鎺?*/

/* SCI2 涓柇鍚戦噺鏄惁宸茬粡鍦?FSP 鍚戦噺琛ㄤ腑鍒嗛厤 */
#if defined(VECTOR_NUMBER_SCI2_RXI) && defined(VECTOR_NUMBER_SCI2_TXI) && \
    defined(VECTOR_NUMBER_SCI2_TEI) && defined(VECTOR_NUMBER_SCI2_ERI)
#define VOICE_CTRL_SCI2_AVAILABLE           (1)
#else
#define VOICE_CTRL_SCI2_AVAILABLE           (0)
#endif

#if VOICE_CTRL_SCI2_AVAILABLE
static sci_b_uart_instance_ctrl_t g_voice_uart2_ctrl;
static sci_b_baud_setting_t g_voice_uart2_baud_setting;

static const sci_b_uart_extended_cfg_t g_voice_uart2_cfg_extend =
{
    .clock = SCI_B_UART_CLOCK_INT,
    .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE,
    .noise_cancel = SCI_B_UART_NOISE_CANCELLATION_DISABLE,
    .p_baud_setting = &g_voice_uart2_baud_setting,
    .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_1,
    .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
    .flow_control = SCI_B_UART_FLOW_CONTROL_RTS,
    .rs485_setting =
    {
        .enable = SCI_B_UART_RS485_DISABLE,
        .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
        .assertion_time = 1,
        .negation_time = 1,
    },
    .delay_cycles = 0,
};

static const uart_cfg_t g_voice_uart2_cfg =
{
    .channel = 2,
    .data_bits = UART_DATA_BITS_8,
    .parity = UART_PARITY_OFF,
    .stop_bits = UART_STOP_BITS_1,
    .rxi_ipl = 15,
    .rxi_irq = VECTOR_NUMBER_SCI2_RXI,
    .txi_ipl = 12,
    .txi_irq = VECTOR_NUMBER_SCI2_TXI,
    .tei_ipl = 12,
    .tei_irq = VECTOR_NUMBER_SCI2_TEI,
    .eri_ipl = 15,
    .eri_irq = VECTOR_NUMBER_SCI2_ERI,
    .p_transfer_rx = NULL,
    .p_transfer_tx = NULL,
    .p_callback = voice_control_uart_callback,
    .p_context = NULL,
    .p_extend = &g_voice_uart2_cfg_extend,
};

static const uart_instance_t g_voice_uart2 =
{
    .p_ctrl = &g_voice_uart2_ctrl,
    .p_cfg = &g_voice_uart2_cfg,
    .p_api = &g_uart_on_sci_b,
};
#endif

static volatile uint8_t g_voice_rx_ring[VOICE_CTRL_RX_RING_SIZE];
static volatile uint8_t g_voice_rx_ring_head = 0;
static volatile uint8_t g_voice_rx_ring_tail = 0;
static volatile bool g_voice_opened = false;
static volatile voice_command_t g_voice_command = VOICE_COMMAND_NONE;
static volatile uint32_t g_voice_command_sequence = 0U;
static volatile bool g_voice_mode_active = false;
static char g_voice_token[VOICE_CTRL_TOKEN_SIZE];
static volatile uint8_t g_voice_token_length = 0U;

/* 鍥哄畾 7 瀛楃瀹藉害锛岄伩鍏嶆柊鏃х姸鎬侀暱搴︿笉鍚岀暀涓嬫畫褰憋紙"Status:" + 7 瀛楃 = 鍥哄畾鏁磋锛?*/
static const char * const VOICE_COMMAND_TEXT[] =
{
    "balance",   /* VOICE_COMMAND_NONE     锛堜笂鐢甸粯璁ゅ钩琛★級 */
    "GO     ",   /* VOICE_COMMAND_FORWARD  '0' 鍓嶈繘 */
    "BACK   ",   /* VOICE_COMMAND_BACKWARD '1' 鍚庨€€ */
    "LEFT   ",   /* VOICE_COMMAND_LEFT     '2' 宸﹁浆 */
    "RIGHT  ",   /* VOICE_COMMAND_RIGHT    '3' 鍙宠浆 */
    "STOP   ",   /* VOICE_COMMAND_STOP     '4' 鍋滄 */
    "CAM    ",   /* VOICE_COMMAND_CAMERA   '5' 鎽勫儚澶存ā寮?*/
    "INS    ",   /* VOICE_COMMAND_INS      '6' 鎯妯″紡 */
    "JUMP   ",   /* VOICE_COMMAND_JUMP     '7' 璺宠穬妯″紡 */
    "Remote ",   /* VOICE_COMMAND_REMOTE   '8' 閬ユ帶妯″紡 */
    "FOLLOW ",   /* VOICE_COMMAND_FOLLOW   '9' 璺熼殢妯″紡 */
    "VOICE  ",   /* VOICE_COMMAND_VOICE_MODE "10" 璇煶妯″紡 */
};

static bool voice_command_is_motion(voice_command_t command)
{
    return (VOICE_COMMAND_FORWARD == command) ||
           (VOICE_COMMAND_BACKWARD == command) ||
           (VOICE_COMMAND_LEFT == command) ||
           (VOICE_COMMAND_RIGHT == command) ||
           (VOICE_COMMAND_STOP == command);
}

static void voice_process_token(void)
{
    voice_command_t command = VOICE_COMMAND_NONE;

    if((2U == g_voice_token_length) &&
       ('1' == g_voice_token[0]) && ('0' == g_voice_token[1]))
    {
        g_voice_mode_active = true;
        g_voice_command = VOICE_COMMAND_VOICE_MODE;
        g_voice_command_sequence++;
        return;
    }

    if(1U != g_voice_token_length)
    {
        return;
    }

    switch(g_voice_token[0])
    {
        case '0': command = VOICE_COMMAND_FORWARD;  break;
        case '1': command = VOICE_COMMAND_BACKWARD; break;
        case '2': command = VOICE_COMMAND_LEFT;     break;
        case '3': command = VOICE_COMMAND_RIGHT;    break;
        case '4': command = VOICE_COMMAND_STOP;     break;
        case '5': command = VOICE_COMMAND_CAMERA;   break;
        case '6': command = VOICE_COMMAND_INS;      break;
        case '7': command = VOICE_COMMAND_JUMP;     break;
        case '8': command = VOICE_COMMAND_REMOTE;   break;
        case '9': command = VOICE_COMMAND_FOLLOW;   break;
        default:  break;
    }

    if(VOICE_COMMAND_NONE == command)
    {
        return;
    }

    /* 鍦ㄦ憚鍍忓ご銆侀仴鎺с€佹儻瀵肩瓑妯″紡涓紝杩愬姩鎸囦护涓嶄細鏀瑰彉褰撳墠妯″紡鎴栦骇鐢熷姩浣溿€?
     * 渚嬪锛氭憚鍍忓ご锛堝畾鏃惰椹讹級妯″紡涓厑璁歌繍鍔ㄦ寚浠ゆ敞鍐岋紝鐢ㄤ簬閫€鍑鸿妯″紡
     * 锛堣"鍋滄/鍓嶈繘"绛夊嵆鍙灏忚溅鍋滀笅骞跺洖鍒板钩琛′繚鎸侊級銆?*/
    if(voice_command_is_motion(command) && !g_voice_mode_active && !camera_display_mode_active())
    {
        return;
    }

    if(!voice_command_is_motion(command))
    {
        g_voice_mode_active = false;
    }

    g_voice_command = command;
    g_voice_command_sequence++;
}

static void voice_ring_reset(void)
{
    g_voice_rx_ring_head = 0;
    g_voice_rx_ring_tail = 0;
}

static void voice_ring_push_from_isr(uint8_t data)
{
    uint8_t next_head = (uint8_t)((g_voice_rx_ring_head + 1U) % VOICE_CTRL_RX_RING_SIZE);
    if(next_head == g_voice_rx_ring_tail)
    {
        return; /* 鐜舰缂撳啿鍖烘弧锛屼涪寮冩柊瀛楄妭 */
    }

    g_voice_rx_ring[g_voice_rx_ring_head] = data;
    g_voice_rx_ring_head = next_head;
}

static bool voice_ring_pop(uint8_t * p_data)
{
    if(g_voice_rx_ring_tail == g_voice_rx_ring_head)
    {
        return false;
    }

    *p_data = g_voice_rx_ring[g_voice_rx_ring_tail];
    g_voice_rx_ring_tail = (uint8_t)((g_voice_rx_ring_tail + 1U) % VOICE_CTRL_RX_RING_SIZE);
    return true;
}

fsp_err_t voice_control_init(void)
{
#if !VOICE_CTRL_SCI2_AVAILABLE
    g_voice_command = VOICE_COMMAND_NONE;
    g_voice_opened = false;
    return FSP_ERR_UNSUPPORTED;
#else
    /* P802 澶嶇敤涓?SCI2_RXD2锛屽紑涓婃媺 + TTL 杈撳叆锛岃窡鏃犲埛 SCI4 鐨?RX 閰嶇疆涓€鑷?*/
    uint32_t const rx_pin_cfg = (uint32_t) IOPORT_CFG_PERIPHERAL_PIN |
                                (uint32_t) IOPORT_CFG_PULLUP_ENABLE |
                                (uint32_t) IOPORT_CFG_PIM_TTL |
                                (uint32_t) IOPORT_PERIPHERAL_SCI0_2_4_6_8;

    voice_ring_reset();
    g_voice_command = VOICE_COMMAND_NONE;
    g_voice_mode_active = false;
    g_voice_token_length = 0U;
    g_voice_opened = false;

    fsp_err_t err = g_ioport.p_api->pinCfg(g_ioport.p_ctrl, VOICE_CONTROL_RX_PIN, rx_pin_cfg);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_SCI_B_UART_BaudCalculate(VOICE_CONTROL_BAUDRATE, false, 5000U, &g_voice_uart2_baud_setting);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    /* open 鍐呴儴浼氶厤缃?4 涓腑鏂?RXI/TXI/TEI/ERI)骞舵墦寮€ RXI/ERI 鎺ユ敹涓柇 */
    err = g_voice_uart2.p_api->open(g_voice_uart2.p_ctrl, g_voice_uart2.p_cfg);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    g_voice_opened = true;
    return FSP_SUCCESS;
#endif
}

void voice_control_process(void)
{
#if VOICE_CTRL_SCI2_AVAILABLE
    if(!g_voice_opened)
    {
        return;
    }

    for(uint32_t i = 0; i < VOICE_CTRL_RX_PROCESS_LIMIT; i++)
    {
        uint8_t data = 0;
        if(!voice_ring_pop(&data))
        {
            break;
        }

        if(('\r' == data) || ('\n' == data))
        {
            if(g_voice_token_length > 0U)
            {
                voice_process_token();
                g_voice_token_length = 0U;
            }
            continue;
        }

        if(g_voice_token_length < VOICE_CTRL_TOKEN_SIZE)
        {
            g_voice_token[g_voice_token_length++] = (char)data;
        }
        else
        {
            g_voice_token_length = 0U;
        }
    }
#endif
}

voice_command_t voice_control_get_command(void)
{
    return g_voice_command;
}

uint32_t voice_control_get_command_sequence(void)
{
    return g_voice_command_sequence;
}

bool voice_control_mode_active(void)
{
    return g_voice_mode_active;
}

const char * voice_control_get_command_text(void)
{
    uint32_t idx = (uint32_t) g_voice_command;
    if(idx > (uint32_t) VOICE_COMMAND_VOICE_MODE)
    {
        idx = (uint32_t) VOICE_COMMAND_NONE;
    }
    return VOICE_COMMAND_TEXT[idx];
}

void voice_control_display(void)
{
    static voice_command_t last_command = VOICE_COMMAND_NONE;
    static uint32_t refresh_countdown = 0;

    voice_command_t command = voice_control_get_command();
    if((command == last_command) && (refresh_countdown > 0U))
    {
        refresh_countdown--;
        return;
    }

    last_command = command;
    refresh_countdown = VOICE_CTRL_DISPLAY_REFRESH_TICKS;

    /* 鏁磋榛勮壊鏄剧ず "Status:" + 鑻辨枃鐘舵€侊紝渚嬪 Status:GO / Status:balance锛?
     * 鏄剧ず瀹屾仮澶嶇豢鑹诧紙椤甸潰鍏跺畠鍐呭缁熶竴鐢ㄧ豢鑹诧級銆?*/
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(VOICE_CTRL_DISPLAY_X, VOICE_CTRL_DISPLAY_Y, "Status:");
    ips200_show_string(VOICE_CTRL_DISPLAY_X + (7U * 8U), VOICE_CTRL_DISPLAY_Y,
                       voice_control_get_command_text());
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
}

void voice_control_uart_callback(uart_callback_args_t * p_args)
{
#if !VOICE_CTRL_SCI2_AVAILABLE
    FSP_PARAMETER_NOT_USED(p_args);
#else
    if((NULL == p_args) || (2U != p_args->channel))
    {
        return;
    }

    /* RXI 涓柇锛氬彧鎶婃敹鍒扮殑瀛楄妭涓㈣繘鐜舰缂撳啿鍖猴紝瑙ｆ瀽鏀惧埌涓诲惊鐜噷鍋氾紝
     * 閬垮厤鍦ㄤ腑鏂噷鍋氬鏉傚鐞嗗鑷寸郴缁熷崱浣忥紙璺熸棤鍒?SCI4 涓€涓€濊矾锛夈€?*/
    if(UART_EVENT_RX_CHAR == p_args->event)
    {
        uint8_t data = (uint8_t)(p_args->data & 0xFFU);
        if(('4' == data) && g_voice_mode_active)
        {
            /* 鍋滄鍛戒护鐩存帴鍦ㄦ帴鏀朵腑鏂腑閿佸瓨锛屾姠鍗犻槦鍒楅噷灏氭湭澶勭悊鐨勮繍鍔ㄥ懡浠ゃ€?
             * 鎺у埗鐜笅涓€鍛ㄦ湡锛堟渶闀跨害 5ms锛夊嵆鍙竻闄よ繍鍔ㄨ緭鍑哄苟缁х画骞宠　銆?*/
            g_voice_command = VOICE_COMMAND_STOP;
            g_voice_command_sequence++;
            g_voice_rx_ring_tail = g_voice_rx_ring_head;
            g_voice_token_length = 0U;
        }
        else
        {
            voice_ring_push_from_isr(data);
        }
    }
#endif
}
