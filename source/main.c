#include <stdlib.h>
#include "anders/anders.h"
#include "anders/palette.h"
#include "anders/3D.h"

int main(void)
{
    struct Anders *a = Anders_Initialise("render/", 400, 300, 60, ANDERS_COMPRESSION_SPEED_OPTIMIZED);
    if(NULL == a)
    {
        return 1;
    }

    struct Anders_3D_Camera camera = { .theta = 0, .phi = -0.2, 
                                       .x = 0, .y = 0, .z = 0 };

    for(size_t i = 0; i < 120; i++)
    {
        Anders_Palette_Clear(a, GooglePalette[GOOGLE_BLUE_MEDIUM]);
        Anders_Palette_Triangle(a, 100 + i * 2, 300 - i, 200, 50, 300 - i, 150, GooglePalette[GOOGLE_YELLOW_MEDIUM]);
        Anders_Frame(a);
    }

    Anders_Compose(a);
    Anders_Destroy(a);

    return 0;
}