/*
 * Menu.h
 *
 *  Created on: 2026楠?閺?4閺?
 *      Author: Super_burger
 */

#ifndef CODE_MENU_H_
#define CODE_MENU_H_



//閸戣姤鏆熸竟鐗堟
void  Menu(void);//閼挎粌宕熼崙鑺ユ殶



//缂佹挻鐎担鎾筹紣閺?
typedef struct
{
    int current;
    int up;//閸氭垳绗傜紙鑽ゅ偍瀵洖褰?
    int down;//閸氭垳绗呯紙鑽ゅ偍瀵洖褰?
    int enter;//绾喛顓荤槐銏犵穿閸?
    void (*current_operation)();//瑜版挸澧犳い鐢告桨閻ㄥ嫮鍌ㄥ鏇炲娇鐟曚焦澧界悰宀€娈戦弰鍓с仛閸戣姤鏆熼敍宀冪箹閺勵垯绔存稉顏勫毐閺佺増瀵氶柦?
}key_table;



extern  key_table table[100];
extern float step_set;
extern float step_3;
extern void fun_a1();
extern void fun_b1();
extern void fun_c1();
extern void fun_d1();
extern void fun_e1();
extern void fun_f1();

///////////////////////////////////////////////

extern void fun_a21();
extern void fun_a22();
extern void fun_a23();
extern void fun_a24();
extern void fun_a25();
extern void fun_a26();

extern void fun_b21();
extern void fun_b22();
extern void fun_b23();
extern void fun_b24();
extern void fun_b25();
extern void fun_b26();

extern void fun_c21();
extern void fun_c22();
extern void fun_c23();
extern void fun_c24();
extern void fun_c25();
extern void fun_c26();

extern void fun_d21();
extern void fun_d22();
extern void fun_d23();
extern void fun_d24();
extern void fun_d25();
extern void fun_d26();

extern void fun_e21();
extern void fun_e22();
extern void fun_e23();
extern void fun_e24();
extern void fun_e25();
extern void fun_e26();

//////////////////////////////////////////////

extern void fun_a31();
extern void fun_a32();
extern void fun_a33();
extern void fun_a34();
extern void fun_a35();

extern void fun_b31();
extern void fun_b32();
extern void fun_b33();
extern void fun_b34();
extern void fun_b35();

extern void fun_c31();
extern void fun_c32();
extern void fun_c33();
extern void fun_c34();
extern void fun_c35();

extern void fun_d31();
extern void fun_d32();
extern void fun_d33();
extern void fun_d34();
extern void fun_d35();

extern void fun_e31();
extern void fun_e32();
extern void fun_e33();
extern void fun_e34();
extern void fun_e35();



extern void fun_0();

extern int SUB_flag;
void Sub_select(int sub);


#endif /* CODE_MENU_H_ */
