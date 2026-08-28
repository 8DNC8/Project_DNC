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
* 鏂囦欢鍚嶇О          main
* 鍏徃鍚嶇О          鎴愰兘閫愰绉戞妧鏈夐檺鍏徃
* 鐗堟湰淇℃伅          鏌ョ湅 libraries/doc 鏂囦欢澶瑰唴 version 鏂囦欢 鐗堟湰璇存槑
* 寮€鍙戠幆澧?         MDK 5.37
* 閫傜敤骞冲彴          MSPM0G3507
* 搴楅摵閾炬帴          https://seekfree.taobao.com/
********************************************************************************************************************/

#include "zf_common_headfile.h"
// 鎵撳紑鏂扮殑宸ョ▼鎴栬€呭伐绋嬬Щ鍔ㄤ簡浣嶇疆鍔″繀鎵ц浠ヤ笅鎿嶄綔
// 绗竴姝?鍏抽棴涓婇潰鎵€鏈夋墦寮€鐨勬枃浠?
// 绗簩姝?project->clean  绛夊緟涓嬫柟杩涘害鏉¤蛋瀹?

// *************************** 渚嬬▼鍔熻兘璇存槑 ***************************
// 1.鏈緥绋嬮渶瑕佷娇鐢ㄧ數鏈烘ā鍧椼€両PS200PRO灞忓箷妯″潡銆?60RC闄€铻轰华妯″潡銆丟S08RA妯″潡锛岄渶瑕佹牴鎹‖浠惰繛鎺ョ浉鍏虫ā鍧?
//
// 2.鏈緥绋嬩富瑕佸疄鐜?024骞寸數璧汬棰樿繘琛屽叓瀛楀惊杩圭殑浠诲姟銆?
//
// 3.鏈緥绋嬮渶瑕佷娇鐢ㄦ寜閿繘琛屽姛鑳借Е鍙戙€?
//
// 4.鎸夐敭1鐨勫姛鑳戒负锛氬垏鎹PS200PRO灞忓箷鏄剧ず椤甸潰
//
// 5.鎸夐敭2鐨勫姛鑳戒负锛氬惎鍔ㄥ皬杞?
//
// 6.鎸夐敭1鐨勫姛鑳戒负锛氶噸缃厜鐢电涓存椂璁板綍鐨勬渶澶ф渶灏忓€?
//
// 7.鎸夐敭2鐨勫姛鑳戒负锛氬啓鍏ュ厜鐢电涓存椂璁板綍鐨勬渶澶ф渶灏忓€?

// **************************** 浠ｇ爜鍖哄煙 ****************************

#define MOTOR_NUM       ( 2 )                   // 瀹氫箟鐢垫満鏁伴噺

#define MOTOR_SPEED     ( 30 )                  // 瀹氫箟鐢垫満閫熷害

#define MOTOR1_PWM_PIN  PWM_TIM_A0_CH0_A0      	// 瀹氫箟鐢垫満A閫氶亾1    
#define MOTOR1_DIR_PIN  A1    

#define MOTOR2_PWM_PIN  PWM_TIM_A0_CH2_B12       // 瀹氫箟鐢垫満B閫氶亾1    
#define MOTOR2_DIR_PIN  B13                     

#define ENCODER1_TIMER  TIM_G7                  // 瀹氫箟缂栫爜鍣ㄦ柟鍚戝紩鑴?
#define ENCODER1_LSB    TIMG7_ENCODER1_CH1_A26  // 瀹氫箟缂栫爜鍣ㄨ剦鍐插紩鑴?   
#define ENCODER1_DIR    B27                     // 瀹氫箟缂栫爜鍣ㄦ柟鍚戝紩鑴?

#define ENCODER2_TIMER  TIM_G6                  // 瀹氫箟缂栫爜鍣ㄦ柟鍚戝紩鑴?
#define ENCODER2_LSB    TIMG6_ENCODER1_CH1_B10  // 瀹氫箟缂栫爜鍣ㄨ剦鍐插紩鑴?   
#define ENCODER2_DIR    B11                     // 瀹氫箟缂栫爜鍣ㄦ柟鍚戝紩鑴?

#define	BEEP_PIN		A18						// 瀹氫箟铚傞福鍣ㄥ紩鑴?
 
