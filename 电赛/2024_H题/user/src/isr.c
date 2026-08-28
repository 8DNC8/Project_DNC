/*********************************************************************************************************************
* MSPM0G3507 Opensource Library 鍗筹紙MSPM0G3507 寮€婧愬簱锛夋槸涓€涓熀浜庡畼鏂?SDK 鎺ュ彛鐨勭涓夋柟寮€婧愬簱
* Copyright (c) 2022 SEEKFREE 閫愰绉戞妧
* 
* 鏈枃浠舵槸 MSPM0G3507 寮€婧愬簱鐨勪竴閮ㄥ垎
* 
* MSPM0G3507 寮€婧愬簱 鏄厤璐硅蒋浠?
* 鎮ㄥ彲浠ユ牴鎹嚜鐢辫蒋浠跺熀閲戜細鍙戝竷鐨?GPL锛圙NU General Public License锛屽嵆 GNU閫氱敤鍏叡璁稿彲璇侊級鐨勬潯娆?
* 鍗?GPL 鐨勭3鐗堬紙鍗?GPL3.0锛夋垨锛堟偍閫夋嫨鐨勶級浠讳綍鍚庢潵鐨勭増鏈紝閲嶆柊鍙戝竷鍜?鎴栦慨鏀瑰畠
* 
* 鏈紑婧愬簱鐨勫彂甯冩槸甯屾湜瀹冭兘鍙戞尌浣滅敤锛屼絾骞舵湭瀵瑰叾浣滀换浣曠殑淇濊瘉
* 鐢氳嚦娌℃湁闅愬惈鐨勯€傞攢鎬ф垨閫傚悎鐗瑰畾鐢ㄩ€旂殑淇濊瘉
* 鏇村缁嗚妭璇峰弬瑙?GPL
* 
* 鎮ㄥ簲璇ュ湪鏀跺埌鏈紑婧愬簱鐨勫悓鏃舵敹鍒颁竴浠?GPL 鐨勫壇鏈?
* 濡傛灉娌℃湁锛岃鍙傞槄<https://www.gnu.org/licenses/>
* 
* 棰濆娉ㄦ槑锛?
* 鏈紑婧愬簱浣跨敤 GPL3.0 寮€婧愯鍙瘉鍗忚 浠ヤ笂璁稿彲鐢虫槑涓鸿瘧鏂囩増鏈?
* 璁稿彲鐢虫槑鑻辨枃鐗堝湪 libraries/doc 鏂囦欢澶逛笅鐨?GPL3_permission_statement.txt 鏂囦欢涓?
* 璁稿彲璇佸壇鏈湪 libraries 鏂囦欢澶逛笅 鍗宠鏂囦欢澶逛笅鐨?LICENSE 鏂囦欢
* 娆㈣繋鍚勪綅浣跨敤骞朵紶鎾湰绋嬪簭 浣嗕慨鏀瑰唴瀹规椂蹇呴』淇濈暀閫愰绉戞妧鐨勭増鏉冨０鏄庯紙鍗虫湰澹版槑锛?
* 
* 鏂囦欢鍚嶇О          isr
* 鍏徃鍚嶇О          鎴愰兘閫愰绉戞妧鏈夐檺鍏徃
* 鐗堟湰淇℃伅          鏌ョ湅 libraries/doc 鏂囦欢澶瑰唴 version 鏂囦欢 鐗堟湰璇存槑
* 寮€鍙戠幆澧?         MDK 5.37
* 閫傜敤骞冲彴          MSPM0G3507
* 搴楅摵閾炬帴          https://seekfree.taobao.com/
********************************************************************************************************************/


#include "isr.h"

void TIMA0_IRQHandler (void)
{
    pit_callback_list[0](0, pit_callback_ptr_list[0]);
}

void TIMA1_IRQHandler (void)
{
    pit_callback_list[1](0, pit_callback_ptr_list[1]);
}

void TIMG0_IRQHandler (void)
{
    pit_callback_list[2](0, pit_callback_ptr_list[2]);
}

void TIMG6_IRQHandler (void)
{
    pit_callback_list[3](0, pit_callback_ptr_list[3]);
}

