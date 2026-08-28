/*
 * Imu_init.h
 *
 *  Created on: 2026楠?閺?8閺?
 *      Author: Super_burger
 */

#ifndef CODE_IMU_INIT_H_
#define CODE_IMU_INIT_H_





// 闂勨偓閾昏桨鍗庨弫鐗堝祦鏉烆剚宕查敍鍫濆斧婵鏆熼幑顔挎祮閹诡澁绱?
//#define GYRO_DATA_X              (-imu963ra_gyro_y)        // 闂勨偓閾昏桨鍗?X 鏉炴潙甯慨瀣殶閹?
//#define GYRO_DATA_Y              (imu963ra_gyro_x)        // 闂勨偓閾昏桨鍗?Y 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define GYRO_DATA_Z              (imu963ra_gyro_z)        // 闂勨偓閾昏桨鍗?Z 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define GYRO_TRANSITION_FACTOR   (14.3)                 // 闂勨偓閾昏桨鍗庨弫鐗堝祦鏉烆剚宕茬化缁樻殶閿涘湢SB/(鎺?s)閿?
//
//// 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喛娴嗛幑?
//#define ACC_DATA_X               (-imu963ra_acc_y)         // 閸旂娀鈧喎瀹崇拋?X 鏉炴潙甯慨瀣殶閹?
//#define ACC_DATA_Y               (imu963ra_acc_x)         // 閸旂娀鈧喎瀹崇拋?Y 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define ACC_DATA_Z               (imu963ra_acc_z)         // 閸旂娀鈧喎瀹崇拋?Z 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define ACC_TRANSITION_FACTOR    (4098.0f)                 // 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喛娴嗛幑銏㈤兇閺佸府绱橪SB/g閿?


#define GYRO_DATA_X              (-imu660rb_gyro_y)        // 闂勨偓閾昏桨鍗?X 鏉炴潙甯慨瀣殶閹?
#define GYRO_DATA_Y              (imu660rb_gyro_x)        // 闂勨偓閾昏桨鍗?Y 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
#define GYRO_DATA_Z              (imu660rb_gyro_z)        // 闂勨偓閾昏桨鍗?Z 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
#define GYRO_TRANSITION_FACTOR   (14.3f)                 // 闂勨偓閾昏桨鍗庨弫鐗堝祦鏉烆剚宕茬化缁樻殶閿涘湢SB/(鎺?s)閿?

// 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喛娴嗛幑?
#define ACC_DATA_X               (-imu660rb_acc_y)         // 閸旂娀鈧喎瀹崇拋?X 鏉炴潙甯慨瀣殶閹?
#define ACC_DATA_Y               (imu660rb_acc_x)         // 閸旂娀鈧喎瀹崇拋?Y 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
#define ACC_DATA_Z               (imu660rb_acc_z)         // 閸旂娀鈧喎瀹崇拋?Z 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
#define ACC_TRANSITION_FACTOR    (4098.0f)                 // 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喛娴嗛幑銏㈤兇閺佸府绱橪SB/g閿?

//#define GYRO_DATA_X              (imu660ra_gyro_x)        // 闂勨偓閾昏桨鍗?X 鏉炴潙甯慨瀣殶閹?
//#define GYRO_DATA_Y              (imu660ra_gyro_y)        // 闂勨偓閾昏桨鍗?Y 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define GYRO_DATA_Z              (imu660ra_gyro_z)        // 闂勨偓閾昏桨鍗?Z 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define GYRO_TRANSITION_FACTOR   (16.4f)                 // 闂勨偓閾昏桨鍗庨弫鐗堝祦鏉烆剚宕茬化缁樻殶閿涘湢SB/(鎺?s)閿?
//
//// 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喛娴嗛幑?
//#define ACC_DATA_X               (imu660ra_acc_x)         // 閸旂娀鈧喎瀹崇拋?X 鏉炴潙甯慨瀣殶閹?
//#define ACC_DATA_Y               (imu660ra_acc_y)         // 閸旂娀鈧喎瀹崇拋?Y 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define ACC_DATA_Z               (imu660ra_acc_z)         // 閸旂娀鈧喎瀹崇拋?Z 鏉炴潙甯慨瀣殶閹诡噯绱欑敮锔芥煙閸氭垼娴嗛幑顫礆
//#define ACC_TRANSITION_FACTOR    (4096.0f)                 // 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喛娴嗛幑銏㈤兇閺佸府绱橪SB/g閿?
////
//#define ACC_GRAVITY               (9.80665f)               // 闁插秴濮忛崝鐘烩偓鐔峰閸欏倽鈧啫鈧》绱檓/s2閿?

