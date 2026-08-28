
/*
 * Ins.h
 *
 *  Created on: 2026楠?閺?閺?
 *      Author: Super_burger
 */

#ifndef CODE_INS_H_
#define CODE_INS_H_

#define dis_position 0.01       //鐠侊紕鐣绘担宥囩枂鐠烘繄顬?
#define MAX 1000                //閺堚偓婢舵艾鐡ㄩ悙閫涢嚋閺?
#define FLASH_H_PAGE 8          //鐎涙ê銇旈柈銊ュ棘閺?
#define FLASH_X_PAGE 9          //鐎涙Φ閻ㄥ嫰銆夐弫?
#define FLASH_Y_PAGE 10         //鐎涙Χ閻ㄥ嫰銆夐弫?
#define FLASH_YAW_PAGE 11       //鐎涙Χ閻ㄥ嫰銆夐弫?

typedef enum
{
    guandao_pass_mode = 0,     // 缁屾椽妫?
    guandao_record_mode,       // 瑜版洖鍩楅懜顏勬倻
    guandao_load_mode          // 閸ョ偞鏂佸顏囨姉
} guandao_mode;

union FloatInspector {
    float    f;   // 4 鐎涙濡?
    uint32   i;   // 4 鐎涙濡?
};

extern guandao_mode gd_mode;
extern union FloatInspector data_x,data_y,data_yaw;

extern float dis_record;
extern int guandao_index;         //鐠佹澘缍嶉悙鍦畱閻╊喖缍?
extern int guandao_cnt;           //瑜版挸澧犲顏囨姉閻?
extern int guandao_last_cnt;
extern int guandao_new_cnt;
extern float guandao_lucheng;
extern float X_Record_f[MAX];
extern float Y_Record_f[MAX];
extern float Yaw_Record_f[MAX];
extern uint32 Yaw_Record_i[MAX];

extern float Motor_Standard_Speed;
extern float Motor_Standard_Speed_real;
extern float Motor_Standard_Speed_real_set;
extern float theta0;    //theta0   鐠佹澘缍嶈ぐ鎾冲閸嬪繗鍩呯憴?   鏉堟挸鍤惄顔界垼閸婄厧浜搁懜顏囶潡
extern float theta1;    //theta1   flash娑擃厾娈戦崑蹇氬焻鐟?
extern float theta2;    //theta2   閸ф劖鐖ｇ拋锛勭暬瀵版鍤惃鍕焊閼割亣顫?
extern float c_error;
extern float int_c_error;
extern float stl_kp;    //8.5
extern float stl_ki;
extern float step;

void guandao_record(void);
void guandao_flash_record(void);
void guandao_flash_load(void);
void guandao_load(void);
void guandao_Init(void);
void guandao_task(void);

void Ins_text(void);
void INS_log(void);
void INS_data_get(void);

#endif /* CODE_INS_H_ */
