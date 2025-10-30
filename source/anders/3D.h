#ifndef ANDERS_THREE_H
#define ANDERS_THREE_H

#include <stdlib.h>
#include "anders.h"

struct Anders_3D_Vertex
{
    uint16_t x, y, z;
};

struct Anders_3D_Triangle
{
    struct Anders_3D_Vertex vertices[3];
};

struct Anders_3D_Camera
{
    float theta, phi;
    uint16_t x, y, z;
};

void Anders_3D_ComputeCameraMatrices(struct Anders_3D_Camera *camera);
/**
 * @brief Renders a triangle in 3D space
 * 
 * @param a A pointer to the current Anders drawing context
 * @param camera The camera perspective
 * @param triangle The triangle to rende
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void Anders_3D_RenderTriangle(struct Anders *a, struct Anders_3D_Camera camera, struct Anders_3D_Triangle triangle, uint8_t r, uint8_t g, uint8_t b);

#endif // ANDERS_THREE_H