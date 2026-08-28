#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
K230 Ball Track-Line Detect (only balls ON the track line)
  PX_PER_CM = |ball_cx_at_+5cm - 320| / 5
"""
import os, gc, time, json
from libs.PlatTasks import DetectionApp
from libs.PipeLine import PipeLine, ScopedTiming
from libs.Utils import *
from ybUtils.YbUart import YbUart

uart = YbUart(baudrate=38400)

ROOT = "/sdcard/mp_deployment_source/"
deploy_conf = read_json(ROOT + "deploy_config.json")

display_mode = "lcd"
rgb888p_size = [640, 480]
CENTER_X = 293  # new origin, shifted left 2cm from 343
CENTER_Y = 358  # new origin
CONF_THRESH = 0.08
PX_PER_CM = 25.0  # (430-305)/5
HOLD_FRAMES = 30
CIRCLE_THRESH = 1800
TRACK_Y_TOL = 28           # only accept balls within +/-14px of track center

kmodel_path = ROOT + deploy_conf["kmodel_path"]
labels = deploy_conf["categories"]
nms_threshold = deploy_conf["nms_threshold"]
model_input_size = deploy_conf["img_size"]
model_type = deploy_conf["model_type"]
anchors = sum(deploy_conf.get("anchors", [[]]*3), []) if model_type == "AnchorBaseDet" else []

prev_offset_cm = 0.0
last_cx = 0
last_cy = CENTER_Y
lost = 0
frame_cnt = 0
rejected = 0


def send_frame(offset_cm, found):
    if not found:
        uart.send('{"d":0.0,"n":0}\n')
    else:
        d = max(-99.9, min(99.9, round(offset_cm, 1)))
        uart.send('{"d":%.1f,"n":1}\n' % d)


def on_track(cy):
    """Ball must be ON the track line, not above or below"""
    return abs(cy - CENTER_Y) < TRACK_Y_TOL


def main():
    global prev_offset_cm, last_cx, last_cy, lost, frame_cnt, rejected
    pl = None
    det = None

    try:
        pl = PipeLine(rgb888p_size=rgb888p_size, display_mode=display_mode)
        pl.create()
        display_size = pl.get_display_size()

        det = DetectionApp(
            "video", kmodel_path, labels, model_input_size,
            anchors, model_type, CONF_THRESH, nms_threshold,
            rgb888p_size, display_size, debug_mode=0
        )
        det.config_preprocess()

        print("[INFO] Track-only | Y=%d +/-%dpx | %.1f px/cm" %
              (CENTER_Y, TRACK_Y_TOL, PX_PER_CM))

        while True:
            os.exitpoint()
            frame_cnt += 1
            img = pl.get_frame()
            res = det.run(img)

            ball_found = False
            ball_cx, ball_cy = 0, 0
            max_conf = 0.0
            best_score = -9999.0

            boxes = res.get('boxes') if isinstance(res, dict) else (
                res if isinstance(res, (list, tuple)) else []
            )

            if boxes is not None and len(boxes) > 0:
                for b in boxes:
                    if len(b) < 4:
                        continue
                    x1, y1, x2, y2 = int(b[0]), int(b[1]), int(b[2]), int(b[3])
                    conf = float(b[4]) if len(b) > 4 else 1.0
                    cx = (x1 + x2) / 2
                    cy = (y1 + y2) / 2
                    # track first, then confidence
                    if not on_track(cy):
                        rejected += 1; continue
                    if conf < 0.01:
                        rejected += 1; continue
                    dist = abs(cx - last_cx) + abs(cy - last_cy)
                    score = conf * 100 - dist * 0.3
                    if score > best_score:
                        best_score = score
                        ball_cx, ball_cy, max_conf = cx, cy, conf
                        ball_found = True

            # CV fallback: only search the track line area
            if not ball_found:
                try:
                    circles = img.find_circles(
                        threshold=CIRCLE_THRESH, r_min=4, r_max=30,
                        roi=(0, CENTER_Y - TRACK_Y_TOL,
                             640, TRACK_Y_TOL * 2 + 20))
                    if circles and len(circles) > 0:
                        for c in circles:
                            cv_cx, cv_cy = c.x(), c.y()
                            if not on_track(cv_cy):
                                continue
                            cv_dist = abs(cv_cx - last_cx) + abs(cv_cy - last_cy)
                            cv_score = 30 - cv_dist * 0.3
                            if cv_score > best_score:
                                best_score = cv_score
                                ball_cx, ball_cy, max_conf = cv_cx, cv_cy, 0.3
                                ball_found = True
                except:
                    pass

            if ball_found:
                lost = 0
                last_cx, last_cy = ball_cx, ball_cy
            else:
                lost += 1
                if lost <= HOLD_FRAMES:
                    ball_cx, ball_cy = last_cx, last_cy
                    ball_found = True

            if ball_found:
                offset_px = ball_cx - CENTER_X  # right=positive
                offset_cm = offset_px / PX_PER_CM
                offset_cm = prev_offset_cm * 0.3 + offset_cm * 0.7
                prev_offset_cm = offset_cm
                send_frame(offset_cm, True)
            else:
                prev_offset_cm = 0.0
                send_frame(0.0, False)

            if frame_cnt % 30 == 0:
                if ball_found:
                    print("OK[%d] cx=%.0f cy=%.0f cm=%.1f rej=%d" %
                          (frame_cnt, ball_cx, ball_cy, prev_offset_cm, rejected))
                else:
                    print("LOST[%d] lost=%d rej=%d" % (frame_cnt, lost, rejected))

            det.draw_result(pl.osd_img, res)

            mot = CENTER_Y - TRACK_Y_TOL
            mob = CENTER_Y + TRACK_Y_TOL
            pl.osd_img.draw_rectangle(0, 0, 640, mot, color=(180, 0, 0, 0), fill=True)
            pl.osd_img.draw_rectangle(0, mob, 640, 480 - mob, color=(180, 0, 0, 0), fill=True)

            # track line boundary
            pl.osd_img.draw_rectangle(0, CENTER_Y - TRACK_Y_TOL, 640,
                                      TRACK_Y_TOL * 2,
                                      color=(255, 255, 200, 0), thickness=1)

            for cm_val in [-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5]:
                tick_x = int(CENTER_X + cm_val * PX_PER_CM)
                if 0 <= tick_x < rgb888p_size[0]:
                    color = (255, 0, 255, 0) if cm_val == 0 else (255, 200, 200, 200)
                    h = 16 if cm_val == 0 else 10
                    pl.osd_img.draw_line(tick_x, CENTER_Y-h, tick_x, CENTER_Y+h, color=color, thickness=1)
                    if abs(cm_val) == 5:
                        pl.osd_img.draw_string_advanced(tick_x-6, CENTER_Y-26, 11, "%+d" % cm_val, color=(255, 200, 200, 200))

            cs = 24
            pl.osd_img.draw_line(CENTER_X-cs, CENTER_Y, CENTER_X+cs, CENTER_Y, color=(255, 0, 255, 0), thickness=2)
            pl.osd_img.draw_line(CENTER_X, CENTER_Y-cs, CENTER_X, CENTER_Y+cs, color=(255, 0, 255, 0), thickness=2)
            pl.osd_img.draw_circle(CENTER_X, CENTER_Y, 5, color=(255, 0, 255, 0), fill=True)

            if ball_found:
                tag = "HOLD%d" % lost if lost > 0 else "OK"
                d_str = "%s DIST:%+.1fcm c:%.2f" % (tag, prev_offset_cm, max_conf)
                pl.osd_img.draw_string_advanced(8, display_size[1]-36, 18, d_str, color=(255, 0, 255, 0))
                pl.osd_img.draw_line(int(ball_cx), CENTER_Y-8, CENTER_X, CENTER_Y, color=(255, 255, 255, 0), thickness=2)
            else:
                pl.osd_img.draw_string_advanced(8, display_size[1]-36, 20, "LOST", color=(255, 255, 100, 0))

            pl.show_image()
            gc.collect()

    except KeyboardInterrupt:
        print("[INFO] User stop")
    except Exception as e:
        import sys
        sys.print_exception(e)
    finally:
        send_frame(0.0, False)
        time.sleep_ms(50)
        if det is not None:
            det.deinit()
        if pl is not None:
            pl.destroy()
        print("[INFO] Exit")


if __name__ == "__main__":
    main()
