/*
 * Imu_init.c
 *
 *  Created on: 2026楠?閺?8閺?
 *      Author: Super_burger
 */

#include "zf_common_headfile.h"
float zero_set=-4.0f;
cascade_value_struct roll_balance_cascade;          // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸欏倹鏆熺紒鎾寸€担?
cascade_value_struct roll_balance_cascade_resave;   // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸欏倹鏆熺紒鎾寸€担鎾冲灥婵顦禒?
cascade_value_struct pitch_balance_cascade;         // 濡亝绮撮獮瀹犮€€閸欏倹鏆熺痪褑浠堢紒鎾寸€担?
cascade_value_struct pitch_balance_cascade_resave;  // 濡亝绮撮獮瀹犮€€閸欏倹鏆熺痪褑浠堢紒鎾寸€担鎾冲灥婵顦禒?

// 閸戣姤鏆熺粻鈧禒?   鐠侊紕鐣诲锝呭瀼閸婅偐娈戦崣宥嗩劀閸掑浄绱欐潻鎴滄妧鐠侊紕鐣婚敍?
// 鏉╂柨娲栭崣鍌涙殶   float - 閸欏秵顒滈崚鍥潡鎼达箑鈧》绱欓崡鏇氱秴閿涙艾瀹抽敍?
// 娴ｈ法鏁ょ粈杞扮伐   float angle = arctan1(1.0f); // 鐠侊紕鐣?tan(45鎺? 閻ㄥ嫬寮藉锝呭瀼閿涘矁绻戦崶?45.0 鎼?
// 婢跺洦鏁炴穱鈩冧紖   闁插洨鏁ら崚鍡橆唽鏉╂垳鎶€閸忣剙绱＄拋锛勭暬閿涘瞼鐣濋崠鏍箥缁犳绱濋柅鍌滄暏娴滃骸顕划鎯у鐟曚焦鐪版稉宥夌彯閻ㄥ嫬婧€閺?
static float arctan1(float tan)
{
    // 閸掑棙顔岀拋锛勭暬閸欏秵顒滈崚鍥箮娴肩厧鈧》绱濋弽瑙勫祦 tan 閻ㄥ嫮绮风€电懓鈧吋妲搁崥锕€銇囨禍?1 闁瀚ㄦ稉宥呮倱閸忣剙绱?
    float angle = (func_abs(tan) > 1.0f) ? 90.0f - (func_abs(1.0f / tan)) * (45.0f - (func_abs(1.0f / tan) - 1.0f) * (14.0f + 3.83f * func_abs(1.0f / tan))) :
                                          func_abs(tan) * (45.0f - (func_abs(tan) - 1.0f) * (14.0f + 3.83f * func_abs(tan)));
    return (tan > 0) ? angle : -angle;  // 閺嶈宓?tan 缁楋箑褰跨涵顔肩暰鐟欐帒瀹冲锝堢
}

// 閸戣姤鏆熺粻鈧禒?   鐠侊紕鐣绘禍宀€娣崸鎰垼閻ㄥ嫬寮藉锝呭瀼閿涘牏琚导?atan2閿涘矁绻戦崶鐐额潡鎼达讣绱?
// 鏉╂柨娲栭崣鍌涙殶   float - 鐟欐帒瀹抽崐纭风礄閸楁洑缍呴敍姘閿涘矁瀵栭崶?-180鎺硚180鎺抽敍?
// 娴ｈ法鏁ょ粈杞扮伐   float angle = arctan2(1.0f, 1.0f); // 鐠侊紕鐣?(1,1) 閻愬湱娈戠憴鎺戝閿涘矁绻戦崶?45.0 鎼?
// 婢跺洦鏁炴穱鈩冧紖   閸╄桨绨?arctan1 鐎圭偟骞囬敍灞筋槱閻炲棗娼楅弽鍥叡閻楄鐣╅幆鍛枌閿涘澑 閹?y 娑?0 閺冭绱?
static float arctan2(float x, float y)
{
    float tan, angle;

    if (x == 0 && y == 0) return 0;    // 閸樼喓鍋ｉ悧瑙勭暕婢跺嫮鎮婇敍宀冪箲閸?0 鎼?
    if (x == 0)                         // x 娑?0 閺冭绱濈憴鎺戝娑?鍗?0 鎼?
    {
        if (y > 0) return 90;
        else return -90;
    }
    if (y == 0)                         // y 娑?0 閺冭绱濈憴鎺戝娑?0 閹?-180 鎼?
    {
        if (x > 0) return 0;
        else return -180.0f;
    }
    tan = y / x;                        // 鐠侊紕鐣诲锝呭瀼閸?
    angle = arctan1(tan);               // 鐠嬪啰鏁?arctan1 鐠侊紕鐣婚崺铏诡攨鐟欐帒瀹?

    if (x < 0 && angle > 0)             // 閺嶈宓?x 閻ㄥ嫮顑侀崣鐤殶閺佺顫楁惔锔肩礉绾喕绻氶懠鍐ㄦ纯濮濓絿鈥?
    {
        angle -= 180.0f;
    }
    else if (x < 0 && angle < 0)
    {
        angle += 180.0f;
    }
    return angle;
}

// 閸戣姤鏆熺粻鈧禒?   鐠侊紕鐣诲锝呴浮閸婅偐娈戦崣宥嗩劀瀵讣绱欐潻鎴滄妧鐠侊紕鐣婚敍?
// 鏉╂柨娲栭崣鍌涙殶   float - 閸欏秵顒滃锕侇潡鎼达箑鈧》绱欓崡鏇氱秴閿涙艾瀹抽敍?
// 娴ｈ法鏁ょ粈杞扮伐   float angle = arcsin(1.0f); // 鐠侊紕鐣?sin(90鎺? 閻ㄥ嫬寮藉锝呴浮閿涘矁绻戦崶?90.0 鎼?
// 婢跺洦鏁炴穱鈩冧紖   閸╄桨绨?arctan1 鐎圭偟骞囬敍灞藉焺閻劍浜界粵澶婄础 arcsin(x) = arctan(x / sqrt(1 - x2))
static float arcsin(float i)
{
    return arctan1(i / sqrt(1 - i * i)); // 鏉烆剚宕叉稉鍝勫冀濮濓絽鍨忕拋锛勭暬
}

// 閸戣姤鏆熺粻鈧禒?   閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喕缍嗛柅姘姢濞?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   acc_lowpass_filter(&ax, &ay, &az, &fax, &fay, &faz, 0.8f);
// 婢跺洦鏁炴穱鈩冧紖   娑撯偓闂冩湹缍嗛柅姘姢濞夘澁绱濋崗顒€绱￠敍姝爄ltered = alpha * filtered + (1 - alpha) * raw, alpha 鐡掑﹤銇囧銈嗗皾鐡掑﹤宸?
static void acc_lowpass_filter(float *raw_x, float *raw_y, float *raw_z, float *filtered_x, float *filtered_y, float *filtered_z, float alpha)
{
    *filtered_x = alpha * *filtered_x + (1 - alpha) * *raw_x;  // X 鏉炲瓨鎶ゅ▔銏ｎ吀缁?
    *filtered_y = alpha * *filtered_y + (1 - alpha) * *raw_y;  // Y 鏉炲瓨鎶ゅ▔銏ｎ吀缁?
    *filtered_z = alpha * *filtered_z + (1 - alpha) * *raw_z;  // Z 鏉炲瓨鎶ゅ▔銏ｎ吀缁?
}

