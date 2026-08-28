# RA8P1 轮腿平衡车调试记录

更新时间：2026-08-17

## 2026-08-17 摄像头模式（画面+背心框，不跟随）与跟随模式（auto_driver）独立触发

### 需求

1. 语音说**「摄像头模式」**（`'5'`）：IPS200 显示 SCC8660 摄像头画面，识别荧光绿背心并框住（vest_follow 只识别**不跟随**）；小车为基础自平衡，**摄像头返回的任何参数不影响运动**。
2. 语音说**「跟随模式」**（`'9'`）：**任意状态可直接触发**（无需先进入摄像头模式），清空画面，IPS200 显示 auto_drive 参数页（SPD/T1/T2），执行定时行驶（行驶 T1 → 停止维持 T2 → 持续以 SPD 行驶，参数按键可调）。

### 模式状态机

```
正常页面 --'5'--> 摄像头画面模式(画面+背心框,车只平衡)
   ^                     |
   |                     |--'9'--> auto_drive(参数页+定时行驶)
   |                     |          ^  '9' 也可从正常页面直接触发
   |______其它语音指令退出___|__________|
```

- `'5'`（摄像头模式）：OFF → 画面；AUTO_DRIVE → 回画面；VIEW → 保持。
- `'9'`（跟随模式）：OFF/VIEW → auto_drive；AUTO_DRIVE → 保持。
- 画面模式/auto_drive 下说其它任意语音指令（前进/停止/惯导/遥控等）→ 退出，恢复原页面。
- 画面模式下按键忽略（清事件防残留）；auto_drive 子模式下按键调参（K3 切项/K2 加/K1 减/K4 重新开始）。

### 改动

- `src/camera_display.c/.h`：三态（OFF / VIEW / AUTO_DRIVE）
  - VIEW：摄像头采集显示（CEU 每帧武装）、vest_follow 识别+画框、信息条（LOCK/A/X/H/距离），**不产生运动输出**（`follow_command.active=false`，车只平衡）
  - AUTO_DRIVE：`'9'` 任意状态触发，清屏显示参数页 + `auto_drive_start()` + 命令转发（复用 `camera_follow_command_t` 通道，`balance_control` 无需改动）
  - 新增 `camera_display_auto_drive_active()` 供按键路由区分子模式
- `src/key_tune.c`：按键仅在 auto_drive 子模式路由到 auto_drive；画面模式按键忽略并清事件。
- `README_调试记录.md`：本记录。

### 说明 / 注意事项

- 摄像头没接/初始化失败不影响小车：画面模式显示 `CAMERA INIT FAIL`，`'9'` 仍可正常执行 auto_drive。
- CH3 急停（下一条记录）在两种模式下均生效（最高优先级）。

---

## 2026-08-17 CH3 运行闸门急停（最高优先级，任何模式生效）

### 需求

- 遥控器 **CH3 运行闸门**优先级调高：不再依赖语音"遥控模式"使能，只要接收机在线+标定完成，**任何状态**（摄像头模式、语音运动、惯导、遥控）下都直接生效。
- **CH3 低位 = 停止（急停）**：轮子 `set_duty(0,0)` 直接为零、舵机 `servo_hold_disable()` 掉电、**不维持平衡**（车会倒），等效电源开关关闭。
- **CH3 高位 = 启动**：解除急停，恢复平衡；正在执行的模式（如摄像头定时行驶）**暂停后继续**（急停期间状态机时间冻结）。

### 设计决策（用户确认）

- **上电默认**：接收机上线并标定完成后，CH3 在低位就立即急停（车不维持平衡会倒）；CH3 拨高才"启动"。测试时先把 CH3 拨到高位。
- **急停期间状态机**：暂停（时间冻结），解除后从暂停处继续当前阶段。

### 改动

- `src/balance_control.c`：
  - `balance_control_update()` 顶部新增 CH3 闸门检查（最高优先级）：接收机在线+标定后，`run_enabled==0` → `g_balance_shutdown=true`（复用原急停分支：轮子 0、舵机掉电、不维持平衡）；`run_enabled!=0` → 解除急停、恢复舵机、腿回零点。
  - 解除后新增**姿态恢复保护**：`BALANCE_CONTROL_KILL_RESUME_ANGLE_DEG(45°)` 内才恢复平衡输出，车倒了保持零输出，扶正后自动恢复（防倒在地上全力挣扎损坏电机/舵机）。
  - 新增 `balance_control_kill_active()` 供其它模块查询急停状态。
  - 诊断打印：`[KILL] CH3 low -> emergency stop.` / `[KILL] CH3 high -> resume.`
- `src/servo_hold.c/.h`：新增 `servo_hold_enable()`（disable 后恢复舵机供电）。
- `src/auto_drive.c`：`auto_drive_update()` 急停时直接返回（阶段计时冻结）；参数页 ST 行急停时显示 `KILL`。
- `README_调试记录.md`：本记录。

### 说明 / 注意事项

- **接收机不在线/未标定时不干预**：上电后接收机未连/标定中（约 0.5s），CH3 无效，车正常平衡。
- **掉线时保持当前状态**：急停中掉线 → 保持急停（安全）；运行中掉线 → 保持运行（但 steering/throttle 已清零）。
- **暂停覆盖范围**：本次暂停实现覆盖摄像头定时行驶（有阶段的状态机）；语音运动/惯导复现为无阶段连续动作，急停期间输出被压零，解除后继续。
- **急停≠语音"停止"**：语音「停止」仍是"停止运动、保留平衡站住"；只有 CH3 低位才是"舵机死掉、不维持平衡"的急停。
- 遥控器通道约定：CH1=方向、CH2=油门、CH3=运行闸门（>991 为启动，172~1811 量程中位）。

