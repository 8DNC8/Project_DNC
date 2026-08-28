/*
 * Ins.c
 *
 *  Created on: 2026楠?閺?閺?
 *      Author: Super_burger
 */

#include "zf_common_headfile.h"

float Motor_Standard_Speed = 1000.0f;         //閻╊喗鐖ｆ潪锕傗偓?
float Motor_Standard_Speed_real_set=33;
float Motor_Standard_Speed_real=33;
float step = 8.0;//2





//閸欐﹢鍣?
#define GD_PI       3.1415926f
#define GD_DEG2RAD  (GD_PI / 180.0f)

guandao_mode gd_mode = guandao_pass_mode;

union FloatInspector data_x,data_y,data_yaw;  //鐠佹澘缍嶈ぐ鎾冲閻?

float dis_record = 0.03;//缁夋垹娲?閸滃瞼顫栭惄?闂団偓鐟曚焦鏁奸幋?.2-20CM

int guandao_index = 0;         //鐠佹澘缍嶉悙鍦畱閻╊喖缍?
int guandao_cnt = 0;           //瑜版挸澧犲顏囨姉閻?
int guandao_last_cnt = 0;
int guandao_new_cnt = 0;
int forward = 3;               //閸撳秶鐏梹鍨閿涘牆甯?閳?閿涘本褰侀崜宥堟祮閸氭埊绱濆顖炰壕閺囨潙鍣敍?
int window = 10;               //閹兼粎鍋ｇ粣妤€褰?

float guandao_lucheng = 0;          //鐠佹澘缍嶇挧棰佺啊婢舵艾鐨捄婵堫瀲
float X_Record_f[MAX] = {0};        //鐠囪褰囧妤€鍩岄惃鍓庨惃鍕殶缂?
float Y_Record_f[MAX] = {0};      //鐠囪褰囧妤€鍩岄惃鍓忛惃鍕殶缂?
float Yaw_Record_f[MAX] = {0};    //鐠囪褰囧妤€鍩岄惃鍓廰w閺佹壆绮?
uint32 X_Record_i[MAX] = {0};       //鐟曚椒绗傛导鐘垫畱x閻ㄥ嫭鏆熺紒?
uint32 Y_Record_i[MAX] = {0};       //鐟曚椒绗傛导鐘垫畱y閻ㄥ嫭鏆熺紒?
uint32 Yaw_Record_i[MAX] = {0};     //鐟曚椒绗傛导鐘垫畱yaw閺佹壆绮?

float theta0 = 0;    //theta0   鐠佹澘缍嶈ぐ鎾冲閸嬪繗鍩呯憴?   鏉堟挸鍤惄顔界垼閸婄厧浜搁懜顏囶潡
float theta1 = 0;    //theta1   flash娑擃厾娈戦崑蹇氬焻鐟?
float theta2 = 0;    //theta2   閸ф劖鐖ｇ拋锛勭暬瀵版鍤惃鍕焊閼割亣顫?
float c_error = 0;
float int_c_error = 0, int_c_max = 30, int_c_min = -30;//濡亜鎮滅拠顖氭▕缁夘垰鍨庢い鍦畱闂勬劕绠欓崐纭风礄缁夘垰鍨庨幎妤呫偙閸滃奔绻氶幎銈忕礆
//濞戝牓娅庣粙铏偓浣姑崥鎴濅焊瀹割喓鈧倹鐦俊鍌濇簠鏉堝棗婀惄瀵稿殠娑撳﹤顫愮紒鍫濅焊瀹?cm閿涘妞ょ櫢绱檚tl_kp閿涘楠囬悽鐔烘畱娣囶喗顒滈柌蹇旀箒闂勬劧绱濈粔顖氬瀻妞ら€涚窗閹镐胶鐢荤槐顖氬閿涘瞼娲块崚鐗堝Ω濡亜鎮滅拠顖氭▕閸樺鍩岄幒銉ㄧ箮0閵?
float stl_kp = 15;//8 //8.5
float stl_ki = 2000;    //2000
/*閹繆鐭剧€圭偟骞囬敍?
 * 1.鐠佹澘缍嶅В蹇庣娑擃亞鍋ｉ惃鍓廰w
 * 2.鐠佹澘缍嶅В蹇庣娑擃亞鍋ｉ惃鍓庨敍瀵夐崸鎰垼
 * 3.鐠囪褰囨稉瀣╃娑擃亞鍋ｉ惃鍓庨敍瀵夐崸鎰垼娑撳骸缍嬮崜宄归敍瀵夐崸鎰垼鏉╃偟鍤庨敍灞界繁閸戦缚娴嗛崥鎴犳窗閺嶅洤鈧?
 */

