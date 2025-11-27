# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

EPDiy is an ESP-IDF component library for driving parallel e-paper displays on ESP32 microcontrollers. It supports multiple board variants (V2-V7, LilyGo T5-4.7, M5Stack PaperS3) and various E-Ink display models (ED060SC4, ED097OC4, ED097TC2, ED133UT2, etc.).

The library targets use with M5Stack PaperS3 on ESPHome using the ESP-IDF platform.

## Build System

This is an **ESP-IDF component** that uses CMake. It is not a standalone application.

### Integration as Component

To use this library in an ESP-IDF project:
- Place in the `components/` directory of an ESP-IDF project
- Or reference via `idf_component.yml` (ESP Component Manager)
- The component will be automatically discovered by ESP-IDF's build system

### Testing

```bash
# The test/ directory contains unit tests that can be built as part of an ESP-IDF test app
# Tests require the unity framework (part of ESP-IDF)
```

### Code Formatting

```bash
# Format all source files using clang-format
make format

# Check formatting without modifying files
make format-check
```

### Waveform Generation

```bash
# Generate default waveform headers for all supported displays
make

# Generate waveform for a specific display (e.g., ED097TC2)
make src/waveforms/epdiy_ED097TC2.h

# Clean generated waveform files
make clean
```

## Architecture

### Layer Structure

The codebase is organized into distinct layers:

1. **Board Abstraction Layer** (`src/board/`)
   - Hardware-specific implementations for different EPDiy board versions
   - Power management (TPS65185 PMIC driver)
   - I/O expansion (PCA9555 GPIO expander)
   - Each board variant implements the `EpdBoardDefinition` interface

2. **Display Definitions** (`src/displays.c`, `src/epd_display.h`)
   - Display-specific configurations (dimensions, bus width, waveforms)
   - Supported displays: ED060SCT, ED060XC3, ED097OC4, ED097TC2, ED133UT2, ED047TC1, ED047TC2, ED078KC1, ED052TC4

3. **Output Drivers**
   - **I2S-based output** (`src/output_i2s/`): Used on V2-V6 boards, drives parallel bus using I2S peripheral with RMT for timing
   - **LCD-based output** (`src/output_lcd/`): Used on V7+ boards, uses ESP32-S3 LCD peripheral for higher performance
   - Common rendering logic (`src/output_common/`): LUT management, line queuing, render context

4. **Core Drawing API** (`src/epdiy.c`, `src/epdiy.h`)
   - Low-level drawing primitives (pixels, lines, circles, rectangles, triangles)
   - Framebuffer operations with 4-bit grayscale (2 pixels per byte, MODE_PACKING_2PPB)
   - Text rendering with custom font format (compressed glyphs, unicode intervals)
   - Base `epd_draw_base()` function with extensive mode flags and waveform support

5. **High-Level API** (`src/highlevel.c`, `src/epd_highlevel.h`)
   - Double-buffered framebuffer management
   - Automatic difference calculation for partial updates
   - Simplified update API: `epd_hl_update_screen()`, `epd_hl_update_area()`

### Rendering Flow

1. **Initialization**: `epd_init()` with board definition, display model, and options (LUT size, queue size)
2. **Drawing**: Use drawing functions to modify a framebuffer in memory
3. **Update**: Call `epd_draw_base()` (low-level) or `epd_hl_update_screen()` (high-level) to push to display
4. **Waveform Selection**: Drawing modes (MODE_GC16, MODE_GL16, MODE_DU, etc.) control update quality/speed

### ESP-IDF Version Compatibility

The CMakeLists.txt handles compatibility between ESP-IDF v4 and v5+:
- **IDF v5+**: Requires `driver`, `esp_timer`, `esp_adc`, `esp_lcd` components
- **IDF v4**: Requires `esp_adc_cal`, `esp_timer`, `esp_lcd` components

The codebase uses conditional compilation and backports where necessary (see `src/output_lcd/idf-4-backports.h`).

