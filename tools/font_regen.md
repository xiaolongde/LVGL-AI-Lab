# Font blob regen recipe

When you need to update the embedded SD-installed font (montserrat_28_blob.c):

```bash
# 1. generate the .bin via lv_font_conv (npm)
cd D:/projects/LVGL-AI-Lab
npx --yes lv_font_conv \
  --font external/lvgl/scripts/built_in_font/Montserrat-Medium.ttf \
  --size 28 --bpp 4 --range 0x20-0x7F \
  --format bin --no-compress \
  --lv-include lvgl.h \
  -o tools/font_out/montserrat_28.bin

# 2. convert to C array
python -c "
import sys
data = open('tools/font_out/montserrat_28.bin', 'rb').read()
print('#include <stdint.h>')
print(f'const uint32_t montserrat_28_bin_size = {len(data)};')
print('const uint8_t montserrat_28_bin_data[] = {')
for i in range(0, len(data), 16):
    chunk = data[i:i+16]
    print('    ' + ', '.join(f'0x{b:02x}' for b in chunk) + ',')
print('};')
" > src/devices/sd/montserrat_28_blob.c

# 3. rebuild + flash; first boot will f_write to SD card automatically
cmake --build build && bash tools/flash.sh
```

## Why embed + auto-install?

v0.2 北极星 hands-off：用户不用拆卡，固件自装资源到 SD。
Cost: ~16K Flash for 28px ASCII font.
Benefit: zero physical intervention, sd_disk_write data path validated as side effect.

For larger resources (>32KB images, full Chinese 字库) — use manual SD copy instead, embed cost too high.
