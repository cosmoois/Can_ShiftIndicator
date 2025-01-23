#pragma once

class LGFX_ESP32_SSD1306_I2C : public lgfx::LGFX_Device
{
  lgfx::Panel_SSD1306 _panel_instance;
  lgfx::Bus_I2C       _bus_instance;
public:
  LGFX_ESP32_SSD1306_I2C(int sck, int sda, int i2c_addr = 0x3C, int rst = -1, int vcc = -1, int gnd = -1)
  {
    {
      if (gnd != -1) {
        pinMode(gnd, OUTPUT);
        gpio_set_drive_capability((gpio_num_t)gnd, GPIO_DRIVE_CAP_3);
        digitalWrite(gnd, LOW);
      }
      if (vcc != -1) {
        pinMode(vcc, OUTPUT);
        gpio_set_drive_capability((gpio_num_t)vcc, GPIO_DRIVE_CAP_3);
        digitalWrite(vcc, HIGH);
      }

      {
        auto cfg = _bus_instance.config();

        cfg.pin_scl = sck;
        cfg.pin_sda = sda;
        // cfg.i2c_port = i2c_port;
        cfg.i2c_addr  = i2c_addr;

        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
      }

      {
        auto cfg = _panel_instance.config();

        cfg.offset_rotation = 2;
        cfg.pin_rst = rst;

        _panel_instance.config(cfg);
      }

      setPanel(&_panel_instance);
    }
  }
};
