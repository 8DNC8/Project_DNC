#include "vest_follow.h"

#include <string.h>

/* 鎵弿姝ラ暱锛氭瘡闅?N 鍍忕礌閲囨牱涓€娆★紝鍑忓皯璁＄畻閲忥紙4 = 80x60 缃戞牸锛孊FS 鏇村揩锛?*/
#define VEST_FOLLOW_SCAN_STEP              (4U)

/* 鍖呭洿妗嗗昂瀵哥害鏉?*/
#define VEST_FOLLOW_MIN_WIDTH_PIXELS       (8U)
#define VEST_FOLLOW_MIN_HEIGHT_PIXELS      (8U)
#define VEST_FOLLOW_MAX_ASPECT_X100        (320U)   /* 瀹介珮姣斾笂闄?x100 */
#define VEST_FOLLOW_MIN_ASPECT_X100        (25U)    /* 瀹介珮姣斾笅闄?x100 */
#define VEST_FOLLOW_MIN_FILL_X100          (18U)    /* 鏈€灏忓～鍏呯巼 x100 */

/* EMA 骞虫粦绯绘暟锛氭棫鍊?70%锛屾柊鍊?30% */
#define VEST_FOLLOW_SMOOTH_OLD_X100        (70U)
#define VEST_FOLLOW_SMOOTH_NEW_X100        (30U)

/* 缁樺埗棰滆壊 */
#define VEST_FOLLOW_BOX_COLOR              (RGB565_RED)    /* 璺熻釜涓紙鏈攣瀹氾級绾㈣壊 */
#define VEST_FOLLOW_LOCK_COLOR             (RGB565_GREEN)  /* 宸查攣瀹氱豢鑹?*/

/* 闄嶉噰鏍风綉鏍煎昂瀵?*/
#define VEST_FOLLOW_GRID_W                 (SCC8660_W / VEST_FOLLOW_SCAN_STEP)
#define VEST_FOLLOW_GRID_H                 (SCC8660_H / VEST_FOLLOW_SCAN_STEP)
#define VEST_FOLLOW_GRID_SIZE              (VEST_FOLLOW_GRID_W * VEST_FOLLOW_GRID_H)

/*
 * 鑽у厜鑳屽績妫€娴嬮槇鍊硷紙RGB565 棰滆壊绌洪棿锛屽厤 HSV 杞崲锛屽姞蹇?RA8 澶勭悊閫熷害锛? * 鑽у厜鑳屽績閫氬父楂?G 鍒嗛噺銆侀粍缁胯壊璋? */
#define VEST_FOLLOW_GREEN_MIN              (34U)    /* G 鍒嗛噺鏈€灏忓€?*/
#define VEST_FOLLOW_RED_MIN                (10U)    /* R 鍒嗛噺鏈€灏忓€?*/
#define VEST_FOLLOW_BLUE_MAX               (22U)    /* B 鍒嗛噺鏈€澶у€?*/
#define VEST_FOLLOW_GREEN_RED_MARGIN       (4)      /* G-R 鏈€灏忓樊鍊?*/
#define VEST_FOLLOW_GREEN_BLUE_MARGIN      (12)     /* G-B 鏈€灏忓樊鍊?*/
#define VEST_FOLLOW_BRIGHTNESS_MIN         (42U)    /* 鏈€浣庝寒搴?R+G+B */

/* 闈欐€佸伐浣滅紦鍐插尯锛堥伩鍏?malloc锛屽噺灏戞爤鍘嬪姏锛?*/
static vest_follow_target_t g_vest_target;
static uint8_t g_vest_mask[VEST_FOLLOW_GRID_SIZE];
static uint8_t g_vest_visited[VEST_FOLLOW_GRID_SIZE];
static uint16_t g_vest_queue[VEST_FOLLOW_GRID_SIZE];

