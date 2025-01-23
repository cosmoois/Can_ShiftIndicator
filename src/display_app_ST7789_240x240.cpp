#include "define.h"
#if defined(M5STAMPC3_13)
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "LGFX_ESP32C3_ST7789_SPI.hpp"
#include "display_app.hpp"
#if !defined(UNUSE_U8G2FONT)
#include <clib/u8g2.h>  // フォントのみ利用 https://github.com/olikraus/u8g2/wiki/fntlistall
static const lgfx::U8g2font u8g2font1( u8g2_font_helvR24_tn );
#endif

#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define DISP_GND -1
#define DISP_VCC 4
#define DISP_SCK 5
#define DISP_SDA 6
#define DISP_RES 7
#define DISP_DC  8
#define DISP_BLK 10

#include <stdint.h>
#undef U8G2_FONT_SECTION
#define U8G2_FONT_SECTION(name) 
#include "u8g2_font_FreeSans.h"
static const lgfx::U8g2font u8g2font92(u8g2_font_FreeSansBoldOblique92pt7b);
static const lgfx::U8g2font u8g2font40(u8g2_font_FreeSansBoldOblique40pt7b);

LGFX_ESP32C3_ST7789_SPI display(TFT_WIDTH, TFT_HEIGHT, DISP_SCK, DISP_SDA, DISP_RES, DISP_DC, -1, DISP_BLK, DISP_VCC, DISP_GND);
LGFX_Sprite canvas(&display);

#include "obd2_app.hpp"

int fontsize = 56;
int y1_base = fontsize * 1 - 1; // 1行目ベースライン
int y2_base = fontsize * 2 - 1; // 2行目ベースライン
int y3_base = fontsize * 3 - (fontsize - 31) * 1 - 2; // 3行目ベースライン
int y4_base = fontsize * 4 - (fontsize - 31) * 2 - 2; // 4行目ベースライン
int y5_base = fontsize * 5 - (fontsize - 31) * 3 - 2; // 5行目ベースライン
int y6_base = fontsize * 6 - (fontsize - 31) * 4 - 2; // 6行目ベースライン
int backlight = 16;
bool br_chg_ena = false;
int br_chg_cnt = 0;

extern float fuel_percent_fix;

void DrawCounter(int line, float val, int format, int rx);
void DrawParameter(String PreWord, float value, String SufWord, int warn, int dpt = -1);

void display_init() {
  display.init();
  display.setColorDepth(8);
  display.setRotation(3);
  display.setBrightness(backlight - 1);
  canvas.setColorDepth(8);
  canvas.createSprite(TFT_WIDTH, TFT_HEIGHT);
  canvas.setTextWrap(false);
  canvas.setColor(TFT_WHITE);
  canvas.setTextColor(TFT_WHITE);
  canvas.fillScreen(TFT_NAVY);

  canvas.setFont(&fonts::Font4);
  canvas.setCursor(0, 5);
  canvas.println(" Shift Indicator");
  canvas.println(" ISO 15765-4");
  canvas.println("  (CAN 11/500)");
  canvas.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);  // 描画領域確認用
  canvas.pushSprite(0, 0);
}

void display_inpane_draw(int rpm, int kmh, int shift_pos) {
  int x_base = 180;

  canvas.clear();
  canvas.setTextSize(1);
  canvas.setTextWrap(false);
  canvas.setTextColor(TFT_WHITE);
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

  canvas.setFont(&fonts::FreeSansBoldOblique12pt7b);
  canvas.setTextDatum(textdatum_t::baseline_left);
  canvas.drawString("rpm", x_base, y1_base);
  canvas.drawString("km/h", x_base, y2_base);

  // シフト位置表示
  canvas.setFont(&u8g2font92);
  canvas.setTextDatum(textdatum_t::baseline_left);
  canvas.fillRect(0, 120, 100, 120, TFT_WHITE);
  canvas.setCursor(0 + 8, y6_base - 10);
  canvas.setTextColor(TFT_BLACK);
  canvas.print(shift_pos);  // 白抜き文字
  canvas.setTextColor(TFT_WHITE);

  canvas.setCursor(120, y3_base);
  DrawParameter("燃料", fuel_percent_fix, "%", 17, 10);
  // DrawParameter("燃料", fuel_liter, "l", 10);
  canvas.setCursor(120, y4_base);
  DrawParameter("DPF", obd2_dpf_gen, "g/l", -99, 100);
  canvas.setCursor(120, y5_base);
  DrawParameter("SOC", obd2_batt_soc, "%", -99);
  canvas.setCursor(120, y6_base);
  DrawParameter("水温", obd2_eng_temp, "℃", -99);

  if (br_chg_cnt > 0) {
    canvas.setTextDatum(textdatum_t::top_right);
    canvas.setFont(&fonts::Font4);
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

  // canvas.drawRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_WHITE);  // レイアウト確認用
  // canvas.drawFastHLine(0, y1_base, TFT_WIDTH, TFT_WHITE); // レイアウト確認用
  // canvas.drawFastHLine(0, y2_base, TFT_WIDTH, TFT_WHITE); // レイアウト確認用
  // canvas.drawFastHLine(0, y3_base, TFT_WIDTH, TFT_WHITE); // レイアウト確認用
  // canvas.drawFastHLine(0, y4_base, TFT_WIDTH, TFT_WHITE); // レイアウト確認用
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

  canvas.setFont(&u8g2font40); num_size = 40;  dot_size = 20;  x_adj = 12;
  line_space = 60;
  y_offset = 4;

  canvas.drawString(str, rx - (strlen(str) * num_size) - (format ? 0 : dot_size) + x_adj, (line * line_space) + y_offset);
}

void DrawParameter(String PreWord, float value, String SufWord, int warn, int dpt)
{
  String str_val;
  int int_val = (int)value;

  canvas.setTextColor(TFT_WHITE);
  if (value == -99) {
    str_val = "--";
  } else {
    if (dpt == -1) {
      str_val = String(int_val);
    } else {
      str_val = String(int_val) + "." + String((int)((value - int_val) * dpt));
    }
    if (value < warn) {
      canvas.setTextColor(TFT_RED, TFT_WHITE);
    }
  }
  canvas.setFont(&fonts::lgfxJapanGothic_20);
  canvas.print(PreWord);
#if !defined(UNUSE_U8G2FONT)
  canvas.setFont(&u8g2font1);
#else
  canvas.setFont(&fonts::Font4);
#endif
  canvas.print(str_val);
  canvas.setFont(&fonts::lgfxJapanGothic_20);
  canvas.print(SufWord);
  canvas.setTextColor(TFT_WHITE);
}
#endif