#define MUX_A_PIN    	A16						// 閫夋嫨鍣ㄥ紩鑴欰
#define MUX_B_PIN    	A17						// 閫夋嫨鍣ㄥ紩鑴欱
#define MUX_C_PIN    	B17						// 閫夋嫨鍣ㄥ紩鑴欳

#define GRAY_ADC_PIN	ADC0_CH4_B25			// 鍏夌數绠￠噰闆嗗紩鑴?

#define PULSE_TO_CM     ( 0.03814 )             // 缂栫爜鍣ㄦ暟鍊艰浆鎹负鍘樼背鐨勭郴鏁?鍙牴鎹疄闄呮儏鍐靛杩欎釜鍙傛暟杩涜璋冭妭锛屽鏋滆姹傝蛋50鍘樼背锛岃蛋澶氫簡灏卞澶ц繖涓€硷紝鍙嶄箣鍑忓皬杩欎釜鍊?

#define TRACK_NUM       ( 2 )                   // 瀹氫箟杞ㄨ抗鐨勬暟閲忥紝鍙互鑷澧炲姞锛屽搴攖urn_angle涓殑鏁版嵁鍗冲彲

#define PAGE_NUM 		( 2 )					// 瀹氫箟椤甸潰鏁伴噺涓嶪D

#define TOTAL_LAP		( 3 )					// 鐩爣瀹屾垚鍦堟暟


// 姣忔杞悜鏃剁殑瑙掑害鍗曚綅涓郝?鍙宠浆涓烘锛屽乏杞负璐?
float turn_angle[TRACK_NUM] = {-50, 50.5};

float head_angle;                               // 杞﹀ご瑙掑害
float head_offset;                              // 鏈熸湜鑸悜瑙掍笌瀹為檯鑸悜瑙掔殑鍋忓樊
float head_offset_sum;                          // 鍋忓樊绉垎
int32 turn_duty;                                // 杞悜鍗犵┖姣?
uint8 model = 0;                                // 0:鐩寸嚎妯″紡 1:杞悜妯″紡
uint8 step = 0;
uint8 loss_cnt = 0;
uint8 find_cnt = 0;
uint8 car_move = 0;
uint8 lap_count = 0;        	// 宸插畬鎴愬湀鏁?

// 鍏夌數绠?鏈€澶ф渶灏忓€奸噰闆?
uint16 gray_max[8] = {0};
uint16 gray_min[8] = {4095};  // 12浣岮DC榛樿鏈€澶?
uint8  gray_update_flag = 0;

uint16 page_id[PAGE_NUM];
uint8 now_page;
uint8 gray_display_sel = 0;  // 0:榛樿鏄剧ず鍘熷ADC鍊? 1:鏄剧ず鏈€澶?鏈€灏忓€?
uint16 bar_id[8];
uint16 label_model;
uint16 label_angle;
uint16 label_head;
uint16 label_encoder;
uint16 label_gray1,label_gray2,label_gray3,label_gray4;
uint16 label_offset;


typedef struct
{
    float kp,ki,kd;     // 澧為噺寮廝ID鍙傛暟  
    float out_increment;// 澧為噺寮廝ID杈撳嚭澧為噺  
    float out;          // 杈撳嚭閲? 
                           
    int16 set_speed;    // 鏈熸湜閫熷害  
    int16 ek,ek1,ek2;   // 鍓嶅悗涓夋璇樊  
}pid_increment_struct;

pid_increment_struct pid_increment;


int16 encoder[MOTOR_NUM];       // 鐪熷疄閫熷害    
int32 motor_duty[MOTOR_NUM];    // 鐢垫満鍗犵┖姣?

uint16 lost_flag;
int16 gray_offset;	
int16 last_gray_offset;

uint8 beep_flag;				// 铚傞福鍣ㄥ搷鏍囧織浣?
int16 beep_cnt;					// 铚傞福鍣ㄨ鏁版椂闂?

uint32  systime_20ms;
uint8   state_lock_flag;		// 鐘舵€侀攣瀹氭爣蹇椾綅
uint32  state_lock_cnt;     	// 鐘舵€侀攣瀹氳鏁版椂闂?


void beep_on()
{
	gpio_high(BEEP_PIN);
	beep_flag = 1;
}

void beep_off()
{
	gpio_low(BEEP_PIN);
}