typedef struct quaternion_data
{
    float rot_mat[3][3];                                  // 閺冨娴嗛惌鈺呮█
} quaternion_data;

typedef struct quaternion_process
{
    float qua[4];                                         // 閸ユ稑鍘撻弫鐗堟殶閹诡噯绱檞, x, y, z 妞ゅ搫绨敍?
    float acc_filtered[3];                                // 濠娿倖灏濋崥搴ｆ畱閸旂娀鈧喎瀹抽敍鍫濆礋娴ｅ稄绱癵閿?
} quaternion_process;

typedef struct quaternion_parameter
{
    float acc_err[3];                                    // 閸旂娀鈧喎瀹崇拋鈩冪墡閸戝棜顕ゅ顔尖偓?
} quaternion_parameter;

typedef struct quaternion_module
{
    quaternion_process pro;                              // 閸ユ稑鍘撻弫鏉款槱閻炲棙鏆熼幑?
    quaternion_data data;                                // 閸ユ稑鍘撻弫鎷岊吀缁犳绮ㄩ弸婊勬殶閹?
    quaternion_parameter parameter;                      // 閸ユ稑鍘撻弫鐗堢墡閸戝棗寮弫?
} quaternion_module;

typedef struct
{
    float p;                                             // PID 閹貉冨煑閸ｃ劍鐦笟瀣€?P
    float i;                                             // PID 閹貉冨煑閸ｃ劎袧閸掑棝銆?I
    float d;                                             // PID 閹貉冨煑閸ｃ劌浜曢崚鍡涖€?D
    float p_value_last;                                  // 娑撳﹣绔村▎鈥充焊瀹割喖鈧?
    float i_value;                                       // PID 缁夘垰鍨庨崐?
    float i_value_pro;                                   // PID 缁夘垰鍨庨崐鑲╂畱濮ｆ柧绶ラ敍鍫ｅ瘱閸?0 - 1閿涘瞼鏁ゆ禍搴ㄦ閸掑墎袧閸掑棗顤冮梹鍧椻偓鐔峰閿?
    float i_value_max;                                   // PID 缁夘垰鍨庨崐闂寸瑐闂?
    float out;                                           // PID 閹貉冨煑閸ｃ劏绶崙鍝勨偓?
    float out_max;                                       // PID 鏉堟挸鍤崐闂寸瑐闂?
    float incremental_data[2];                           // 婢х偤鍣哄?PID 閻ㄥ嫬浜稿顔煎坊閸欏弶鏆熼幑?
} pid_cycle_struct;

typedef struct
{
    float correct_kp;                                    // 婵寧鈧焦鐗庨崙鍡樼槷娓氬閮撮弫甯礄閼煎啫娲?0.1 - 0.5閿?
    float correct_ki;                                    // 婵寧鈧焦鐗庨崙鍡櫺濋崚鍡欓兇閺佸府绱欓懠鍐ㄦ纯 0.001 - 0.01閿?
    float call_cycle;                                    // 婵寧鈧焦鎶ゅ▔銏㈢暬濞夋洜娈戠拫鍐暏閸涖劍婀￠敍鍫濆礋娴ｅ稄绱皊閿涘牏顫楅敍澶涚礆
    float mechanical_zero;                               // 閺堢儤顫梿鍓佸仯
    float yaw;                                           // 閸嬪繗鍩呯憴鎺炵礄閸楁洑缍呴敍姘閿?
    float rol;                                           // 濡亝绮寸憴鎺炵礄閸楁洑缍呴敍姘閿?
    float pit;                                           // 娣囶垯璇濈憴鎺炵礄閸楁洑缍呴敍姘閿?
} cascade_common_value_struct;