// 閸戣姤鏆熺粻鈧禒?   閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喖缍婃稉鈧崠鏍电礄閸楁洑缍呴崠鏍电礆
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   acc_normalize(&ax, &ay, &az);
// 婢跺洦鏁炴穱鈩冧紖   鐏忓棗濮為柅鐔峰閸氭垿鍣鸿ぐ鎺嶇閸栨牔璐熼崡鏇氱秴閸氭垿鍣洪敍灞界秼濡繝鏆辨潻鍥х毈閺冭绱?0.1閿涘绱濇妯款吇鐠佸彞璐?(0,0,1)
static void acc_normalize(float *ax, float *ay, float *az)
{
    float norm = sqrt(*ax * *ax + *ay * *ay + *az * *az);    // 鐠侊紕鐣婚崝鐘烩偓鐔峰閸氭垿鍣哄Ο锟犳毐
    if (norm < 0.1f)                                        // 濡繝鏆辨潻鍥х毈閿涘矁顫嬫稉鐑樻￥閺佸牊鏆熼幑顕嗙礉鐠у绮拋銈呪偓?
    {
        *ax = 0.0f;
        *ay = 0.0f;
        *az = 1.0f;
    }
    else                                                    // 濡繝鏆遍張澶嬫櫏閿涘苯缍婃稉鈧崠?
    {
        *ax /= norm;
        *ay /= norm;
        *az /= norm;
    }
}

// 閸戣姤鏆熺粻鈧禒?   閸掋倖鏌囩拋鎯ь槵閺勵垰鎯佹径鍕艾闂堟瑦顒涢悩鑸碘偓?
// 鏉╂柨娲栭崣鍌涙殶   bool - 闂堟瑦顒涢悩鑸碘偓浣界箲閸?true閿涘矁绻嶉崝銊уЦ閹浇绻戦崶?false
// 娴ｈ法鏁ょ粈杞扮伐   bool static_flag = is_static_state(ax_g, ay_g, az_g);
// 婢跺洦鏁炴穱鈩冧紖   闁俺绻冮崝鐘烩偓鐔峰濡繝鏆遍崚銈嗘焽閿涘牓娼ゅ銏℃閸旂娀鈧喎瀹抽幒銉ㄧ箮 1g閿涘绱濆Ο锟犳毐閸?0.9~1.1g 閼煎啫娲块崘鍛邦潒娑撴椽娼ゅ?
static bool is_static_state(float ax_g, float ay_g, float az_g)
{
    float norm = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);  // 鐠侊紕鐣婚崝鐘烩偓鐔峰濡繝鏆遍敍鍧?娑撳搫宕熸担宥忕礆
    return (norm >= 0.9f && norm <= 1.1f) ? true : false;       // 閸掋倖鏌囬弰顖氭儊閸︺劑娼ゅ銏ｅ瘱閸ユ潙鍞?
}

