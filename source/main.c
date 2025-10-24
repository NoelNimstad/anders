#include <stdlib.h>
#include "anders/anders.h"
#include "anders/palette.h"

int main(void)
{
    srand(100);

    struct Anders *a = Anders_Initialise("render/", 400, 300, 120);
    if(NULL == a)
    {
        return 1;
    }

    for(size_t i = 0; i < 100; i++)
    {
        Anders_Palette_Circle(a, 200, 200, i + 1, GooglePalette[GOOGLE_BLUE_MEDIUM]);
        Anders_Frame(a);
    }

    Anders_Compose(a);
    Anders_Destroy(a);

    return 0;
}