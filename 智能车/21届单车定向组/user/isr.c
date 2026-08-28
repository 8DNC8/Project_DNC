/*********************************************************************************************************************
* TC264 Opensourec Library 鍗筹紙TC264 寮€婧愬簱锛夋槸涓€涓熀浜庡畼鏂?SDK 鎺ュ彛鐨勭涓夋柟寮€婧愬簱
* Copyright (c) 2022 SEEKFREE 閫愰绉戞妧
*
* 鏈枃浠舵槸 TC264 寮€婧愬簱鐨勪竴閮ㄥ垎
*
* TC264 寮€婧愬簱 鏄厤璐硅蒋浠?* 鎮ㄥ彲浠ユ牴鎹嚜鐢辫蒋浠跺熀閲戜細鍙戝竷鐨?GPL锛圙NU General Public License锛屽嵆 GNU閫氱敤鍏叡璁稿彲璇侊級鐨勬潯娆?* 鍗?GPL 鐨勭3鐗堬紙鍗?GPL3.0锛夋垨锛堟偍閫夋嫨鐨勶級浠讳綍鍚庢潵鐨勭増鏈紝閲嶆柊鍙戝竷鍜?鎴栦慨鏀瑰畠
*
* 鏈紑婧愬簱鐨勫彂甯冩槸甯屾湜瀹冭兘鍙戞尌浣滅敤锛屼絾骞舵湭瀵瑰叾浣滀换浣曠殑淇濊瘉
* 鐢氳嚦娌℃湁闅愬惈鐨勯€傞攢鎬ф垨閫傚悎鐗瑰畾鐢ㄩ€旂殑淇濊瘉
* 鏇村缁嗚妭璇峰弬瑙?GPL
*
* 鎮ㄥ簲璇ュ湪鏀跺埌鏈紑婧愬簱鐨勫悓鏃舵敹鍒颁竴浠?GPL 鐨勫壇鏈?* 濡傛灉娌℃湁锛岃鍙傞槄<https://www.gnu.org/licenses/>
*
* 棰濆娉ㄦ槑锛?* 鏈紑婧愬簱浣跨敤 GPL3.0 寮€婧愯鍙瘉鍗忚 浠ヤ笂璁稿彲鐢虫槑涓鸿瘧鏂囩増鏈?* 璁稿彲鐢虫槑鑻辨枃鐗堝湪 libraries/doc 鏂囦欢澶逛笅鐨?GPL3_permission_statement.txt 鏂囦欢涓?* 璁稿彲璇佸壇鏈湪 libraries 鏂囦欢澶逛笅 鍗宠鏂囦欢澶逛笅鐨?LICENSE 鏂囦欢
* 娆㈣繋鍚勪綅浣跨敤骞朵紶鎾湰绋嬪簭 浣嗕慨鏀瑰唴瀹规椂蹇呴』淇濈暀閫愰绉戞妧鐨勭増鏉冨０鏄庯紙鍗虫湰澹版槑锛?*
* 鏂囦欢鍚嶇О          isr
* 鍏徃鍚嶇О          鎴愰兘閫愰绉戞妧鏈夐檺鍏徃
* 鐗堟湰淇℃伅          鏌ョ湅 libraries/doc 鏂囦欢澶瑰唴 version 鏂囦欢 鐗堟湰璇存槑
* 寮€鍙戠幆澧?         ADS v1.10.2
* 閫傜敤骞冲彴          TC264D
* 搴楅摵閾炬帴          https://seekfree.taobao.com/
*
* 淇敼璁板綍
* 鏃ユ湡              浣滆€?               澶囨敞
* 2022-09-15       pudding            first version
********************************************************************************************************************/

#include "isr_config.h"
#include "isr.h"

// 瀵逛簬TC绯诲垪榛樿鏄笉鏀寔涓柇宓屽鐨勶紝甯屾湜鏀寔涓柇宓屽闇€瑕佸湪涓柇鍐呬娇鐢?interrupt_global_enable(0); 鏉ュ紑鍚腑鏂祵濂?// 绠€鍗曠偣璇村疄闄呬笂杩涘叆涓柇鍚嶵C绯诲垪鐨勭‖浠惰嚜鍔ㄨ皟鐢ㄤ簡 interrupt_global_disable(); 鏉ユ嫆缁濆搷搴斾换浣曠殑涓柇锛屽洜姝ら渶瑕佹垜浠嚜宸辨墜鍔ㄨ皟鐢?interrupt_global_enable(0); 鏉ュ紑鍚腑鏂殑鍝嶅簲銆?
// **************************** PIT涓柇鍑芥暟 ****************************

IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    pit_clear_flag(CCU60_CH1);




}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    pit_clear_flag(CCU61_CH0);




}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    pit_clear_flag(CCU61_CH1);




}
// **************************** PIT涓柇鍑芥暟 ****************************


// **************************** 澶栭儴涓柇鍑芥暟 ****************************
IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    if(exti_flag_get(ERU_CH0_REQ0_P15_4))           // 閫氶亾0涓柇
    {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);

    }

    if(exti_flag_get(ERU_CH4_REQ13_P15_5))          // 閫氶亾4涓柇
    {
        exti_flag_clear(ERU_CH4_REQ13_P15_5);




    }
}

IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?
    if(exti_flag_get(ERU_CH1_REQ10_P14_3))          // 閫氶亾1涓柇
    {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);

        tof_module_exti_handler();                  // ToF 妯″潡 INT 鏇存柊涓柇

    }

    if(exti_flag_get(ERU_CH5_REQ1_P15_8))           // 閫氶亾5涓柇
    {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);


    }
}

// 鐢变簬鎽勫儚澶磒clk寮曡剼榛樿鍗犵敤浜?2閫氶亾锛岀敤浜庤Е鍙慏MA锛屽洜姝よ繖閲屼笉鍐嶅畾涔変腑鏂嚱鏁?// IFX_INTERRUPT(exti_ch2_ch6_isr, 0, EXTI_CH2_CH6_INT_PRIO)
// {
//  interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?//  if(exti_flag_get(ERU_CH2_REQ7_P00_4))           // 閫氶亾2涓柇
//  {
//      exti_flag_clear(ERU_CH2_REQ7_P00_4);
//  }
//  if(exti_flag_get(ERU_CH6_REQ9_P20_0))           // 閫氶亾6涓柇
//  {
//      exti_flag_clear(ERU_CH6_REQ9_P20_0);
//  }
// }
IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    if(exti_flag_get(ERU_CH3_REQ6_P02_0))           // 閫氶亾3涓柇
    {
        exti_flag_clear(ERU_CH3_REQ6_P02_0);
        camera_vsync_handler();                     // 鎽勫儚澶磋Е鍙戦噰闆嗙粺涓€鍥炶皟鍑芥暟
    }
    if(exti_flag_get(ERU_CH7_REQ16_P15_1))          // 閫氶亾7涓柇
    {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);




    }
}
// **************************** 澶栭儴涓柇鍑芥暟 ****************************


// **************************** DMA涓柇鍑芥暟 ****************************
IFX_INTERRUPT(dma_ch5_isr, 0, DMA_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    camera_dma_handler();                           // 鎽勫儚澶撮噰闆嗗畬鎴愮粺涓€鍥炶皟鍑芥暟
}
// **************************** DMA涓柇鍑芥暟 ****************************


// **************************** 涓插彛涓柇鍑芥暟 ****************************
// 涓插彛0榛樿浣滀负璋冭瘯涓插彛
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?


}
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?
#if DEBUG_UART_USE_INTERRUPT                        // 濡傛灉寮€鍚?debug 涓插彛涓柇
        debug_interrupr_handler();                  // 璋冪敤 debug 涓插彛鎺ユ敹澶勭悊鍑芥暟 鏁版嵁浼氳 debug 鐜舰缂撳啿鍖鸿鍙?#endif                                              // 濡傛灉淇敼浜?DEBUG_UART_INDEX 閭ｈ繖娈典唬鐮侀渶瑕佹斁鍒板搴旂殑涓插彛涓柇鍘?}


// 涓插彛1榛樿杩炴帴鍒版憚鍍忓ご閰嶇疆涓插彛
IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?



}
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    camera_uart_handler();                          // 鎽勫儚澶村弬鏁伴厤缃粺涓€鍥炶皟鍑芥暟
}

// 涓插彛2榛樿杩炴帴鍒版棤绾胯浆涓插彛妯″潡
IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?


}

IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?//    wireless_module_uart_handler();                 // 鏃犵嚎妯″潡缁熶竴鍥炶皟鍑芥暟

    uart_receiver_callback();                        // 涓插彛鎺ユ敹鏈哄洖璋冨嚱鏁?

}
// 涓插彛3榛樿杩炴帴鍒癎PS瀹氫綅妯″潡
IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?


}

IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?//    gnss_uart_callback();                           // GNSS涓插彛鍥炶皟鍑芥暟

    uart_control_callback();


}

// 涓插彛閫氳閿欒涓柇
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    IfxAsclin_Asc_isrError(&uart0_handle);
}
IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    IfxAsclin_Asc_isrError(&uart1_handle);
}
IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    IfxAsclin_Asc_isrError(&uart2_handle);
}
IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 寮€鍚腑鏂祵濂?    IfxAsclin_Asc_isrError(&uart3_handle);
}