// 閸戣姤鏆熺粻鈧禒?   閸ユ稑鍘撻弫鏉垮挤婵寧鈧浇顫楃拋锛勭暬
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   quaternion_module_calculate(&quaternion, 0.001f); // 娴?1 ms 閸涖劍婀＄拫鍐暏
// 婢跺洦鏁炴穱鈩冧紖   閾诲秴鎮庨梽鈧摶杞板崕閸滃苯濮為柅鐔峰鐠佲剝鏆熼幑顕嗙礉闁俺绻冨顖氬娑撳妾峰▔鏇熸纯閺傛澘娲撻崗鍐╂殶閿涘矁顓哥粻妤佹鏉烆剛鐓╅梼闈涙嫲婵寧鈧浇顫?
//            鏉堟挸鍙嗛崣鍌涙殶 cycle 娑撻缚鐨熼悽銊ユ噯閺堢噦绱欓崡鏇氱秴閿涙氨顫楅敍澶涚礉闂団偓缁嬪啿鐣剧拫鍐暏娴犮儰绻氱拠浣筋吀缁犳绨挎惔?
void quaternion_module_calculate(cascade_value_struct *cascade_value)
{
    static float first_count_time = 0;  // 妫ｆ牗顐肩拋锛勭暬閺冨爼妫跨拋鈩冩殶閸ｎ煉绱欓悽銊ょ艾韫囶偊鈧喐鏁归弫娑崇礆
    float length;                       // 閸ユ稑鍘撻弫鐗埬侀梹鍖＄礄閻劋绨ぐ鎺嶇閸栨牭绱?
    float x, y, z;                      // 闂勨偓閾昏桨鍗庣憴鎺椻偓鐔峰閿涘牆濮惔?缁夋帪绱?

    // 闂勨偓閾昏桨鍗庨弫鐗堝祦鏉烆剚宕查敍姘斧婵鏆熼幑?-> (鎺?s) -> 瀵冨/缁夋帪绱欓崗鍫ユ珟娴?10 閸愬秳绠?10 閸嬫氨鐣濋崡鏇熸姢濞夘澁绱?
    x = (float)(GYRO_DATA_X / 10 * 10) / GYRO_TRANSITION_FACTOR * 0.01745329f;  // 0.01745329 娑撳搫瀹虫潪顒€濮惔锔鹃兇閺佸府绱欒熀/180閿?
    y = (float)(GYRO_DATA_Y / 10 * 10) / GYRO_TRANSITION_FACTOR * 0.01745329f;
    z = (float)(GYRO_DATA_Z / 10 * 10) / GYRO_TRANSITION_FACTOR * 0.01745329f;

    // 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喛娴嗛幑顫窗閸樼喎顫愰弫鐗堝祦 -> g 娑撳搫宕熸担宥忕礄1g 閳?9.8 m/s2閿?
    float ax_g = (float)ACC_DATA_X / ACC_TRANSITION_FACTOR;
    float ay_g = (float)ACC_DATA_Y / ACC_TRANSITION_FACTOR;
    float az_g = (float)ACC_DATA_Z / ACC_TRANSITION_FACTOR;

    bool static_state = is_static_state(ax_g, ay_g, az_g);  // 閸掋倖鏌囩拋鎯ь槵閺勵垰鎯侀棃娆愵剾
    float acc_alpha = static_state ? 0.8f : 0.5f;           // 闂堟瑦顒涢弮鑸垫姢濞夈垻閮撮弫鐗堟纯婢堆嶇礄楠炶櫕绮﹂弫鍫熺亯閺囨潙銈介敍?

    // 閸旂娀鈧喎瀹崇拋鈩冩殶閹诡喕缍嗛柅姘姢濞?
    acc_lowpass_filter(&ax_g, &ay_g, &az_g,
                       &cascade_value->quaternion.pro.acc_filtered[0],
                       &cascade_value->quaternion.pro.acc_filtered[1],
                       &cascade_value->quaternion.pro.acc_filtered[2], acc_alpha);

    // 閸欐牗鎶ゅ▔銏犳倵閻ㄥ嫬濮為柅鐔峰閺佺増宓?
    float ax = cascade_value->quaternion.pro.acc_filtered[0];
    float ay = cascade_value->quaternion.pro.acc_filtered[1];
    float az = cascade_value->quaternion.pro.acc_filtered[2];

    acc_normalize(&ax, &ay, &az);  // 閸旂娀鈧喎瀹抽弫鐗堝祦瑜版帊绔撮崠?

    // 閸欐牕鍤ぐ鎾冲閸ユ稑鍘撻弫?(w, x, y, z)
    float q0 = cascade_value->quaternion.pro.qua[0];
    float q1 = cascade_value->quaternion.pro.qua[1];
    float q2 = cascade_value->quaternion.pro.qua[2];
    float q3 = cascade_value->quaternion.pro.qua[3];

    // 閺嶈宓佽ぐ鎾冲閸ユ稑鍘撻弫鎷岊吀缁犳鍣搁崝娑樻倻闁插繐婀張杞扮秼閸ф劖鐖ｇ化璁宠厬閻ㄥ嫭濮囪ぐ鎲嬬礄閻劋绨稉搴″闁喎瀹崇拋鈩冩殶閹诡喖顕В鏃撶礆
    float gx = 2 * (q1 * q3 - q0 * q2);
    float gy = 2 * (q0 * q1 + q2 * q3);
    float gz = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

    // 鐠侊紕鐣婚崝鐘烩偓鐔峰鐠佲€茬瑢闁插秴濮忛幎鏇炲閻ㄥ嫯顕ゅ顕嗙礄濮婎垰瀹崇拠顖氭▕閿?
    float ex = ay * gz - az * gy;
    float ey = az * gx - ax * gz;
    float ez = ax * gy - ay * gx;

    // 閺嶈宓侀棃娆愵剾閻樿埖鈧浇鐨熼弫瀛樼墡濮濓絿閮撮弫甯礄鏉╂劕濮╅弮璺哄櫤瀵鲸鐗庡锝忕礉闁灝鍘ゅ鏇炲弳閸ｎ亜锛愰敍濉奍娑撳秷鈥滈崙蹇ョ礉娣囨繆鐦夌粔顖氬瀻閺嶁剝顒滈崝娑樺
    float kp = static_state ? cascade_value->posture_value.correct_kp : cascade_value->posture_value.correct_kp * 0.8f;
    float ki = cascade_value->posture_value.correct_ki;

    // 妫ｆ牗顐肩拋锛勭暬閻ㄥ嫬澧?0.1 缁夋帪绱濇担璺ㄦ暏婢堆勭槷娓氬閮撮弫鏉垮闁喐鏁归弫?
//    if(first_count_time < 0.1f)
//    {
//        first_count_time += cascade_value->posture_value.call_cycle;  // 缁鳖垰濮為弮鍫曟？
//        kp = 10.0f;  // 瀵儤鐦笟瀣墡濮濓綇绱濊箛顐︹偓鐔告暪閺?
//    }

    if(first_count_time < 0.1f)
    {
        first_count_time += cascade_value->posture_value.call_cycle;  // 缁鳖垰濮為弮鍫曟？
        kp = 100.0f;  // 瀵儤鐦笟瀣墡濮濓綇绱濊箛顐︹偓鐔告暪閺?
    }

    // 閸斻劍鈧焦妞傜粔顖氬瀻閸旀稑瀹抽梽宥勮礋1/10
    float integral_gain = static_state ? 1.0f : 0.1f;

    cascade_value->quaternion.parameter.acc_err[0] += (ex * cascade_value->posture_value.call_cycle) * integral_gain;
    cascade_value->quaternion.parameter.acc_err[1] += (ey * cascade_value->posture_value.call_cycle) * integral_gain;
    cascade_value->quaternion.parameter.acc_err[2] += (ez * cascade_value->posture_value.call_cycle) * integral_gain;

    // 缁夘垰鍨庨梽鎰畽閿涙岸妲诲銏ゃ偙閸?
    float acc_err_limit = 1.0f;
    cascade_value->quaternion.parameter.acc_err[0] = (cascade_value->quaternion.parameter.acc_err[0] > acc_err_limit) ? acc_err_limit :
                                                      (cascade_value->quaternion.parameter.acc_err[0] < -acc_err_limit) ? -acc_err_limit :
                                                      cascade_value->quaternion.parameter.acc_err[0];
    cascade_value->quaternion.parameter.acc_err[1] = (cascade_value->quaternion.parameter.acc_err[1] > acc_err_limit) ? acc_err_limit :
                                                      (cascade_value->quaternion.parameter.acc_err[1] < -acc_err_limit) ? -acc_err_limit :
                                                      cascade_value->quaternion.parameter.acc_err[1];
    cascade_value->quaternion.parameter.acc_err[2] = (cascade_value->quaternion.parameter.acc_err[2] > acc_err_limit) ? acc_err_limit :
                                                      (cascade_value->quaternion.parameter.acc_err[2] < -acc_err_limit) ? -acc_err_limit :
                                                      cascade_value->quaternion.parameter.acc_err[2];

    // 閻劏顕ゅ顔界墡濮濓綁妾ч摶杞板崕閺佺増宓侀敍鍫熺槷娓?缁夘垰鍨庨弽鈩冾劀閿?
    x += kp * ex + ki * cascade_value->quaternion.parameter.acc_err[0];
    y += kp * ey + ki * cascade_value->quaternion.parameter.acc_err[1];
    z += kp * ez + ki * cascade_value->quaternion.parameter.acc_err[2];

    // 閸ユ稑鍘撻弫鏉夸簳閸掑棙鏌熺粙瀣纯閺傚府绱欓弽瑙勫祦闂勨偓閾昏桨鍗庣憴鎺椻偓鐔峰閿?
    cascade_value->quaternion.pro.qua[0] += ((-q1 * x - q2 * y - q3 * z) * cascade_value->posture_value.call_cycle / 2.0f);
    cascade_value->quaternion.pro.qua[1] += (( q0 * x + q2 * z - q3 * y) * cascade_value->posture_value.call_cycle / 2.0f);
    cascade_value->quaternion.pro.qua[2] += (( q0 * y - q1 * z + q3 * x) * cascade_value->posture_value.call_cycle / 2.0f);
    cascade_value->quaternion.pro.qua[3] += (( q0 * z + q1 * y - q2 * x) * cascade_value->posture_value.call_cycle / 2.0f);

    // 閸欐牕鍤ぐ鎾冲閸ユ稑鍘撻弫?(w, x, y, z)
    q0 = cascade_value->quaternion.pro.qua[0];
    q1 = cascade_value->quaternion.pro.qua[1];
    q2 = cascade_value->quaternion.pro.qua[2];
    q3 = cascade_value->quaternion.pro.qua[3];

    // 鐠侊紕鐣婚崶娑樺帗閺佹澘鎮囬崚鍡涘櫤楠炶櫕鏌熼敍鍫濆櫤鐏忔垿鍣告径宥堫吀缁犳绱?
    float q0_2 = q0 * q0;
    float q1_2 = q1 * q1;
    float q2_2 = q2 * q2;
    float q3_2 = q3 * q3;

    // 閸ユ稑鍘撻弫鏉跨秺娑撯偓閸栨牭绱欓柆鍨帳閺佹澘鈧吋绱撶粔浼欑礆
    length = sqrt(q0_2 + q1_2 + q2_2 + q3_2);  // 鐠侊紕鐣诲Ο锟犳毐
    if (length > 0.001f)  // 濡繝鏆遍張澶嬫櫏閺冭埖澧犺ぐ鎺嶇閸?
    {
        cascade_value->quaternion.pro.qua[0] /= length;
        cascade_value->quaternion.pro.qua[1] /= length;
        cascade_value->quaternion.pro.qua[2] /= length;
        cascade_value->quaternion.pro.qua[3] /= length;
    }

    // 閺嶈宓侀崶娑樺帗閺佹媽顓哥粻妤佹鏉烆剛鐓╅梼纰夌礄閻劋绨崥搴ｇ敾婵寧鈧浇顫楃拋锛勭暬閿?
    cascade_value->quaternion.data.rot_mat[0][0] = q0_2 + q1_2 - q2_2 - q3_2;
    cascade_value->quaternion.data.rot_mat[0][1] = 2 * (q1 * q2 + q0 * q3);
    cascade_value->quaternion.data.rot_mat[0][2] = 2 * (q1 * q3 - q0 * q2);
    cascade_value->quaternion.data.rot_mat[1][0] = 2 * (q1 * q2 - q0 * q3);
    cascade_value->quaternion.data.rot_mat[1][1] = q0_2 - q1_2 + q2_2 - q3_2;
    cascade_value->quaternion.data.rot_mat[1][2] = 2 * (q2 * q3 + q0 * q1);
    cascade_value->quaternion.data.rot_mat[2][0] = 2 * (q1 * q3 + q0 * q2);
    cascade_value->quaternion.data.rot_mat[2][1] = 2 * (q2 * q3 - q0 * q1);
    cascade_value->quaternion.data.rot_mat[2][2] = q0_2 - q1_2 - q2_2 + q3_2;

//    // 閺嶈宓侀弮瀣祮閻晠妯€鐠侊紕鐣绘慨鎸庘偓浣筋潡閿涘牊铆濠婃俺顫楅妴浣峰垔娴犳媽顫楅妴浣镐焊閼割亣顫楅敍?

//imu660rb
    cascade_value->posture_value.rol = arctan2(cascade_value->quaternion.data.rot_mat[2][2], cascade_value->quaternion.data.rot_mat[1][2]);  // 濡亝绮寸憴?
    cascade_value->posture_value.pit = -arcsin(cascade_value->quaternion.data.rot_mat[0][2]);                                            // 娣囶垯璇濈憴?
    cascade_value->posture_value.yaw = arctan2(cascade_value->quaternion.data.rot_mat[0][0], cascade_value->quaternion.data.rot_mat[0][1]);   // 閸嬪繗鍩呯憴?

////imu660ra
//       cascade_value->posture_value.rol = -arctan2(cascade_value->quaternion.data.rot_mat[2][2], cascade_value->quaternion.data.rot_mat[1][2]);  // 濡亝绮寸憴?
//       cascade_value->posture_value.pit = -arcsin(cascade_value->quaternion.data.rot_mat[0][2]);                                            // 娣囶垯璇濈憴?
//       cascade_value->posture_value.yaw = arctan2(cascade_value->quaternion.data.rot_mat[0][0], cascade_value->quaternion.data.rot_mat[0][1]);   // 閸嬪繗鍩呯憴?
}

