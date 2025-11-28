#!/usr/bin/env python3
"""
Download and convert weather icons for EPDiy e-ink display
Uses simple SVG to PNG conversion and then converts to EPDiy format
"""

import os
import sys
import subprocess
from pathlib import Path

# Weather icon definitions - using simple unicode weather symbols as fallback
# These can be replaced with actual icon downloads
WEATHER_ICONS = {
    'sunny': '☀️',
    'clear-night': '🌙',
    'cloudy': '☁️',
    'partlycloudy': '⛅',
    'rainy': '🌧️',
    'pouring': '⛈️',
    'snowy': '❄️',
    'fog': '🌫️',
    'windy': '💨',
    'hail': '🌨️',
    'lightning': '⚡',
}

def create_simple_weather_icon(icon_name, emoji, output_path, size=128):
    """
    Create a simple weather icon using PIL
    """
    try:
        from PIL import Image, ImageDraw, ImageFont
    except ImportError:
        print("PIL not installed. Installing...")
        subprocess.run([sys.executable, "-m", "pip", "install", "Pillow"])
        from PIL import Image, ImageDraw, ImageFont

    # Create a white background image
    img = Image.new('L', (size, size), color=255)
    draw = ImageDraw.Draw(img)

    # Try to use a large font for emoji (requires a font that supports emoji)
    # For simplicity, we'll create geometric shapes instead

    # Define simple geometric representations
    if icon_name == 'sunny':
        # Draw a sun (circle with rays)
        center = size // 2
        radius = size // 4
        draw.ellipse([center-radius, center-radius, center+radius, center+radius], fill=0)
        # Rays
        for angle in range(0, 360, 45):
            import math
            rad = math.radians(angle)
            x1 = center + int((radius + 10) * math.cos(rad))
            y1 = center + int((radius + 10) * math.sin(rad))
            x2 = center + int((radius + 25) * math.cos(rad))
            y2 = center + int((radius + 25) * math.sin(rad))
            draw.line([x1, y1, x2, y2], fill=0, width=3)

    elif icon_name == 'cloudy':
        # Draw clouds (overlapping circles)
        y_base = size * 2 // 3
        draw.ellipse([size//6, y_base-30, size//3, y_base], fill=0)
        draw.ellipse([size//4, y_base-40, size*2//3, y_base], fill=0)
        draw.ellipse([size//2, y_base-35, size*5//6, y_base], fill=0)

    elif icon_name == 'rainy':
        # Draw cloud + rain drops
        y_base = size // 2
        draw.ellipse([size//6, y_base-30, size//3, y_base], fill=0)
        draw.ellipse([size//4, y_base-40, size*2//3, y_base], fill=0)
        draw.ellipse([size//2, y_base-35, size*5//6, y_base], fill=0)
        # Rain drops
        for x in range(size//4, 3*size//4, 20):
            draw.line([x, y_base+10, x, y_base+30], fill=0, width=2)
            draw.line([x+10, y_base+20, x+10, y_base+40], fill=0, width=2)

    elif icon_name == 'snowy':
        # Draw cloud + snowflakes
        y_base = size // 2
        draw.ellipse([size//6, y_base-30, size//3, y_base], fill=0)
        draw.ellipse([size//4, y_base-40, size*2//3, y_base], fill=0)
        draw.ellipse([size//2, y_base-35, size*5//6, y_base], fill=0)
        # Snowflakes (asterisks)
        for x in range(size//4, 3*size//4, 25):
            for y in [y_base+15, y_base+35]:
                draw.line([x-5, y, x+5, y], fill=0, width=2)
                draw.line([x, y-5, x, y+5], fill=0, width=2)
                draw.line([x-3, y-3, x+3, y+3], fill=0, width=2)
                draw.line([x-3, y+3, x+3, y-3], fill=0, width=2)

    elif icon_name == 'partlycloudy':
        # Draw sun + cloud
        center = size // 3
        radius = size // 6
        draw.ellipse([center-radius, center-radius, center+radius, center+radius], fill=0)
        # Cloud
        y_base = size * 2 // 3
        draw.ellipse([size//3, y_base-25, size//2, y_base], fill=0)
        draw.ellipse([size//2-10, y_base-30, size*3//4, y_base], fill=0)

    elif icon_name == 'fog':
        # Draw horizontal lines (fog)
        y_start = size // 3
        for i in range(5):
            y = y_start + i * 15
            draw.line([size//6, y, size*5//6, y], fill=0, width=3)

    elif icon_name == 'clear-night':
        # Draw moon (crescent)
        center = size // 2
        radius = size // 4
        draw.ellipse([center-radius, center-radius, center+radius, center+radius], fill=0)
        # Overlay white circle to create crescent
        offset = radius // 2
        draw.ellipse([center-radius+offset, center-radius, center+radius+offset, center+radius], fill=255)

    elif icon_name in ['windy', 'pouring', 'hail', 'lightning']:
        # Default cloud for unimplemented
        y_base = size // 2
        draw.ellipse([size//6, y_base-30, size//3, y_base], fill=0)
        draw.ellipse([size//4, y_base-40, size*2//3, y_base], fill=0)
        draw.ellipse([size//2, y_base-35, size*5//6, y_base], fill=0)

    else:
        # Default: question mark
        try:
            font = ImageFont.truetype("arial.ttf", size//2)
        except:
            font = ImageFont.load_default()
        draw.text((size//3, size//4), "?", fill=0, font=font)

    img.save(output_path)
    print(f"Created {icon_name} icon: {output_path}")

def main():
    script_dir = Path(__file__).parent
    icons_dir = script_dir / "weather_icons"
    icons_dir.mkdir(exist_ok=True)

    print("Creating weather icons...")

    # Create PNG icons
    for icon_name, emoji in WEATHER_ICONS.items():
        png_path = icons_dir / f"{icon_name}.png"
        create_simple_weather_icon(icon_name, emoji, png_path)

    print("\nConverting icons to EPDiy format...")

    # Path to imgconvert.py
    imgconvert = script_dir / "imgconvert.py"
    output_header = script_dir.parent / "weather_icons.h"

    # Start header file
    with open(output_header, 'w') as f:
        f.write("#pragma once\n")
        f.write("// Weather icons for EPDiy - Auto-generated\n")
        f.write("// Icons: " + ", ".join(WEATHER_ICONS.keys()) + "\n\n")

    # Convert each icon
    for icon_name in WEATHER_ICONS.keys():
        png_path = icons_dir / f"{icon_name}.png"
        temp_header = icons_dir / f"{icon_name}_temp.h"

        # Run imgconvert.py
        result = subprocess.run([
            sys.executable,
            str(imgconvert),
            "-i", str(png_path),
            "-n", f"weather_{icon_name}",
            "-o", str(temp_header),
            "-maxw", "128",
            "-maxh", "128"
        ], capture_output=True, text=True)

        if result.returncode == 0:
            # Append to main header
            with open(temp_header, 'r') as tf:
                with open(output_header, 'a') as f:
                    f.write(tf.read())
                    f.write("\n")
            temp_header.unlink()  # Remove temp file
            print(f"[OK] Converted {icon_name}")
        else:
            print(f"[FAIL] Failed to convert {icon_name}: {result.stderr}")

    print(f"\n[SUCCESS] All icons converted to: {output_header}")
    print(f"\nTo use in ESPHome, add this to your configuration:")
    print(f"  external_files:")
    print(f"    - {output_header}")

if __name__ == "__main__":
    main()