// 閲囬泦骞舵洿鏂板厜鐢电鏈€澶ф渶灏忓€?
void gray_max_min_update(void)
{
    uint8 i;
    for(i=0; i<8; i++)
    {
        if(gs08ra_raw_val[i] > gray_max[i]) gray_max[i] = gs08ra_raw_val[i];
        if(gs08ra_raw_val[i] < gray_min[i]) gray_min[i] = gs08ra_raw_val[i];
    }
}

// 閲嶇疆鏈€澶ф渶灏忓€?
void gray_max_min_reset(void)
{
    uint8 i;
    for(i=0; i<8; i++)
    {
        gray_max[i] = 0;
        gray_min[i] = 4095;
    }
}
// 灏嗛噰闆嗙殑鏈€澶ф渶灏忓€肩洿鎺ュ啓鍏?gs08ra_max_val / gs08ra_min_val
void gray_save_max_min_to_array(void)
{
    uint8 i;
    for(i=0; i<8; i++)
    {
        gs08ra_max_val[i] = gray_max[i];  // 鍐欏叆鏈€澶у€兼暟缁?
        gs08ra_min_val[i] = gray_min[i];  // 鍐欏叆鏈€灏忓€兼暟缁?
    }
    beep_on();  // 鎻愮ず淇濆瓨鎴愬姛
}

void pid_increment_calc(pid_increment_struct *data, int16 encoder)
{
    float temp_out;
    
    data->ek2   = data->ek1;                // 淇濆瓨涓婁笂娆¤宸?           
    data->ek1   = data->ek;                 // 淇濆瓨涓婃璇樊            
    data->ek    = data->set_speed - encoder;// 璁＄畻褰撳墠璇樊   
    
    // 杩涜澧為噺寮廝ID杩愮畻            
    data->out_increment = (int16)(data->kp * (data->ek - data->ek1) + data->ki * data->ek + data->kd * (data->ek - 2 * data->ek1 + data->ek2));  
    // 璁＄畻鏂扮殑杈撳嚭
    temp_out = data->out + data->out_increment;   
    // 杈撳嚭闄愬箙锛屼笉鑳借秴杩囧崰绌烘瘮鏈€澶у€?
    data->out = func_limit(temp_out, PWM_DUTY_MAX);
}

void calc_line_position(void)
{
    int8 left = -1, right = -1;
	uint8 i;
	int8 j;
    // 宸︹啋鍙?鎵惧乏杈圭紭
    for(i = 0; i < 8; i++)
    {
        if(!gs08ra_bin_val[i])
        {
            left = i;
            break;
        }
    }

    // 鍙斥啋宸?鎵惧彸杈圭紭
    for(j = 7; j >= 0; j--)
    {
        if(!gs08ra_bin_val[j])
        {
            right = j;
            break;
        }
    }

    // 涓㈢嚎淇濇寔涓婁竴娆?
    if(left == -1 || right == -1)
    {
        lost_flag = 1;
		return;
    }
	
	lost_flag = 0;
    last_gray_offset = gray_offset;
    gray_offset = (left + right) * 5 - 35;
}

void motor_control(int32 duty1, int32 duty2)
{
    duty1 = func_limit(duty1, PWM_DUTY_MAX);
    duty2 = func_limit(duty2, PWM_DUTY_MAX);
    // 鎺у埗鐢垫満1杞姩
    if(duty1 >= 0)  // 鍓嶈繘
    {                
        gpio_set_level(MOTOR1_DIR_PIN,1);                
        pwm_set_duty(MOTOR1_PWM_PIN, duty1);            
    }            
    else            // 鍚庨€€          
    {                
        gpio_set_level(MOTOR1_DIR_PIN,0);                
        pwm_set_duty(MOTOR1_PWM_PIN, -duty1);            
    }
    
    // 鎺у埗鐢垫満2杞姩
    if(duty2 >= 0)  // 鍓嶈繘
    {                
        gpio_set_level(MOTOR2_DIR_PIN,1);
        pwm_set_duty(MOTOR2_PWM_PIN, duty2);            
    }            
    else            // 鍚庨€€          
    {                
        gpio_set_level(MOTOR2_DIR_PIN,0);                
        pwm_set_duty(MOTOR2_PWM_PIN, -duty2);
    }
}

void state_lock(void)
{
    state_lock_flag = 1;
    state_lock_cnt = systime_20ms;
}

