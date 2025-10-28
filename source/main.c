#include <stdlib.h>
#include "anders/anders.h"
#include "anders/palette.h"

int main(void)
{
    struct Anders *a = Anders_Initialise("render/", 3024, 1964, 60);
    if(NULL == a)
    {
        return 1;
    }

    for(size_t i = 0; i < 982; i++)
    {
        struct Anders_Color color = Anders_Palette_ColorLerp(GooglePalette[GOOGLE_BLUE_MEDIUM], GooglePalette[GOOGLE_GREEN_MEDIUM], (float)i / 100);
        Anders_Palette_Clear(a, Anders_Palette_InvertColor(color));
        Anders_Palette_Circle(a, 1514, 982, i, color);
        Anders_Frame(a);
    }

    Anders_SaveAsBMP(a);

    Anders_Compose(a);
    Anders_Destroy(a);

    return 0;
}