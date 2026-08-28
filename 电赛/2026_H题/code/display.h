#ifndef DISPLAY_H
#define DISPLAY_H

#include "zf_common_headfile.h"

/* page count: 0-status 1-gray 2-line tune 3-K230 4-servo 5-Q4 PID 6-H3 tune */
#define PAGE_NUM    ( 7 )

#define DISP_W      ( 320 )  /* IPS200 landscape width */
#define DISP_H      ( 240 )  /* IPS200 landscape height */

extern uint16 page_id[PAGE_NUM];
extern uint8  now_page;

/* init IPS200 2.0 inch (SPI, direction, font, color, clear, page ID) */
void display_init(void);

/* refresh display content based on current page (call in while loop) */
void display_update(void);

#endif