/* 鍒ゆ柇鍗曚釜鍍忕礌鏄惁涓鸿崸鍏夎儗蹇冪豢鑹诧紙鏀寔瀛楄妭搴忎氦鎹級 */
static bool vest_follow_pixel_is_green(uint16_t pixel, bool byte_swapped)
{
    /* 濡傛灉鎽勫儚澶磋緭鍑哄瓧鑺傚簭涓庨鏈熺浉鍙嶏紝浜ゆ崲楂樹綆瀛楄妭 */
    if(byte_swapped)
    {
        pixel = (uint16_t)((pixel >> 8) | (pixel << 8));
    }

    /* 浠?RGB565 鎻愬彇 R/G/B 鍒嗛噺 */
    uint16_t r = (uint16_t)((pixel >> 11) & 0x1FU);
    uint16_t g = (uint16_t)((pixel >> 5) & 0x3FU);
    uint16_t b = (uint16_t)(pixel & 0x1FU);
    uint16_t brightness = (uint16_t)(r + g + b);

    /* 婊¤冻鎵€鏈夐槇鍊兼潯浠舵墠绠楄儗蹇冨儚绱?*/
    return (g >= VEST_FOLLOW_GREEN_MIN) &&
           (r >= VEST_FOLLOW_RED_MIN) &&
           (b <= VEST_FOLLOW_BLUE_MAX) &&
           (((int16_t)g - (int16_t)r) >= VEST_FOLLOW_GREEN_RED_MARGIN) &&
           (((int16_t)g - (int16_t)b) >= VEST_FOLLOW_GREEN_BLUE_MARGIN) &&
           (brightness >= VEST_FOLLOW_BRIGHTNESS_MIN);
}

/* 楠岃瘉杩為€氬煙鏄惁鍚堟硶锛堥潰绉€佸楂樸€佸楂樻瘮銆佸～鍏呯巼閮藉湪鍚堢悊鑼冨洿鍐咃級 */
static bool vest_follow_component_is_valid(vest_follow_target_t const *candidate)
{
    if((candidate->area < VEST_FOLLOW_MIN_AREA_PIXELS) ||
       (candidate->area > VEST_FOLLOW_MAX_AREA_PIXELS) ||
       (candidate->width < VEST_FOLLOW_MIN_WIDTH_PIXELS) ||
       (candidate->height < VEST_FOLLOW_MIN_HEIGHT_PIXELS))
    {
        return false;
    }

    uint32_t aspect_x100 = ((uint32_t)candidate->width * 100U) / candidate->height;
    uint32_t fill_x100 = (candidate->area * 100U) /
                         ((uint32_t)candidate->width * (uint32_t)candidate->height);

    return (aspect_x100 >= VEST_FOLLOW_MIN_ASPECT_X100) &&
           (aspect_x100 <= VEST_FOLLOW_MAX_ASPECT_X100) &&
           (fill_x100 >= VEST_FOLLOW_MIN_FILL_X100);
}

/* EMA 骞虫粦鍗曞€?*/
static uint16_t vest_follow_smooth_u16(uint16_t old_value, uint16_t new_value)
{
    uint32_t smoothed = ((uint32_t)old_value * VEST_FOLLOW_SMOOTH_OLD_X100) +
                        ((uint32_t)new_value * VEST_FOLLOW_SMOOTH_NEW_X100);
    return (uint16_t)((smoothed + 50U) / 100U);
}

static uint32_t vest_follow_smooth_u32(uint32_t old_value, uint32_t new_value)
{
    uint32_t smoothed = (old_value * VEST_FOLLOW_SMOOTH_OLD_X100) +
                        (new_value * VEST_FOLLOW_SMOOTH_NEW_X100);
    return (smoothed + 50U) / 100U;
}