---

## 2026-08-17 摄像头模式改为"定时行驶"（行驶-停止-行驶，参数按键可调）

### 需求

语音说**「摄像头模式」**（串口 `'5'`）进入摄像头模式后，小车执行用户指定的定时动作：

1. 先以固定速度 **SPD** 行驶 **T1** 秒；
2. 然后停止（速度环主动制动）维持 **T2** 秒；
3. 之后持续以 **SPD** 行驶，直到收到其它语音指令退出摄像头模式。

进入摄像头模式时 IPS200 显示三个可调变量（SPD/T1/T2），可用按键选择和设置。

### 设计决策（用户确认）

- **显示**：摄像头模式不再显示摄像头画面，改为整屏显示参数调节页（PAGE CAM）。
- **序列**：一次性执行（行驶 T1 → 停 T2 → 持续行驶），非循环。
- **背心跟随**：本次停用跟随逻辑（代码保留），摄像头模式专做定时行驶。
- **参数默认值/范围/步长**：SPD `0~1000` 步长 `20` 默认 `400`（无刷转速反馈量纲）；T1 `0~30s` 步长 `0.1s` 默认 `3s`；T2 `0~30s` 步长 `0.1s` 默认 `2s`。

### 新增/修改文件

- `src/auto_drive.c/.h`（新增）：
  - 定时行驶状态机：`DRIVE_1`（以 SPD 行驶 T1）→ `HOLD`（停止维持 T2）→ `DRIVE_CONT`（持续以 SPD 行驶）
  - 航向保持：锁定进入时的 yaw，按误差比例差速纠正跑偏（增益 `-8.5`、限幅 ±250，与语音前进一致）
  - T1/T2 设为 0 秒时直接跳过对应阶段
  - 按键调节：K3 切项、K2 加、K1 减、K4 重新开始动作
  - 参数页显示（`PAGE CAM` + SPD/T1/T2 + ST 状态 + 按键提示；选中项黄色高亮）
- `src/camera_display.c/.h`：摄像头模式改为定时行驶模式
  - 进入：`auto_drive_start()` + 绘制参数页；退出：`auto_drive_stop()` + 清屏 + 强制重绘正常页面
  - 不再采集/显示摄像头画面、不再跑背心跟随（去掉 `vest_follow`/`dl1b`/CEU 处理）
  - 运动输出仍走 `camera_follow_command_t` 通道，`balance_control.c` 无需改动
- `src/key_tune.c`：摄像头模式下按键改路由到 `auto_drive`（原来 K1 是切跟随开关）
- `src/voice_control.c`：摄像头模式下允许运动指令（前进/后退/左转/右转/停止）注册，从而**说任意语音指令都能退出摄像头模式**（原来运动指令在非语音模式下被丢弃，只能靠 6/7/8 退出）
- `src/hal_entry.c`：初始化序列加入 `auto_drive_init()`
- `Debug/src/subdir.mk`：加入 `auto_drive.c` 构建项

### 按键功能（摄像头模式参数页底部有提示）

| 按键 | 功能 |
|---|---|
| K3 | 切换选中项 SPD → T1 → T2 |
| K2 | 选中项加（SPD +20 / 时间 +0.1s） |
| K1 | 选中项减 |
| K4 | 重新开始动作序列（从"行驶 T1"阶段重新执行） |

### 说明 / 注意事项

- 进入摄像头模式即开始行驶，若想先调参数再启动：进入后立即按 K4 之前的参数不会生效于当前序列——**先按 K3/K2/K1 调好参数，再按 K4 重新开始**即可按新参数执行。
- 退出摄像头模式（任意语音指令）后，小车回到原地平衡保持（速度环清零、腿回机械零点），与之前行为一致。
- 速度 400 量纲与语音前进一致（无刷轮转速反馈，rpm 左右），实车行驶过快调小 SPD 即可。
- 摄像头画面功能（SCC8660 采集/背心跟随）代码保留在 `vest_follow.c`、`zf_device_scc8660.c`，本次未接入；后续要恢复画面只需改 `camera_display.c`。
- 本机（C:\Users\Su\Desktop\RA8P1yaokong6）无 ARM 工具链，未在本机编译；需在 e2studio 里 `Refresh → Clean → Rebuild`（新增了 `auto_drive.c`，必须重新生成构建清单）。

### 实车问题修复记录（同日）

1. **进入摄像头模式屏幕残留上个页面、显示混乱**
   - 原因：`camera_display_enter()` 漏了 `ips200_clear()`，只清了参数页自己的几行，上个页面（RUN/PID 等）其它行的文字/数值残留。
   - 修复：进入时先 `ips200_clear()` + 复位画笔为绿色，再画参数页。

