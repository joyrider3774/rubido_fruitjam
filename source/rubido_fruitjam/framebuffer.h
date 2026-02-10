#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Framebuffer structure
typedef struct {
    uint16_t* buffer;
    int16_t width;
    int16_t height;
    uint8_t littleEndian;  // 1=little-endian (default), 0=big-endian
    uint8_t bgr;           // 1=BGR format, 0=RGB format (default)
} Framebuffer;

// Helper macros
#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef constrain
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#endif

// Color conversion helper - returns RGB565 in CPU native endianness
inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Color conversion helper - returns RGB565_BE in CPU native endianness
inline uint16_t rgb565_be(uint8_t r, uint8_t g, uint8_t b) {
    return rgb565(r,g,b) >> 8 | rgb565(r,g,b) << 8;
}

// Function declarations
// createFramebuffer: littleEndian=1 for LE (default), 0 for BE; bgr=1 for BGR, 0 for RGB (default)
Framebuffer* createFramebuffer(int16_t width, int16_t height, uint8_t littleEndian, uint8_t bgr);
void destroyFramebuffer(Framebuffer* fb);
inline void clearFramebuffer(Framebuffer* fb, uint16_t color);

// Basic drawing primitives (inlined for performance)
inline void bufferDrawPixel(Framebuffer* fb, int16_t x, int16_t y, uint16_t color);
inline void bufferFillRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
inline void bufferDrawRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
inline void bufferDrawFastHLine(Framebuffer* fb, int16_t x, int16_t y, int16_t w, uint16_t color);
inline void bufferDrawFastVLine(Framebuffer* fb, int16_t x, int16_t y, int16_t h, uint16_t color);

// Lines and shapes
void bufferDrawLine(Framebuffer* fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

// Circles
void bufferDrawCircle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t r, uint16_t color);
void bufferFillCircle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t r, uint16_t color);

// Triangles
void bufferDrawTriangle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void bufferFillTriangle(Framebuffer* fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

// Ellipses
void bufferDrawEllipse(Framebuffer* fb, int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color);
void bufferFillEllipse(Framebuffer* fb, int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color);

// Rounded rectangles
void bufferDrawRoundRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void bufferFillRoundRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

// Text drawing (requires external font array)
void bufferDrawChar(Framebuffer* fb, int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font);
void bufferPrint(Framebuffer* fb, int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font);
void bufferPrintInt(Framebuffer* fb, int16_t x, int16_t y, int value, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font);
void bufferPrintFloat(Framebuffer* fb, int16_t x, int16_t y, float value, int decimals, uint16_t color, uint16_t bg, uint8_t size, const uint8_t* font);

// Buffer to buffer blit with clipping
void blitFramebuffer(Framebuffer* dest, int16_t destX, int16_t destY, Framebuffer* src, int16_t srcX, int16_t srcY, int16_t w, int16_t h);

// ============================================================================
// NEW: Flexible RGB565 image drawing with format options and RLE support
// ============================================================================

/**
 * Draw RGB565 image data to framebuffer with flexible format options
 * 
 * @param fb          Framebuffer to draw to
 * @param x           X position on screen
 * @param y           Y position on screen
 * @param image       RGB565 data array (from PROGMEM)
 * @param w           Image width in pixels
 * @param h           Image height in pixels
 * @param bgr         Color order: 0=RGB (default), 1=BGR
 * @param littleEndian Byte order: 1=little-endian (default), 0=big-endian
 * @param rle         RLE compression: 1=decompress, 0=raw (default)
 */
void bufferDrawImage(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h, 
                     uint8_t bgr, uint8_t littleEndian, uint8_t rle);

/**
 * Draw RGB565 image data with transparent color support
 * 
 * Same as bufferDrawImage but skips pixels matching the transparent color.
 * Useful for sprites with transparency.
 * 
 * @param fb               Framebuffer to draw to
 * @param x                X position on screen
 * @param y                Y position on screen
 * @param image            RGB565 data array (from PROGMEM)
 * @param w                Image width in pixels
 * @param h                Image height in pixels
 * @param bgr              Color order: 0=RGB (default), 1=BGR
 * @param littleEndian     Byte order: 1=little-endian (default), 0=big-endian
 * @param rle              RLE compression: 1=decompress, 0=raw (default)
 * @param transparentColor Color to skip (in framebuffer format!)
 */
void bufferDrawImageTransparent(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h, 
                                uint8_t bgr, uint8_t littleEndian, uint8_t rle, uint16_t transparentColor);

// Convenience wrappers for common formats (backward compatible default)
inline void bufferDrawImageRGB565_LE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 0, 1, 0);
}

inline void bufferDrawImageRGB565_BE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 0, 0, 0);
}

inline void bufferDrawImageBGR565_LE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 1, 1, 0);
}

