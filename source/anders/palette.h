#ifndef ANDERS_PALETTE_H
#define ANDERS_PALETTE_H

#include <stdlib.h>
#include "anders.h"

struct Anders_Color
{
    uint8_t r, g, b;
};
/**
 * @brief Makes a color with the same red, green and blue values.
 * 
 * @param color The color value
 * @return The uniform color
 */
inline struct Anders_Color Anders_Palette_MakeUniformColor(uint8_t color)
{
    return (struct Anders_Color){ .r = color, .g = color, .b = color };
}
/**
 * @brief Lerps between two colors.
 * 
 * @param c1 Color 1
 * @param c2 Color 2
 * @param t Lerp percentage such that t ∈ [0, 1]
 * @return The lerped color
 */
inline struct Anders_Color Anders_Palette_ColorLerp(struct Anders_Color c1, struct Anders_Color c2, float t)
{
    return (struct Anders_Color) { .r = c2.r * t + c1.r * (1.0f - t),
                                   .g = c2.g * t + c1.g * (1.0f - t),
                                   .b = c2.b * t + c1.b * (1.0f - t)};
}
/**
 * @brief Inverts a color.
 * 
 * @param color The color to invert
 * @return The inverted color
 */
inline struct Anders_Color Anders_Palette_InvertColor(struct Anders_Color color)
{
    return (struct Anders_Color){ .r = 255 - color.r, .g = 255 - color.g, .b = 255 - color.b };
}

/**
 * @brief Clears the screen with a color
 * 
 * @param _a The Anders instance
 * @param _c The color to draw with
 */
#define Anders_Palette_Clear(_a, _c) Anders_Clear((_a), (_c).r, (_c).g, (_c).b)
/**
 * @brief Draws a rectangle with a color
 * 
 * @param _a The Anders instance
 * @param _x The x position of the top left corner
 * @param _y The y position of the top left corner
 * @param _w The width of the rectangle
 * @param _h The height of the rectangle
 * @param _c The color to draw with
 */
#define Anders_Palette_Rectangle(_a, _x, _y, _w, _h, _c) Anders_Rectangle((_a), (_x), (_y), (_w), (_h), (_c).r, (_c).g, (_c).b)
/**
 * @brief Draws a circle with a color
 * 
 * @param _a The Anders instance
 * @param _x The x position of the center of the circle
 * @param _y The y position of the center of the circle
 * @param _r The radius of the cricle
 * @param _c The color to draw with
 */
#define Anders_Palette_Circle(_a, _x, _y, _r, _c) Anders_Circle((_a), (_x), (_y), (_r), (_c).r, (_c).g, (_c).b)
/**
 * @brief Draws a triangle with a color
 * 
 * @param a A pointer to the current Anders drawing context
 * @param x1 X coordinate of vertex 1
 * @param y1 Y coordinate of vertex 1
 * @param x2 X coordinate of vertex 2
 * @param y2 Y coordinate of vertex 2
 * @param x3 X coordinate of vertex 3
 * @param y3 Y coordinate of vertex 3
 * @param _c The color to draw with
 */
#define Anders_Palette_Triangle(_a, _x1, _y1, _x2, _y2, _x3, _y3, _c) Anders_Triangle((_a), (_x1), (_y1), (_x2), (_y2), (_x3), (_y3), (_c).r, (_c).g, (_c).b)

#define GOOGLE_BLUE_MEDIUM      0
#define GOOGLE_RED_MEDIUM       1
#define GOOGLE_YELLOW_MEDIUM    2
#define GOOGLE_GREEN_MEDIUM     3
struct Anders_Color GooglePalette[] = { { 66, 103, 210 }, { 234, 67, 53 }, { 251, 188, 4 }, { 52, 168, 83 } };

#endif // ANDERS_PALETTE_H