void TIMG7_IRQHandler (void)
{
    pit_callback_list[4](0, pit_callback_ptr_list[4]);
}

void TIMG8_IRQHandler (void)
{
    pit_callback_list[5](0, pit_callback_ptr_list[5]);
}

void TIMG12_IRQHandler (void)
{
    pit_callback_list[6](0, pit_callback_ptr_list[6]);
}

void UART0_IRQHandler (void)
{
	switch(DL_UART_getPendingInterrupt(UART0))
	{
		case DL_UART_IIDX_TX:
        {
            uart_callback_list[0](UART_INTERRUPT_STATE_TX, uart_callback_ptr_list[0]);
        }break;
		case DL_UART_IIDX_RX:
        {
            uart_callback_list[0](UART_INTERRUPT_STATE_RX, uart_callback_ptr_list[0]);
#if DEBUG_UART_USE_INTERRUPT
                debug_interrupr_handler();
#endif
        }break;

		default:    break;
	}
    DL_UART_clearInterruptStatus(UART0, UART0->CPU_INT.RIS);
}

void UART1_IRQHandler (void)
{
	switch(DL_UART_getPendingInterrupt(UART1))
	{
		case DL_UART_IIDX_TX:
        {
            uart_callback_list[1](UART_INTERRUPT_STATE_TX, uart_callback_ptr_list[1]);
        }break;
		case DL_UART_IIDX_RX:
        {
            uart_callback_list[1](UART_INTERRUPT_STATE_RX, uart_callback_ptr_list[1]);
					
					 //wifi_uart_callback();
			
			wireless_module_uart_handler();                 // ??????????
					
        }break;

		default:    break;
	}
    DL_UART_clearInterruptStatus(UART1, UART1->CPU_INT.RIS);
}

void UART2_IRQHandler (void)
{
	switch(DL_UART_getPendingInterrupt(UART2))
	{
		case DL_UART_IIDX_TX:
        {
            uart_callback_list[2](UART_INTERRUPT_STATE_TX, uart_callback_ptr_list[2]);
        }break;
		case DL_UART_IIDX_RX:
        {
            uart_callback_list[2](UART_INTERRUPT_STATE_RX, uart_callback_ptr_list[2]);
        }break;

		default:    break;
	}
    DL_UART_clearInterruptStatus(UART2, UART2->CPU_INT.RIS);
}

void UART3_IRQHandler (void)
{
	switch(DL_UART_getPendingInterrupt(UART3))
	{
		case DL_UART_IIDX_TX:
        {
            uart_callback_list[3](UART_INTERRUPT_STATE_TX, uart_callback_ptr_list[3]);
        }break;
		case DL_UART_IIDX_RX:
        {
            uart_callback_list[3](UART_INTERRUPT_STATE_RX, uart_callback_ptr_list[3]);
        }break;

		default:    break;
	}
    DL_UART_clearInterruptStatus(UART3, UART3->CPU_INT.RIS);
}

void GROUP1_IRQHandler (void)
{
    uint8 exti_index = 0;
    uint8 exti_event = 0;

    uint32  register_temp = gpio_group[0]->CPU_INT.IIDX;
    if(register_temp)
    {
        exti_index = register_temp - 1;

        if(15 >= exti_index)
        {
            exti_event  = (gpio_group[0]->POLARITY15_0 >> ((exti_index % 16) * 2)) & 0x03;
        }
        else
        {
            exti_event  = (gpio_group[0]->POLARITY31_16 >> ((exti_index % 16) * 2)) & 0x03;
        }
        exti_callback_list[exti_index](exti_event, exti_callback_ptr_list[exti_index]);
    }
    else
    {
        register_temp = gpio_group[1]->CPU_INT.IIDX;
        if(register_temp)
        {
            exti_index = register_temp - 1;

            if(15 >= exti_index)
            {
                exti_event  = (gpio_group[1]->POLARITY15_0 >> ((exti_index % 16) * 2)) & 0x03;
            }
            else
            {
                exti_event  = (gpio_group[1]->POLARITY31_16 >> ((exti_index % 16) * 2)) & 0x03;
            }
            exti_callback_list[exti_index](exti_event, exti_callback_ptr_list[exti_index]);
        }
    }
}
