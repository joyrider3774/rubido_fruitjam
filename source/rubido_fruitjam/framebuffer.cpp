#include "framebuffer.h"

#if !defined(ARDUINO)
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>  // For snprintf
#endif

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#elif defined(ARDUINO_ARCH_SAMD)
// SAMD doesn't need pgmspace, just define the macro
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#else
#define PROGMEM
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif

// Create a new framebuffer
Framebuffer* createFramebuffer(int16_t width, int16_t height, uint8_t littleEndian, uint8_t bgr) {
    Framebuffer* fb = (Framebuffer*)malloc(sizeof(Framebuffer));
    if (!fb) return NULL;
    
    fb->width = width;
    fb->height = height;
    fb->littleEndian = littleEndian;
    fb->bgr = bgr;
    fb->buffer = (uint16_t*)malloc(width * height * sizeof(uint16_t));
    
    if (!fb->buffer) {
        free(fb);
        return NULL;
    }
    
    // Initialize to black
    memset(fb->buffer, 0, width * height * sizeof(uint16_t));
    
    return fb;
}

// Destroy a framebuffer and free memory
void destroyFramebuffer(Framebuffer* fb) {
    if (fb) {
        if (fb->buffer) {
            free(fb->buffer);
        }
        free(fb);
    }
}

// Draw a line using Bresenham's algorithm
void bufferDrawLine(Framebuffer* fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;
    int16_t y = y0;

    for (int16_t x = x0; x <= x1; x++) {
        if (steep) {
            bufferDrawPixel(fb, y, x, color);
        } else {
            bufferDrawPixel(fb, x, y, color);
        }
        err -= dy;
        if (err < 0) {
            y += ystep;
            err += dx;
        }
    }
}

// Draw a circle outline
void bufferDrawCircle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    bufferDrawPixel(fb, x0, y0 + r, color);
    bufferDrawPixel(fb, x0, y0 - r, color);
    bufferDrawPixel(fb, x0 + r, y0, color);
    bufferDrawPixel(fb, x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        bufferDrawPixel(fb, x0 + x, y0 + y, color);
        bufferDrawPixel(fb, x0 - x, y0 + y, color);
        bufferDrawPixel(fb, x0 + x, y0 - y, color);
        bufferDrawPixel(fb, x0 - x, y0 - y, color);
        bufferDrawPixel(fb, x0 + y, y0 + x, color);
        bufferDrawPixel(fb, x0 - y, y0 + x, color);
        bufferDrawPixel(fb, x0 + y, y0 - x, color);
        bufferDrawPixel(fb, x0 - y, y0 - x, color);
    }
}

// Draw a filled circle
void bufferFillCircle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    // Draw center horizontal line through the middle
    bufferDrawFastHLine(fb, x0 - r, y0, 2 * r + 1, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        // Draw horizontal lines at +/- x distance
        bufferDrawFastHLine(fb, x0 - x, y0 + y, 2 * x + 1, color);
        bufferDrawFastHLine(fb, x0 - x, y0 - y, 2 * x + 1, color);
        
        // Draw horizontal lines at +/- y distance
        bufferDrawFastHLine(fb, x0 - y, y0 + x, 2 * y + 1, color);
        bufferDrawFastHLine(fb, x0 - y, y0 - x, 2 * y + 1, color);
    }
}

// Draw a triangle outline
void bufferDrawTriangle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    bufferDrawLine(fb, x0, y0, x1, y1, color);
    bufferDrawLine(fb, x1, y1, x2, y2, color);
    bufferDrawLine(fb, x2, y2, x0, y0, color);
}

// Draw a filled triangle
void bufferFillTriangle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1);
        _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }

    if (y0 == y2) {
        a = b = x0;
        if (x1 < a) a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a) a = x2;
        else if (x2 > b) b = x2;
        bufferDrawFastHLine(fb, a, y0, b - a + 1, color);
        return;
    }

    int16_t dx01 = x1 - x0;
    int16_t dy01 = y1 - y0;
    int16_t dx02 = x2 - x0;
    int16_t dy02 = y2 - y0;
    int16_t dx12 = x2 - x1;
    int16_t dy12 = y2 - y1;
    int32_t sa = 0;
    int32_t sb = 0;

    if (y1 == y2) {
        last = y1;
    } else {
        last = y1 - 1;
    }

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        if (a > b) _swap_int16_t(a, b);
        bufferDrawFastHLine(fb, a, y, b - a + 1, color);
    }

    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) _swap_int16_t(a, b);
        bufferDrawFastHLine(fb, a, y, b - a + 1, color);
    }
}

