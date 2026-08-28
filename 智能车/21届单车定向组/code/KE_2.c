/*
 * KE_2.c
 *
 *  Created on: 2026楠?閺?閺? *      Author: Super_burger
 *
 *  閹垰顕遍崶鐐存杹濡€崇础閿? *    1. 閹靛濮╅幒銊ㄦ簠瑜版洖鍩楅崗顐㈢摟鐠侯垰绶為敍鍦攐dy_keep + guandao_record閿? *    2. 閸ョ偞鏂侀弮?guandao_load() 閼奉亜濮╅弴瀛樻煀 Taget_angle
 *    3. 鏉烆剙鎮滈悳顖滄纯閹恒儴绐￠煪?Taget_angle閿涘潛uandao_load 閸愬懘鍎村鎻掍粵闂冭埖顫柅鑹扮箮閿? *    4. 鐠烘垵鐣懛顏勫З閸嬫粏婧? */

#include "zf_common_headfile.h"
float k2_speed=330;//闂傤厾骞?30閿涘苯绱戦悳?00

void Body_ctrl_2(void)
{
    static bool once = false;     //娣囨繆鐦夐崣顏囩ゴ閸婇棿绔村▎?    if (!once)
    {
      balance_mode_parameter(2);   //閸掑棝鍘ょ粔鎴犳窗2閸欏倹鏆?
      /* 閸ョ偞鏂侀弮鍓佹窗閺嶅洩顫楁禒搴＄秼閸?yaw 瀵偓婵绱漡uandao_load 娴兼岸鈧劖顒為幒銊ユ倻鐠侯垰绶為弬鐟版倻 */
      if(gd_mode == guandao_load_mode && guandao_index > 0)
      {
          Taget_angle = roll_balance_cascade.posture_value.yaw;
      }

      once = true;
    }


    if(sys_times%5==0)//鐟欐帒瀹抽悳?    {
        pid_control(&roll_balance_cascade.angle_cycle, roll_balance_cascade.turn_cycle.out-roll_balance_cascade.posture_value.mechanical_zero, -roll_balance_cascade.posture_value.rol);
    }

    if(sys_times%20==0)//鏉烆剙鎮滈悳?    {
        /* Taget_angle 閻?guandao_load() 閸?ISR 娑擃叀鍤滈崝銊︻劄鏉╂稒娲块弬甯礉娑撳秹娓剁憰浣稿晙婵?StepApproach */
        pid_control(&roll_balance_cascade.turn_cycle, AngleErrorNormalize(Taget_angle - roll_balance_cascade.posture_value.yaw), 0);
    }


     Steer_set(SERVO_MOTOR_MID + roll_balance_cascade.angle_cycle.out);

     /* 鐠侯垰绶炵紒鍫㈠仯濡偓濞村绱濋懛顏勫З閸嬫粏婧?*/
     if(guandao_new_cnt >= guandao_index)
     {
//         CYT2_S_motor_loop_ctrl(0);
         CYT2_S_motor_ctrl(0);
     }
     else
     {
       CYT2_S_motor_loop_ctrl(k2_speed);//闂傤厾骞?//         CYT2_S_motor_ctrl(k2_speed);//瀵偓閻?     }

}


/*
 * LCD 鐠嬪啳鐦弰鍓с仛 閳?閸?ips200 娑撳﹥妯夌粈鐑樺劵鐎佃壈绐￠煪顏勫彠闁款喗鏆熼幑? */
void Balance_2_text(void)
{
    float yaw_err = AngleErrorNormalize(Taget_angle - roll_balance_cascade.posture_value.yaw);
    float yaw_ref = 0;
    if(guandao_new_cnt < guandao_index) yaw_ref = Yaw_Record_f[guandao_new_cnt];

    ips_show_string(8*0,  16*0, "YAW:");      ips_show_float(8*10, 16*0, roll_balance_cascade.posture_value.yaw, 3, 6);
    ips_show_string(8*0,  16*1, "T_A:");       ips_show_float(8*10, 16*1, Taget_angle, 3, 6);
    ips_show_string(8*0,  16*2, "Y_ERR:");     ips_show_float(8*10, 16*2, yaw_err, 3, 6);
    ips_show_string(8*0,  16*3, "c_err:");     ips_show_float(8*10, 16*3, c_error, 3, 6);
    ips_show_string(8*0,  16*4, "Y_ref:");     ips_show_float(8*10, 16*4, yaw_ref, 3, 6);
    ips_show_string(8*0,  16*5, "idx:");       ips_show_uint(8*10, 16*5, guandao_new_cnt, 5);
    ips_show_string(8*0,  16*6, "total:");     ips_show_uint(8*10, 16*6, guandao_index, 5);
    ips_show_string(8*0,  16*7, "X:");         ips_show_float(8*10, 16*7, data_x.f, 3, 6);
    ips_show_string(8*0,  16*8, "Y:");         ips_show_float(8*10, 16*8, data_y.f, 3, 6);
    ips_show_string(8*0,  16*9, "G_out:");     ips_show_float(8*10, 16*9, roll_balance_cascade.angle_cycle.out, 3, 6);

    /* 閺堚偓閸氬孩妯夌粈楦跨箥鐞涘瞼濮搁幀?*/
    if(guandao_new_cnt >= guandao_index && guandao_index > 0)
        ips_show_string(8*0, 16*11, " === FINISHED ===");
    else if(gd_mode == guandao_load_mode)
        ips_show_string(8*0, 16*11, " >>> TRACKING <<<");
    else
        ips_show_string(8*0, 16*11, " --- IDLE ---");
}


/*
 * 娑撴彃褰涚拫鍐槸鏉堟挸鍤?閳?濮?100ms 閹垫挸宓冩稉鈧悰灞惧劵鐎佃壈绐￠煪顏呮殶閹? * 閸︺劋瑕嗛崣锝呭И閹靛鑵戦弻銉ф箙閿涘本鏌熸笟鍨秿閺佺増宓侀崑姘禈鐞涖劌鍨庨弸? */
void Balance_2_printf(void)
{
    static uint32 print_tick = 0;
    print_tick++;

    if(print_tick < 100) return;  // 100ms 鏉堟挸鍤稉鈧悰?(sys_times 1ms)
    print_tick = 0;

    float yaw     = roll_balance_cascade.posture_value.yaw;
    float yaw_ref = 0;
    if(guandao_new_cnt < guandao_index) yaw_ref = Yaw_Record_f[guandao_new_cnt];

    printf("INS|idx=%4d/%d|yaw=%+7.2f|T_A=%+7.2f|c_err=%+6.3f|yaw_ref=%+7.2f|X=%+6.3f|Y=%+6.3f\r\n",
           guandao_new_cnt, guandao_index,
           yaw, Taget_angle, c_error, yaw_ref,
           data_x.f, data_y.f);
}
