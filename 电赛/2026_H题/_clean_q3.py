# -*- coding: utf-8 -*-
import os
import re

BASE = r"C:\Users\65347\Desktop\chao (2)"

def read_text(path):
    data = open(path, "rb").read()
    try:
        s = data.decode("utf-8")
        enc = "utf-8"
    except UnicodeDecodeError:
        s = data.decode("gbk")
        enc = "gbk"
    s = s.replace("\r\n", "\n").replace("\r", "\n")  # 归一化行尾，避免 CRLF 匹配失败
    return s, enc

def write_text(path, s, enc):
    s = s.replace("\r\n", "\n").replace("\r", "\n").replace("\n", "\r\n")  # 还原 CRLF
    open(path, "wb").write(s.encode(enc))

# ============ BUJIN.c ============
p = os.path.join(BASE, "code", "BUJIN.c")
s, enc = read_text(p)
lines = s.split("\n")
out = []
skip_brace = 0
dead_macros = ["BALL_SETTLE_ERR", "BALL_STUCK_ERR", "BALL_STUCK_VEL",
               "BALL_BOOST_START", "BALL_BOOST_MAX", "BALL_BOOST_STEP",
               "BALL_ESCAPE_ERR", "BALL_ESCAPE_TICK", "BALL_ESCAPE_MAX",
               "BALL_ACTION_POS_MIN", "BALL_ACTION_POS_MAX"]
dead_vars = ["uint8 ball_action_active", "uint16 ball_action_tick",
             "uint16 ball_action_t1", "uint16 ball_action_t2", "uint16 ball_action_t3",
             "int16 ball_action_r1", "int16 ball_action_l2", "int16 ball_action_r3"]

for line in lines:
    if any(m in line for m in dead_macros):
        continue
    if any(v in line for v in dead_vars):
        continue
    if skip_brace > 0:
        if line.strip() == "}":   # 扁平函数，遇列0的闭合花括号即结束跳过
            skip_brace = 0
        continue
    if "static void ball_control_state_reset(" in line or "void bujin_action_start(void)" in line:
        skip_brace = 1
        continue
    out.append(line)
s = "\n".join(out)

# 给 bujin_action_stop 加中文注释（回中并关闭输出）
s = s.replace(
    "void bujin_action_stop(void)\n{",
    "// 停车时调用：清零舵机设定、回中并关闭输出\nvoid bujin_action_stop(void)\n{",
)

# 给 ball_action_step 变量加注释
s = s.replace(
    "int16 ball_action_step = STEPPER_MAX_STEP_20MS;",
    "int16 ball_action_step = STEPPER_MAX_STEP_20MS;  // 舵机运行速度（每20ms步进数，0=停止）",
)

# 重写 servo_tune_change（正则整函数替换，避免内部空格敏感）
new_switch_func = r'''void servo_tune_change(uint8 item, int8 dir)
{
    switch(item)
    {
        case 0:  servo_p          += dir; break;
        case 1:  servo_d          += dir; break;
        case 2:  servo_set_x      += dir; break;
        case 3:  servo_set_y      += dir; break;
        case 4:  ball_action_step += dir; break;   // 舵机运行速度
        default: servo_tune_item = 0; break;
    }

    if(servo_p < -50)  servo_p = -50;
    if(servo_p >  50)  servo_p =  50;
    if(servo_d < -50)  servo_d = -50;
    if(servo_d >  50)  servo_d =  50;
    if(servo_set_x < 200) servo_set_x = 200;
    if(servo_set_x > 440) servo_set_x = 440;
    if(servo_set_y < 120) servo_set_y = 120;
    if(servo_set_y > 360) servo_set_y = 360;
    if(ball_action_step < 1) ball_action_step = 1;
    if(ball_action_step > STEPPER_MAX_STEP_20MS) ball_action_step = STEPPER_MAX_STEP_20MS;
}'''
m = re.sub(r"void servo_tune_change\(uint8 item, int8 dir\)\n\{.*?\n\}\n",
           new_switch_func + "\n", s, count=1, flags=re.DOTALL)
assert m != s, "servo_tune_change function not found/replaced in BUJIN.c"
s = m
write_text(p, s, enc)
print("BUJIN.c done, enc=", enc)