// Draw an ellipse outline - FIXED VERSION
void bufferDrawEllipse(Framebuffer* fb, int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color) {
    if (!fb || !fb->buffer) return;
    if (rx < 0 || ry < 0) return;
    
    // Midpoint ellipse algorithm
    int32_t rx2 = (int32_t)rx * rx;
    int32_t ry2 = (int32_t)ry * ry;
    int32_t two_rx2 = 2 * rx2;
    int32_t two_ry2 = 2 * ry2;
    
    int16_t x = 0;
    int16_t y = ry;
    int32_t px = 0;
    int32_t py = two_rx2 * y;
    
    // Plot initial points
    bufferDrawPixel(fb, x0 + x, y0 + y, color);
    bufferDrawPixel(fb, x0 - x, y0 + y, color);
    bufferDrawPixel(fb, x0 + x, y0 - y, color);
    bufferDrawPixel(fb, x0 - x, y0 - y, color);
    
    // Region 1
    int32_t p = ry2 - (rx2 * ry) + (rx2 / 4);
    while (px < py) {
        x++;
        px += two_ry2;
        if (p < 0) {
            p += ry2 + px;
        } else {
            y--;
            py -= two_rx2;
            p += ry2 + px - py;
        }
        
        bufferDrawPixel(fb, x0 + x, y0 + y, color);
        bufferDrawPixel(fb, x0 - x, y0 + y, color);
        bufferDrawPixel(fb, x0 + x, y0 - y, color);
        bufferDrawPixel(fb, x0 - x, y0 - y, color);
    }
    
    // Region 2
    p = ry2 * (x * x + x) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    while (y > 0) {
        y--;
        py -= two_rx2;
        if (p > 0) {
            p += rx2 - py;
        } else {
            x++;
            px += two_ry2;
            p += rx2 - py + px;
        }
        
        bufferDrawPixel(fb, x0 + x, y0 + y, color);
        bufferDrawPixel(fb, x0 - x, y0 + y, color);
        bufferDrawPixel(fb, x0 + x, y0 - y, color);
        bufferDrawPixel(fb, x0 - x, y0 - y, color);
    }
}

// Draw a filled ellipse - FIXED VERSION
void bufferFillEllipse(Framebuffer* fb, int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color) {
    if (!fb || !fb->buffer) return;
    if (rx < 0 || ry < 0) return;
    
    // Midpoint ellipse algorithm with horizontal line filling
    int32_t rx2 = (int32_t)rx * rx;
    int32_t ry2 = (int32_t)ry * ry;
    int32_t two_rx2 = 2 * rx2;
    int32_t two_ry2 = 2 * ry2;
    
    int16_t x = 0;
    int16_t y = ry;
    int32_t px = 0;
    int32_t py = two_rx2 * y;
    
    // Draw initial horizontal line
    bufferDrawFastHLine(fb, x0 - x, y0 + y, 2 * x + 1, color);
    bufferDrawFastHLine(fb, x0 - x, y0 - y, 2 * x + 1, color);
    
    // Region 1
    int32_t p = ry2 - (rx2 * ry) + (rx2 / 4);
    while (px < py) {
        x++;
        px += two_ry2;
        if (p < 0) {
            p += ry2 + px;
        } else {
            y--;
            py -= two_rx2;
            p += ry2 + px - py;
        }
        
        bufferDrawFastHLine(fb, x0 - x, y0 + y, 2 * x + 1, color);
        bufferDrawFastHLine(fb, x0 - x, y0 - y, 2 * x + 1, color);
    }
    
    // Region 2
    p = ry2 * (x * x + x) + rx2 * (y - 1) * (y - 1) - rx2 * ry2;
    while (y > 0) {
        y--;
        py -= two_rx2;
        if (p > 0) {
            p += rx2 - py;
        } else {
            x++;
            px += two_ry2;
            p += rx2 - py + px;
        }
        
        bufferDrawFastHLine(fb, x0 - x, y0 + y, 2 * x + 1, color);
        bufferDrawFastHLine(fb, x0 - x, y0 - y, 2 * x + 1, color);
    }
}

// Draw a rounded rectangle outline - FIXED VERSION
void bufferDrawRoundRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    // Draw the four edges (not including corners)
    bufferDrawFastHLine(fb, x + r, y, w - 2 * r, color);                    // Top
    bufferDrawFastHLine(fb, x + r, y + h - 1, w - 2 * r, color);            // Bottom
    bufferDrawFastVLine(fb, x, y + r, h - 2 * r, color);                    // Left
    bufferDrawFastVLine(fb, x + w - 1, y + r, h - 2 * r, color);            // Right

    // Draw the four rounded corners using circle algorithm
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t cx = 0;
    int16_t cy = r;

    while (cx < cy) {
        if (f >= 0) {
            cy--;
            ddF_y += 2;
            f += ddF_y;
        }
        cx++;
        ddF_x += 2;
        f += ddF_x;

        // Top-left corner (quadrant 2)
        bufferDrawPixel(fb, x + r - cx, y + r - cy, color);
        bufferDrawPixel(fb, x + r - cy, y + r - cx, color);
        
        // Top-right corner (quadrant 1)
        bufferDrawPixel(fb, x + w - r - 1 + cx, y + r - cy, color);
        bufferDrawPixel(fb, x + w - r - 1 + cy, y + r - cx, color);
        
        // Bottom-left corner (quadrant 3)
        bufferDrawPixel(fb, x + r - cx, y + h - r - 1 + cy, color);
        bufferDrawPixel(fb, x + r - cy, y + h - r - 1 + cx, color);
        
        // Bottom-right corner (quadrant 4)
        bufferDrawPixel(fb, x + w - r - 1 + cx, y + h - r - 1 + cy, color);
        bufferDrawPixel(fb, x + w - r - 1 + cy, y + h - r - 1 + cx, color);
    }
}