// 閸戣姤鏆熺粻鈧禒?   PID闂傤厾骞嗙拋锛勭暬
// 閸欏倹鏆熺拠瀛樻   pid_cycle        PID閸欏倹鏆熺紒鎾寸€担?
// 閸欏倹鏆熺拠瀛樻   target           閻╊喗鐖ｉ崐?
// 閸欏倹鏆熺拠瀛樻   real             瑜版挸澧犻崐?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   pid_control(&roll_balance_cascade.speed_cycle, 0, (left_motor.encoder_data + right_motor.encoder_data) / 2);
// 婢跺洦鏁炴穱鈩冧紖
void pid_control (pid_cycle_struct *pid_cycle, float target, float real)
{
    float    proportion_value    = 0;          // 濮ｆ柧绶ラ柌?
    float    differential_value  = 0;          // 瀵邦喖鍨庨柌?

    proportion_value = target - real;          // 濮ｆ柧绶ラ柌?= 閻╊喗鐖ｉ崐?- 鐎圭偤妾崐?



    pid_cycle->i_value += (proportion_value * pid_cycle->i_value_pro);  // 缁夘垰鍨庨柌?= 缁夘垰鍨庨柌?+ 濮ｆ柧绶ラ柌?* 缁夘垰鍨庣粙瀣

    pid_cycle->i_value = func_limit_ab(pid_cycle->i_value, -pid_cycle->i_value_max, pid_cycle->i_value_max);  // 缁夘垰鍨庨柌蹇涙楠?

    differential_value = proportion_value - pid_cycle->p_value_last;  // 瀵邦喖鍨庨柌?= 濮ｆ柧绶ラ柌?- 娑撳﹣绔村▎鈩冪槷娓氬鍣?

    pid_cycle->out = (pid_cycle->p * proportion_value + pid_cycle->i * pid_cycle->i_value + pid_cycle->d * differential_value);  // PID閹风喎鎮?

    pid_cycle->out = func_limit_ab(pid_cycle->out, -pid_cycle->out_max, pid_cycle->out_max);  // PID鏉堟挸鍤梽鎰畽

    pid_cycle->p_value_last = proportion_value;        // 娣囨繂鐡ㄥВ鏂剧伐闁?
}

// 閸戣姤鏆熺粻鈧禒?   PID闂傤厾骞嗙拋锛勭暬(婢х偤鍣哄?
// 閸欏倹鏆熺拠瀛樻   pid_cycle        PID閸欏倹鏆熺紒鎾寸€担?
// 閸欏倹鏆熺拠瀛樻   target           閻╊喗鐖ｉ崐?
// 閸欏倹鏆熺拠瀛樻   real             瑜版挸澧犻崐?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   pid_control_incremental(&roll_balance_cascade.speed_cycle, 0, (left_motor.encoder_data + right_motor.encoder_data) / 2);
// 婢跺洦鏁炴穱鈩冧紖
void pid_control_incremental (pid_cycle_struct *pid_cycle, float target, float real)
{
    float    proportion_value    = 0,          // 濮ｆ柧绶ラ柌?
             differential_value  = 0;          // 瀵邦喖鍨庨柌?

    pid_cycle->i_value = target - real;          // 缁夘垰鍨庨柌?= 閻╊喗鐖ｉ崐?- 鐎圭偤妾崐? 婢х偤鍣哄寤滻D P ---> I

    differential_value = proportion_value - 2 * pid_cycle->incremental_data[0] - pid_cycle->incremental_data[1];  // 瀵邦喖鍨庨柌?婢х偤鍣哄寤滻D I ---> D

    proportion_value  = proportion_value - pid_cycle->incremental_data[0];  // 濮ｆ柧绶ラ柌?婢х偤鍣哄寤滻D D ---> P

    pid_cycle->incremental_data[1] = pid_cycle->incremental_data[0];          // 婢х偤鍣哄寤滻D 娣囨繂鐡ㄩ崣鍌涙殶

    pid_cycle->incremental_data[0] = proportion_value;

    pid_cycle->out += (pid_cycle->p * proportion_value + pid_cycle->i * pid_cycle->i_value + pid_cycle->d * differential_value);  // PID閹风喎鎮?

    pid_cycle->out = func_limit_ab(pid_cycle->out, -pid_cycle->out_max, pid_cycle->out_max);          // PID鏉堟挸鍤梽鎰畽
}