uint8 state_lock_check(void)
{
    if(state_lock_flag)
    {
        // 閿佸畾1s
        if(systime_20ms - state_lock_cnt < 50)
        {
            return 1;   // 杩樺湪閿?
        }
        else
        {
            state_lock_flag = 0;  // 瑙ｉ攣
            return 0;
        }
    }
    return 0;
}


float angle_limit(float angle, float limit_value)
{
    angle = angle >  limit_value ? angle - 360 : angle;
    angle = angle < -limit_value ? angle + 360 : angle;
    return angle;
}


void pit_callback (uint32 event, void *ptr)
{
	systime_20ms++;
    char temp[100];
    // 姝ゅ缂栧啓闇€瑕佸惊鐜墽琛岀殑浠ｇ爜
    encoder[0] =  encoder_get_count(ENCODER1_TIMER);// 閲囬泦缂栫爜鍣ㄦ暟鎹?
    encoder[1] = -encoder_get_count(ENCODER2_TIMER);// 閲囬泦缂栫爜鍣ㄦ暟鎹?
    encoder_clear_count(ENCODER1_TIMER);            // 缂栫爜鍣ㄦ暟鎹噰闆嗗畬鎴愬悗鍔″繀娓呴浂
    encoder_clear_count(ENCODER2_TIMER);            // 缂栫爜鍣ㄦ暟鎹噰闆嗗畬鎴愬悗鍔″繀娓呴浂

    // head_offset 鍙宠浆涓烘 鍙嶄箣涓鸿礋
    head_offset = head_angle - imu660rc_yaw;
    // 鍋忓樊杞崲涓?180鍒?180涔嬮棿
    head_offset = angle_limit(head_offset, 180);

    // 瀵瑰亸宸眰绉垎锛岃繘涓€姝ョ‘淇濊繍琛屾椂濮挎€佷笌鏈熸湜鍊兼洿鎺ヨ繎
    head_offset_sum += head_offset;
    head_offset_sum = func_limit(head_offset_sum, 90);
    // 鍋忓樊涓庣Н鍒嗘瀬鎬т笉鍚屾椂锛屽皢绉垎娓呯┖
    if(((0 < head_offset) && (0 > head_offset_sum)) || ((0 > head_offset) && (0 < head_offset_sum)) || (1 > func_abs(head_offset)))
    {
        head_offset_sum = 0;
    }
	
	// 铚傞福鍣ㄦ帶鍒?
	if(beep_flag)
	{
		beep_cnt++;
		// 褰撹渹楦ｅ櫒鍝?.5s鍚庯紝鍏抽棴铚傞福鍣紝閲嶇疆璁℃暟鍊?
		if(beep_cnt >= 25)  // 25*20ms = 500ms = 0.5s
		{
			beep_flag = 0;
			beep_cnt  = 0;
			beep_off();  // 鍏宠渹楦ｅ櫒
		}
	}

	if(0 == car_move) 	return;
	
    // 澧為噺寮廝ID璁＄畻 涓や釜鐢垫満浣跨敤涓€涓狿ID璁＄畻锛岃緭鍑虹敤浜庢帶鍒朵袱涓數鏈猴紝鐩告瘮杈冧簬姣忎釜鐢垫満浣跨敤鍗曠嫭鐨凱ID
    // 杩欐牱鐨勪紭鍔垮湪涓庡彲浠ョ畝鍗曠殑瀹炵幇琚姩宸€熺殑鏁堟灉
    pid_increment_calc(&pid_increment, (encoder[0] + encoder[1]) / 2);
    

	// 鏍规嵁妯″紡鍒囨崲鎵€浣跨敤鐨勫亸宸?寰抗妯″紡涓嬶紝浣跨敤鐏板害浼犳劅鍣ㄥ亸宸?鍏朵粬妯″紡浣跨敤鑸悜瑙掑亸宸?
	if(1 == model)
	{	
		turn_duty = gray_offset * 65 + (gray_offset - last_gray_offset) * 120; 
	}
	else
	{	// 璁＄畻鑸悜瑙掑亸宸紝閬垮厤灏忚溅鍑虹幇璺戝亸鐨勯棶棰?
		turn_duty = head_offset * 100 + head_offset_sum * 6;
		turn_duty = func_limit(turn_duty, 2000);
	}

	
    if(2 == model)
    {   // 瑙掑害妯″紡閫熷害鐜緭鍑鸿缃负0
        pid_increment.out = 0;
    }
	// imu660rc_gyro_z / 4鐨勭洰鐨勬槸澧炲姞杞悜鐨勯樆灏硷紝浣垮緱灏忚溅杞悜鏇村姞骞崇ǔ
    motor_duty[0] = (int32)(pid_increment.out + turn_duty - imu660rc_gyro_z / 4);
    motor_duty[1] = (int32)(pid_increment.out - turn_duty + imu660rc_gyro_z / 4);
    
    motor_control(motor_duty[0], motor_duty[1]);
	

}



