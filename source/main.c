#include <stdlib.h>
#include "anders/anders.h"
#include "anders/palette.h"

int main(void)
{
    srand(100);

    struct Anders *a = Anders_Initialise("render/", 3024, 1964, 120);
    if(NULL == a)
    {
        return 1;
    }

    size_t i = 0;
    for(size_t j = 0; j < 250; j++)
    {
        struct Anders_Color color = Anders_Palette_ColorLerp(GooglePalette[GOOGLE_BLUE_MEDIUM], GooglePalette[GOOGLE_GREEN_MEDIUM], (float)i++ / 100);
        Anders_Palette_Clear(a, Anders_Palette_InvertColor(color));
        Anders_Palette_Circle(a, 1514, 982, i, color);
        Anders_Frame(a);
    }
    for(size_t j = 0; j < 250; j++)
    {
        struct Anders_Color color = Anders_Palette_ColorLerp(GooglePalette[GOOGLE_BLUE_MEDIUM], GooglePalette[GOOGLE_GREEN_MEDIUM], (float)i++ / 100);
        Anders_Palette_Clear(a, Anders_Palette_InvertColor(color));
        Anders_Palette_Rectangle(a, 1514 - i, 982 - i, i * 2, i * 2, color);
        Anders_Frame(a);
    }

    Anders_Compose(a);
    Anders_Destroy(a);

    return 0;
}