// 閸戣姤鏆熺粻鈧禒?   閸掓繂顫愰崠鏍ф磽閸忓啯鏆熷Ο鈥虫健
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   quaternion_module_init(&quaternion);
// 婢跺洦鏁炴穱鈩冧紖   閸掓繂顫愰崠鏍ф磽閸忓啯鏆熸稉鍝勫礋娴ｅ秴娲撻崗鍐╂殶閿涘苯协閹浇顫楁稉?0閿涘苯濮為柅鐔峰濠娿倖灏濋崚婵嗩潗閸婇棿璐熻ぐ鎾冲閸旂娀鈧喎瀹抽弫鐗堝祦閿涘矂鍣哥純顔夹幀浣筋潡娴滐箑褰茬拫鍐暏濮濄倕鍤遍弫?
void quaternion_module_init(cascade_value_struct *cascade_value)
{
    // 閸掓繂顫愰崠鏍ф磽閸忓啯鏆熸稉鍝勫礋娴ｅ秴娲撻崗鍐╂殶 (w=1, x=y=z=0)
    cascade_value->quaternion.pro.qua[0] = 1.0f;
    cascade_value->quaternion.pro.qua[1] = 0.0f;
    cascade_value->quaternion.pro.qua[2] = 0.0f;
    cascade_value->quaternion.pro.qua[3] = 0.0f;

    // 閸掓繂顫愰崠鏍幀浣筋潡娑?0 鎼?
    cascade_value->posture_value.yaw = 0.0f;
    cascade_value->posture_value.rol = 0.0f;
    cascade_value->posture_value.pit = 0.0f;

    // 閸掓繂顫愰崠鏍у闁喎瀹冲銈嗗皾閸婇棿璐熻ぐ鎾冲閸旂娀鈧喎瀹抽弫鐗堝祦閿涘潛 娑撳搫宕熸担宥忕礆
    cascade_value->quaternion.pro.acc_filtered[0] = (float)ACC_DATA_X / ACC_TRANSITION_FACTOR;
    cascade_value->quaternion.pro.acc_filtered[1] = (float)ACC_DATA_Y / ACC_TRANSITION_FACTOR;
    cascade_value->quaternion.pro.acc_filtered[2] = (float)ACC_DATA_Z / ACC_TRANSITION_FACTOR;

    // 閸掓繂顫愰崠鏍嚖瀹割喚袧閸掑棔璐?0
    cascade_value->quaternion.parameter.acc_err[0] = 0.0f;
    cascade_value->quaternion.parameter.acc_err[1] = 0.0f;
    cascade_value->quaternion.parameter.acc_err[2] = 0.0f;
}