// Draw a filled rounded rectangle
void bufferFillRoundRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    bufferFillRect(fb, x + r, y, w - 2 * r, h, color);
    bufferFillRect(fb, x, y + r, r, h - 2 * r, color);
    bufferFillRect(fb, x + w - r, y + r, r, h - 2 * r, color);

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t cx = 0;
    int16_t cy = r;

    while (cx < cy) {
        if (f >= 0) {
            cy--;
            ddF_y += 2;
            f += ddF_y;
        }
        cx++;
        ddF_x += 2;
        f += ddF_x;

        bufferDrawFastHLine(fb, x + r - cx, y + r - cy, 2 * cx + 1, color);
        bufferDrawFastHLine(fb, x + w - r - cx - 1, y + r - cy, 2 * cx + 1, color);
        bufferDrawFastHLine(fb, x + r - cy, y + r - cx, 2 * cy + 1, color);
        bufferDrawFastHLine(fb, x + w - r - cy - 1, y + r - cx, 2 * cy + 1, color);
        
        bufferDrawFastHLine(fb, x + r - cx, y + h - r - 1 + cy, 2 * cx + 1, color);
        bufferDrawFastHLine(fb, x + w - r - cx - 1, y + h - r - 1 + cy, 2 * cx + 1, color);
        bufferDrawFastHLine(fb, x + r - cy, y + h - r - 1 + cx, 2 * cy + 1, color);
        bufferDrawFastHLine(fb, x + w - r - cy - 1, y + h - r - 1 + cx, 2 * cy + 1, color);
    }
}

// Draw a character
void bufferDrawChar(Framebuffer* fb, int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font) {
    if (!fb || !fb->buffer || !font) return;
    
    if (c >= 176) c++;
    
    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = pgm_read_byte(&font[c * 5 + i]);
        
        for (int8_t j = 0; j < 8; j++) {
            if (line & 0x1) {
                if (size == 1) {
                    bufferDrawPixel(fb, x + i, y + j, color);
                } else {
                    bufferFillRect(fb, x + i * size, y + j * size, size, size, color);
                }
            } else if (bg != color) {
                if (size == 1) {
                    bufferDrawPixel(fb, x + i, y + j, bg);
                } else {
                    bufferFillRect(fb, x + i * size, y + j * size, size, size, bg);
                }
            }
            line >>= 1;
        }
    }
    
    if (bg != color) {
        if (size == 1) {
            for (int8_t j = 0; j < 8; j++) {
                bufferDrawPixel(fb, x + 5, y + j, bg);
            }
        } else {
            bufferFillRect(fb, x + 5 * size, y, size, 8 * size, bg);
        }
    }
}

// Print a string
void bufferPrint(Framebuffer* fb, int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font) {
    if (!fb || !fb->buffer || !str || !font) return;
    
    int16_t cursorX = x;
    int16_t cursorY = y;
    
    while (*str) {
        if(*str == '\n')
        {
            cursorY += (9 * size);
            cursorX = x;
            str++;
            continue;
        }
        
        bufferDrawChar(fb, cursorX, cursorY, *str, color, bg, size, font);
        cursorX += (6 * size);
        str++;
    }
}

// Print an integer
void bufferPrintInt(Framebuffer* fb, int16_t x, int16_t y, int value, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font) {
    if (!fb || !fb->buffer || !font) return;
    
    char buf[12];
    
#if defined(ARDUINO)
    itoa(value, buf, 10);
#else
    snprintf(buf, sizeof(buf), "%d", value);
#endif
    
    bufferPrint(fb, x, y, buf, color, bg, size, font);
}

// Print a float
void bufferPrintFloat(Framebuffer* fb, int16_t x, int16_t y, float value, int decimals, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font) {
    if (!fb || !fb->buffer || !font) return;
    
    char buf[16];
    int idx = 0;
    
    if (value < 0) {
        buf[idx++] = '-';
        value = -value;
    }
    
    int wholePart = (int)value;
    
    char temp[10];
    int tempIdx = 0;
    if (wholePart == 0) {
        temp[tempIdx++] = '0';
    } else {
        while (wholePart > 0) {
            temp[tempIdx++] = '0' + (wholePart % 10);
            wholePart /= 10;
        }
    }
    
    for (int i = tempIdx - 1; i >= 0; i--) {
        buf[idx++] = temp[i];
    }
    
    buf[idx++] = '.';
    
    float fraction = value - (int)value;
    
    for (int i = 0; i < decimals; i++) {
        fraction *= 10.0;
        int digit = (int)fraction;
        buf[idx++] = '0' + digit;
        fraction -= digit;
    }
    
    buf[idx] = '\0';
    
    bufferPrint(fb, x, y, buf, color, bg, size, font);
}

