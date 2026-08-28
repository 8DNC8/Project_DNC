#ifndef NAG_FLASH_H_
#define NAG_FLASH_H_

#include "zf_common_headfile.h"

/*
 * 鎯瀛樺偍灞傘€? *
 * 鎺ュ彛瀵归綈閫愰 zf_driver_flash锛坒lash_check / flash_erase_page /
 * flash_write_page_from_buffer / flash_read_page_to_buffer / flash_buffer_clear锛夛紝
 * 鐩墠鐢?RAM 瀹炵幇锛堟柇鐢典涪澶憋級銆傚悗缁帴鍏ヨ姱鐗囦唬鐮?Flash 鏃讹紝鍙浛鎹?nag_flash.c 鐨? * 鍐呴儴瀹炵幇锛屾儻瀵奸€昏緫锛坣ag_navigation.c锛変笉闇€瑕佹敼鍔ㄣ€? *
 * 椤垫ā鍨嬩笌 CYT4BB7 鍙傝€冨伐绋嬩竴鑷达細
 *   - 姣忛〉 512 涓?uint32锛?KB锛? *   - 鏁版嵁椤典粠 95 寰€涓嬪啓锛岀 1 椤靛瓨鍏冩暟鎹紙鎬荤偣鏁帮級
 *   - 鎿﹂櫎鍚庡唴瀹逛负 0xFFFFFFFF
 */

#define NAG_FLASH_PAGE_LENGTH   (512U)   /* 姣忛〉 uint32 涓暟 */
#define NAG_FLASH_PAGE_NUM      (96U)    /* 椤垫暟锛?~95 */

typedef union
{
    float   float_type;
    uint32  uint32_type;
    int32   int32_type;
    uint16  uint16_type;
    int16   int16_type;
    uint8   uint8_type;
    int8    int8_type;
} nag_flash_data_union;

extern nag_flash_data_union nag_flash_union_buffer[NAG_FLASH_PAGE_LENGTH];

uint8  nag_flash_check(uint32 sector_num, uint32 page_num);
void   nag_flash_erase_page(uint32 sector_num, uint32 page_num);
void   nag_flash_read_page_to_buffer(uint32 sector_num, uint32 page_num, uint32 len);
uint8  nag_flash_write_page_from_buffer(uint32 sector_num, uint32 page_num, uint32 len);
void   nag_flash_buffer_clear(void);
void   nag_flash_init(void);

#endif /* NAG_FLASH_H_ */
