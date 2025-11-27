# EPDiy E-Paper Driver
## Use with M5Stack PaperS3 on ESPHome (ESP-IDF Platform)

This fork of the EPDiy library has been updated with **ESP-IDF 5.5.1 compatibility fixes** for use with M5Stack PaperS3 and ESPHome.

## Features

- ✅ **ESP-IDF 5.5.1 Support** - Full compatibility with the latest ESP-IDF framework
- ✅ **M5Stack PaperS3** - Optimized for M5Stack's e-paper display device
- ✅ **ESPHome Integration** - Easy integration with Home Assistant via ESPHome
- ✅ **Backward Compatible** - Works with ESP-IDF 4.x, 5.0-5.4, and 5.5+

## Quick Start with ESPHome

### 1. Add Library to ESPHome Configuration

```yaml
esphome:
  name: m5papers3
  libraries:
    - epidy=https://github.com/Frogy76/epdiy

esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: esp-idf
    version: latest  # Works with ESP-IDF 5.5.1+
```

### 2. Configure Display

```yaml
display:
  - platform: ed047tc1
    id: ed047tc1_display
    # Pin configuration for M5Stack PaperS3
    pwr_pin: GPIO45
    bst_en_pin: GPIO46
    xstl_pin: GPIO13
    xle_pin: GPIO15
    spv_pin: GPIO17
    ckv_pin: GPIO18
    pclk_pin: GPIO16
    d0_pin: GPIO6
    d1_pin: GPIO14
    d2_pin: GPIO7
    d3_pin: GPIO12
    d4_pin: GPIO9
    d5_pin: GPIO11
    d6_pin: GPIO8
    d7_pin: GPIO10
    rotation: 90
    update_interval: never
    lambda: |-
      it.print(10, 10, id(my_font), "Hello World!");
```

### 3. Control from Home Assistant

Create text entities to control display content:

```yaml
text:
  - platform: template
    name: "Display Line 1"
    id: textline01
    optimistic: true
    max_length: 26
    on_value:
      - component.update: ed047tc1_display
```

Then update from Home Assistant:

```yaml
service: text.set_value
target:
  entity_id: text.display_line_1
data:
  value: "Your message here"
```

## ESP-IDF 5.5.1 Compatibility

This fork includes fixes for ESP-IDF 5.5.1 breaking changes:

- **gpio_hal_iomux_func_sel** → Replaced with inline function using `PIN_FUNC_SELECT`
- **__DECLARE_RCC_ATOMIC_ENV** → Compatibility shim for removed RCC atomic macro
- **GDMA API** → Handles deprecated API warnings (still functional in 5.5.1)

See [lcd_driver.c](src/output_lcd/lcd_driver.c) for implementation details.

## Example Configuration

A complete example configuration is available in [papers3.yaml](papers3.yaml) showing:
- Display setup for M5Stack PaperS3
- 12 text lines with Home Assistant integration
- Touch screen support (GT911)
- Battery monitoring
- RTC (PCF8563/BM8563) with time sync

## Requirements

- ESP32-S3 microcontroller (M5Stack PaperS3)
- ESPHome 2024.11.0 or newer
- Home Assistant (for remote control)
- PSRAM (required for framebuffer allocation)

## Supported Displays

- ED047TC1 (4.7" - M5Stack PaperS3)
- ED060SC4, ED060SCT, ED060XC3 (6")
- ED097OC4, ED097TC2 (9.7")
- ED133UT2 (13.3")

## Troubleshooting

### Compilation Errors with ESP-IDF 5.5+

If you encounter compilation errors, ensure you're using this fork with the compatibility fixes. The original epdiy repository may not yet support ESP-IDF 5.5+.

### Display Not Updating

- Check that `system_initialized` is true (waits 5s after boot)
- Verify `update_interval: never` is set in display config
- Ensure text changes trigger `component.update: ed047tc1_display`

### GDMA Deprecation Warnings

Deprecation warnings for `gdma_new_channel` and `gdma_set_transfer_ability` are harmless - the old API still works in ESP-IDF 5.5.1.

## Original Project

This is a fork of the original [EPDiy project by vroland](https://github.com/vroland/epdiy) with ESP-IDF 5.5.1 compatibility patches.

Licenses
--------

The weather example is Copyright (c) David Bird 2018 (except for minor modifications). Please refer to `examples/weather/README.md` for details.

The board and schematic are licensed under a <a rel="license" href="https://creativecommons.org/licenses/by-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-sa/4.0/80x15.png" /></a> <a rel="license" href="https://creativecommons.org/licenses/by-sa/4.0/">Creative Commons Attribution-ShareAlike 4.0 International License</a>.

Firmware and remaining examples are licensed under the terms of the GNU Lesser GPL version 3.
Utilities are licensed under the terms of the MIT license.