2. **进入摄像头模式小车不动**
   - 排查：逻辑链路（语音 '5' → `camera_display_enter` → `auto_drive_start` → `follow_command` → `balance_control` 速度环 → 腿/轮）与已验证的惯导复现走的是同一条通道，静态检查无断点。
   - **重大嫌疑（已修复）**：`balance_control.c` 的语音「停止」会把 `g_balance_shutdown` 置位，而这个标志**从不复位**——一旦置位，电机/舵机被永久关闭，之后无论说什么（包括进摄像头模式）小车都动不了。已改为「停车」只停止运动、保留平衡（与 README 记录的"停车只保留平衡站住"一致），不再永久关停。
   - **诊断打印（新增，一次性）**：
     - `[CAM] enter SPD=... T1=... T2=...`：进入摄像头模式并打印当前参数；
     - `[AD] state x->y tick=...`：定时行驶状态机切换（0=IDLE 1=DRIVE 2=HOLD 3=RUN）；
     - `[BAL] cam drive on speed=...`：平衡环收到摄像头驱动命令。
   - 若仍不动，按串口打印判断断在哪一环：有 `[CAM]` 无 `[AD]` → 状态机没跑；有 `[AD]` 无 `[BAL]` → 命令没到平衡环；有 `[BAL]` 仍不动 → 查机械/舵机/轮速反馈。另外注意**测试时车要放地上**（托空时平衡输出≈0，轮子不会转）。

3. **HOLD 停止阶段"往后退"（失控倒车）**
   - 原因：HOLD 制动用 **|轮速|（绝对值）** 反馈，腿反向压车制动；一旦制动力把车推得倒溜，|轮速| 仍为正，误差恒为负，腿一直反向压 → 车越退越快，无法自行纠正。
   - 修复（`balance_control.c`）：HOLD 阶段改用**带符号轮速反馈**；行驶阶段自动标定"前进方向"符号（`[BAL] cam speed sign=±1` 打印，不依赖驱动板正负约定），倒溜时误差自动反号、腿回正把车拉停。未标定（如速度太小没动起来）时退回 |轮速| 行为。

---

## 2026-08-17 语音控制真驱动接通（前进/后退/左转/右转/停车）

### 需求

语音说话不再只改屏幕状态，而是要**实际驱动小车**，并满足：
- 说**「前进」**：以 150 速度前进，保持平衡（`Status:GO`）。
- 说**「后退」**：以 150 速度后退，保持平衡（`Status:BACK`）。
- 说**「左转」**：**原地旋转**，依靠陀螺仪(yaw)向左转 90 度到目标航向，转完保持平衡（`Status:LEFT`）。
- 说**「右转」**：**原地旋转**向右转 90 度（`Status:RIGHT`）。
- 说**「停车」**：**不管处于哪种状态**（前进/后退/转向、摄像头跟随、惯导复现/直行），立即停止运动输出，只保留平衡站住。

### 改动（`src/balance_control.c`）

- `balance_update_voice_command()` 增加显式 `case VOICE_COMMAND_STOP`：清空语音运动/转向模式与目标航向，回到原地保持。
- `balance_control_update()`：
  - 新增 `stop_requested`（语音为 STOP），**最高优先级**：一旦为真，强制 `voice_mode=IDLE`、并覆盖掉 `camera_following` 和 `inertial_mode`，使 `replaying=false` → 速度环清零、腿回机械零点，不产生前进/转向，仅保留平衡 PID 让小车站住。
  - `replaying` 判定加 `!stop_requested`，避免停在惯导复现/直行上继续跑。

### 布局/参数调整（2026 追加）

- 前进/后退目标速度 `BALANCE_CONTROL_VOICE_SPEED` 从 `80 → 150 → 200`（无刷轮转速反馈量纲，rpm 左右；实车按需标定）。
- **左转/右转改为"原地旋转"**：新增 `BALANCE_CONTROL_VOICE_SPIN_DUTY(300)`，在收到"左转/右转"命令时锁定旋转方向（`spin_dir`），左右轮以固定反向差速原地转，用陀螺仪 yaw 判到位后停止、回到保持平衡。
- **左右方向对调**：实车验证左转会右旋，故将 `VOICE_COMMAND_LEFT` 的 `spin_dir` 改为 `+1`、`RIGHT` 改为 `-1`。
- **转不准/转过头修复**：转向到位判定收紧为 `TURN_DONE_DEG(2°)`，并新增减速区 `TURN_SLOW_DEG(18°)`——接近 90° 时按剩余角度比例减小旋转占空比，让车减速而非急停，准停在 90°；偏离死区会重新回转校正到目标角。
- 增加 `TURN_TIMEOUT(1000 tick)` 超时保护，防方向异常时一直转。

> 若实车再出现左右方向反，改 `VOICE_COMMAND_LEFT/RIGHT` 的 `spin_dir` 符号（+1↔-1）即可，其它逻辑不用动。

### 说明 / 注意事项（重要）

- **「停车」不等于电机完全不转**：自平衡车要站住，平衡电机必须持续以很小的占空比微调（由角度环+角速度环维持），否则会倒下。停车后**不会再有前进/后退/转向的运动指令**，只是站住。
- "速度 200" 量纲为无刷轮转速反馈（rpm 左右），实车前进量过大时把 `BALANCE_CONTROL_VOICE_SPEED` 调小即可。
- 原地旋转方向按上述 `spin_dir` 约定；若实车左/右方向反了，只需对调 `spin_dir` 符号。
- 语音 `7/8`（跳跃/遥控）仍只显示 `Status:JUMP/REMOTE`，不产生运动，本次未接执行逻辑。
- 保留按键调参（K1~K4 分页），与语音控制并存：语音驱动 + 按键调 PID/惯导参数互不干扰。

---