// Blit one framebuffer to another with clipping
void blitFramebuffer(Framebuffer* dest, int16_t destX, int16_t destY, Framebuffer* src, int16_t srcX, int16_t srcY, int16_t w, int16_t h) {
    if (!dest || !dest->buffer || !src || !src->buffer) return;
    
    // Clip source coordinates
    if (srcX < 0) {
        w += srcX;
        destX -= srcX;
        srcX = 0;
    }
    if (srcY < 0) {
        h += srcY;
        destY -= srcY;
        srcY = 0;
    }
    if (srcX + w > src->width) w = src->width - srcX;
    if (srcY + h > src->height) h = src->height - srcY;
    
    // Clip destination coordinates
    if (destX < 0) {
        w += destX;
        srcX -= destX;
        destX = 0;
    }
    if (destY < 0) {
        h += destY;
        srcY -= destY;
        destY = 0;
    }
    if (destX + w > dest->width) w = dest->width - destX;
    if (destY + h > dest->height) h = dest->height - destY;
    
    if (w <= 0 || h <= 0) return;
    
    // Fast blit using pointers
    uint16_t* srcPtr = src->buffer + (srcY * src->width + srcX);
    uint16_t* destPtr = dest->buffer + (destY * dest->width + destX);
    int16_t srcSkip = src->width - w;
    int16_t destSkip = dest->width - w;
    
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            *destPtr++ = *srcPtr++;
        }
        srcPtr += srcSkip;
        destPtr += destSkip;
    }
}

// ============================================================================
// HIGHLY OPTIMIZED: Flexible RGB565 image drawing
// ============================================================================

/**
 * Draw RGB565 image data to framebuffer - OPTIMIZED FOR SPEED
 * 
 * Optimizations:
 * - Uses framebuffer format info for fastest possible path selection
 * - Direct scanline memcpy when formats match exactly
 * - Eliminates conversions when source and dest formats are identical
 * - Batch operations at scanline level
 * 
 * @param fb          Framebuffer to draw to (contains format info)
 * @param x           X position on screen
 * @param y           Y position on screen
 * @param image       RGB565 data array (from PROGMEM)
 * @param w           Image width in pixels
 * @param h           Image height in pixels
 * @param bgr         Source color order: 0=RGB (default), 1=BGR
 * @param littleEndian Source byte order: 1=little-endian (default), 0=big-endian
 * @param rle         RLE compression: 1=decompress, 0=raw (default)
 */
