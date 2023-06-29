#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include <tusb.h>

/*

0xA8-0x3F Set Multiplex Tatio 		ディスプレイのCOM0を起点として表示対象の画像の行数（row）を指定する
0xD3-0x00 Set Display Offset 		表示開始行を0x00~0x3Fの値で設定した行数分移動する
0x40~0x7F Set Display Start Line  	表示開始行に割り当てるGDDRAMのRow（横列）の位置を0～63で設定する
0xA0/0xA1 Set Segment Re-map　		SEGの並び方向とjの並び方向が同じor反対。
0xC0/0xC8 Set COM Output Scan Direction 		通常モード/リマップモードの切り替え。上と似てる、詳しくはデータシートで確認。
0xDA-0x12 Set COM Pins Hardware Conjiguration 	自分のやつは0x12にする。
0x81-0x7F Set Contrast Control for BANK0 		ディスプレイの光度を0x00~0xFFまでの256会長で設定する。（ただし、i2cは256階調のような細かい調節はできない。）
0xA4/0xA5 Entire Display ON 					GDDRAMの内容を表示or全画面点灯
0xA6/0xA7 Set Normal/Inverse Display 			GDDRAM1->表示/GDDRAM1->非表示
0xD5-0x80 Set Display Clock Divide Ratio / Oscillator Frequency 	とりあえずこれで
0x8D-0x14 ChargePump Setting		とりあえずこれ
0xAF/0xAE Set Display ON/OFF 		表示/非表示（スリープモード）


以下の送出動作しかできない
Start -> Address -> 0x00(Co = 0, D/C 0) -> Command -> Command -> Command ... -> Stop
Start -> Address -> 0x40(Co = 0, D/C 1) -> Data -> Data -> Data ... -> Stop
Start -> Address -> 0x80(Co = 1, D/C 0) -> Command -> Next Control Byte
Start -> Address -> 0xC0(Co = 1, D/C 1) -> Data -> Next Control Byte

*/

void display_init() {
	const uint8_t CMD[29] = {
		0x00,0xAE,	// Display OFF 2
		0xA8,0x3F,	// Multiplex Ratio 64 3
		0xD3,0x00,	// Display Offset 0 3
		0x40,		// Display Start Line 0 2
		0xA1,		// Segment Remap is 0 2 
		0xC8,		// Com Output Scan Dir Is normal 2
		0xDA,0x12,	// Com Pin Hardware is Alt COM 3
		0x81,0x7F,	// Set Constrant 7F; 3
		0xA4,		// Entire Display On 2
		0xA6,		// Normal Display 2
		0xD5,0x80,	// DCLK = 0, Fosc = 8 3
		0x20,0x00,	// Set Memory Addressing Mode 20h, Horizontal addressing mode 00h 
		0x21,0x00,0x7F,
		0x22,0x00,0x07,
		0x2E,
        0x8D,0x14,	// Charge Pump is Enabled 3
		0xAF		// Display on. 2
	};

	i2c_write_blocking(i2c0,0x3C,CMD,29,false);

}


void write_all_framebuffer(uint8_t framebuffer[8][129]){
	for(int i = 0; i < 8; i++){
		i2c_write_blocking(i2c0,0x3C,framebuffer[i],129,false);
	}
}


// uint8_t* clear_framebuffer(uint8_t* framebuffer, uint8_t raw, uint8_t j)
void clear_framebuffer(uint8_t framebuffer[8][129]){

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 129; j++){
			if(j == 0){
				framebuffer[i][j] = 0x40;
			}else{
				framebuffer[i][j] = 0x00;
			}

		}
	}
}


void display_figure(uint8_t framebuffer[8][129]){
	// make your framebuffers
	uint8_t num = 0x01;
	uint8_t shift_Flg = 0;
	for(int i = 0; i < 8; i++){
		for(int j = 1; j < 129; j++){
			if(j % 8 == 0){
				framebuffer[i][j] = 0xAA;
			}else{
				framebuffer[i][j] = 0x00;
			}

			if(shift_Flg == 0){
				if(num == 0x01){
					shift_Flg = 1;
					num <<= 1;	
				}else{
					num >>= 1;
				}

			}else if(shift_Flg == 1){
				if(num == 0x80){
					shift_Flg = 0;
					num >>= 1;	
				}else{
					num <<= 1;
				}
			}

			framebuffer[i][j] = num;

		}
	}
}