/* 淇濆瓨鍘熷妫€娴嬬粨鏋滐紝鍋?EMA 骞虫粦 + 閿佸畾/涓㈠け璁℃暟 */
static void vest_follow_store_raw(vest_follow_target_t *candidate)
{
    candidate->raw_found = true;

    /* 棣栨妫€娴嬪埌鎴栦箣鍓嶅凡涓㈠け锛氱洿鎺ヨ祴鍊硷紝涓嶅钩婊?*/
    if(!g_vest_target.stable_found && (0U == g_vest_target.lock_count))
    {
        g_vest_target.x = candidate->x;
        g_vest_target.y = candidate->y;
        g_vest_target.width = candidate->width;
        g_vest_target.height = candidate->height;
        g_vest_target.center_x = candidate->center_x;
        g_vest_target.center_y = candidate->center_y;
        g_vest_target.area = candidate->area;
    }
    else
    {
        /* 宸查攣瀹氾細EMA 骞虫粦锛屽噺灏戞姈鍔?*/
        g_vest_target.x = vest_follow_smooth_u16(g_vest_target.x, candidate->x);
        g_vest_target.y = vest_follow_smooth_u16(g_vest_target.y, candidate->y);
        g_vest_target.width = vest_follow_smooth_u16(g_vest_target.width, candidate->width);
        g_vest_target.height = vest_follow_smooth_u16(g_vest_target.height, candidate->height);
        g_vest_target.center_x = vest_follow_smooth_u16(g_vest_target.center_x, candidate->center_x);
        g_vest_target.center_y = vest_follow_smooth_u16(g_vest_target.center_y, candidate->center_y);
        g_vest_target.area = vest_follow_smooth_u32(g_vest_target.area, candidate->area);
    }

    g_vest_target.raw_found = true;
    g_vest_target.lost_count = 0;

    /* 閿佸畾璁℃暟閫掑锛岃揪鍒伴槇鍊煎悗鏍囪涓虹ǔ瀹氶攣瀹?*/
    if(g_vest_target.lock_count < VEST_FOLLOW_LOCK_FRAMES)
    {
        g_vest_target.lock_count++;
    }
    g_vest_target.stable_found = (g_vest_target.lock_count >= VEST_FOLLOW_LOCK_FRAMES);
}

/* 鏍囪褰撳墠甯т涪澶憋紝淇濇寔鑻ュ共甯у悗鎵嶆竻闄ら攣瀹氱姸鎬侊紙闃叉姈锛?*/
static void vest_follow_mark_lost(void)
{
    g_vest_target.raw_found = false;
    g_vest_target.lock_count = 0;

    if(g_vest_target.lost_count < VEST_FOLLOW_LOST_HOLD_FRAMES)
    {
        g_vest_target.lost_count++;
    }
    else
    {
        g_vest_target.stable_found = false;
        g_vest_target.area = 0;
    }
}

/* 缁樺埗鐭╁舰杈规 */
static void vest_follow_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    if((0U == width) || (0U == height))
    {
        return;
    }

    uint16_t x2 = (uint16_t)(x + width - 1U);
    uint16_t y2 = (uint16_t)(y + height - 1U);

    /* 鍥涙潯杈?*/
    ips200_draw_line(x, y, x2, y, color);       /* 涓?*/
    ips200_draw_line(x, y2, x2, y2, color);     /* 涓?*/
    ips200_draw_line(x, y, x, y2, color);       /* 宸?*/
    ips200_draw_line(x2, y, x2, y2, color);     /* 鍙?*/
}

void vest_follow_init(void)
{
    memset(&g_vest_target, 0, sizeof(g_vest_target));
    memset(g_vest_mask, 0, sizeof(g_vest_mask));
    memset(g_vest_visited, 0, sizeof(g_vest_visited));
}

