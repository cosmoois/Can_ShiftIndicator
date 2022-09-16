#include "define.h"
#if defined(D1mini_ESP32)
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "LGFX_ESP32_SSD1306_I2C.hpp"
#include "display_app.hpp"

#define TFT_WIDTH  128
#define TFT_HEIGHT 64
#define DISP_GND 5
#define DISP_VCC 23
#define DISP_SCK 19
#define DISP_SDA 18
#define DISP_RST -1

LGFX_ESP32_SSD1306_I2C display(DISP_SCK, DISP_SDA, 0x3C, DISP_RST, DISP_VCC, DISP_GND);
LGFX_Sprite canvas(&display);

int fontsize = 32;
int y1_base = fontsize * 1 - 5; // 1行目ベースライン
int y2_base = fontsize * 2 - 5; // 2行目ベースライン
int backlight = 128;
bool br_chg_ena = false;
int br_chg_cnt = 0;

void DrawCounter(int line, float val, int format, int rx);

void display_init() {
  display.init();
  display.setRotation(0);
  display.setBrightness(backlight - 1);
  canvas.createSprite(TFT_WIDTH, TFT_HEIGHT);
  canvas.setTextWrap(false);
  canvas.setTextColor(0xFFFF); // 図形描画のデフォルト色を白に指定する

  canvas.setFont(&fonts::Font0);
  canvas.setCursor(0, 5);
  canvas.println(" Shift Indicator");
  canvas.println(" ISO 15765-4");
  canvas.println("  (CAN 11/500)");
  canvas.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT);  // 描画領域確認用
  canvas.pushSprite(0, 0);
}

void display_inpane_draw(int rpm, int kmh, int shift_pos) {
  int x_base = 89;

  canvas.clear();
  canvas.setTextSize(1);
  canvas.setTextWrap(false);
  canvas.setTextDatum(textdatum_t::baseline_right);
  canvas.setTextColor(0xFFFF);
  if (br_chg_ena == true) {
    br_chg_ena = false;
    if (backlight == 1) {
      backlight = 256;
    } else {
      backlight /= 2;
    }
    br_chg_cnt = 100;
    display.setBrightness(backlight - 1);
  }

  DrawCounter(0, rpm, 0, x_base);
  DrawCounter(1, kmh, 0, x_base);

  canvas.setFont(&fonts::FreeSansBoldOblique9pt7b);
  canvas.setTextDatum(textdatum_t::baseline_left);
  canvas.drawString("rpm", x_base, y1_base);
  canvas.drawString("kmh", x_base, y2_base);

  // シフト位置表示
  canvas.setFont(&fonts::FreeSansBoldOblique18pt7b);
  canvas.fillRect(0, 32, 24, 32, 0xFFFF);
  canvas.setCursor(0, y2_base);
  canvas.setTextColor(0x0000);
  canvas.print(shift_pos);  // 白抜き文字はdrawStringで表示できない
  canvas.setTextColor(0xFFFF);

  if (br_chg_cnt > 0) {
    canvas.setTextDatum(textdatum_t::top_right);
    canvas.setFont(&fonts::Font2);
    canvas.drawString(String(backlight), TFT_WIDTH, 0);
    br_chg_cnt--;
  }

  if (wificnt > 0) {
    canvas.setTextDatum(textdatum_t::top_right);
    canvas.setFont(&fonts::Font0);
    char wificntbuf[9];
    sprintf(wificntbuf, "%8X", wificnt);
    canvas.drawString(String(wificntbuf), TFT_WIDTH, 0);
  }

  // canvas.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, 0xFFFF); // レイアウト確認用
  // canvas.drawFastHLine(0, y1_base, TFT_WIDTH, 0xFFFF);  // レイアウト確認用
  canvas.pushSprite(0, 0);
}

void display_brightness_shift() {
  br_chg_ena = true;
}

/**
 * @brief カウンタ表示
 * 
 * @param line 描画行
 * @param val 描画値
 * @param format 少数点位置
 * @param rx 右寄せ位置
 */
void DrawCounter(int line, float val, int format, int rx)
{
  String Str_set;
  char str[32];
  int num_size;   // 数字フォント基準幅（フォント毎に固定：調整不可）
  int dot_size;   // 小数点フォント幅（フォント毎に固定：調整不可）
  int x_adj;      // x軸右寄せ調整値（フォント毎に固定：調整不可）

  int line_space; // 行間隔
  int y_offset;   // Y軸オフセット

  if (format == 0) {
    Str_set = String((int)val);
  } else {
    Str_set = String(val, format);
  }
  Str_set.toCharArray(str, sizeof(str));

  canvas.setTextColor(TFT_WHITE);
  canvas.setTextDatum(textdatum_t::top_left);

  canvas.setFont(&fonts::FreeSansBoldOblique18pt7b); num_size = 19;  dot_size = 9;   x_adj = 4;
  line_space = 32;
  y_offset = 2;

  canvas.drawString(str, rx - (strlen(str) * num_size) - (format ? 0 : dot_size) + x_adj, (line * line_space) + y_offset);
}

#endif