void wait_for_serial() {
    while(!tud_cdc_connected()) sleep_ms(100);
    printf("tud_cdc_connected()\n");
}


int main(){
	stdio_init_all();

    // Enable UART so we can print status output
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c0, 100 * 1000); // 100kHz
    gpio_set_function(12, GPIO_FUNC_I2C);
    gpio_set_function(13, GPIO_FUNC_I2C);
    gpio_pull_up(12);
    gpio_pull_up(13);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(12, 13, GPIO_FUNC_I2C));

    display_init();

	// clear framebuffer
	uint8_t framebuffer[8][129];

	clear_framebuffer(framebuffer);
	for(int i = 0; i < 8; i++){
    	i2c_write_blocking(i2c0,0x3C,framebuffer[i],129,false);
	}

	const uint8_t rokutosei_char[55] = {
		0x40,
		0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 
		0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,
		0x7F, 0x08, 0x14, 0x22, 0x41, 0x00,
		0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00,
		0x01, 0x01, 0x7F, 0x01, 0x01, 0x00,
		0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00,
		0x46, 0x49, 0x49, 0x49, 0x31, 0x00,
		0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 
		0x00, 0x41, 0x7F, 0x41, 0x00, 0x00
	};

	//display framebuffer
	while(1){
		int display_cnt = 0;

		clear_framebuffer(framebuffer);

		int tmp_raw = 0;
		int tmp_column = 0; 
		/*

		・1ループにつき、128回の書き込みを8回する -> 1ループにつき1回、全ての行の更新をする
		・1サイクルにつき 20column ずつ文字が横に移動する
			・Memory Addressing Mode は Horizontal addressing mode をつかう。 (水平にアドレスを指定する)

			・whileループ 開始
			
			・columnカウントは 0 で初期化。
			・rawカウントは 0 で初期化。

			・描画処理
				
				・描画した文字の最前列の座標（column と raw) の記録
					・書き込みを1回するごとに
						・columnカウント が 128未満の時
							・columnカウントを +1 する
						・columnカウント が 128の時
							・rawカウントが7未満なら
								・columnカウント を 0 にする。
								・rawカウントを +1 する。
							・rawカウントが7なら
								・columnカウント を 0 にする。
								・rawカウントを 0 にする。

			・オフセット処理
				・columnカウント を +20 する。
				・columnカウント が 128以下の時
					・カウントはそのまま
				・columnカウント が 128を超えた時
					・rawカウントが7未満なら
						・columnカウント を columnカウントの合計 - 128 にする。
						・rawカウントを +1 する。
					・rawカウントが7なら
						・columnカウント を columnカウントの合計 - 128 にする。
						・rawカウントを 0 にする。

			・whileループ 終了

		*/
		for(int i = 0, i < 8, i++){
			for(int j = 1, j < 129, j++){
				// i,j が 文字の先頭位置なら
				if(i == display_cnt * 20 / 128 && j == display_cnt * 20 % 128){
					for(int k = 0, k < 55, k++){
						if(j + k < 129){
							framebuffer[i][j + k] = rokutosei_char[k];
							tmp_raw = i;
							tmp_column = j + k;
						}else{
							framebuffer[i + 1][k] = rokutosei_char[k];
							tmp_raw = i + 1;
							tmp_column = k;
						} 

					for(int l = tmp_raw, l < 8, l++){
						for(int m = tmp_column, m < 129, m++){
							framebuffer[l][m] = 0x00;
						}
					}
					
					break;
					}
				}else{
					framebuffer[i][j] = 0x00;
				}
			}
		}
		display_cnt += 1;

		write_all_framebuffer(framebuffer);

		i2c_write_blocking(i2c0,0x3C,rokutosei_char,55,false);		
		sleep_ms(1200);

/*
		sleep_ms(1200);

		display_figure(framebuffer);
		write_all_framebuffer(framebuffer);

		sleep_ms(1200);
		
		clear_framebuffer(framebuffer);
    	for(int i = 0; i < 8; i++){
    		if(i == 7){
    			i2c_write_blocking(i2c0,0x3C,framebuffer[i],129 - 34,false);
    		}else{
    			i2c_write_blocking(i2c0,0x3C,framebuffer[i],129,false);
			}
		}	
*/
	}

	return 0;

}