/* BFS 杩為€氬煙鎼滅储锛氬湪闄嶉噰鏍风綉鏍间腑鎵惧埌鏈€澶х殑鍚堟硶鑳屽績鍖哄煙 */
static bool vest_follow_find_largest_component(const uint16 image[SCC8660_H][SCC8660_W],
                                               bool byte_swapped,
                                               vest_follow_target_t *best_target)
{
    memset(g_vest_visited, 0, sizeof(g_vest_visited));

    /* 绗竴姝ワ細闄嶉噰鏍?+ 浜屽€煎寲锛岀敓鎴愯儗蹇冩帺鐮?*/
    for(uint16_t grid_y = 0; grid_y < VEST_FOLLOW_GRID_H; grid_y++)
    {
        uint16_t image_y = (uint16_t)(grid_y * VEST_FOLLOW_SCAN_STEP);
        for(uint16_t grid_x = 0; grid_x < VEST_FOLLOW_GRID_W; grid_x++)
        {
            uint16_t image_x = (uint16_t)(grid_x * VEST_FOLLOW_SCAN_STEP);
            uint16_t index = (uint16_t)((grid_y * VEST_FOLLOW_GRID_W) + grid_x);
            g_vest_mask[index] = vest_follow_pixel_is_green(image[image_y][image_x], byte_swapped) ? 1U : 0U;
        }
    }

    /* 绗簩姝ワ細BFS 鎵炬墍鏈夎繛閫氬煙锛屼繚鐣欓潰绉渶澶х殑鍚堟硶鐩爣 */
    bool found = false;
    vest_follow_target_t best;
    memset(&best, 0, sizeof(best));

    for(uint16_t grid_y = 0; grid_y < VEST_FOLLOW_GRID_H; grid_y++)
    {
        for(uint16_t grid_x = 0; grid_x < VEST_FOLLOW_GRID_W; grid_x++)
        {
            uint16_t start_index = (uint16_t)((grid_y * VEST_FOLLOW_GRID_W) + grid_x);
            if((0U == g_vest_mask[start_index]) || (0U != g_vest_visited[start_index]))
            {
                continue;  /* 涓嶆槸鑳屽績鍍忕礌鎴栧凡璁块棶杩?*/
            }

            /* BFS 鍒濆鍖?*/
            uint16_t head = 0;
            uint16_t tail = 0;
            uint16_t min_grid_x = grid_x;
            uint16_t min_grid_y = grid_y;
            uint16_t max_grid_x = grid_x;
            uint16_t max_grid_y = grid_y;
            uint32_t sample_count = 0;
            uint32_t sum_x = 0;
            uint32_t sum_y = 0;

            g_vest_visited[start_index] = 1U;
            g_vest_queue[tail] = start_index;
            tail++;

            /* BFS 涓诲惊鐜細鍥涢偦鍩熸墿灞?*/
            while(head < tail)
            {
                uint16_t index = g_vest_queue[head];
                head++;

                uint16_t current_grid_y = (uint16_t)(index / VEST_FOLLOW_GRID_W);
                uint16_t current_grid_x = (uint16_t)(index - (current_grid_y * VEST_FOLLOW_GRID_W));
                uint16_t image_x = (uint16_t)(current_grid_x * VEST_FOLLOW_SCAN_STEP);
                uint16_t image_y = (uint16_t)(current_grid_y * VEST_FOLLOW_SCAN_STEP);

                sample_count++;
                sum_x += image_x;
                sum_y += image_y;

                /* 鏇存柊杈圭晫 */
                if(current_grid_x < min_grid_x) { min_grid_x = current_grid_x; }
                if(current_grid_y < min_grid_y) { min_grid_y = current_grid_y; }
                if(current_grid_x > max_grid_x) { max_grid_x = current_grid_x; }
                if(current_grid_y > max_grid_y) { max_grid_y = current_grid_y; }

                /* 妫€鏌ュ洓涓偦灞?*/
                if(current_grid_x > 0U)
                {
                    uint16_t next = (uint16_t)(index - 1U);
                    if((0U != g_vest_mask[next]) && (0U == g_vest_visited[next]))
                    {
                        g_vest_visited[next] = 1U;
                        g_vest_queue[tail] = next;
                        tail++;
                    }
                }
                if((current_grid_x + 1U) < VEST_FOLLOW_GRID_W)
                {
                    uint16_t next = (uint16_t)(index + 1U);
                    if((0U != g_vest_mask[next]) && (0U == g_vest_visited[next]))
                    {
                        g_vest_visited[next] = 1U;
                        g_vest_queue[tail] = next;
                        tail++;
                    }
                }
                if(current_grid_y > 0U)
                {
                    uint16_t next = (uint16_t)(index - VEST_FOLLOW_GRID_W);
                    if((0U != g_vest_mask[next]) && (0U == g_vest_visited[next]))
                    {
                        g_vest_visited[next] = 1U;
                        g_vest_queue[tail] = next;
                        tail++;
                    }
                }
                if((current_grid_y + 1U) < VEST_FOLLOW_GRID_H)
                {
                    uint16_t next = (uint16_t)(index + VEST_FOLLOW_GRID_W);
                    if((0U != g_vest_mask[next]) && (0U == g_vest_visited[next]))
                    {
                        g_vest_visited[next] = 1U;
                        g_vest_queue[tail] = next;
                        tail++;
                    }
                }
            }

            /* 鏋勯€犲€欓€夌洰鏍?*/
            vest_follow_target_t candidate;
            memset(&candidate, 0, sizeof(candidate));
            candidate.x = (uint16_t)(min_grid_x * VEST_FOLLOW_SCAN_STEP);
            candidate.y = (uint16_t)(min_grid_y * VEST_FOLLOW_SCAN_STEP);
            candidate.width = (uint16_t)(((max_grid_x - min_grid_x) + 1U) * VEST_FOLLOW_SCAN_STEP);
            candidate.height = (uint16_t)(((max_grid_y - min_grid_y) + 1U) * VEST_FOLLOW_SCAN_STEP);
            candidate.center_x = (uint16_t)(sum_x / sample_count);
            candidate.center_y = (uint16_t)(sum_y / sample_count);
            candidate.area = sample_count * (uint32_t)(VEST_FOLLOW_SCAN_STEP * VEST_FOLLOW_SCAN_STEP);

            /* 鍚堟硶鎬ф鏌?+ 淇濈暀鏈€澶х殑 */
            if(vest_follow_component_is_valid(&candidate) && (!found || (candidate.area > best.area)))
            {
                best = candidate;
                found = true;
            }
        }
    }

    if(found && (NULL != best_target))
    {
        *best_target = best;
    }

    return found;
}