// 閸戣姤鏆熺粻鈧禒?   娑撹尙楠囬獮瀹犮€€閹貉冨煑閸掓繂顫愰崠?
// 鏉╂柨娲栭崣鍌涙殶   void
// 娴ｈ法鏁ょ粈杞扮伐   balance_cascade_init();
// 婢跺洦鏁炴穱鈩冧紖   閸掓繂顫愰崠鏍ч挬鐞涒剝甯堕崚鍓佺波閺嬪嫪缍嬮崣鍌涙殶閿涘苯瀵橀幏顒€协閹焦鐗庨崙鍡欓兇閺佽埇鈧赋ID 閸氬嫮骞嗛懞鍌氬棘閺佸府绱橮/I/D閵嗕線妾洪獮鍛搼閿?
//            楠炴湹绻氱€涙ê鍨垫慨瀣Ц閹礁鍩屾径鍥﹀敜缂佹挻鐎担鎿勭礉閺堚偓閸氬骸鍨垫慨瀣閸ユ稑鍘撻弫鐗埬侀崸?
void balance_cascade_init (void)
{
    // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戞慨鎸庘偓浣稿棘閺?
    roll_balance_cascade.posture_value.call_cycle        = 0.001;      // 鐠嬪啰鏁ら崨銊︽埂 0.001s (1ms)
    roll_balance_cascade.posture_value.mechanical_zero  = zero_set;      // 閺堢儤顫梿鍓佸仯閸掓繂顫愰崠鏍﹁礋 0
    roll_balance_cascade.posture_value.correct_kp        = 0.4f;       // 婵寧鈧焦鐗庨崙鍡樼槷娓氬閮撮弫?0.4
    roll_balance_cascade.posture_value.correct_ki        = 0.015f;     // 婵寧鈧焦鐗庨崙鍡櫺濋崚鍡欓兇閺?0.015

    // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戠憴鎺椻偓鐔峰閻?PID 闂勬劕绠欓崣鍌涙殶
//    roll_balance_cascade.angular_speed_cycle.i_value_max     = 1000;      // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡曠瑐闂?
//    roll_balance_cascade.angular_speed_cycle.i_value_pro    = 0.1f;       // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡樼槷娓?
//    roll_balance_cascade.angular_speed_cycle.out_max        = 50;      // 鐟欐帡鈧喎瀹抽悳顖濈翻閸戣桨绗傞梽?
//
//    // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戠憴鎺戝閻?PID 闂勬劕绠欓崣鍌涙殶
//    roll_balance_cascade.angle_cycle.i_value_max        = 1000;      // 鐟欐帒瀹抽悳顖溞濋崚鍡曠瑐闂?
//    roll_balance_cascade.angle_cycle.i_value_pro         = 2.0f;       // 鐟欐帒瀹抽悳顖溞濋崚鍡樼槷娓?
//    roll_balance_cascade.angle_cycle.out_max            = 10000;      // 鐟欐帒瀹抽悳顖濈翻閸戣桨绗傞梽?

//    roll_balance_cascade.angle_cycle.i_value_max        = 1000;      // 鐟欐帒瀹抽悳顖溞濋崚鍡曠瑐闂?
//    roll_balance_cascade.angle_cycle.i_value_pro        = 2.0f;       // 鐟欐帒瀹抽悳顖溞濋崚鍡樼槷娓?
//    roll_balance_cascade.angle_cycle.out_max            = 50;      // 鐟欐帒瀹抽悳顖濈翻閸戣桨绗傞梽?

//    roll_balance_cascade.turn_cycle.i_value_max        = 1000;      // 鏉烆剙鎮滈悳顖溞濋崚鍡曠瑐闂?
//    roll_balance_cascade.turn_cycle.i_value_pro         = 2.0f;     // 鏉烆剙鎮滈悳顖溞濋崚鍡樼槷娓?
//    roll_balance_cascade.turn_cycle.out_max            = 180;      //  鏉烆剙鎮滈悳顖濈翻閸戣桨绗傞梽?



    // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戦柅鐔峰閻?PID 闂勬劕绠欓崣鍌涙殶
    roll_balance_cascade.speed_cycle.i_value_max        = 500;      // 闁喎瀹抽悳顖溞濋崚鍡曠瑐闂?
    roll_balance_cascade.speed_cycle.i_value_pro         = 0.005f;   // 闁喎瀹抽悳顖溞濋崚鍡樼槷娓?
    roll_balance_cascade.speed_cycle.out_max            = 9000;      // 闁喎瀹抽悳顖濈翻閸戣桨绗傞梽?

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸氬嫮骞?PID 閻?P/I/D 缁粯鏆?
//    roll_balance_cascade.angular_speed_cycle.p    = 0.008f;      // 鐟欐帡鈧喎瀹抽悳?P
//    roll_balance_cascade.angular_speed_cycle.i    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?I
//    roll_balance_cascade.angular_speed_cycle.d    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?D
//
//    roll_balance_cascade.angle_cycle.p    = 150.0f;      // 鐟欐帒瀹抽悳?P
//    roll_balance_cascade.angle_cycle.i    = 0.0f;        // 鐟欐帒瀹抽悳?I
//    roll_balance_cascade.angle_cycle.d    = 0.0f;       // 鐟欐帒瀹抽悳?D
//
////    roll_balance_cascade.angle_cycle.p    = 10.0f;      // 鐟欐帒瀹抽悳?P
////    roll_balance_cascade.angle_cycle.i    = 0.0f;        // 鐟欐帒瀹抽悳?I
////    roll_balance_cascade.angle_cycle.d    = 0.0f;       // 鐟欐帒瀹抽悳?D
//
//    roll_balance_cascade.turn_cycle.p     = 0.2f;      // 鏉烆剙鎮滈悳?D
//    roll_balance_cascade.turn_cycle.i     = 0.0f;      // 鏉烆剙鎮滈悳?D
//    roll_balance_cascade.turn_cycle.d     = 0.0f;      // 鏉烆剙鎮滈悳?D
//
////        roll_balance_cascade.turn_cycle.p     = 0.2f;      // 鏉烆剙鎮滈悳?D
////        roll_balance_cascade.turn_cycle.i     = 0.0f;      // 鏉烆剙鎮滈悳?D
////        roll_balance_cascade.turn_cycle.d     = 0.0f;      // 鏉烆剙鎮滈悳?D
//
//
//    roll_balance_cascade.speed_cycle.p    = 20.0f;      // 闁喎瀹抽悳?P
//    roll_balance_cascade.speed_cycle.i    = 0.0f;      // 闁喎瀹抽悳?I
//    roll_balance_cascade.speed_cycle.d    = 0.0f;      // 闁喎瀹抽悳?D


//        roll_balance_cascade.angular_speed_cycle.p    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?P
//        roll_balance_cascade.angular_speed_cycle.i    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?I
//        roll_balance_cascade.angular_speed_cycle.d    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?D
//
//        roll_balance_cascade.angle_cycle.p    = 0.0f;      // 鐟欐帒瀹抽悳?P
//        roll_balance_cascade.angle_cycle.i    = 0.0f;        // 鐟欐帒瀹抽悳?I
//        roll_balance_cascade.angle_cycle.d    = 0.0f;       // 鐟欐帒瀹抽悳?D
//
//        roll_balance_cascade.turn_cycle.p     = 0.0f;      // 鏉烆剙鎮滈悳?D
//        roll_balance_cascade.turn_cycle.i     = 0.0f;      // 鏉烆剙鎮滈悳?D
//        roll_balance_cascade.turn_cycle.d     = 0.0f;      // 鏉烆剙鎮滈悳?D
//
//        roll_balance_cascade.speed_cycle.p    = 0.0f;      // 闁喎瀹抽悳?P
//        roll_balance_cascade.speed_cycle.i    = 0.0f;      // 闁喎瀹抽悳?I
//        roll_balance_cascade.speed_cycle.d    = 0.0f;      // 闁喎瀹抽悳?D


    //////////////////////////////////////////////////////////////////////////////////////////////////

    // 娣囨繂鐡ㄦ穱顖欒瘽楠炲疇銆€閹貉冨煑閸掓繂顫愰崣鍌涙殶閸掓澘顦禒鐣岀波閺嬪嫪缍?
    memcpy(&roll_balance_cascade_resave, &roll_balance_cascade, sizeof(roll_balance_cascade_resave));
    // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戦崶娑樺帗閺佺増膩閸?
    quaternion_module_init(&roll_balance_cascade);

    // 閸掓繂顫愰崠鏍姘挬鐞涒剝甯堕崚鍓佹畱婵寧鈧礁寮弫?
    pitch_balance_cascade.posture_value.call_cycle        = 0.001;      // 鐠嬪啰鏁ら崨銊︽埂 0.001s (1ms)
    pitch_balance_cascade.posture_value.mechanical_zero  = 0.0f;       // 閺堢儤顫梿鍓佸仯閸掓繂顫愰崠鏍﹁礋 0
    pitch_balance_cascade.posture_value.correct_kp        = 0.4f;       // 婵寧鈧焦鐗庨崙鍡樼槷娓氬閮撮弫?0.4
    pitch_balance_cascade.posture_value.correct_ki        = 0.015f;     // 婵寧鈧焦鐗庨崙鍡櫺濋崚鍡欓兇閺?0.015

    // 閸掓繂顫愰崠鏍姘挬鐞涒剝甯堕崚鍓佹畱鐟欐帡鈧喎瀹抽悳?PID 闂勬劕绠欓崣鍌涙殶
    pitch_balance_cascade.angular_speed_cycle.i_value_max     = 1000;      // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡曠瑐闂?
    pitch_balance_cascade.angular_speed_cycle.i_value_pro    = 0.3f;       // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡樼槷娓?
    pitch_balance_cascade.angular_speed_cycle.out_max        = 10000;      // 鐟欐帡鈧喎瀹抽悳顖濈翻閸戣桨绗傞梽?

    // 閸掓繂顫愰崠鏍姘挬鐞涒剝甯堕崚鍓佹畱鐟欐帒瀹抽悳?PID 闂勬劕绠欓崣鍌涙殶
    pitch_balance_cascade.angle_cycle.i_value_max        = 300;      // 鐟欐帒瀹抽悳顖溞濋崚鍡曠瑐闂?
    pitch_balance_cascade.angle_cycle.i_value_pro         = 0.8f;       // 鐟欐帒瀹抽悳顖溞濋崚鍡樼槷娓?
    pitch_balance_cascade.angle_cycle.out_max            = 300;      // 鐟欐帒瀹抽悳顖濈翻閸戣桨绗傞梽?

    // 閸掓繂顫愰崠鏍姘挬鐞涒剝甯堕崚鍓佹畱闁喎瀹抽悳?PID 闂勬劕绠欓崣鍌涙殶
    pitch_balance_cascade.speed_cycle.i_value_max        = 4000;      // 闁喎瀹抽悳顖溞濋崚鍡曠瑐闂?
    pitch_balance_cascade.speed_cycle.i_value_pro         = 0.05f;   // 闁喎瀹抽悳顖溞濋崚鍡樼槷娓?
    pitch_balance_cascade.speed_cycle.out_max            = 1500;      // 闁喎瀹抽悳顖濈翻閸戣桨绗傞梽?

    // 濡亝绮撮獮瀹犮€€閹貉冨煑閸氬嫮骞?PID 閻?P/I/D 缁粯鏆?
    pitch_balance_cascade.angular_speed_cycle.p    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?P
    pitch_balance_cascade.angular_speed_cycle.i    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?I
    pitch_balance_cascade.angular_speed_cycle.d    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?D

    pitch_balance_cascade.angle_cycle.p    = 0.0f;      // 鐟欐帒瀹抽悳?P
    pitch_balance_cascade.angle_cycle.i    = 1.0f;        // 鐟欐帒瀹抽悳?I
    pitch_balance_cascade.angle_cycle.d    = 0.0f;       // 鐟欐帒瀹抽悳?D

    pitch_balance_cascade.speed_cycle.p    = 0.0f;      // 闁喎瀹抽悳?P
    pitch_balance_cascade.speed_cycle.i    = 0.0f;      // 闁喎瀹抽悳?I
    pitch_balance_cascade.speed_cycle.d    = 0.0f;      // 闁喎瀹抽悳?D

    // 娣囨繂鐡ㄥΟ顏呯泊楠炲疇銆€閹貉冨煑閸掓繂顫愰崣鍌涙殶閸掓澘顦禒鐣岀波閺嬪嫪缍?
    memcpy(&pitch_balance_cascade_resave, &pitch_balance_cascade, sizeof(pitch_balance_cascade_resave));
}





