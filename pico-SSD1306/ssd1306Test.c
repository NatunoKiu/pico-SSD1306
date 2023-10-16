#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include <tusb.h>

/*
0xA8-0x3F Set Multiplex Tatio ディスプレイのCOM0を起点として表示対象の画像の行数（row）を指定する
0xD3-0x00 Set Display Offset 表示開始行を0x00~0x3Fの値で設定した行数分移動する
0x40~0x7F Set Display Start Line  表示開始行に割り当てるGDDRAMのRow（横列）の位置を0～63で設定する
0xA0/0xA1 Set Segment Re-map　SEGの並び方向とColumnの並び方向が同じor反対。
0xC0/0xC8 Set COM Output Scan Direction 通常モード/リマップモードの切り替え。上と似てる、詳しくはデータシートで確認。
0xDA-0x12 Set COM Pins Hardware Conjiguration 自分のやつは0x12にする。
0x81-0x7F Set Contrast Control for BANK0 ディスプレイの光度を0x00~0xFFまでの256会長で設定する。（ただし、i2cは256階調のような細かい調節はできない。）
0xA4/0xA5 Entire Display ON GDDRAMの内容を表示or全画面点灯
0xA6/0xA7 Set Normal/Inverse Display GDDRAM1->表示/GDDRAM1->非表示
0xD5-0x80 Set Display Clock Divide Ratio / Oscillator Frequency とりあえずこれで
0x8D-0x14 ChargePump Setting とりあえずこれ
0xAF/0xAE Set Display ON/OFF 表示/非表示（スリープモード）

0x00,10101000,00111111,11010011,0x00,1000000,10100001,11000000,
11011010,00010010,10000001,01111111,10100101,10100110,11010101,10000000,
10001101,00010100,10101111,


以下の送出動作しかできない
Start -> Address -> 0x00(Co = 0, D/C 0) -> Command -> Command -> Command ... -> Stop
Start -> Address -> 0x40(Co = 0, D/C 1) -> Data -> Data -> Data ... -> Stop
Start -> Address -> 0x80(Co = 1, D/C 0) -> Command -> Next Control Byte
Start -> Address -> 0xC0(Co = 1, D/C 1) -> Data -> Next Control Byte


    const uint8_t CMD1[25] = {0x00,0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0x00,
                              0x40,0x8D,0x14,0x20,0x00,0xA1,0xC8,0xDA,
                              0x12,0x81,0xCF,0xD9,0xF1,0xDB,0x40,0xA4,
                              0xA6};
*/


void wait_for_serial() {
    while(!tud_cdc_connected()) sleep_ms(100);
    printf("tud_cdc_connected()\n");
}

int main() {

    const uint8_t CMD[19] = {0x00,0xA8,0x3F,0xD3,0x00,0x40,0xA0,0xC0,
                             0xDA,0x12,0x81,0x7F,0xA5,0xA6,0xD5,0x80,
                             0x8D,0x14,0xAF};
//,0x20,0x00
//    const uint8_t WRITE_RAM[1] = {0x40};

    uint8_t framebuffer[8][129];

    uint8_t framebuffer2[8][128];

    uint8_t dot[8][129];

    uint8_t dot2[8][128];

    uint8_t dotTest[2][129];

    uint8_t dotTest2[129];

    uint8_t dotTest3[3] = {0x40,0xFF,0xFF};

    for(int columun = 0; columun < 129; columun++){
        for(int row = 8; row < 8; row++){
            if(columun == 0){
                framebuffer[row][columun] = 0x00;    
    
            }else{
                framebuffer[row][columun] = 0x00;
            }
        }
    }
/*
    for(int columun = 0; columun < 129; columun++){
        for(int row = 8; row < 8; row++){
            if(columun == 0){
                dot[row][columun] = 0x00;

            }else if(columun < 64){
                dot[row][columun] = 0xFF;    
    
            }else{
                dot[row][columun] = 0x00;
 
            }
        }
    }
*/
    for(int columun = 0; columun < 129; columun++){
        for(int row = 0; row < 2; row++){
            if(columun == 0){
                dotTest[row][columun] = 0x00;

            }else{
                dotTest[row][columun] = 0xFF;
 
            }
        }
    }

    for(int i = 0; i < 129; i++){
        if(i = 0){
            dotTest2[i] = 0x00;
        }else{
            dotTest2[i] = 0xFF;
        }
    }

/*
    for(int columun = 0; columun < 128; columun++){
        for(int row = 0; row < 8; row++){
            framebuffer2[row][columun] = 0x00;
        }
    }

    for(int columun = 0; columun < 128; columun++){
        for(int row = 8; row < 8; row++){
            if(columun < 64){
                dot2[row][columun] = 0xFF;    
    
            }else{
                dot2[row][columun] = 0x00;
 
            }
        }

    }
*/


    // Enable UART so we can print status output
    stdio_init_all();
    sleep_ms(5*1000);

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(16, 17, GPIO_FUNC_I2C));

    //wait_for_serial();
 	
 	//LCD全点灯
    i2c_write_blocking(i2c0,0x3C,CMD,19,false);

    i2c_write_blocking(i2c0,0x3C,dotTest3,3,false);

/*    for(int i = 0; i < 8; i++){
        i2c_write_blocking(i2c0,0x3C,framebuffer[i],1,true);
        i2c_write_blocking(i2c0,0x3C,framebuffer[i]+1,128,false);
    }
*/
/*    for(int i = 0; i < 8; i++){
        i2c_write_blocking(i2c0,0x3C,framebuffer[i],129,false);
    }
*/
/*    for(int i = 0; i < 2; i++){
        i2c_write_blocking(i2c0,0x3C,dotTest[i],129,false);
    }
*/
/*    for(int i = 0; i < 8; i++){
        i2c_write_blocking(i2c0,0x3C,dot[i],129,false);
    }
*/
/*
    i2c_write_blocking(i2c0,0x3C,WRITE_RAM,1,true);

    for(int i = 0; i < 8; i++){
        i2c_write_blocking(i2c0,0x3C,framebuffer2[i],128,true);
    }

    i2c_write_blocking(i2c0,0x3C,WRITE_RAM,1,true);

    for(int i = 0; i < 8; i++){
        i2c_write_blocking(i2c0,0x3C,dot[i],128,true);
    }
*/


 	while(1){

 	}
	return 0;

}