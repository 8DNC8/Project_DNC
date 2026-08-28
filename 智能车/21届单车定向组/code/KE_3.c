/*
 * KE_3.c
 *
 *  Created on: 2026楠?閺?4閺? *      Author: Super_burger
 *
 *  缁夋垹娲?閿涙氨顫栭惄?閻?閻滎垯瑕嗙痪褎甯堕崚?+ 閹垰顕辩憴鎺戝 + 瀵垶浜鹃懛顏勫З閸戝繘鈧? *    閹貉冨煑閿涙瓬alance_mode_parameter(3) 閳?閺堝顫楅柅鐔峰閻? *    鐟欐帒瀹抽敍姘降閼?INS閿涘潛uandao_load 閺囧瓨鏌?Taget_angle閿? *    闁喎瀹抽敍姘辨纯闁?SPEED_MAX閿涘苯闆嗛柆鎾瑰殰閸斻劑妾烽崚?SPEED_MIN
 */

#include "zf_common_headfile.h"

/* 瀵垶浜鹃懛顏勫З閸戝繘鈧喎寮弫?*/
#define SPEED_MAX   1200
#define SPEED_MIN   800


void Body_ctrl_3(void)
{
    static bool once = false;
    if (!once)
    {
      balance_mode_parameter(3);            // 缁夋垹娲?閸欏倹鏆熼敍鍫熸箒鐟欐帡鈧喎瀹抽悳顖ょ礆
      Taget_angle = roll_balance_cascade.posture_value.yaw;
      once = true;
    }


    Imu_lowpass_filter();                   // IMU閺佺増宓佹稉鈧梼鏈电秵闁碍鎶ゅ▔?

////    /* ===== 瀵垶浜鹃懛顏勫З閸戝繘鈧?===== */
////
//    float turn_err = fabsf(AngleErrorNormalize(Taget_angle - roll_balance_cascade.posture_value.yaw));
//    float speed_target = SPEED_MAX;
//    if(turn_err > 20.0f)
//    {
//        speed_target = SPEED_MIN;
//    }
//    else if(turn_err > 0.5f)
//    {
//        speed_target = SPEED_MAX - (SPEED_MAX - SPEED_MIN) * (turn_err - 0.5f) / 19.5f;
//    }


    /* ===== 3閻滎垯瑕嗙痪褎甯堕崚璁圭礄閸氬瞼顫栭惄?閿?===== */

    if(sys_times%1==0)//鐟欐帡鈧喎瀹抽悳?    {
        if(Imu_type==1)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle, roll_balance_cascade.angle_cycle.out, imu660ra_gyro_x);
        }
        else if(Imu_type==2)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle, roll_balance_cascade.angle_cycle.out, imu660rb_gyro_y);
        }
        else if(Imu_type==3)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle, roll_balance_cascade.angle_cycle.out, imu963ra_gyro_y);
        }
    }

    if(sys_times%5==0)//鐟欐帒瀹抽悳?    {
        pid_control(&roll_balance_cascade.angle_cycle, roll_balance_cascade.turn_cycle.out-roll_balance_cascade.posture_value.mechanical_zero, -roll_balance_cascade.posture_value.rol);
    }

    if(sys_times%20==0)//鏉烆剙鎮滈悳?閳?Taget_angle 閻?INS 閺囧瓨鏌婇敍灞肩瑝閻?StepApproach
    {
        pid_control(&roll_balance_cascade.turn_cycle, AngleErrorNormalize(Taget_angle - roll_balance_cascade.posture_value.yaw), 0);
    }


     Steer_set(SERVO_MOTOR_MID + roll_balance_cascade.angular_speed_cycle.out);

     /* 鐠侯垰绶炵紒鍫㈠仯濡偓濞村绱濋懛顏勫З閸嬫粏婧?*/
     if(guandao_new_cnt >= guandao_index)
     {
         CYT2_S_motor_loop_ctrl(0);
     }
     else
     {
         CYT2_S_motor_loop_ctrl(Motor_Standard_Speed);//闂傤厾骞嗛柅鐔峰
//       CYT2_S_motor_ctrl(1200);//瀵偓閻滎垶鈧喎瀹?     }


}