void balance_mode_parameter(int Mode_change_flag)
{
    if(Mode_change_flag==1)
    {

        roll_balance_cascade.angular_speed_cycle.i_value_max     = 1000;      // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡曠瑐闂?
        roll_balance_cascade.angular_speed_cycle.i_value_pro    = 0.1f;       // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡樼槷娓?
        roll_balance_cascade.angular_speed_cycle.out_max        = 50;      // 鐟欐帡鈧喎瀹抽悳顖濈翻閸戣桨绗傞梽?

        // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戠憴鎺戝閻?PID 闂勬劕绠欓崣鍌涙殶
        roll_balance_cascade.angle_cycle.i_value_max        = 1000;      // 鐟欐帒瀹抽悳顖溞濋崚鍡曠瑐闂?
        roll_balance_cascade.angle_cycle.i_value_pro         = 2.0f;       // 鐟欐帒瀹抽悳顖溞濋崚鍡樼槷娓?
        roll_balance_cascade.angle_cycle.out_max            = 10000;      // 鐟欐帒瀹抽悳顖濈翻閸戣桨绗傞梽?-----------韫囶偊鈧喐妲告稉澶屽箚娑撹尙楠?閺堝顫楅柅鐔峰,閹碘偓娴犮儴顫楁惔锔惧箚閸欘亝妲告稉顓㈡？娴ｆ粎鏁ゆ稉宥勭稊娑撹櫣娲块幒銉ㄧ翻閸?

        roll_balance_cascade.turn_cycle.i_value_max        = 1000;      // 鏉烆剙鎮滈悳顖溞濋崚鍡曠瑐闂?
        roll_balance_cascade.turn_cycle.i_value_pro         = 2.0f;     // 鏉烆剙鎮滈悳顖溞濋崚鍡樼槷娓?
        roll_balance_cascade.turn_cycle.out_max            = 180;      //  鏉烆剙鎮滈悳顖濈翻閸戣桨绗傞梽?

        // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戦柅鐔峰閻?PID 闂勬劕绠欓崣鍌涙殶
        roll_balance_cascade.speed_cycle.i_value_max        = 600;      // 闁喎瀹抽悳顖溞濋崚鍡曠瑐闂? 500
        roll_balance_cascade.speed_cycle.i_value_pro         = 0.005f;   // 闁喎瀹抽悳顖溞濋崚鍡樼槷娓?
        roll_balance_cascade.speed_cycle.out_max            = 9000;      // 闁喎瀹抽悳顖濈翻閸戣桨绗傞梽?

        // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸氬嫮骞?PID 閻?P/I/D 缁粯鏆?
        roll_balance_cascade.angular_speed_cycle.p    = 0.008f;      // 鐟欐帡鈧喎瀹抽悳?P
        roll_balance_cascade.angular_speed_cycle.i    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?I
        roll_balance_cascade.angular_speed_cycle.d    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?D

        roll_balance_cascade.angle_cycle.p    = 150.0f;      // 鐟欐帒瀹抽悳?P
        roll_balance_cascade.angle_cycle.i    = 0.0f;        // 鐟欐帒瀹抽悳?I
        roll_balance_cascade.angle_cycle.d    = 0.0f;        // 鐟欐帒瀹抽悳?D

        roll_balance_cascade.turn_cycle.p     = 0.3f;        // 鏉烆剙鎮滈悳?D
        roll_balance_cascade.turn_cycle.i     = 0.0f;        // 鏉烆剙鎮滈悳?D
        roll_balance_cascade.turn_cycle.d     = 0.0f;        // 鏉烆剙鎮滈悳?D

        roll_balance_cascade.speed_cycle.p    = 3.5f;       // 闁喎瀹抽悳? P   //max:4.0
        roll_balance_cascade.speed_cycle.i    = 0.4f;        // 闁喎瀹抽悳?I   //max:0.4
        roll_balance_cascade.speed_cycle.d    = 1.0f;        // 闁喎瀹抽悳?D
    }
    else if(Mode_change_flag==2)
    {
        // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戠憴鎺戝閻?PID 闂勬劕绠欓崣鍌涙殶
        roll_balance_cascade.angle_cycle.i_value_max        = 1000;      // 鐟欐帒瀹抽悳顖溞濋崚鍡曠瑐闂?
        roll_balance_cascade.angle_cycle.i_value_pro        = 2.0f;       // 鐟欐帒瀹抽悳顖溞濋崚鍡樼槷娓?
        roll_balance_cascade.angle_cycle.out_max            = 50;      // 鐟欐帒瀹抽悳顖濈翻閸戣桨绗傞梽?

        roll_balance_cascade.turn_cycle.i_value_max        = 1000;      // 鏉烆剙鎮滈悳顖溞濋崚鍡曠瑐闂?
        roll_balance_cascade.turn_cycle.i_value_pro         = 2.0f;     // 鏉烆剙鎮滈悳顖溞濋崚鍡樼槷娓?
        roll_balance_cascade.turn_cycle.out_max            = 180;      //  鏉烆剙鎮滈悳顖濈翻閸戣桨绗傞梽?

        // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戦柅鐔峰閻?PID 闂勬劕绠欓崣鍌涙殶
        roll_balance_cascade.speed_cycle.i_value_max        = 500;      // 闁喎瀹抽悳顖溞濋崚鍡曠瑐闂?
        roll_balance_cascade.speed_cycle.i_value_pro         = 0.001f;   // 闁喎瀹抽悳顖溞濋崚鍡樼槷娓氬绱欏В?.001韫囶偓绱濈悰銉ョ繁娑撳﹤宕岄柅鐕傜礆
        roll_balance_cascade.speed_cycle.out_max            = 9000;      // 闁喎瀹抽悳顖濈翻閸戣桨绗傞梽?

        // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸氬嫮骞?PID 閻?P/I/D 缁粯鏆?
        roll_balance_cascade.angle_cycle.p    = 10.0f;      // 鐟欐帒瀹抽悳?P
        roll_balance_cascade.angle_cycle.i    = 0.0f;        // 鐟欐帒瀹抽悳?I
        roll_balance_cascade.angle_cycle.d    = 0.0f;       // 鐟欐帒瀹抽悳?D

        roll_balance_cascade.turn_cycle.p     = 0.2f;      // 鏉烆剙鎮滈悳?D
        roll_balance_cascade.turn_cycle.i     = 0.0f;      // 鏉烆剙鎮滈悳?D
        roll_balance_cascade.turn_cycle.d     = 0.0f;      // 鏉烆剙鎮滈悳?D

        roll_balance_cascade.speed_cycle.p    = 3.3f;      // 闁喎瀹抽悳?P 閳?闁洤鍩岄梼璇插閸楄櫕妞傜悰銉ュ//20.0
        roll_balance_cascade.speed_cycle.i    = 0.4f;      // 闁喎瀹抽悳?I//3.0
        roll_balance_cascade.speed_cycle.d    = 1.0f;      // 闁喎瀹抽悳?D//0

    }
    else if(Mode_change_flag==3)
    {
               roll_balance_cascade.angular_speed_cycle.i_value_max     = 1000;      // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡曠瑐闂?
               roll_balance_cascade.angular_speed_cycle.i_value_pro    = 0.1f;       // 鐟欐帡鈧喎瀹抽悳顖溞濋崚鍡樼槷娓?
               roll_balance_cascade.angular_speed_cycle.out_max        = 50;      // 鐟欐帡鈧喎瀹抽悳顖濈翻閸戣桨绗傞梽?

               // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戠憴鎺戝閻?PID 闂勬劕绠欓崣鍌涙殶
               roll_balance_cascade.angle_cycle.i_value_max        = 1000;      // 鐟欐帒瀹抽悳顖溞濋崚鍡曠瑐闂?
               roll_balance_cascade.angle_cycle.i_value_pro         = 2.0f;       // 鐟欐帒瀹抽悳顖溞濋崚鍡樼槷娓?
               roll_balance_cascade.angle_cycle.out_max            = 10000;      // 鐟欐帒瀹抽悳顖濈翻閸戣桨绗傞梽?-----------韫囶偊鈧喐妲告稉澶屽箚娑撹尙楠?閺堝顫楅柅鐔峰,閹碘偓娴犮儴顫楁惔锔惧箚閸欘亝妲告稉顓㈡？娴ｆ粎鏁ゆ稉宥勭稊娑撹櫣娲块幒銉ㄧ翻閸?

               roll_balance_cascade.turn_cycle.i_value_max        = 1000;      // 鏉烆剙鎮滈悳顖溞濋崚鍡曠瑐闂?
               roll_balance_cascade.turn_cycle.i_value_pro         = 2.0f;     // 鏉烆剙鎮滈悳顖溞濋崚鍡樼槷娓?
               roll_balance_cascade.turn_cycle.out_max            = 180;      //  鏉烆剙鎮滈悳顖濈翻閸戣桨绗傞梽?

               // 閸掓繂顫愰崠鏍﹀垔娴犳澘閽╃悰鈩冨付閸掑墎娈戦柅鐔峰閻?PID 闂勬劕绠欓崣鍌涙殶
               roll_balance_cascade.speed_cycle.i_value_max        = 600;      // 闁喎瀹抽悳顖溞濋崚鍡曠瑐闂?
               roll_balance_cascade.speed_cycle.i_value_pro         = 0.005f;   // 闁喎瀹抽悳顖溞濋崚鍡樼槷娓?
               roll_balance_cascade.speed_cycle.out_max            = 9000;      // 闁喎瀹抽悳顖濈翻閸戣桨绗傞梽?

               // 娣囶垯璇濋獮瀹犮€€閹貉冨煑閸氬嫮骞?PID 閻?P/I/D 缁粯鏆?
               roll_balance_cascade.angular_speed_cycle.p    = 0.008f;      // 鐟欐帡鈧喎瀹抽悳?P
               roll_balance_cascade.angular_speed_cycle.i    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?I
               roll_balance_cascade.angular_speed_cycle.d    = 0.0f;      // 鐟欐帡鈧喎瀹抽悳?D

               roll_balance_cascade.angle_cycle.p    = 150.0f;      // 鐟欐帒瀹抽悳?P
               roll_balance_cascade.angle_cycle.i    = 0.0f;        // 鐟欐帒瀹抽悳?I
               roll_balance_cascade.angle_cycle.d    = 0.0f;        // 鐟欐帒瀹抽悳?D

               roll_balance_cascade.turn_cycle.p     = 0.2f;        // 鏉烆剙鎮滈悳?D
               roll_balance_cascade.turn_cycle.i     = 0.0f;        // 鏉烆剙鎮滈悳?D
               roll_balance_cascade.turn_cycle.d     = 0.0f;        // 鏉烆剙鎮滈悳?D

               roll_balance_cascade.speed_cycle.p    = 3.5f;        // 闁喎瀹抽悳?P
               roll_balance_cascade.speed_cycle.i    = 0.4f;        // 闁喎瀹抽悳?I
               roll_balance_cascade.speed_cycle.d    = 1.0f;        // 闁喎瀹抽悳?D
    }


}

