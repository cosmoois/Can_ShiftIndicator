// -- board select
// #define D1mini_ESP32        // MH-ET LIVE MiniKit for ESP32 + SSD1306 0.96inch 128x64(I2C)
// #define LILYGO_TDisplay     // LILYGO T-Display MIC
#define M5STAMPC3_13        // M5Stamp C3 Mate + ST7789 1.3inch 240×240(SPI) 7pin(CSなし)
// #define M5STAMPC3_20        // M5Stamp C3 Mate + ST7789 2.0inch 240×320(SPI) 7pin(BLKなし)

// -- option
// #define DISPLAY_OFF     // 表示不使用(CAN->WiFi転送でのパフォーマンスアップ用等に利用)
// #define WIFI_OFF        // WiFi不使用
// #define CAN_OFF         // CAN不使用
// #define VIEW_DEBUG      // デバッグ表示有効
// #define UNUSE_U8G2FONT  // U8g2フォント不使用
// #define FORCE_DEMO      // 強制デモ表示

// -- 変更不可
#define CAN_PACKETSIZE  8
#if defined (CONFIG_IDF_TARGET_ESP32C3)
#define CAN_OFF         // ESP32-C3はArduinoライブラリで対応していないためCANを無効化する
#endif