// 缁狙嗕粓閹貉冨煑缂佹挻鐎担?
typedef struct
{
    quaternion_module          quaternion;               // 閸ユ稑鍘撻弫鎵祲閸忚櫕鏆熼幑?
    cascade_common_value_struct posture_value;          // 閸斻劍鈧礁协閹焦鏆熼幑?
    pid_cycle_struct           angular_speed_cycle;      // 鐟欐帡鈧喎瀹抽悳顖涘付閸掕泛娅?
    pid_cycle_struct           angle_cycle;              // 鐟欐帒瀹抽悳顖涘付閸掕泛娅?
    pid_cycle_struct           speed_cycle;              // 闁喎瀹抽悳顖涘付閸掕泛娅?
    pid_cycle_struct           turn_cycle;              // 闁喎瀹抽悳顖涘付閸掕泛娅?

} cascade_value_struct;

extern cascade_value_struct roll_balance_cascade;         // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸欏倹鏆熺紒鎾寸€担?
extern cascade_value_struct roll_balance_cascade_resave;  // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸欏倹鏆熺紒鎾寸€担鎾冲灥婵顦禒?
extern cascade_value_struct pitch_balance_cascade;        // 濡亝绮撮獮瀹犮€€閸欏倹鏆熺痪褑浠堢紒鎾寸€担?
extern cascade_value_struct pitch_balance_cascade_resave; // 濡亝绮撮獮瀹犮€€閸欏倹鏆熺痪褑浠堢紒鎾寸€担鎾冲灥婵顦禒?
extern float zero_set;

// 閸戣姤鏆熺粻鈧禒?   閸ユ稑鍘撻弫鐗埬侀崸妤勵吀缁犳绱欓弴瀛樻煀婵寧鈧焦鏆熼幑顕嗙礆
// 閸欏倹鏆熺拠瀛樻   cascade_value - 缁狙嗕粓閹貉冨煑缂佹挻鐎担鎾村瘹闁藉牞绱欑€涙ê鍋嶉崶娑樺帗閺佹澘寮锋慨鎸庘偓浣规殶閹诡噯绱?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   quaternion_module_calculate(&balance_cascade);
// 婢跺洦鏁炴穱鈩冧紖   鐠囥儱鍤遍弫浼粹偓姘崇箖閾诲秴鎮庨梽鈧摶杞板崕閸滃苯濮為柅鐔峰鐠佲剝鏆熼幑顕嗙礉閺囧瓨鏌婇崶娑樺帗閺佹澘寮烽弮瀣祮閻晠妯€閿涘本娓剁紒鍫ｎ吀缁犳鍤慨鎸庘偓浣筋潡閿涘牊铆濠婃俺顫楅妴浣峰垔娴犳媽顫楅妴浣镐焊閼割亣顫楅敍?
void quaternion_module_calculate(cascade_value_struct *cascade_value);