int main (void)
{
    clock_init(SYSTEM_CLOCK_80M);   // 鏃堕挓閰嶇疆鍙婄郴缁熷垵濮嬪寲<鍔″繀淇濈暀>
    debug_init();					// 璋冭瘯涓插彛淇℃伅鍒濆鍖?
	// 姝ゅ缂栧啓鐢ㄦ埛浠ｇ爜 渚嬪澶栬鍒濆鍖栦唬鐮佺瓑
	// IMU660RC鍒濆鍖?
    imu660rc_init(IMU660RC_QUARTERNION_120HZ);
    // 鍒濆鍖朓MU660RC涔嬪悗寤鸿绛夊緟3绉掞紝鏈熼棿淇濇寔闈欐锛屼互渚夸簬妯″潡鍐呴儴鑷姩鏍″噯闆跺亸
    system_delay_ms(3000);
	
	// 鍏夌數绠″垵濮嬪寲
	gs08ra_init();
	
	// 鎸夐敭鍒濆鍖?
	key_init(10);
	
	// 铚傞福鍣ㄥ垵濮嬪寲
	gpio_init(BEEP_PIN, GPO, 0, GPO_PUSH_PULL);
	
    // 浣跨敤DRV8701鐢垫満椹卞姩
    pwm_init(MOTOR1_PWM_PIN, 17*1000, 0);  // 鍒濆鍖朠WM棰戠巼涓?7khz   
    gpio_init(MOTOR1_DIR_PIN, GPO, 0, GPO_PUSH_PULL);   
    
    pwm_init(MOTOR2_PWM_PIN, 17*1000, 0);  // 鍒濆鍖朠WM棰戠巼涓?7khz   
    gpio_init(MOTOR2_DIR_PIN, GPO, 0, GPO_PUSH_PULL);
           
    encoder_dir_init(ENCODER1_TIMER, ENCODER1_LSB, ENCODER1_DIR);  // 鍒濆鍖栫紪鐮佸櫒1绔彛  
    encoder_dir_init(ENCODER2_TIMER, ENCODER2_LSB, ENCODER2_DIR);  // 鍒濆鍖栫紪鐮佸櫒2绔彛  
    
	
	// 姝ゅ缂栧啓鐢ㄦ埛浠ｇ爜 渚嬪澶栬鍒濆鍖栦唬鐮佺瓑
    
    // 璁剧疆PID绯绘暟            
    pid_increment.kp = 70;            
    pid_increment.ki = 30;            
    pid_increment.kd = 20;         
	// 灞忓箷澶氶〉闈㈠垵濮嬪寲
    page_id[0] = ips200pro_init("浠诲姟灏忚溅", IPS200PRO_TITLE_BOTTOM, 30);
	page_id[1] = ips200pro_page_create("鍏夌數绠?);
    ips200pro_set_direction(IPS200PRO_CROSSWISE_180);
	ips200pro_page_switch(page_id[0],PAGE_ANIM_OFF);
    // 鍒涘缓椤甸潰0 label鏍囩
	label_model     = ips200pro_label_create(0, 00, 320, 20);
    label_angle     = ips200pro_label_create(0, 20, 320, 20);
    label_head      = ips200pro_label_create(0, 40, 320, 20);
    label_encoder   = ips200pro_label_create(0, 60, 320, 20);
	// 鍒涘缓椤甸潰1 label鏍囩
	ips200pro_page_switch(page_id[1],PAGE_ANIM_OFF);
	label_gray1  = ips200pro_label_create(0, 0, 320, 20);
	label_gray2  = ips200pro_label_create(0, 20, 320, 20);
	label_gray3  = ips200pro_label_create(0, 40, 320, 20);
	label_gray4  = ips200pro_label_create(0, 60, 320, 20);
	label_offset = ips200pro_label_create(0, 80, 320, 20);
	bar_id[0] = ips200pro_progress_bar_create( 68, 100, 20, 100);
	bar_id[1] = ips200pro_progress_bar_create( 93, 100, 20, 100);
	bar_id[2] = ips200pro_progress_bar_create(118, 100, 20, 100);
	bar_id[3] = ips200pro_progress_bar_create(143, 100, 20, 100);
	bar_id[4] = ips200pro_progress_bar_create(168, 100, 20, 100);
	bar_id[5] = ips200pro_progress_bar_create(193, 100, 20, 100);
	bar_id[6] = ips200pro_progress_bar_create(218, 100, 20, 100);
	bar_id[7] = ips200pro_progress_bar_create(243, 100, 20, 100);
	uint8 i;
	for(i = 0; i < 8; i++)
    {
        ips200pro_set_color(bar_id[i], COLOR_FOREGROUND, IPS200PRO_RGB888_TO_RGB565(255,150,78));
        ips200pro_set_color(bar_id[i], COLOR_BACKGROUND, RGB565_WHITE);
    }

	// 鍒囧洖涓婚〉闈?
	ips200pro_page_switch(page_id[0],PAGE_ANIM_OFF);
	
    // 灏嗗綋鍓嶈搴﹁缃负杞﹀ご瑙掑害
    head_angle = imu660rc_yaw;

    pit_ms_init(PIT_TIM_G12, 20, pit_callback, NULL);
    interrupt_set_priority(TIMG12_INT_IRQn, 7);
    interrupt_set_priority(GPIOA_INT_IRQn, 1);
    
    while(true)
    {
		// -------------------- 浼犳劅鍣ㄥ鐞?--------------------
		gs08ra_scan_read();		// 鏁版嵁閲囬泦+澶勭悊
		calc_line_position();	// 鍋忓樊璁＄畻

		// -------------------- 灏忚溅鑷姩娴佺▼鎺у埗 --------------------
		if(car_move == 1)
		{
			if(state_lock_check())
			continue;
			
			if(0 == model)
			{   // 鐩寸嚎妯″紡
				pid_increment.set_speed = MOTOR_SPEED;   // 璁剧疆鐢垫満鏈熸湜鐨勮浆閫?
				// 鍒ゆ柇鏄惁鎼滃埌杞ㄨ抗
				if(lost_flag == 0)
				{
					find_cnt++;
					if(find_cnt > 3)
					{
						// 鍒囨崲妯″紡+閲嶇疆鍙傛暟
						model = 1;
						head_offset_sum   = 0;
						pid_increment.out = 0;
						// 铚傞福鍣ㄦ彁绀?
						beep_on();
						state_lock();
					}
				}
				else
				{
					find_cnt = 0;
				}
			}
			else if(1 == model)
			{	// 寰抗妯″紡
				// 鍒ゆ柇鏄惁涓㈠け杞ㄨ抗
				if(lost_flag == 1)
				{
					loss_cnt++;
					if(loss_cnt > 3)
					{
						// 閲嶆柊璁＄畻杞﹀ご瑙掑害
						head_angle = imu660rc_yaw + turn_angle[step];
						// 瑙掑害鑼冨洿闄愬埗鍒?360鍒?360涔嬮棿
						head_angle = angle_limit(head_angle, 360);
						head_offset_sum = 0;
						step = (step + 1 >= TRACK_NUM) ? 0 : step + 1;
						// 浠诲姟鍒囨崲+閲嶇疆璁℃椂
						model    = 2;        // 寰抗璧板畬 鈫?鍒囪浆寮?
						loss_cnt = 0;
						// 铚傞福鍣ㄦ彁绀?
						beep_on();
						
						if(step == 0)
						{
							lap_count++;  // 瀹屾垚涓€鍦堬紝璁℃暟+1
						}
						if(lap_count >= TOTAL_LAP) // 杈惧埌TOTAL_LAP鍦?
						{
							car_move = 0;        // 鍏抽棴灏忚溅杩愬姩浣胯兘
							motor_control(0, 0); // 绔嬪嵆鍋滅數鏈?
						}
					}
				}
			}
			else if(2 == model)
			{   // 瑙掑害妯″紡
				pid_increment.set_speed = 0;    // 璁剧疆鐢垫満鏈熸湜鐨勮浆閫?
				if(2 >= func_abs(head_offset))  // 杩欓噷杞悜瀹屾垚涔嬪悗锛岃櫧鐒惰繕鏈変竴瀹氱殑璇樊锛屼絾鏄宸細鍦ㄧ洿绾胯椹朵腑绾犳
				{
					model = 0;
				}
				else
				{
					// 閬垮厤鍋忓樊杩囧皬瀵艰嚧鐢垫満鏃犳硶杞姩
					if((head_offset < 5) && (head_offset > -5))
					{
						head_offset = head_offset / func_abs(head_offset) * 5;
					}
				}
			}
		}
		
		// -------------------- 鎸夐敭澶勭悊 --------------------
		key_scanner();
		
		 // 鎸夐敭1锛氬垏鎹㈤〉闈?
		if(key_get_state(KEY_1) == KEY_SHORT_PRESS)
		{
			now_page++;
			if(now_page >= PAGE_NUM) now_page = 0;
			ips200pro_page_switch(page_id[now_page], PAGE_ANIM_ON);
		}
		
		// 鎸夐敭2锛氬惎鍔ㄥ皬杞?
		if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
		{
			car_move = 1;
			head_angle = imu660rc_yaw;
		}
		
		// 鎸夐敭3锛氬垏鎹㈡樉绀?鏈€澶?鏈€灏忓€?
		if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
		{
			gray_max_min_reset();
			gray_display_sel = 1;
		}

		// 鎸夐敭4锛氬垏鎹㈠洖鏄剧ず 鍘熷ADC鍊?
		if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
		{
			gray_save_max_min_to_array();
			gray_display_sel = 0;
		}
		
		// -------------------- 鍏夌數绠℃渶澶ф渶灏忓€奸噰闆?--------------------
		gray_max_min_update();
		
		// -------------------- 灞忓箷鏁版嵁鍒锋柊 --------------------
		// 椤甸潰1鏄剧ず鍐呭
		ips200pro_label_printf(label_model   , "褰撳墠妯″紡: %d"			, model);
		ips200pro_label_printf(label_angle   , "褰撳墠杞: %.1f"         , turn_angle[step]);
		ips200pro_label_printf(label_head    , "褰撳墠瑙掑害: %.1f, %.1f"   , imu660rc_yaw  , head_angle);
		ips200pro_label_printf(label_encoder , "褰撳墠杞€? L:%d, R:%d"   , encoder[0]    , encoder[1]);

		// 椤甸潰2鏄剧ず鍐呭
		if(gray_display_sel == 0)
		{
			// 榛樿锛氭樉绀哄師濮?璺疉DC鍊?
			ips200pro_label_printf(label_gray1 , "RAW1-4: %d %d %d %d", gs08ra_raw_val[0],gs08ra_raw_val[1],gs08ra_raw_val[2],gs08ra_raw_val[3]);
			ips200pro_label_printf(label_gray2 , "RAW5-8: %d %d %d %d", gs08ra_raw_val[4],gs08ra_raw_val[5],gs08ra_raw_val[6],gs08ra_raw_val[7]);
			ips200pro_label_printf(label_gray3 , "       ");
			ips200pro_label_printf(label_gray4 , "       ");
		}
		else
		{
			// 鎸変笅KEY3锛氭樉绀烘渶澶с€佹渶灏忓€?
			ips200pro_label_printf(label_gray1 , "MAX1-4: %d %d %d %d", gray_max[0],gray_max[1],gray_max[2],gray_max[3]);
			ips200pro_label_printf(label_gray2 , "MAX5-8: %d %d %d %d", gray_max[4],gray_max[5],gray_max[6],gray_max[7]);
			ips200pro_label_printf(label_gray3 , "MIN1-4: %d %d %d %d", gray_min[0],gray_min[1],gray_min[2],gray_min[3]);
			ips200pro_label_printf(label_gray4 , "MIN5-8: %d %d %d %d", gray_min[4],gray_min[5],gray_min[6],gray_min[7]);
		}
		ips200pro_label_printf(label_offset, "鍋忓樊: %d", gray_offset);    	
		
		for(i = 0; i < 8; i++)
        {
            // 8浣嶆暟鍊?~255 鏄犲皠杩涘害鏉?~100
            uint8 bar_val = (gs08ra_raw_val[i] * 100) / 255;
            ips200pro_progress_bar_set_value(bar_id[i], 0, bar_val);
        }

        // 姝ゅ缂栧啓闇€瑕佸惊鐜墽琛岀殑浠ｇ爜
    }
}

