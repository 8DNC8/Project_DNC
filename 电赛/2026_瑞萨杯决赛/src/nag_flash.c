#include "nag_flash.h"

nag_flash_data_union nag_flash_union_buffer[NAG_FLASH_PAGE_LENGTH];

/* RAM 妯℃嫙鐨?Flash锛氭瘡椤?NAG_FLASH_PAGE_LENGTH 涓?uint32锛屾摝闄ゅ悗涓?0xFFFFFFFF */
static uint32_t nag_flash_ram[NAG_FLASH_PAGE_NUM][NAG_FLASH_PAGE_LENGTH];

void nag_flash_init(void)
{
    for(uint32_t p = 0; p < NAG_FLASH_PAGE_NUM; p++)
    {
        for(uint32_t i = 0; i < NAG_FLASH_PAGE_LENGTH; i++)
        {
            nag_flash_ram[p][i] = 0xFFFFFFFFU;
        }
    }
    nag_flash_buffer_clear();
}

uint8 nag_flash_check(uint32 sector_num, uint32 page_num)
{
    (void) sector_num;

    if(page_num >= NAG_FLASH_PAGE_NUM)
    {
        return 0;
    }

    for(uint32_t i = 0; i < NAG_FLASH_PAGE_LENGTH; i++)
    {
        if(nag_flash_ram[page_num][i] != 0xFFFFFFFFU)
        {
            return 1;   /* 鏈夋暟鎹?*/
        }
    }
    return 0;           /* 绌虹櫧 */
}

void nag_flash_erase_page(uint32 sector_num, uint32 page_num)
{
    (void) sector_num;

    if(page_num >= NAG_FLASH_PAGE_NUM)
    {
        return;
    }

    for(uint32_t i = 0; i < NAG_FLASH_PAGE_LENGTH; i++)
    {
        nag_flash_ram[page_num][i] = 0xFFFFFFFFU;
    }
}

void nag_flash_read_page_to_buffer(uint32 sector_num, uint32 page_num, uint32 len)
{
    (void) sector_num;

    if(page_num >= NAG_FLASH_PAGE_NUM)
    {
        return;
    }
    if(len > NAG_FLASH_PAGE_LENGTH)
    {
        len = NAG_FLASH_PAGE_LENGTH;
    }

    for(uint32_t i = 0; i < len; i++)
    {
        nag_flash_union_buffer[i].uint32_type = nag_flash_ram[page_num][i];
    }
}

uint8 nag_flash_write_page_from_buffer(uint32 sector_num, uint32 page_num, uint32 len)
{
    (void) sector_num;

    if(page_num >= NAG_FLASH_PAGE_NUM)
    {
        return 1;   /* 澶辫触 */
    }
    if(len > NAG_FLASH_PAGE_LENGTH)
    {
        len = NAG_FLASH_PAGE_LENGTH;
    }

    for(uint32_t i = 0; i < len; i++)
    {
        nag_flash_ram[page_num][i] = nag_flash_union_buffer[i].uint32_type;
    }
    return 0;       /* 鎴愬姛 */
}

void nag_flash_buffer_clear(void)
{
    for(uint32_t i = 0; i < NAG_FLASH_PAGE_LENGTH; i++)
    {
        nag_flash_union_buffer[i].uint32_type = 0xFFFFFFFFU;
    }
}
