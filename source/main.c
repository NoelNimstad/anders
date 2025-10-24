#include <stdlib.h>
#include "anders/anders.h"
#include "anders/palette.h"

int main(void)
{
    srand(100);

    struct Anders *a = Anders_Initialise("render/", 400, 300, 60);
    if(NULL == a)
    {
        return 1;
    }

    for(size_t i = 0; i < 100; i++)
    {
        Anders_Palette_Clear(a, Anders_Palette_ColorLerp(GooglePalette[GOOGLE_BLUE_MEDIUM], GooglePalette[GOOGLE_GREEN_MEDIUM], (float)i / 100));
        // Anders_Palette_Rectangle(a, 3, 3, 394, 294, Anders_Palette_MakeUniformColor(10));
        Anders_Frame(a);
    }

    for(size_t i = 0; i < 100; i++)
    {
        Anders_Palette_Clear(a, Anders_Palette_ColorLerp(GooglePalette[GOOGLE_GREEN_MEDIUM], GooglePalette[GOOGLE_YELLOW_MEDIUM], (float)i / 100));
        // Anders_Palette_Rectangle(a, 3, 3, 394, 294, Anders_Palette_MakeUniformColor(10));
        Anders_Frame(a);
    }

    for(size_t i = 0; i < 100; i++)
    {
        Anders_Palette_Clear(a, Anders_Palette_ColorLerp(GooglePalette[GOOGLE_YELLOW_MEDIUM], GooglePalette[GOOGLE_RED_MEDIUM], (float)i / 100));
        // Anders_Palette_Rectangle(a, 3, 3, 394, 294, Anders_Palette_MakeUniformColor(10));
        Anders_Frame(a);
    }

    Anders_Compose(a);
    
    Anders_Destroy(a);

    return 0;
}