**ESP-IDF 5.5+ Compatibility**: The [lcd_driver.c](src/output_lcd/lcd_driver.c) file has been updated to support ESP-IDF 5.5.x with:
- `gpio_hal_iomux_func_sel` → `esp_rom_gpio_iomux_func_sel` mapping
- `__DECLARE_RCC_ATOMIC_ENV` macro compatibility shim
- GDMA API migration: `gdma_new_channel` → `gdma_new_ahb_channel`, `gdma_set_transfer_ability` → `gdma_config_transfer`

### Important Quirks

- **Framebuffer Format**: Default is MODE_PACKING_2PPB (2 pixels per byte, upper nibble = left pixel). A byte cannot wrap across rows; odd-width images need padding nibbles.
- **Color Values**: Only upper 4 bits matter. Use 0x00 (black) to 0xF0 (white). Values like 0x0F appear black!
- **Rotation**: Software rotation affects drawing/font functions via `epd_set_rotation()`, impacting coordinate interpretation
- **PSRAM Required**: The high-level API allocates large buffers (front/back/difference framebuffers) in external PSRAM
- **Power Management**: Always call `epd_poweron()` before drawing and `epd_poweroff()` after to manage display power

## File Organization

### Key Headers (Public API)
- [epdiy.h](src/epdiy.h) - Core drawing API, data types (EpdRect, draw modes, errors)
- [epd_highlevel.h](src/epd_highlevel.h) - High-level double-buffered API
- [epd_board.h](src/epd_board.h) - Board hardware abstraction interface
- [epd_display.h](src/epd_display.h) - Display model definitions
- [epd_internals.h](src/epd_internals.h) - Waveform and font internal structures

### Scripts (`scripts/`)
- `fontconvert.py` - Convert TTF fonts to EPDiy's compressed glyph format
- `imgconvert.py` - Convert images to 4-bit framebuffer arrays
- `waveform_hdrgen.py` - Generate C headers from waveform JSON files
- `epdiy_waveform_gen.py` - Generate epdiy-specific waveform JSON

### Output Drivers
- [render_i2s.c](src/output_i2s/render_i2s.c) - I2S parallel output implementation
- [render_lcd.c](src/output_lcd/render_lcd.c) - LCD peripheral output implementation
- Both implement the same `EpdRenderMethod` interface defined in [render_method.h](src/output_common/render_method.h)

## Common Development Patterns

### Adding Support for a New Board

1. Create new file in `src/board/epd_board_<name>.c`
2. Implement all functions in `EpdBoardDefinition` struct
3. Export the board definition with `extern const EpdBoardDefinition epd_board_<name>;` in [epd_board.h](src/epd_board.h)
4. Add source file to `app_sources` in [CMakeLists.txt](CMakeLists.txt)

### Adding Support for a New Display

1. Add waveform JSON in `src/waveforms/epdiy_<model>.json` (or generate with `epdiy_waveform_gen.py`)
2. Generate header: `make src/waveforms/epdiy_<model>.h`
3. Add display definition in [displays.c](src/displays.c) with dimensions and waveform reference
4. Export in [epd_display.h](src/epd_display.h)

### Custom Waveforms

Waveforms control the voltage sequences applied to pixels for different grayscale transitions. The default waveforms are in `src/waveforms/epdiy_*.h`. Custom waveforms must follow the `EpdWaveform` structure defined in [epd_internals.h](src/epd_internals.h).

## Important Constants

- `MINIMUM_FRAME_TIME`: 12ms minimum for proper particle settling
- `MONOCHROME_FRAME_TIME`: 120 (in 1/10 µs units) for monochrome mode
- LUT sizes: 1K (EPD_LUT_1K) or 64K (EPD_LUT_64K, default)
- Feed queue sizes: 8 lines (EPD_FEED_QUEUE_8) or 32 lines (EPD_FEED_QUEUE_32, default)

## License

- Firmware and examples: LGPL-3.0-or-later
- Utilities: MIT
- Board schematics: CC BY-SA 4.0
