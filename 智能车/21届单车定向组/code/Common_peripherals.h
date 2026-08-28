/*
 * Common_peripherals.h
 *
 *  Created on: 2026楠?閺?閺?
 *      Author: Super_burger
 */

#ifndef CODE_COMMON_PERIPHERALS_H_
#define CODE_COMMON_PERIPHERALS_H_

#define Buzzer_pin  P33_10

#define KEY1        P20_6
#define KEY2        P11_2
#define KEY3        P11_3
#define KEY4        P33_8

#define Switch1     P33_11
#define Switch2     P33_12

#define LED1                    (P20_9)
#define LED2                    (P20_8)
#define LED3                    (P21_5)
#define LED4                    (P21_4)

//IPS114鐎瑰繐鐣炬稊?    240*114
//#define ips_show_string                  ips114_show_string
//#define ips_show_int                     ips114_show_int
//#define ips_show_uint                    ips114_show_uint
//#define ips_show_float                   ips114_show_float
//#define ips_show_chinese                 ips114_show_chinese
//#define ips_clear                        ips114_clear
//#define ips_init                         ips114_init
//#define ips_show_rgb565_image            ips114_show_rgb565_image

//IPS200鐎瑰繐鐣炬稊?    240*320
#define ips_show_string                  ips200_show_string
#define ips_show_int                     ips200_show_int
#define ips_show_uint                    ips200_show_uint
#define ips_show_float                   ips200_show_float
#define ips_show_chinese                 ips200_show_chinese
#define ips_clear                        ips200_clear
#define ips_init                         ips200_init
#define ips_show_rgb565_image            ips200_show_rgb565_image


#define M_MAX    5000
#define M_MIN   -5000

//#define ENCODER_1                   (TIM6_ENCODER)
//#define ENCODER_1_A                 (TIM6_ENCODER_CH1_P20_3)
//#define ENCODER_1_B                 (TIM6_ENCODER_CH2_P20_0)

#define ENCODER_1                   (TIM2_ENCODER)
#define ENCODER_1_A                 (TIM2_ENCODER_CH1_P33_7)
#define ENCODER_1_B                 (TIM2_ENCODER_CH2_P33_6)

#define SERVO_MOTOR_PWM             (ATOM1_CH1_P33_9)                           // 鐎规矮绠熸稉缁樻緲娑撳﹨鍩栭張鍝勵嚠鎼存柨绱╅懘?
#define SERVO_MOTOR_FREQ            (300)                                        // 鐎规矮绠熸稉缁樻緲娑撳﹨鍩栭張娲暥閻? 鐠囧嘲濮熻箛鍛暈閹板繗瀵栭崶?
//#define SERVO_MOTOR_MID             (122)                                        //92
//#define SERVO_MOTOR_LMAX            (72)                                        //44
//#define SERVO_MOTOR_RMAX            (172)                                       //140


//#define SERVO_MOTOR_MID             (82)
//#define SERVO_MOTOR_LMAX            (32)
//#define SERVO_MOTOR_RMAX            (132)


//#define SERVO_MOTOR_MID             (132)
//#define SERVO_MOTOR_LMAX            (82)
//#define SERVO_MOTOR_RMAX            (192)

//#define SERVO_MOTOR_MID             (112)
//#define SERVO_MOTOR_LMAX            (62)
//#define SERVO_MOTOR_RMAX            (162)

//#define SERVO_MOTOR_MID             (117)
//#define SERVO_MOTOR_LMAX            (67)
//#define SERVO_MOTOR_RMAX            (167)

//#define SERVO_MOTOR_MID             (142)
//#define SERVO_MOTOR_LMAX            (92)
//#define SERVO_MOTOR_RMAX            (192)

#define SERVO_MOTOR_MID             (89)
#define SERVO_MOTOR_LMAX            (39)
#define SERVO_MOTOR_RMAX            (139)

#define SERVO_MOTOR_DUTY(x)         ((float)PWM_DUTY_MAX/(1000.0/(float)SERVO_MOTOR_FREQ)*(0.5+(float)(x)/90.0))// ------------------ 閼稿灚婧€閸楃姷鈹栧В鏃囶吀缁犳鏌熷?------------------

extern uint8 key1_flag;
extern uint8 key2_flag;
extern uint8 key3_flag;
extern uint8 key4_flag;

extern float A_SPEED;
extern int32 Distance;
extern int32 g_encoder_raw;

extern float G_KP;
extern float G_KD;
extern float A_KP;
extern float N_SPEED;


extern int CTRL_flag;



void Buzzer_init(void);
void Buzzer_check(int TIME1);
void Key_init(void);
void Key_scan(void);
void key1_clear(void);
void key2_clear(void);
void key3_clear(void);
void key4_clear(void);


void CYT2_S_motor_ctrl(int32 SPEED);
void CYT2_S_motor_loop_ctrl(float T_SPEED);
void Motor_text(void);

void QUD_encoder_init(void);
void QUD_encoder_pulse_get(void);
float Cal_Distance(int32 A_SPEED);
float CYT2_get_distance_mag(int16 speed);
void Encoder_text(void);

void Steer_init(void);
void Steer_set(int angle);
void Steer_text(void);

void New_ctrl(void);
int16_t Remap_Angle_Linear(int16_t channel);
float KnobStepAngle(int16_t ch);
void GUN_ctrl_text(void);

//void Wx_paramt_init(void);
//void paramt_give(void);
void new_ctrl(void);
#endif /* CODE_COMMON_PERIPHERALS_H_ */