// 閸戣姤鏆熺粻鈧禒?   娴ｅ秶鐤嗗?PID 閹貉冨煑鐠侊紕鐣?
// 閸欏倹鏆熺拠瀛樻   pid_cycle - PID 閹貉冨煑閸ｃ劎绮ㄩ弸鍕秼閹稿洭鎷￠敍娉僡rget - 閻╊喗鐖ｉ崐纭风幢real - 鐎圭偤妾ù瀣櫤閸?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   pid_control(&balance_cascade.speed_cycle, 0, (left_enc + right_enc) / 2);
// 婢跺洦鏁炴穱鈩冧紖   鐠侊紕鐣绘担宥囩枂瀵?PID 閻ㄥ嫯绶崙鐚寸礉閸栧懎鎯堝В鏂剧伐閵嗕胶袧閸掑棎鈧礁浜曢崚鍡欏箚閼哄偊绱濋獮璺侯嚠缁夘垰鍨庨崪宀冪翻閸戦缚绻樼悰宀勬楠?
void pid_control (pid_cycle_struct *pid_cycle, float target, float real);

// 閸戣姤鏆熺粻鈧禒?   婢х偤鍣哄?PID 閹貉冨煑鐠侊紕鐣?
// 閸欏倹鏆熺拠瀛樻   pid_cycle - PID 閹貉冨煑閸ｃ劎绮ㄩ弸鍕秼閹稿洭鎷￠敍娉僡rget - 閻╊喗鐖ｉ崐纭风幢real - 鐎圭偤妾ù瀣櫤閸?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   pid_control_incremental(&balance_cascade.angle_cycle, 0, balance_cascade.posture_value.pit);
// 婢跺洦鏁炴穱鈩冧紖   鐠侊紕鐣绘晶鐐哄櫤瀵?PID 閻ㄥ嫯绶崙鐚寸礉闁俺绻冮崑蹇撴▕閻ㄥ嫬褰夐崠鏍櫤鐠侊紕鐣婚幒褍鍩楅柌蹇擃杻闁插骏绱濈槐顖氬閸氬簼缍旀稉楦跨翻閸戝搫鑻熼梽鎰畽
void pid_control_incremental (pid_cycle_struct *pid_cycle, float target, float real);

// 閸戣姤鏆熺粻鈧禒?   閸ユ稑鍘撻弫鐗埬侀崸妤€鍨垫慨瀣
// 閸欏倹鏆熺拠瀛樻   cascade_value - 缁狙嗕粓閹貉冨煑缂佹挻鐎担鎾村瘹闁藉牞绱欓棁鈧憰浣稿灥婵瀵查惃鍕磽閸忓啯鏆熷Ο鈥虫健閿?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   quaternion_module_init(&balance_cascade);
// 婢跺洦鏁炴穱鈩冧紖   閸掓繂顫愰崠鏍ф磽閸忓啯鏆熸稉鍝勫礋娴ｅ秴娲撻崗鍐╂殶閿涘苯协閹浇顫楁稉?0閿涘苯濮為柅鐔峰濠娿倖灏濋崚婵嗩潗閸婅壈顔曟稉鍝勭秼閸撳秴濮為柅鐔峰鐠佲剝鏆熼幑顕嗙礉閺嶁€冲櫙鐠囶垰妯婂〒鍛存祩
void quaternion_module_init (cascade_value_struct *cascade_value);

// 閸戣姤鏆熺粻鈧禒?   楠炲疇銆€缁狙嗕粓閹貉冨煑閸掓繂顫愰崠?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   balance_cascade_init();
// 婢跺洦鏁炴穱鈩冧紖   閸掓繂顫愰崠鏍ч挬鐞涒剝甯堕崚璺烘嫲鏉烆剙鎮滈獮瀹犮€€閹貉冨煑閻ㄥ嫮楠囬懕鏃傜波閺嬪嫪缍嬮崣鍌涙殶閿涘苯瀵橀幏顒€协閹焦鐗庨崙鍡欓兇閺佽埇鈧赋ID 閸欏倹鏆熺粵澶涚礉楠炴湹绻氱€涙ê鍨垫慨瀣Ц閹?
void balance_cascade_init (void);

void balance_mode_parameter(int Mode_change_flag);

void Imu_init(void);
void Imu_attitude_scan(void);
void Imu_lowpass_filter(void);
void IMU_text(void);

#endif /* CODE_IMU_INIT_H_ */