inline void bufferDrawImageBGR565_BE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 1, 0, 0);
}

// RLE versions
inline void bufferDrawImageRGB565_LE_RLE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 0, 1, 1);
}

inline void bufferDrawImageRGB565_BE_RLE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 0, 0, 1);
}

inline void bufferDrawImageBGR565_LE_RLE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 1, 1, 1);
}

inline void bufferDrawImageBGR565_BE_RLE(Framebuffer* fb, int16_t x, int16_t y, const uint8_t* image, int16_t w, int16_t h) {
    bufferDrawImage(fb, x, y, image, w, h, 1, 0, 1);
}

// ============================================================================
// INLINE FUNCTION IMPLEMENTATIONS (for performance)
// ============================================================================

// Clear framebuffer to a color (optimized with memset for black)
inline void clearFramebuffer(Framebuffer* fb, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    if (color == 0) {
        memset(fb->buffer, 0, fb->width * fb->height * sizeof(uint16_t));
    } else {
        uint16_t* ptr = fb->buffer;
        int32_t count = (int32_t)fb->width * (int32_t)fb->height;
        while (count--) {
            *ptr++ = color;
        }
    }
}

// Draw a single pixel
inline void bufferDrawPixel(Framebuffer* fb, int16_t x, int16_t y, uint16_t color) {
    if (!fb || !fb->buffer) return;
    if (x >= 0 && x < fb->width && y >= 0 && y < fb->height) {
        *(fb->buffer + y * fb->width + x) = color;
    }
}

// Fill a rectangle
inline void bufferFillRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!fb || !fb->buffer) return;
    
    // Clip to screen bounds
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > fb->width) w = fb->width - x;
    if (y + h > fb->height) h = fb->height - y;
    if (w <= 0 || h <= 0) return;
    
    // Use pointer arithmetic
    uint16_t* ptr = fb->buffer + (y * fb->width + x);
    int16_t rowSkip = fb->width - w;
    
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            *ptr++ = color;
        }
        ptr += rowSkip;
    }
}

// Draw a rectangle outline
inline void bufferDrawRect(Framebuffer* fb, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!fb || !fb->buffer) return;
    if (w <= 0 || h <= 0) return;
    
    // Top line
    if (y >= 0 && y < fb->height) {
        int16_t x1 = max(0, x);
        int16_t x2 = min(fb->width, x + w);
        uint16_t* ptr = fb->buffer + (y * fb->width + x1);
        for (int16_t i = x1; i < x2; i++) {
            *ptr++ = color;
        }
    }
    
    // Bottom line
    int16_t by = y + h - 1;
    if (by >= 0 && by < fb->height) {
        int16_t x1 = max(0, x);
        int16_t x2 = min(fb->width, x + w);
        uint16_t* ptr = fb->buffer + (by * fb->width + x1);
        for (int16_t i = x1; i < x2; i++) {
            *ptr++ = color;
        }
    }
    
    // Left line
    if (x >= 0 && x < fb->width) {
        int16_t y1 = max(0, y);
        int16_t y2 = min(fb->height, y + h);
        uint16_t* ptr = fb->buffer + (y1 * fb->width + x);
        for (int16_t j = y1; j < y2; j++) {
            *ptr = color;
            ptr += fb->width;
        }
    }
    
    // Right line
    int16_t rx = x + w - 1;
    if (rx >= 0 && rx < fb->width) {
        int16_t y1 = max(0, y);
        int16_t y2 = min(fb->height, y + h);
        uint16_t* ptr = fb->buffer + (y1 * fb->width + rx);
        for (int16_t j = y1; j < y2; j++) {
            *ptr = color;
            ptr += fb->width;
        }
    }
}

// Draw a horizontal line
inline void bufferDrawFastHLine(Framebuffer* fb, int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (!fb || !fb->buffer) return;
    if (y < 0 || y >= fb->height || w <= 0) return;
    
    if (x < 0) { w += x; x = 0; }
    if (x + w > fb->width) w = fb->width - x;
    if (w <= 0) return;
    
    uint16_t* ptr = fb->buffer + (y * fb->width + x);
    for (int16_t i = 0; i < w; i++) {
        *ptr++ = color;
    }
}

// Draw a vertical line
inline void bufferDrawFastVLine(Framebuffer* fb, int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (!fb || !fb->buffer) return;
    if (x < 0 || x >= fb->width || h <= 0) return;
    
    if (y < 0) { h += y; y = 0; }
    if (y + h > fb->height) h = fb->height - y;
    if (h <= 0) return;
    
    uint16_t* ptr = fb->buffer + (y * fb->width + x);
    for (int16_t i = 0; i < h; i++) {
        *ptr = color;
        ptr += fb->width;
    }
}

#endif // FRAMEBUFFER_H