//瀹搞儱鍙块崙鑺ユ殶
float gougu(float x1,float y1,float x2,float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

/*
 * x,y閺勵垰缍嬮崜宥呮綏閺?
 * X_Record_f,Y_Record_f閺勵垵鐭惧鍕綏閺嶅洦鏆熺紒?
 * len閺勵垱鏆熺紒鍕毐鎼?
 * per_cnt閺勵垯绗傛稉鈧▎鈥冲爱闁板秶娈戠槐銏犵穿
 */
float cross_error(float x , float y , float *X_Record_f , float *Y_Record_f , int len , int pre_cnt , int *new_cnt)    //濡亜鎮滅拠顖氭▕鐠侊紕鐣?
{
    int start = pre_cnt - window; // 娴犮儰绗傚▎鈥冲爱闁板秶鍋ｆ稉杞拌厬韫囧喛绱濋崥鎴濆閸氬骸鎮囬幖?0娑擃亞鍋?
    int end = pre_cnt + window;

    if (start < 0) start = 0;               //闂勬劕鐣剧槐銏犵穿閼煎啫娲?
    if (end >= len) end = len - 1;

    int best_cnt = pre_cnt;
    float min_dist2 = INFINITY;

    for (int i = start; i <= end; ++i)// 閸︺劏绻栨稉顏勭毈缁愭褰涢崘鍛鐠烘繄顬囪ぐ鎾冲娴ｅ秶鐤嗛張鈧潻鎴犳畱鐠侯垳鍋?
    {
        float dx = x - X_Record_f[i];
        float dy = y - Y_Record_f[i];
        float d2 = dx*dx + dy*dy;
        if (d2 < min_dist2)
        {
            min_dist2 = d2;
            best_cnt = i;
        }
    }

    *new_cnt = best_cnt + forward;// 閺堚偓鏉╂垹鍋?+ 閸撳秶鐏?= 閺傛壆娈戦崣鍌濃偓鍐仯

    // 閻劌灏柊宥囧仯閻ㄥ嫬鍨忕痪鎸庢煙閸氭垼顓哥粻妤伱崥鎴ｎ嚖瀹?
    float theta = Yaw_Record_f[best_cnt] * 3.1415926f / 180.0f;
    float dx = x - X_Record_f[best_cnt];
    float dy = y - Y_Record_f[best_cnt];
    float cross_track = dy * cosf(theta) - dx * sinf(theta);

    return cross_track;
}

//鐎圭偟骞囬崙鑺ユ殶

//鐠佹澘缍嶉崸鎰垼
void guandao_record(void)
{
    if(guandao_lucheng <= dis_record)
    {
        return;
    }

    guandao_lucheng -= dis_record;

    if(gd_mode == guandao_record_mode)
    {
        X_Record_i[guandao_index] = data_x.i;
        Y_Record_i[guandao_index] = data_y.i;
        Yaw_Record_i[guandao_index] = data_yaw.i;

        guandao_index ++;
    }
}

//娑撳﹣绱堕崸鎰垼
void guandao_flash_record(void)
{
    uint16 lenh,lenx,leny,lenyaw;
    uint32 *ph,*px,*py,*pyaw;

    lenh = 1;
    lenx = MAX;
    leny = MAX;
    lenyaw = MAX;
    ph = (uint32)&guandao_index;
    px = X_Record_i;
    py = Y_Record_i;
    pyaw = Yaw_Record_i;

    flash_erase_page(0, FLASH_H_PAGE);
    flash_erase_page(0, FLASH_X_PAGE);
    flash_erase_page(0, FLASH_Y_PAGE);
    flash_erase_page(0, FLASH_YAW_PAGE);

    flash_write_page(0, FLASH_H_PAGE, ph, lenh);
    flash_write_page(0, FLASH_X_PAGE, px, lenx);
    flash_write_page(0, FLASH_Y_PAGE, py, leny);
    flash_write_page(0, FLASH_YAW_PAGE, pyaw, lenyaw);
}

//鐠囪褰囬崸鎰垼,楠炴儼娴嗛崠鏍﹁礋float娓氭稑鎯婃潻?
void guandao_flash_load(void)
{
    uint16 lenh,lenx,leny,lenyaw;
    uint32 *ph,*px,*py,*pyaw;
    int i;

    lenh = 1;
    lenx = MAX;
    leny = MAX;
    lenyaw = MAX;
    ph = (uint32)&guandao_index;
    px = X_Record_i;
    py = Y_Record_i;
    pyaw = Yaw_Record_i;

    flash_read_page(0, FLASH_H_PAGE, ph, lenh);
    flash_read_page(0, FLASH_X_PAGE, px, lenx);
    flash_read_page(0, FLASH_Y_PAGE, py, leny);
    flash_read_page(0, FLASH_YAW_PAGE, pyaw, lenyaw);

    for(i = 0; i < guandao_index; i++)
    {
        data_x.i = X_Record_i[i];
        X_Record_f[i] = data_x.f;
        data_y.i = Y_Record_i[i];
        Y_Record_f[i] = data_y.f;
        data_yaw.i = Yaw_Record_i[i];
        Yaw_Record_f[i] = data_yaw.f;


 //閹垫挸宓冨В蹇庨嚋缁便垹绱╅惃鍕殶閹?
        printf("INDEX %d: X=%f, Y=%f, YAW=%f\r\n", i, X_Record_f[i], Y_Record_f[i], Yaw_Record_f[i]);

        printf("(%f,%f)\r\n",X_Record_f[i], Y_Record_f[i]);


    }

}


float T_A=0;
void guandao_load(void)
{
    if(guandao_lucheng <= dis_record)
    {
        return;
    }

    guandao_lucheng -= dis_record;

    c_error = cross_error(data_x.f , data_y.f , X_Record_f , Y_Record_f , guandao_index , guandao_last_cnt , &guandao_new_cnt);//缁犳鍤Ο顏勬倻鐠囶垰妯婇崥灞炬閺囧瓨鏌婅ぐ鎾冲鐠烘垹娈戦悙?

    int_c_error += stl_ki * c_error * 0.001;  //0.001閸滃奔鑵戦弬顓熺梾閸忓磭閮撮弰顖欓嚋鐢悂鍣?

    if(int_c_error >= int_c_max){int_c_error = int_c_max;}
    if(int_c_error <= int_c_min){int_c_error = int_c_min;}

    /* 閸欐牕澧犻惉鑽ゅ仯閻?yaw 娴ｆ粈璐?Stanley 閼割亜鎮滈崣鍌濃偓鍐跨礄閻?guandao_new_cnt = best_cnt + 2閿?*/
    int yaw_ref_idx = guandao_new_cnt;
    if(yaw_ref_idx >= guandao_index) { yaw_ref_idx = guandao_index - 1; }

    float temp = Yaw_Record_f[yaw_ref_idx] - atanf(stl_kp * c_error / Motor_Standard_Speed_real)*180.0f / 3.1415926f - int_c_error;//temp-閻╊喗鐖ｇ憴鎺戝
//        float temp = Yaw_Record_f[yaw_ref_idx] - atanf(stl_kp * c_error / 33)*180.0f / 3.1415926f - int_c_error;//temp-閻╊喗鐖ｇ憴鎺戝

    temp = AngleErrorNormalize(temp);


//    printf("temp=%f\r\n",temp);

    float diff = AngleErrorNormalize(temp - Taget_angle);


    if(fabsf(diff) <= step)
       {
        Taget_angle = temp;
       }
       else if(diff > 0.0f)
       {
           Taget_angle = AngleErrorNormalize(Taget_angle+ step);
       }
       else
       {
           Taget_angle = AngleErrorNormalize(Taget_angle - step);
       }
//    printf("T_A=%f\r\n",T_A);
    guandao_last_cnt = guandao_new_cnt;
}

//if(guandao_new_cnt>=-INDEX)
//{
//  閻愮顕扮€瑰奔绨?
//}

//閸掓繂顫愰崠鏍у毐閺?
//閸忋劑鍎撮崚婵嗩潗閸?
void guandao_Init(void)
{
    memset(&X_Record_f, 0, sizeof(X_Record_f));
    memset(&Y_Record_f, 0, sizeof(Y_Record_f));

    memset(&Yaw_Record_f, 0, sizeof(Yaw_Record_f));
    memset(&X_Record_i, 0, sizeof(X_Record_i));

    memset(&Y_Record_i, 0, sizeof(Y_Record_i));
    memset(&Yaw_Record_i, 0, sizeof(Yaw_Record_i));

    guandao_lucheng = 0;
    guandao_index = 0;
    guandao_cnt = 0;
    guandao_last_cnt = 0;
    guandao_new_cnt = 0;

    data_x.f = 0;
    data_y.f = 0;

    gd_mode = guandao_pass_mode;    //姒涙顓绘稉铏光敄闂傝尙濮搁幀?
}

void guandao_task(void)
{
    if(gd_mode == guandao_record_mode)
    {
        guandao_record();
    }
    else if(gd_mode == guandao_load_mode)
    {
        guandao_load();
    }
}

void Ins_text(void)
{
        if(key1_flag==1)//瀵偓婵缍嶉崚?
        {
            key1_flag=0;

            Buzzer_check(50);
            gpio_set_level(LED1,0);

            /* 瑜版挸澧犵粚娲＝ -> 瀵偓婵缍嶉崚?*/
            if(gd_mode == guandao_pass_mode)
            {
                /* 瀵偓婵缍嶉崚鎯板焻閸氭垵绨崚?*/
                gd_mode = guandao_record_mode;

                Mode_chage=1;//瀵偓閸氼垵婧呮径鎾閸?
            }

        }

        if(key2_flag==1)//閸掓壆绮撻悙鐟颁粻濮濄垹缍嶉崚璺鸿嫙娣囨繂鐡?
        {
            key2_flag=0;
            Buzzer_check(100);
            gpio_set_level(LED2,0);

            if(gd_mode == guandao_record_mode)
            {
                /* 娣囨繂鐡ㄨぐ鏇炲煑缂佹挻鐏夐崚?Flash */
                   guandao_flash_record();

                   gd_mode = guandao_pass_mode;
            }

        }

        if(key3_flag==1)//婢跺秶骞囩捄顖氱窞
        {
            key3_flag=0;
            Buzzer_check(300);
            gpio_set_level(LED3,0);


            if(gd_mode == guandao_pass_mode)
            {
                /* 娴?Flash 娑擃叀顕伴崣鏍х秿婵傜晫娈戦懜顏勬倻鎼村繐鍨敍灞借嫙瀵偓婵娲栭弨?*/
                guandao_flash_load();

                /* 闁插秶鐤嗛幆顖氼嚤娴ｅ秶鐤嗛崪宀€袧閸掑棝銆嶉敍灞剧Х闂勩倕缍嶉崚鍨涘晪閸ョ偞鏂佹稊瀣？閻ㄥ嫮鐤粔顖涚磽缁?*/
                data_x.f = 0;
                data_y.f = 0;
                guandao_lucheng = 0;
                c_error = 0;
                int_c_error = 0;
                guandao_last_cnt = 0;
                guandao_new_cnt = 0;

                gd_mode = guandao_load_mode;


                Mode_chage=2;



            }




        }




}


void INS_log(void)
{
    ips_show_string(8*0, 16*0, "Distance:");        ips_show_float(8*10,16*0, guandao_lucheng,3,6);
    ips_show_string(8*0, 16*1, "YAW:");             ips_show_float(8*10,16*1, theta0,3,6);
    ips_show_string(8*0, 16*2, "X:");               ips_show_float(8*10,16*2, data_x.f ,3,6);
    ips_show_string(8*0, 16*3, "Y");                ips_show_float(8*10,16*3, data_y.f ,3,6);
    ips_show_string(8*0, 16*4, "D_YAW");            ips_show_float(8*10,16*4, data_yaw.f,3,6);
    ips_show_string(8*0, 16*5, "INDEX");            ips_show_uint(8*10,16*5, guandao_index,5);
    ips_show_string(8*0, 16*6, "T_A");              ips_show_float(8*10,16*6, Taget_angle,3,6);

}


void INS_data_get(void)
{
    // float d = Cal_Distance(g_encoder_raw);              // 缂傛牜鐖滈崳銊︾ゴ鐠烘繐绱欓弮褝绱?    float d = CYT2_get_distance_mag(motor_value.receive_left_speed_data); // 绾句胶绱ù瀣獩閿涘牊鏌婇敍?

    guandao_lucheng += d;                               //缁夘垰鍨庣捄婵堫瀲

    theta0 = roll_balance_cascade.posture_value.yaw;    //閼惧嘲褰囬崑蹇氬焻鐟?
    data_x.f += d * cosf(theta0 * 3.1415926f / 180.0f); //閸楁洑缍呯捄婵堫瀲閻ㄥ垕鏉炴潙鍨庢担宥囆?
    data_y.f += d * sinf(theta0 * 3.1415926f / 180.0f); //閸楁洑缍呯捄婵堫瀲閻ㄥ垖鏉炴潙鍨庢担宥囆?
    data_yaw.f = theta0;
}