## 2026-08-17 屏幕页面指示 + RUN 页底部状态行补充

### 需求

- 屏幕**右上角**显示当前页页码 `PG:0 / PG:1 ...`，随按键切页变化，指示当前所在页。
- **首个页面（RUN 页）最后一行**上电用**黄色**显示 `Status:balance` 表示平衡状态；说出语音指令后随命令变化（前进→`Status:GO`、后退→`Status:BACK`、跳跃→`Status:JUMP`、遥控→`Status:REMOTE` 等）。
- 结合天问语音文件 `语音控制小车.hd`：目前命令已写到 **遥控模式**（串口 `'8'`），本次为这些新命令补上状态显示文本。

### 改动

- `src/voice_control.h/.c`：新增 `VOICE_COMMAND_JUMP('7')`、`VOICE_COMMAND_REMOTE('8')` 两个枚举，串口解析 `7/8`，显示文本 `Status:JUMP` / `Status:REMOTE`；状态文本固定 7 字符宽度不变。
- `src/imu_angle_display.c`：
  - 新增 `imu_angle_display_draw_page_indicator()`，在每页标题行**右上角**画 `PG:页码`（绿色）。
  - 新增 `imu_angle_display_draw_run_status_line()`，仅 RUN 页调用 `voice_control_display()` 在**最后一行(y=215)** 画黄色 `Status:xxx`（上电默认 `balance`，随语音变化）。
  - **切页残留修复**：`imu_angle_display_select_page()` 切页时改为先 `ips200_clear()` 整屏清屏 + 显式复位画笔为绿色，再完整重画当前页，彻底消除上一页文字/数值残影。
- `src/voice_control.c`：`voice_control_display()` 的 Y 坐标改为最后一行 `215`（原来 195）。
- `README_调试记录.md`：语音串口协议表补上行 `7`/`8`。

### 说明 / 注意事项

- 状态行**只在 RUN 页显示**，切到 PID/IMU/NAG/STR 页时不显示（符合"首个页面"的定位）。
- `balance_control.c` 不把 `JUMP/REMOTE` 当作运动指令：加入枚举后它们仍落在 `default` 分支，不会引起误动；目前这两条只影响界面状态文本，具体执行逻辑（遥控通道、跳跃）后续再接入。
- 摄像头模式（`Status:CAM`）期间只显示摄像头画面，不显示状态行，保持原设计。

---

## 当前工程

- RA8P1 主工程：`F:\RA8P1\RA8P1yaokong`
- 旧惯导/PID 调试工程：`F:\RA8P1\cpkhmi_ra8p1_zf_device_imu660rb_demo`
- 参考成熟工程：`F:\RA8P1\2026_7_4\2026_7_4`
- 当前目标烧录文件：`F:\RA8P1\RA8P1yaokong\Debug\RA8P1yaokong.srec`

## 当前已经跑通的模块

- IPS200 屏幕：已正常显示，方向为 `IPS200_CROSSWISE_180`
- IMU660RB：已正常读取，并显示偏航角、俯仰角、横滚角
- 舵机：四路舵机可上电保持，机械零点已人工调好
- 无刷驱动：TX-only 控制可用，轮子已经能动
- 平衡控制：能基本站住，但有前后抖动，需要继续调 PID
- 四个按键：已接入，用于分页调参

## 关键接线记录

### IPS200

IPS200 上排接线：

| IPS200 | RA8P1 |
|---|---|
| BLK | 已按工程接线 |
| CS | 已按工程接线 |
| DC | 已按工程接线 |
| RST | 已按工程接线 |
| SDA | 已按工程接线 |
| SCL | 已按工程接线 |
| VCC | 3.3V |
| GND | GND |

下排 NC 不接。

### IMU660RB

| IMU660RB | RA8P1 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | P601 |
| SDA/SDI | P603 |
| SDO | P602 |
| CS | P604 |
| INT1 | 暂未使用 |
| INT2 | 暂未使用 |

### 无刷驱动串口

当前稳定方案是 TX-only：

| RA8P1 | 无刷驱动 |
|---|---|
| P714 / TXD4 | 驱动 RX |
| P715 / RXD4 | 当前不使用 |
| GND | GND |

注意：之前 SCI4 RX 接收/中断会导致屏幕和主循环卡住，所以目前不要重新打开 RX 反馈。

### 四个按键

按键输入使用内部上拉，按下接 GND，松手触发一次。

| 按键 | RA8P1 引脚 | 接口位置 |
|---|---|---|
| K1 | P501 | P4-7 |
| K2 | P811 | P4-8 |
| K3 | P502 | P4-9 |
| K4 | P812 | P4-10 |

当前按键功能：

| 按键 | 功能 |
|---|---|
| K4 | 换页面 |
| K3 | 换当前页项目 |
| K2 | 当前参数加 |
| K1 | 当前参数减 |

## 当前软件结构

新增/主要模块：

- `src/key_tune.c` / `src/key_tune.h`
  - 按键扫描
  - 消抖
  - 分页调参
  - PID/IMU 参数运行时修改

- `src/imu_angle_display.c`
  - RUN/PID/IMU 三页显示
  - 不使用 `ips200_show_float()`
  - 使用字符串定点显示，避免屏幕黑屏/花屏

- `src/imu_process.c`
  - 姿态解算
  - 角度环、角速度环 PID 参数

- `src/balance_control.c`
  - 平衡控制主逻辑
  - 舵机保持
  - 无刷输出限幅和平滑