/* 涓绘洿鏂板嚱鏁帮細鎽勫儚澶村浐瀹氳緭鍑?RGB565 瀛楄妭浜ゆ崲鏍煎紡銆?*/
void vest_follow_update(const uint16 image[SCC8660_H][SCC8660_W])
{
    if(NULL == image)
    {
        vest_follow_mark_lost();
        return;
    }

    vest_follow_target_t target;
    if(vest_follow_find_largest_component(image, true, &target))
    {
        vest_follow_store_raw(&target);
    }
    else
    {
        vest_follow_mark_lost();
    }
}

void vest_follow_get_target(vest_follow_target_t *target)
{
    if(NULL != target)
    {
        *target = g_vest_target;
    }
}

/* 鍦ㄦ樉绀虹敾闈笂缁樺埗鑳屽績鍖呭洿妗嗭紙鍧愭爣鎸夋樉绀哄昂瀵哥缉鏀撅級 */
void vest_follow_draw_overlay(uint16_t display_width, uint16_t display_height)
{
    if((0U == display_width) || (0U == display_height) || !g_vest_target.stable_found)
    {
        return;
    }

    /* 鍧愭爣浠?SCC8660 鍘熷鍒嗚鲸鐜囨槧灏勫埌鏄剧ず鍒嗚鲸鐜?*/
    uint16_t x = (uint16_t)(((uint32_t)g_vest_target.x * display_width) / SCC8660_W);
    uint16_t y = (uint16_t)(((uint32_t)g_vest_target.y * display_height) / SCC8660_H);
    uint16_t width = (uint16_t)(((uint32_t)g_vest_target.width * display_width) / SCC8660_W);
    uint16_t height = (uint16_t)(((uint32_t)g_vest_target.height * display_height) / SCC8660_H);

    if(0U == width)  { width = 1U; }
    if(0U == height) { height = 1U; }
    if((x + width) > display_width)  { width = (uint16_t)(display_width - x); }
    if((y + height) > display_height) { height = (uint16_t)(display_height - y); }

    /* 閿佸畾鐢ㄧ豢鑹插弻妗嗭紝鏈攣瀹氱敤绾㈣壊鍗曟 */
    uint16_t color = g_vest_target.raw_found ? VEST_FOLLOW_LOCK_COLOR : VEST_FOLLOW_BOX_COLOR;
    vest_follow_draw_rect(x, y, width, height, color);
    if((width > 8U) && (height > 8U))
    {
        /* 鍐呮锛氱缉杩?1 鍍忕礌锛屽舰鎴愬弻绾挎晥鏋?*/
        vest_follow_draw_rect((uint16_t)(x + 1U), (uint16_t)(y + 1U),
                              (uint16_t)(width - 2U), (uint16_t)(height - 2U), color);
    }
}
