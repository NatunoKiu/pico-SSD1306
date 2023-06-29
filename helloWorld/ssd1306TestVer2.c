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
const uint8_t CMD[35] = {
		0x80,0xAE, // Display OFF 2
		0x00,0xA8,0x3F, // Multiplex Ratio 64 3
		0x00,0xD3,0x00, // Display Offset 0 3
		0x80,0x40, // Display Start Line 0 2
		0x80,0xA0, // Segment Remap is 0 2 
		0x80,0xC0,  // Com Output Scan Dir Is normal 2
		0x00,0xDA,0x12, // Com Pin Hardware is Alt COM 3
		0x00,0x81,0x7F, // Set Constrant 7F; 3
		0x80,0xA4, // Entire Display On 2
		0x80,0xA6, // Normal Display 2
		0x00,0xD5,0x80, // DCLK = 0, Fosc = 8 3
        0x00,0x8D,0x14, // Charge Pump is Enabled 3
		0x00,0x20,0x00,
		0x80,0xAF // Display on. 2
};

const uint8_t WRITE_RAM[11] = {0x00,0x21,0x00,0x7F,0x00,0x22,0,7,0x80,0xB0,0x40};

void display_init() {
/*
	i2c_write_blocking(i2c0,0x3C,CMD+0,2,false);
	i2c_write_blocking(i2c0,0x3C,CMD+2,3,false);
	i2c_write_blocking(i2c0,0x3C,CMD+5,3,false);
	i2c_write_blocking(i2c0,0x3C,CMD+8,2,false);
	i2c_write_blocking(i2c0,0x3C,CMD+10,2,false);
	i2c_write_blocking(i2c0,0x3C,CMD+12,2,false);
	i2c_write_blocking(i2c0,0x3C,CMD+14,3,false);
	i2c_write_blocking(i2c0,0x3C,CMD+17,3,false);
	i2c_write_blocking(i2c0,0x3C,CMD+20,2,false);
	i2c_write_blocking(i2c0,0x3C,CMD+22,2,false);
	i2c_write_blocking(i2c0,0x3C,CMD+24,3,false);
	i2c_write_blocking(i2c0,0x3C,CMD+27,3,false);
	i2c_write_blocking(i2c0,0x3C,CMD+30,3,false);
	i2c_write_blocking(i2c0,0x3C,CMD+33,2,false);
*/
	i2c_write_blocking(i2c0,0x3C,CMD,35,false);
}


void wait_for_serial() {
    while(!tud_cdc_connected()) sleep_ms(100);
    printf("tud_cdc_connected()\n");
}

int main() {
	stdio_init_all();
    sleep_ms(5*1000);

    display_init();

    uint8_t framebuffer[8][128];

    for(int column = 0; column < 128; column++){
        for(int row = 8; row < 8; row++){
            framebuffer[row][column] = 0x00;
        }
    }

    // Enable UART so we can print status output


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
	// Init Code
	i2c_write_blocking(i2c0,0x3C,WRITE_RAM,11,true);
    i2c_write_blocking(i2c0,0x3C,framebuffer[0],64,false);

	for(int column = 0; column < 128; column++){
		framebuffer[0][column] = 0xAA; // 市松模様
    }
	
	
	i2c_write_blocking(i2c0,0x3C,WRITE_RAM,11,true);
    i2c_write_blocking(i2c0,0x3C,framebuffer[0],64,false);

 	while(1){

 	}
	return 0;

}