- `src/brushless_driver.c`
  - 当前使用 TX-only 控制
  - 不依赖 RX 速度反馈

## 屏幕页面

### RUN 页

显示：

- Yaw
- Pit
- Rol
- B：平衡控制状态
- L/R：左右轮输出
- IE/TE：无刷初始化/发送错误状态

### PID 页

显示并可调：

| 名称 | 含义 | 当前调节步长 |
|---|---|---|
| AP | 角度环 P | 10.0 |
| AD | 角度环 D | 1.0 |
| GP | 角速度环 P | 0.05 |
| Z0 | 机械零点偏置 | 0.1 |

屏幕右侧 `<` 表示当前选中的项目。

### IMU 页

显示并可调：

| 名称 | 含义 | 当前调节步长 |
|---|---|---|
| CKP | 姿态解算加速度修正 KP | 0.05 |
| CKI | 姿态解算加速度修正 KI | 0.001 |

同时显示 Yaw/Pit/Rol。

## 当前 PID 默认参数

在 `src/imu_process.c` 中：

```c
imu_roll_balance.angular_speed_cycle.p = 0.85f;
imu_roll_balance.angle_cycle.p = 520.0f;
imu_roll_balance.angle_cycle.i = 0.0f;
imu_roll_balance.angle_cycle.d = 25.0f;
```

在 `src/balance_control.c` 中：

```c
#define BALANCE_CONTROL_MAX_DUTY        (600)
#define BALANCE_CONTROL_DUTY_FILTER_OLD (0.65f)
#define BALANCE_CONTROL_DUTY_FILTER_NEW (0.35f)
#define BALANCE_CONTROL_DUTY_SLEW_STEP  (120)
```

## PID 调试建议

当前现象：车能基本保持平衡，但前后抖动。

优先调法：

1. 先在 PID 页调 `AP`
   - 太大：反应猛，容易前后抖
   - 太小：车软，扶不住
   - 建议从 520 往下试：500、480、450

2. 再调 `GP`
   - 太大：轮子输出猛，高频抖
   - 太小：电机扶不住车
   - 建议小步调：0.80、0.75 或 0.90

3. 最后调 `AD`
   - 适当增大可以压冲过头
   - 太大会对噪声敏感
   - 建议从 25 试到 30、35

4. `I` 当前保持 0
   - 平衡初期不建议开 I
   - 否则容易积分导致来回冲

## 已踩坑记录

- IPS200 花屏/黑屏：
  - 高频复杂显示容易出问题
  - `ips200_show_float()` 曾导致黑屏/异常
  - 当前使用字符串定点显示，低频刷新

- 无刷 RX 反馈：
  - SCI4 RX 接收/中断曾导致主循环/屏幕卡住
  - 当前使用 TX-only，稳定性更好

- 无刷不动：
  - 曾经是接线掉了或 TX/RX 接反
  - 软件层不要通过交换 RX/TX 解决已经打板的线序问题，优先确认实际 TX -> 驱动 RX

- 屏幕和舵机异常：
  - 多次实际原因是接线问题
  - 代码排查前先确认供电、GND 共地、信号线顺序

## 下一步计划

1. 用 PID 页调到前后抖动明显减小
2. 记录一组能站稳的 AP/AD/GP/Z0
3. 把调好的参数写回 `imu_process.c`
4. 后续再考虑保存到 Flash
5. 继续移植轮腿姿态控制，不只是轮子平衡

## 新对话恢复提示

如果开启新对话，可以直接告诉 Codex：

“读取 `F:\RA8P1\cpkhmi_ra8p1_zf_device_imu660rb_demo\README_调试记录.md`，继续 RA8P1 轮腿平衡车调试。”

---

## 2026-08-15 无刷速度反馈稳定方案补充

### 本轮最终结论

无刷驱动速度反馈已经跑通，能够读取左右轮速度。最终稳定方向不是 TX-only，也不是主循环轮询串口，而是：

- `P714 / SCI4_TXD4 -> 无刷驱动 RX`
- `P715 / SCI4_RXD4 <- 无刷驱动 TX`
- 必须共地
- SCI4 波特率：`460800`
- 速度请求帧：`A5 02 00 00 00 00 A7`
- 无刷驱动收到速度请求后，会默认约 10ms 周期回传速度帧

### 关键排查过程

1. 主循环轮询 SCI4 RX 不可靠。
   - 460800 波特率下 7 字节一帧约 0.15ms 传完。
   - 主循环 5ms 才跑一次，必然漏字节或溢出。
   - 现象是 `FR=0`、`ER` 增长、速度全 0。

2. P714/P715 自环测试证明 RA8P1 SCI4 引脚可用。
   - `ra_cfg.txt` 中确认：`P714` 支持 `SCI4: TXD4`，`P715` 支持 `SCI4: RXD4`。
   - 短接测试后，改用 RXI 中断可以收到数据。

3. 直接在 RXI 回调里解析完整协议会增加卡住概率。
   - 后来改成 RXI 中断只把字节放进环形缓冲区。
   - 主循环里再解析协议帧。

4. 屏幕显示大量调试计数会拖慢系统。
   - 运行页显示 `FR/ER/RB/LB` 时容易卡。
   - 降载后只显示 `Yaw / Pit / LS / RS`，系统稳定很多。