void bufferDrawImage(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h,
                     uint8_t bgr, uint8_t littleEndian, uint8_t rle) {
    if (!fb || !fb->buffer || !image) return;
    
    // Calculate clipping for destination
    int16_t srcX = 0;
    int16_t srcY = 0;
    int16_t drawWidth = w;
    int16_t drawHeight = h;
    
    // Clip left
    if (x < 0) {
        srcX = -x;
        drawWidth += x;
        x = 0;
    }
    
    // Clip top
    if (y < 0) {
        srcY = -y;
        drawHeight += y;
        y = 0;
    }
    
    // Clip right
    if (x + drawWidth > fb->width) {
        drawWidth = fb->width - x;
    }
    
    // Clip bottom
    if (y + drawHeight > fb->height) {
        drawHeight = fb->height - y;
    }
    
    // Nothing to draw
    if (drawWidth <= 0 || drawHeight <= 0) return;
    
    // Determine if conversion is needed
    uint8_t needsBGRConversion = (bgr != fb->bgr);
    uint8_t needsEndiannessConversion = (littleEndian != fb->littleEndian);
    uint8_t needsAnyConversion = needsBGRConversion || needsEndiannessConversion;
    
    // ========================================================================
    // RLE DECOMPRESSION PATH - OPTIMIZED
    // ========================================================================
    if (rle) {
        uint32_t dataIndex = 0;
        int16_t pixelX = 0;
        int16_t pixelY = 0;
        
        // Pre-calculate destination pointer starting position
        uint16_t* destBase = fb->buffer + (y * fb->width + x);
        int16_t destRowSkip = fb->width;
        
        while (pixelY < h) {
            uint8_t count = pgm_read_byte(&image[dataIndex++]);
            
            if (count == 0) {
                // Literal mode
                uint8_t literalCount = pgm_read_byte(&image[dataIndex++]);
                
                for (uint8_t i = 0; i < literalCount; i++) {
                    uint8_t b1 = pgm_read_byte(&image[dataIndex++]);
                    uint8_t b2 = pgm_read_byte(&image[dataIndex++]);
                    
                    // Draw pixel if in clipped region
                    if (pixelX >= srcX && pixelX < srcX + drawWidth &&
                        pixelY >= srcY && pixelY < srcY + drawHeight) {
                        uint16_t* dest = destBase + ((pixelY - srcY) * destRowSkip) + (pixelX - srcX);
                        
                        if (!needsEndiannessConversion && !needsBGRConversion) {
                            // SAME FORMAT: Write bytes directly, avoiding uint16_t!
                            uint8_t* destBytes = (uint8_t*)dest;
                            destBytes[0] = b1;
                            destBytes[1] = b2;
                        } else {
                            // Need conversion: go through uint16_t
                            uint16_t rgb565;
                            if (littleEndian) {
                                rgb565 = b1 | (b2 << 8);
                            } else {
                                rgb565 = (b1 << 8) | b2;
                            }
                            
                            if (needsBGRConversion) {
                                uint8_t r5 = rgb565 & 0x1F;
                                uint8_t g6 = (rgb565 >> 5) & 0x3F;
                                uint8_t b5 = (rgb565 >> 11) & 0x1F;
                                rgb565 = (r5 << 11) | (g6 << 5) | b5;
                            }
                            
                            if (needsEndiannessConversion) {
                                rgb565 = (rgb565 >> 8) | (rgb565 << 8);
                            }
                            
                            *dest = rgb565;
                        }
                    }
                    
                    // Advance pixel position
                    pixelX++;
                    if (pixelX >= w) {
                        pixelX = 0;
                        pixelY++;
                    }
                }
            } else {
                // Run mode - repeat pixel 'count' times
                uint8_t b1 = pgm_read_byte(&image[dataIndex++]);
                uint8_t b2 = pgm_read_byte(&image[dataIndex++]);
                
                uint16_t rgb565 = 0;
                bool needsConversion = needsEndiannessConversion || needsBGRConversion;
                
                if (needsConversion) {
                    // Pre-convert once for the run
                    if (littleEndian) {
                        rgb565 = b1 | (b2 << 8);
                    } else {
                        rgb565 = (b1 << 8) | b2;
                    }
                    
                    if (needsBGRConversion) {
                        uint8_t r5 = rgb565 & 0x1F;
                        uint8_t g6 = (rgb565 >> 5) & 0x3F;
                        uint8_t b5 = (rgb565 >> 11) & 0x1F;
                        rgb565 = (r5 << 11) | (g6 << 5) | b5;
                    }
                    
                    if (needsEndiannessConversion) {
                        rgb565 = (rgb565 >> 8) | (rgb565 << 8);
                    }
                }
                
                // Repeat pixel - DIRECT WRITES
                for (uint8_t i = 0; i < count; i++) {
                    if (pixelX >= srcX && pixelX < srcX + drawWidth &&
                        pixelY >= srcY && pixelY < srcY + drawHeight) {
                        uint16_t* dest = destBase + ((pixelY - srcY) * destRowSkip) + (pixelX - srcX);
                        
                        if (!needsConversion) {
                            // SAME FORMAT: Write bytes directly!
                            uint8_t* destBytes = (uint8_t*)dest;
                            destBytes[0] = b1;
                            destBytes[1] = b2;
                        } else {
                            *dest = rgb565;
                        }
                    }
                    
                    // Advance pixel position
                    pixelX++;
                    if (pixelX >= w) {
                        pixelX = 0;
                        pixelY++;
                    }
                }
            }
        }
    } 
    // ========================================================================
    // RAW DATA PATH - HIGHLY OPTIMIZED WITH FORMAT AWARENESS
    // ========================================================================
    else {
        const uint8_t* srcPtr = image + ((srcY * w + srcX) * 2);
        uint16_t* destPtr = fb->buffer + (y * fb->width + x);
        int16_t srcSkip = (w - drawWidth) * 2;
        int16_t destSkip = fb->width - drawWidth;
        
        // FASTEST PATH: Formats match exactly - direct scanline copy!
        if (!needsAnyConversion) {
            #if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
                // AVR/ESP with PROGMEM - word-by-word copy
                for (int16_t row = 0; row < drawHeight; row++) {
                    const uint16_t* srcRow = (const uint16_t*)(srcPtr + row * w * 2);
                    uint16_t* destRow = destPtr + row * fb->width;
                    
                    for (int16_t col = 0; col < drawWidth; col++) {
                        destRow[col] = pgm_read_word(&srcRow[col]);
                    }
                }
            #else
                // SAMD/ARM - Ultra-optimized with unrolled memcpy
                // Check if we can do full-width scanline copy (no clipping on sides)
                if (srcX == 0 && drawWidth == w && x == 0 && drawWidth == fb->width) {
                    // ULTRA-FAST: Single memcpy for entire clipped region
                    const uint16_t* srcRow = (const uint16_t*)(srcPtr);
                    uint16_t* destRow = destPtr;
                    size_t totalBytes = (size_t)drawWidth * drawHeight * sizeof(uint16_t);
                    memcpy(destRow, srcRow, totalBytes);
                } else {
                    // Fast: memcpy per scanline with loop unrolling
                    int16_t row = 0;
                    
                    // Unroll by 4 for better performance
                    int16_t unrolledHeight = drawHeight & ~3; // Round down to multiple of 4
                    
                    for (; row < unrolledHeight; row += 4) {
                        const uint16_t* srcRow0 = (const uint16_t*)(srcPtr + (row + 0) * w * 2);
                        const uint16_t* srcRow1 = (const uint16_t*)(srcPtr + (row + 1) * w * 2);
                        const uint16_t* srcRow2 = (const uint16_t*)(srcPtr + (row + 2) * w * 2);
                        const uint16_t* srcRow3 = (const uint16_t*)(srcPtr + (row + 3) * w * 2);
                        
                        uint16_t* destRow0 = destPtr + (row + 0) * fb->width;
                        uint16_t* destRow1 = destPtr + (row + 1) * fb->width;
                        uint16_t* destRow2 = destPtr + (row + 2) * fb->width;
                        uint16_t* destRow3 = destPtr + (row + 3) * fb->width;
                        
                        size_t rowBytes = drawWidth * sizeof(uint16_t);
                        memcpy(destRow0, srcRow0, rowBytes);
                        memcpy(destRow1, srcRow1, rowBytes);
                        memcpy(destRow2, srcRow2, rowBytes);
                        memcpy(destRow3, srcRow3, rowBytes);
                    }
                    
                    // Handle remaining rows
                    for (; row < drawHeight; row++) {
                        const uint16_t* srcRow = (const uint16_t*)(srcPtr + row * w * 2);
                        uint16_t* destRow = destPtr + row * fb->width;
                        memcpy(destRow, srcRow, drawWidth * sizeof(uint16_t));
                    }
                }
            #endif
        }
        // Fast path: Only BGR conversion needed (same endianness)
        else if (!needsEndiannessConversion && needsBGRConversion) {
            const uint16_t* src16 = (const uint16_t*)srcPtr;
            
            for (int16_t j = 0; j < drawHeight; j++) {
                for (int16_t i = 0; i < drawWidth; i++) {
                    #if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
                        uint16_t rgb565 = pgm_read_word(src16++);
                    #else
                        uint16_t rgb565 = *src16++;
                    #endif
                    
                    // Swap BGR to RGB (or vice versa)
                    uint8_t r5 = rgb565 & 0x1F;
                    uint8_t g6 = (rgb565 >> 5) & 0x3F;
                    uint8_t b5 = (rgb565 >> 11) & 0x1F;
                    rgb565 = (r5 << 11) | (g6 << 5) | b5;
                    
                    // Handle CPU vs framebuffer endianness mismatch
                    
                    *destPtr++ = rgb565;
                }
                src16 += (w - drawWidth);
                destPtr += destSkip;
            }
        }
        // Fast path: Only endianness differs between source and framebuffer
        else if (needsEndiannessConversion && !needsBGRConversion) {
            const uint16_t* src16 = (const uint16_t*)srcPtr;
            
            for (int16_t j = 0; j < drawHeight; j++) {
                for (int16_t i = 0; i < drawWidth; i++) {
                    #if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
                        uint16_t rgb565 = pgm_read_word(src16++);
                    #else
                        uint16_t rgb565 = *src16++;
                    #endif
                    
                    // Source and framebuffer have different endianness
                    // But we also need to account for CPU endianness
                    // 
                    // Example: Source is BE, Framebuffer is LE, CPU is LE
                    // - Source BE means we read and get the BE value
                    // - CPU is LE so it stores LE
                    // - Framebuffer is LE so no adjustment needed - they match!
                    //
                    // Example: Source is LE, Framebuffer is BE, CPU is LE  
                    // - Source LE means we read and get the LE value
                    // - CPU is LE so it stores LE
                    // - Framebuffer is BE so we need to swap
                    
                    
                    *destPtr++ = rgb565;
                }
                src16 += (w - drawWidth);
                destPtr += destSkip;
            }
        }
        // Slower path: Both conversions needed
        else {
            for (int16_t j = 0; j < drawHeight; j++) {
                for (int16_t i = 0; i < drawWidth; i++) {
                    uint8_t b1 = pgm_read_byte(srcPtr++);
                    uint8_t b2 = pgm_read_byte(srcPtr++);
                    
                    // Reconstruct pixel from source bytes
                    uint16_t rgb565;
                    if (littleEndian) {
                        rgb565 = b1 | (b2 << 8);
                    } else {
                        rgb565 = (b1 << 8) | b2;
                    }
                    
                    // Swap BGR/RGB if needed (operates on RGB565 bit positions)
                    if (needsBGRConversion) {
                        uint8_t r5 = rgb565 & 0x1F;
                        uint8_t g6 = (rgb565 >> 5) & 0x3F;
                        uint8_t b5 = (rgb565 >> 11) & 0x1F;
                        rgb565 = (r5 << 11) | (g6 << 5) | b5;
                    }
                    
                    // Handle CPU vs framebuffer endianness mismatch
                    
                    *destPtr++ = rgb565;
                }
                srcPtr += srcSkip;
                destPtr += destSkip;
            }
        }
    }
}