void Imu_init(void)
{
    if(Imu_type==1)
    {
        imu660ra_init();
    }
    else if(Imu_type==2)
    {
        imu660rb_init();
    }
    else if(Imu_type==3)
    {
        imu963ra_init();
    }

}

void Imu_attitude_scan(void)
{
    if(Imu_type==1)
    {
        imu660ra_get_gyro();                             // 閼惧嘲褰?闂勨偓閾昏桨鍗庨弫鐗堝祦
        imu660ra_get_acc();                              // 閼惧嘲褰?閸旂娀鈧喎瀹崇拋鈩冩殶閹?
        quaternion_module_calculate(&roll_balance_cascade); // 鐠侊紕鐣婚崶娑樺帗閺佸府绱濋弴瀛樻煀婵寧鈧焦鏆熼幑?
    }
    else if(Imu_type==2)
    {
        imu660rb_get_gyro();                             // 閼惧嘲褰?闂勨偓閾昏桨鍗庨弫鐗堝祦
        imu660rb_get_acc();                              // 閼惧嘲褰?閸旂娀鈧喎瀹崇拋鈩冩殶閹?
        quaternion_module_calculate(&roll_balance_cascade); // 鐠侊紕鐣婚崶娑樺帗閺佸府绱濋弴瀛樻煀婵寧鈧焦鏆熼幑?
    }
    else if(Imu_type==3)
    {
        imu963ra_get_gyro();                             // 閼惧嘲褰?闂勨偓閾昏桨鍗庨弫鐗堝祦
        imu963ra_get_acc();                              // 閼惧嘲褰?閸旂娀鈧喎瀹崇拋鈩冩殶閹?
        quaternion_module_calculate(&roll_balance_cascade); // 鐠侊紕鐣婚崶娑樺帗閺佸府绱濋弴瀛樻煀婵寧鈧焦鏆熼幑?
    }

}

void Imu_lowpass_filter(void)
{
    static float last_gyro_y = 0.0f;

    if(Imu_type==1)
    {
        imu660ra_gyro_x=LowPassFilter(imu660ra_gyro_x, last_gyro_y, 0.1f);
        last_gyro_y=imu660ra_gyro_x;
    }
    else if(Imu_type==2)
    {
        imu660rb_gyro_y=LowPassFilter(imu660rb_gyro_y, last_gyro_y, 0.1f);
        last_gyro_y=imu660rb_gyro_y;
    }
    else if(Imu_type==3)
    {
        imu963ra_gyro_y=LowPassFilter(imu963ra_gyro_y, last_gyro_y, 0.1f);
        last_gyro_y=imu963ra_gyro_y;
    }
}



void IMU_text(void)
{

    ips_show_string(8*0, 16*0, "Pitch:");    ips_show_float(8*10,16*0, roll_balance_cascade.posture_value.pit,3,6);
    ips_show_string(8*0, 16*1, "ROLL:");     ips_show_float(8*10,16*1, roll_balance_cascade.posture_value.rol,3,6);
    ips_show_string(8*0, 16*2, "Yaw:");      ips_show_float(8*10,16*2, roll_balance_cascade.posture_value.yaw,3,6);

//       printf("%d,%d,%d\n",imu963ra_gyro_x, imu963ra_gyro_y, imu963ra_gyro_z);
//       printf("%d,%d,%d\n",imu660ra_gyro_x, imu660ra_gyro_y, imu660ra_gyro_z);
         printf("%d,%d,%d\n",imu660rb_gyro_x, imu660rb_gyro_y, imu660rb_gyro_z);

//       printf("%f,%f,%f\r\n",roll_balance_cascade.posture_value.pit, roll_balance_cascade.posture_value.rol, roll_balance_cascade.posture_value.yaw);
             system_delay_ms(10);
}