5. 无刷重复上电偶发卡住，与上电瞬间乱帧有关。
   - 当前代码在 SCI4 初始化后前约 300ms 忽略 RX 垃圾。
   - 300ms 后再发送速度请求。
   - 正常收到速度帧后不再反复请求。

### 当前代码状态

当前可用文件：

`F:\RA8P1\cpkhmi_ra8p1_zf_device_imu660rb_demo\Debug\cpkhmi_ra8p1_zf_device_imu660rb_demo.srec`

核心文件：

- `src/brushless_driver.c`
  - SCI4 发送控制帧
  - RXI 中断收字节
  - 环形缓冲区暂存 RX 字节
  - 主循环解析速度反馈帧
  - ERI 错误中断关闭，主循环清错误标志
  - 启动前约 300ms 忽略无刷上电乱帧

- `src/imu_angle_display.c`
  - 当前运行页低负载显示：`Yaw / Pit / LS / RS`
  - 不再显示持续增长的 `FR/ER/RB/LB`

### 当前注意事项

- 如果为了调试重新显示 `FR/ER/RB/LB`，有可能再次增加卡住概率。
- 不建议恢复主循环轮询 SCI4 RX。
- 不建议在 RXI 回调里做完整帧解析或复杂显示。
- 如果后续仍有极小概率卡住，可以继续加“长时间没有新速度帧则重启 SCI4”的自动恢复。

### 新对话建议开头

新对话可以直接说：

“读 `F:\RA8P1\cpkhmi_ra8p1_zf_device_imu660rb_demo\README_调试记录.md`，继续 RA8P1 轮腿平衡车。当前无刷速度反馈已经跑通，最终方案是 SCI4 RXI 中断 + 环形缓冲 + 低负载显示 + 300ms 启动忽略乱帧。”

---

## 2026-08-17 屏幕界面与语音状态优化

### 界面配色

- 语音状态行 `Status:xxx`（y=195）保持**黄色**
- 页面其它所有内容统一**绿色**（标题、标签、数值、按键提示、PG 页码、右上角调试数字）

### 页面结构（右上角显示 `PG:页码`）

- `PG:0` RUN 页：`RUN` / Yaw / Pit / LS / RS + 状态行 + 按键提示
- `PG:1` PID 页：AP / AD / GP / Z0 可调参（`->` 光标指示当前项，右侧显示步长 STEP）
- `PG:2` IMU 页：CKP / CKI 可调参，同时显示 Yaw / Pit / Rol

### 按键功能（每页底部提示行）

| 按键 | 功能 | 提示 |
|---|---|---|
| K1 (P501) | 当前参数减 | `K1:-` |
| K2 (P811) | 当前参数加 | `K2:+` |
| K3 (P502) | 切换光标（当前调节项） | `K3:CUR` |
| K4 (P812) | 循环切换页面 RUN→PID→IMU | `K4:PAGE` |

注意：K4 现在循环三页（原代码只会在 RUN↔PID 之间切换，IMU 页代码已存在但进不去，现已接通）。

### 语音串口协议（与 语音控制小车.hd 对应）

| 天问语音 | 串口字符 | 屏幕显示 |
|---|---|---|
| 前进 | `0` | Status:GO |
| 后退 | `1` | Status:BACK |
| 左转 | `2` | Status:LEFT |
| 右转 | `3` | Status:RIGHT |
| 停止 | `4` | Status:STOP |
| 摄像头模式 | `5` | Status:CAM |
| 惯导模式 | `6` | Status:INS |
| 跳跃模式 | `7` | Status:JUMP |
| 遥控模式 | `8` | Status:REMOTE |
| （上电默认/未收到命令） | - | Status:balance |

解析在 `src/voice_control.c` 的 `switch`，状态文本在 `voice_control_get_command_text()`；显示在 `src/imu_angle_display.c`（仅 RUN 页最后一行，黄色）。

---

## 2026-08-17 摄像头模式移植（CAM 工程 → RA8P1）

### 功能

- 语音说"**摄像头模式**"（串口 `'5'`）→ 进入摄像头模式：SCC8660 画面 2 倍放大铺满屏幕显示（320x240），底部黄色 `Status:CAM`
- 画面带绿色目标追踪（与 CAM 工程一致）：红色矩形框住绿色目标 + 绿色十字准星（EMA 平滑）
- 说**其它任意语音指令**（前进/停止/惯导模式等）→ 退出摄像头模式，恢复原来的页面显示（RUN/PID/IMU + 状态行）
- 上电即尝试初始化摄像头（有限重试 3 次），摄像头没接/失败不影响小车正常功能，进入摄像头模式时显示 `CAMERA INIT FAIL`

### 摄像头引脚（按 CAM 工程）

| 信号 | RA8P1 引脚 |
|---|---|
| 摄像头配置软 IIC SCL | **P706** |
| 摄像头配置软 IIC SDA | **P707** |
| CEU PCLK | P414 |
| CEU HSYNC | P415 |
| CEU VSYNC | P708 |
| CEU D0~D7 | P400 P401 P405 P406 P700 P701 P702 P703 |

注：软 IIC 原为 P410/P409，已按 CAM 改为 P706/P707（`configuration.xml`、`pin_data.c`、`bsp_pin_cfg.h` 同步修改）。

### 新增/修改文件