// ============================================================================
// Transparent color variant - skips pixels matching transparent color
// ============================================================================

/**
 * Draw RGB565 image with transparent color support
 * 
 * Same as bufferDrawImage but skips pixels matching transparentColor.
 * The transparentColor parameter should be in framebuffer format.
 */
void bufferDrawImageTransparent(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h,
                                uint8_t bgr, uint8_t littleEndian, uint8_t rle, uint16_t transparentColor) {
    if (!fb || !fb->buffer || !image) return;
    
    // Calculate clipping for destination
    int16_t srcX = 0;
    int16_t srcY = 0;
    int16_t drawWidth = w;
    int16_t drawHeight = h;
    
    // Clip left
    if (x < 0) {
        srcX = -x;
        drawWidth += x;
        x = 0;
    }
    
    // Clip top
    if (y < 0) {
        srcY = -y;
        drawHeight += y;
        y = 0;
    }
    
    // Clip right
    if (x + drawWidth > fb->width) {
        drawWidth = fb->width - x;
    }
    
    // Clip bottom
    if (y + drawHeight > fb->height) {
        drawHeight = fb->height - y;
    }
    
    // Nothing to draw
    if (drawWidth <= 0 || drawHeight <= 0) return;
    
    // Determine if conversion is needed
    uint8_t needsBGRConversion = (bgr != fb->bgr);
    uint8_t needsEndiannessConversion = (littleEndian != fb->littleEndian);
    uint8_t needsAnyConversion = needsBGRConversion || needsEndiannessConversion;
    
    // ========================================================================
    // RLE DECOMPRESSION PATH WITH TRANSPARENCY
    // ========================================================================
    if (rle) {
        uint32_t dataIndex = 0;
        int16_t pixelX = 0;
        int16_t pixelY = 0;
        
        // Pre-calculate destination pointer starting position
        uint16_t* destBase = fb->buffer + (y * fb->width + x);
        int16_t destRowSkip = fb->width;
        
        while (pixelY < h) {
            uint8_t count = pgm_read_byte(&image[dataIndex++]);
            
            if (count == 0) {
                // Literal mode
                uint8_t literalCount = pgm_read_byte(&image[dataIndex++]);
                
                for (uint8_t i = 0; i < literalCount; i++) {
                    uint8_t b1 = pgm_read_byte(&image[dataIndex++]);
                    uint8_t b2 = pgm_read_byte(&image[dataIndex++]);
                    
                    // Draw pixel if in clipped region
                    if (pixelX >= srcX && pixelX < srcX + drawWidth &&
                        pixelY >= srcY && pixelY < srcY + drawHeight) {
                        uint16_t* dest = destBase + ((pixelY - srcY) * destRowSkip) + (pixelX - srcX);
                        
                        if (!needsAnyConversion) {
                            // SAME FORMAT: Check bytes directly against transparent color
                            uint16_t pixelValue = (b1) | (b2 << 8);
                            if (pixelValue != transparentColor) {
                                uint8_t* destBytes = (uint8_t*)dest;
                                destBytes[0] = b1;
                                destBytes[1] = b2;
                            }
                        } else {
                            // Need conversion
                            uint16_t rgb565;
                            if (littleEndian) {
                                rgb565 = b1 | (b2 << 8);
                            } else {
                                rgb565 = (b1 << 8) | b2;
                            }
                            
                            if (needsBGRConversion) {
                                uint8_t r5 = rgb565 & 0x1F;
                                uint8_t g6 = (rgb565 >> 5) & 0x3F;
                                uint8_t b5 = (rgb565 >> 11) & 0x1F;
                                rgb565 = (r5 << 11) | (g6 << 5) | b5;
                            }
                            
                            if (needsEndiannessConversion) {
                                rgb565 = (rgb565 >> 8) | (rgb565 << 8);
                            }
                            
                            // Check against transparent color after conversion
                            if (rgb565 != transparentColor) {
                                *dest = rgb565;
                            }
                        }
                    }
                    
                    // Advance pixel position
                    pixelX++;
                    if (pixelX >= w) {
                        pixelX = 0;
                        pixelY++;
                    }
                }
            } else {
                // Run mode - repeat pixel 'count' times
                uint8_t b1 = pgm_read_byte(&image[dataIndex++]);
                uint8_t b2 = pgm_read_byte(&image[dataIndex++]);
                
                uint16_t rgb565 = 0;
                bool needsConversion = needsAnyConversion;
                bool isTransparent = false;
                
                if (!needsConversion) {
                    // Check if this run is transparent
                    uint16_t pixelValue = (b1) | (b2 << 8);
                    isTransparent = (pixelValue == transparentColor);
                } else {
                    // Pre-convert once for the run
                    if (littleEndian) {
                        rgb565 = b1 | (b2 << 8);
                    } else {
                        rgb565 = (b1 << 8) | b2;
                    }
                    
                    if (needsBGRConversion) {
                        uint8_t r5 = rgb565 & 0x1F;
                        uint8_t g6 = (rgb565 >> 5) & 0x3F;
                        uint8_t b5 = (rgb565 >> 11) & 0x1F;
                        rgb565 = (r5 << 11) | (g6 << 5) | b5;
                    }
                    
                    if (needsEndiannessConversion) {
                        rgb565 = (rgb565 >> 8) | (rgb565 << 8);
                    }
                    
                    isTransparent = (rgb565 == transparentColor);
                }
                
                // Only draw if not transparent
                if (!isTransparent) {
                    for (uint8_t i = 0; i < count; i++) {
                        if (pixelX >= srcX && pixelX < srcX + drawWidth &&
                            pixelY >= srcY && pixelY < srcY + drawHeight) {
                            uint16_t* dest = destBase + ((pixelY - srcY) * destRowSkip) + (pixelX - srcX);
                            
                            if (!needsConversion) {
                                uint8_t* destBytes = (uint8_t*)dest;
                                destBytes[0] = b1;
                                destBytes[1] = b2;
                            } else {
                                *dest = rgb565;
                            }
                        }
                        
                        pixelX++;
                        if (pixelX >= w) {
                            pixelX = 0;
                            pixelY++;
                        }
                    }
                } else {
                    // Transparent run - just advance position
                    for (uint8_t i = 0; i < count; i++) {
                        pixelX++;
                        if (pixelX >= w) {
                            pixelX = 0;
                            pixelY++;
                        }
                    }
                }
            }
        }
    } 
    // ========================================================================
    // RAW DATA PATH WITH TRANSPARENCY
    // ========================================================================
    else {
        const uint8_t* srcPtr = image + ((srcY * w + srcX) * 2);
        uint16_t* destPtr = fb->buffer + (y * fb->width + x);
        int16_t srcSkip = (w - drawWidth) * 2;
        int16_t destSkip = fb->width - drawWidth;
        
        // FASTEST PATH: Formats match exactly but check transparency
        if (!needsAnyConversion) {
            #if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
                const uint16_t* src16 = (const uint16_t*)srcPtr;
                
                for (int16_t j = 0; j < drawHeight; j++) {
                    uint16_t* rowDest = destPtr;
                    const uint16_t* rowSrc = src16;
                    for (int16_t i = 0; i < drawWidth; i++) {
                        uint16_t pixel = pgm_read_word(rowSrc++);
                        if (pixel != transparentColor) {
                            *rowDest = pixel;
                        }
                        rowDest++;
                    }
                    
                    src16 += w;
                    destPtr += fb->width;
                }
            #else
                const uint16_t* src16 = (const uint16_t*)srcPtr;
                
                for (int16_t j = 0; j < drawHeight; j++) {
                    for (int16_t i = 0; i < drawWidth; i++) {
                        uint16_t pixel = *src16++;
                        if (pixel != transparentColor) {
                            *destPtr = pixel;
                        }
                        destPtr++;
                    }
                    
                    src16 += (w - drawWidth);
                    destPtr += destSkip;
                }
            #endif
        }
        // Fast path: Only BGR conversion needed (same endianness)
        else if (!needsEndiannessConversion && needsBGRConversion) {
            const uint16_t* src16 = (const uint16_t*)srcPtr;
            
            for (int16_t j = 0; j < drawHeight; j++) {
                for (int16_t i = 0; i < drawWidth; i++) {
                    #if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
                        uint16_t rgb565 = pgm_read_word(src16++);
                    #else
                        uint16_t rgb565 = *src16++;
                    #endif
                    
                    // Swap BGR to RGB (or vice versa)
                    uint8_t r5 = rgb565 & 0x1F;
                    uint8_t g6 = (rgb565 >> 5) & 0x3F;
                    uint8_t b5 = (rgb565 >> 11) & 0x1F;
                    rgb565 = (r5 << 11) | (g6 << 5) | b5;
                    
                    if (rgb565 != transparentColor) {
                        *destPtr = rgb565;
                    }
                    destPtr++;
                }
                src16 += (w - drawWidth);
                destPtr += destSkip;
            }
        }
        // Fast path: Only endianness conversion needed (same BGR/RGB)
        else if (needsEndiannessConversion && !needsBGRConversion) {
            const uint16_t* src16 = (const uint16_t*)srcPtr;
            
            for (int16_t j = 0; j < drawHeight; j++) {
                for (int16_t i = 0; i < drawWidth; i++) {
                    #if defined(__AVR__) || defined(ESP8266) || defined(ESP32)
                        uint16_t rgb565 = pgm_read_word(src16++);
                    #else
                        uint16_t rgb565 = *src16++;
                    #endif
                    
                    // Swap byte order
                    rgb565 = (rgb565 >> 8) | (rgb565 << 8);
                    
                    if (rgb565 != transparentColor) {
                        *destPtr = rgb565;
                    }
                    destPtr++;
                }
                src16 += (w - drawWidth);
                destPtr += destSkip;
            }
        }
        // Slower path: Both conversions needed
        else {
            for (int16_t j = 0; j < drawHeight; j++) {
                for (int16_t i = 0; i < drawWidth; i++) {
                    uint8_t b1 = pgm_read_byte(srcPtr++);
                    uint8_t b2 = pgm_read_byte(srcPtr++);
                    
                    uint16_t rgb565;
                    if (littleEndian) {
                        rgb565 = b1 | (b2 << 8);
                    } else {
                        rgb565 = (b1 << 8) | b2;
                    }
                    
                    // Convert to framebuffer endianness
                    if (needsEndiannessConversion) {
                        rgb565 = (rgb565 >> 8) | (rgb565 << 8);
                    }
                    
                    // Swap BGR/RGB if needed
                    if (needsBGRConversion) {
                        uint8_t r5 = rgb565 & 0x1F;
                        uint8_t g6 = (rgb565 >> 5) & 0x3F;
                        uint8_t b5 = (rgb565 >> 11) & 0x1F;
                        rgb565 = (r5 << 11) | (g6 << 5) | b5;
                    }
                    
                    if (rgb565 != transparentColor) {
                        *destPtr = rgb565;
                    }
                    destPtr++;
                }
                srcPtr += srcSkip;
                destPtr += destSkip;
            }
        }
    }
}
