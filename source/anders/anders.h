#ifndef ANDERS_H
#define ANDERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct _Anders_pixel
{
    uint8_t r, g, b;
};

struct Anders
{
    // public
    uint16_t BMPCount;
    uint32_t frame;

    // private
    struct _Anders_pixel *_pixels;
    uint32_t _width;
    uint32_t _height;
    uint8_t _FPS;
    char *_outputDir;
    FILE *_FFmpeg;

    // BMP
    uint8_t *_BMPHeaderBytes;
    uint8_t *_DIBHeaderBytes;
    uint8_t *_rawPixelBuffer;
    uint32_t _PIXEL_DATA_SIZE;
    uint32_t _PADDING_BYTES;
};

enum Anders_CompressionSetting
{
    ANDERS_COMPRESSION_DEFAULT = 0,
    ANDERS_COMPRESSION_UNCOMPRESSED = 1,
    ANDERS_COMPRESSION_SPEED_OPTIMIZED = 2,
    ANDERS_COMPRESSION_SIZE_OPTIMIZED = 3
};

/**
 * @brief Initialises the Anders drawing context
 * 
 * @param outputDir Working output directory. Obs. WILL BE CLEARED!
 * @param width Video width
 * @param height Video height
 * @param FPS Video framerate
 * @return `struct Anders*`: A pointer towards to an Anders drawing context
 */
struct Anders *Anders_Initialise(char *outputDir, uint32_t width, uint32_t height, uint8_t FPS, enum Anders_CompressionSetting compression);
/**
 * @brief Destroys the current Anders drawing context
 * 
 * @param a A pointer to the current Anders drawing context
 */
void Anders_Destroy(struct Anders *a);

/**
 * @brief Clears screen with certain color
 * 
 * @param a A pointer to the current Anders drawing context
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void Anders_Clear(struct Anders *a, uint8_t r, uint8_t g, uint8_t b);
/**
 * @brief Draws a rectangle
 * 
 * @param a A pointer to the current Anders drawing context
 * @param x The x position of the top left corner
 * @param y The y position of the top left corner
 * @param width The width of the rectangle
 * @param height The height of the rectangle
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void Anders_Rectangle(struct Anders *a, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b);
/**
 * @brief Draws a circle
 * 
 * @param a A pointer to the current Anders drawing context
 * @param x The x position of the center of the circle
 * @param y The y position of the center of the circle
 * @param radius The radius of the cricle
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void Anders_Circle(struct Anders *a, uint16_t x, uint16_t y, uint16_t radius, uint8_t r, uint8_t g, uint8_t b);
/**
 * @brief Draws a triangle
 * 
 * @param a A pointer to the current Anders drawing context
 * @param x1 X coordinate of vertex 1
 * @param y1 Y coordinate of vertex 1
 * @param x2 X coordinate of vertex 2
 * @param y2 Y coordinate of vertex 2
 * @param x3 X coordinate of vertex 3
 * @param y3 Y coordinate of vertex 3
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void Anders_Triangle(struct Anders *a, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Pipes current frame data to FFmpeg
 * 
 * @param a A pointer to the current Anders drawing context
 */
void Anders_Frame(struct Anders *a);
/**
 * @brief Saves current drawing as a BMP file
 * 
 * @param a A pointer to the current Anders drawing context
 */
void Anders_SaveAsBMP(struct Anders *a);
/**
 * @brief Pieces together video shards and remaining unsharded frames
 * 
 * @param a A pointer to the current Anders drawing context
 */
void Anders_Compose(struct Anders *a);

#endif // ANDERS_H