- `src/camera_display.c/.h`（新增）：摄像头模式管理与画面显示
- `zf_device/zf_device_color_track.c/.h`（从 CAM 复制）：绿色目标识别
- `zf_device/zf_device_ips200.c/.h`：新增 `ips200_draw_rect` / `ips200_draw_cross`
- `zf_device/zf_device_scc8660.c`：软 IIC 引脚改为 P706/P707
- `ra_gen/pin_data.c`、`ra_cfg/fsp_cfg/bsp/bsp_pin_cfg.h`、`configuration.xml`：摄像头引脚配置同步
- `src/hal_entry.c`：上电初始化摄像头 + 主循环摄像头模式分支
- `src/imu_angle_display.c/.h`：新增 `imu_angle_display_force_refresh()`（退出摄像头模式后强制重绘页面）
- `Debug/zf_device/subdir.mk`、`Debug/src/subdir.mk`：新源文件加入构建

### 提醒

- `ra_cfg.txt` 是旧文档，里面摄像头引脚仍写 P409/P410，不影响编译（CAM 工程里同样过期）
- 摄像头没上电时 CEU 不工作；若接线无误仍 `CAMERA INIT FAIL`，检查软 IIC 两线（P706/P707）和 8 根数据线
- 摄像头模式显示一帧约 40ms（SPI 传整幅 320x240），期间主循环会慢一些，属正常现象

### 摄像头画面模糊/花屏/闪烁/退出残留 — 修复记录（同日）

- **根因 1（花屏/闪烁）**：CEU 中断里原来每帧完成就 `captureStart` 自动重启采集，而显示一帧要 ~40ms，主循环拷贝图像时 CEU 正在往同一缓冲区写新帧 → 读到半帧 → 撕裂/花屏/闪烁
  - 修复：`zf_device_scc8660.c` 中断只置完成标志、不再自动重启；`camera_display.c` 在拷贝前手动 `captureStart` 重新武装（CEU 会等下一个 VSYNC 才开始写），保证拷贝的永远是完整帧
- **根因 2（模糊）**：2 倍直接像素复制放大有马赛克感
  - 修复：ips200 新增 `ips200_show_rgb565_image_smooth2x()`，对相邻像素做通道级平均插值，画面更平滑
- **根因 3（退出残留"若隐若现"）**：页面重绘只清固定几行，不清整屏
  - 修复：退出摄像头模式时先 `ips200_clear()` 整屏清屏，再重绘正常页面
- 附带：摄像头模式下不再绘制右上角调试数字，避免在画面上闪烁

### 摄像头模式仍花屏 — 平均函数 bug（同日再修）

- **根因**：`ips200_show_rgb565_image_smooth2x()` 里通道平均函数传参有误——`fx`/`fy` 单独为 1 的分支传了 4 个参数（两两重复）却除以 `count=2`，导致求和翻倍、通道值溢出 5/6 位域 → 半数像素变成垃圾色 → 花屏
  - 修复：拆成 `avg2`/`avg4` 两个函数，各自按真实像素个数求平均，颜色恢复正常
- 摄像头模式下**不再显示 Status**（用户要求：进入摄像头模式只显示摄像头画面）

### 摄像头画面正确性自查要点（供后续排查）

- 若画面颜色偏紫/偏绿/彩噪：优先怀疑 CEU 字节交换配置（`byte_swapping.swap_16bit_units`）与摄像头 `SCC8660_DATA_FORMAT` 是否匹配（当前 DATA_FORMAT=1 字节交换 + CEU swap_16bit=1）
- 若画面有水平撕裂带：检查是否在 CEU 写入缓冲期间拷贝图像（当前已改为手动武装一帧再拷贝，避免此问题）
- 若整屏黑：检查 CEU 是否成功 `captureStart`、软 IIC 两线是否正常（`scc8660_init` 返回 0 才算成功）

---

## 2026-08-17 RA8P1yaokong 与旧惯导/PID 工程合并记录

### 合并目标

以新工程 `F:\RA8P1\RA8P1yaokong` 为主工程，保留队友新增的串口、语音、摄像头、背心跟随等功能；从旧工程 `F:\RA8P1\cpkhmi_ra8p1_zf_device_imu660rb_demo` 移植已经调过的平衡 PID、腿控制、惯导记录/保存/复现功能。

### 本次保留的新工程功能

- SCI2 语音控制：`src/voice_control.c/.h`，P802 RX，9600 baud。
- SCI6 遥控接收：`src/remote_control.c/.h`，接收机 TX 接 P909 RX，SBUS 100000 baud、8E2；`hal_entry.c` 已初始化并周期解析，调试串口约每 100 ms 输出一次 CH1~CH6。
- 摄像头模式：`src/camera_display.c/.h`，语音命令 `5` 进入摄像头/背心跟随模式。
- 背心跟随识别：`src/vest_follow.c/.h`。
- 新工程 FSP/引脚配置继续保留，不用旧工程覆盖 `configuration.xml`。

### 从旧工程移植进 RA8P1yaokong 的内容

- 惯导模块：
  - `src/nag_navigation.c/.h`
  - `src/nag_flash.c/.h`
- 按键调参页面：
  - `RUN -> PID -> IMU -> NAG -> LEG -> RUN`
  - PID 页恢复 `AP / AI / AD / GP / Z0`
  - NAG 页恢复 `REC / SAV / REP / GAIN / SPD / SPDP`
  - LEG 页恢复 `S1 / S2 / S3 / S4` 四路舵机独立调零