# ============ BUJIN.h ============
p = os.path.join(BASE, "code", "BUJIN.h")
s, enc = read_text(p)
old_externs = r'''extern uint8 ball_action_active;
extern uint16 ball_action_tick;
extern uint16 ball_action_t1;
extern uint16 ball_action_t2;
extern uint16 ball_action_t3;
extern int16 ball_action_r1;
extern int16 ball_action_l2;
extern int16 ball_action_r3;
extern int16 ball_action_step;'''
new_externs = "extern int16 ball_action_step;  // 舵机运行速度（每20ms步进数）"
assert old_externs in s, "ball_action externs not found in BUJIN.h"
s = s.replace(old_externs, new_externs)
assert "void bujin_action_start(void);\n" in s, "bujin_action_start decl not found"
s = s.replace("void bujin_action_start(void);\n", "")
write_text(p, s, enc)
print("BUJIN.h done, enc=", enc)

# ============ display.c ============
p = os.path.join(BASE, "code", "display.c")
s, enc = read_text(p)
assert '    else if(model == 3) ips200_show_string(48, 16, "BALL ");\n' in s, "model==3 BALL not found"
s = s.replace('    else if(model == 3) ips200_show_string(48, 16, "BALL ");\n', "")
assert '"SERVO/Q3 K2> K3+ K4-"' in s, "SERVO/Q3 title not found"
s = s.replace('"SERVO/Q3 K2> K3+ K4-"', '"SERVO K2> K3+ K4-"')
old_page = r'''    ips200_show_string(0, 16, servo_tune_item == 0 ? ">SVP:" : " SVP:"); ips200_show_int(56, 16, servo_p, 4);
    ips200_show_string(0, 32, servo_tune_item == 1 ? ">SVD:" : " SVD:"); ips200_show_int(56, 32, servo_d, 4);
    ips200_show_string(0, 48, servo_tune_item == 2 ? ">SX :" : " SX :"); ips200_show_int(56, 48, servo_set_x, 4);
    ips200_show_string(0, 64, servo_tune_item == 3 ? ">SY :" : " SY :"); ips200_show_int(56, 64, servo_set_y, 4);

    ips200_show_string(0, 88,  servo_tune_item == 4 ? ">T1 :" : " T1 :"); ips200_show_int(56, 88,  ball_action_t1, 4);
    ips200_show_string(0, 104, servo_tune_item == 5 ? ">T2 :" : " T2 :"); ips200_show_int(56, 104, ball_action_t2, 4);
    ips200_show_string(0, 120, servo_tune_item == 6 ? ">T3 :" : " T3 :"); ips200_show_int(56, 120, ball_action_t3, 4);
    ips200_show_string(120, 88,  servo_tune_item == 7 ? ">R1 :" : " R1 :"); ips200_show_int(176, 88,  ball_action_r1, 4);
    ips200_show_string(120, 104, servo_tune_item == 8 ? ">L2 :" : " L2 :"); ips200_show_int(176, 104, ball_action_l2, 4);
    ips200_show_string(120, 120, servo_tune_item == 9 ? ">R3 :" : " R3 :"); ips200_show_int(176, 120, ball_action_r3, 4);
    ips200_show_string(0, 144, servo_tune_item == 10 ? ">STP:" : " STP:"); ips200_show_int(56, 144, ball_action_step, 4);'''
new_page = r'''    ips200_show_string(0, 16, servo_tune_item == 0 ? ">SVP:" : " SVP:"); ips200_show_int(56, 16, servo_p, 4);
    ips200_show_string(0, 32, servo_tune_item == 1 ? ">SVD:" : " SVD:"); ips200_show_int(56, 32, servo_d, 4);
    ips200_show_string(0, 48, servo_tune_item == 2 ? ">SX :" : " SX :"); ips200_show_int(56, 48, servo_set_x, 4);
    ips200_show_string(0, 64, servo_tune_item == 3 ? ">SY :" : " SY :"); ips200_show_int(56, 64, servo_set_y, 4);
    ips200_show_string(0, 80, servo_tune_item == 4 ? ">STP:" : " STP:"); ips200_show_int(56, 80, ball_action_step, 4);'''
assert old_page in s, "servo tune page items not found"
s = s.replace(old_page, new_page)
write_text(p, s, enc)
print("display.c done, enc=", enc)

# ============ key.c (两份) ============
for kp in [os.path.join(BASE, "code", "key.c"), os.path.join(BASE, "user", "src", "key.c")]:
    s, enc = read_text(kp)
    assert "if(servo_tune_item >= 11) servo_tune_item = 0;" in s, "servo wrap >=11 not found in " + kp
    s = s.replace("if(servo_tune_item >= 11) servo_tune_item = 0;",
                  "if(servo_tune_item >= 5) servo_tune_item = 0;")
    write_text(kp, s, enc)
    print("key.c done:", kp, "enc=", enc)

print("ALL DONE")