- 舵机：`servo_hold.c/.h` 改回四路独立脉宽，可支持腿单独调节。
- 平衡控制：`balance_control.c` 改回旧工程已调版本，包含：
  - 更大的电机输出限幅
  - 更快电机发送周期
  - 复现时速度环接腿
  - 惯导 `Final_Out` 叠加左右轮差速转向
  - LEG 页面时不覆盖手动舵机脉宽
- 显示页面：`imu_angle_display.c` 恢复 NAG/LEG 页面，同时保留新工程需要的 `imu_angle_display_force_refresh()`，用于退出摄像头模式后强制重绘页面。

### 当前 PID/零点参数

来自实车已稳定版本：

| 参数 | 当前值 | 说明 |
|---|---:|---|
| `mechanical_zero` | `0.0f` | 用户机械拆装调中后，软件零点改为 0 |
| `AP` | `600.0f` | 角度环 P |
| `AI` | `1.0f` | 角度环 I |
| `AD` | `55.0f` | 角度环 D |
| `GP` | `0.85f` | 角速度环 P |
| `SPDP` | `5.0f` 默认，可页面调 | 复现腿速度环 P，纯平衡可先调小/为 0 |
| `GAIN` | `0.5f` 默认，可页面调 | 惯导 Final_Out 差速增益 |
| `SPD` | `400.0f` 默认，可页面调 | 惯导复现前进速度目标 |
| `TRIM` | `0.0f` 默认，可页面调 | 惯导固定差速补偿，用于抵消车体天然左/右偏 |

### 腿/舵机方向记录

车头在前：

| 舵机 | 位置 | `+us` 现象 | 对车体影响 |
|---|---|---|---|
| S1 | 左前 | 舵机杆下降 | 车头左边升高 |
| S2 | 右前 | 舵机杆上升 | 车头右边下降 |
| S3 | 左后 | 舵机杆上升 | 车尾左边下降 |
| S4 | 右后 | 舵机杆下降 | 车尾右边升高 |

复现前进腿输出映射当前为：

```c
S1 = 1500 + leg_us;
S2 = 1500 - leg_us;
S3 = 1500 + leg_us;
S4 = 1500 - leg_us;
```

之前反向映射会导致复现倒退，已修正。

### hal_entry 主循环状态

初始化顺序当前为：

1. `imu_process_init()`
2. `balance_control_init()`
3. `key_tune_init()`
4. `nag_navigation_init()`
5. `voice_control_init()`
6. `imu_angle_display_init()`
7. `camera_display_init()`

主循环每 5ms：

1. `imu_process_update()`
2. `key_tune_update()` + `voice_control_process()`
3. `balance_control_update()`
4. `nag_navigation_update()`
5. 非摄像头模式下刷新 IMU/PID/NAG/LEG 页面
6. 摄像头模式下由 `camera_display_process()` 接管画面

### 当前显示状态

- 已移除 `voice_control_display()` 的**全局**底部状态栏；状态行改为仅 RUN 页最后一行显示（黄色 `Status:xxx`，见文件顶部 2026-08-17 记录）。
- `imu_angle_display.c` 仍保留页面内显示，但 NAG 页目前还存在显示拥挤/显示不全问题，后续需重新整理 NAG 布局。
- 目前新增了 `STR` 直行慢跑命令页项，用于不录制轨迹时低速直行调 `TRIM`。
- `NAG` 页已加入 `TRIM` 参数显示和调节项。

### 构建注意事项

新增了 `nag_flash.c` 和 `nag_navigation.c` 后，必须让 e2studio 重新生成 Debug 构建清单：

1. 右键 `RA8P1yaokong` 工程，点 `Refresh`。
2. 执行 `Project -> Clean...`，选择 `RA8P1yaokong`。
3. 重新 Build。

原因：当前 `Debug/src/subdir.mk` 是 e2studio 自动生成文件，旧版本不一定包含新加的 NAG 源文件；Clean/Rebuild 会自动把整个 `src` 目录重新扫描进去。

### 调试启动修复记录

`RA8P1yaokong` 曾出现：

```text
Cannot find smart bundle for : RA8P1yaokong.
```

已处理：

- `.project` 工程名统一为 `RA8P1yaokong`
- `.cproject` Debug/Release 的 `artifactName` 统一为 `RA8P1yaokong`
- buildPath 统一为 `${workspace_loc:/RA8P1yaokong}/Debug` / `Release`
- 补齐调试器需要的：
  - `Debug/RA8P1yaokong.sbd`
  - `Debug/RA8P1yaokong.rpd`
  - `RA8P1yaokong.elf.jlink`

如果再次出现 smart bundle 错误，优先 Clean/Rebuild Debug，再确认 `Debug/RA8P1yaokong.elf/.sbd/.rpd` 是否同时存在。

### 下一步测试建议

1. 先不进复现，只测试平衡：确认上电四腿机械中位，车能稳定站住。
2. PID 页确认：`AP=600 AI=1.0 AD=55 GP=0.85 Z0=0.0`。
3. LEG 页确认 S1/S2/S3/S4 单独调节正常，退出 LEG 页后平衡控制会重新接管腿。
4. NAG 页录一条短直线：REC 开始，推车直走，SAV 保存，确认 `MI/CNT` 增长。
5. REP 复现：先用低 `SPD`，`GAIN` 从 `0.5` 附近试，必要时再调 `SPDP`。
6. 直线补偿调试：先用 `STR` 直行命令，把 `GAIN` 降低后调 `TRIM`，用来抵消右轮内扣导致的天然左拐。
