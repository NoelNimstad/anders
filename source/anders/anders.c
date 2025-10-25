#include "anders.h"

// Endian
#ifdef __BIG_ENDIAN__
    #define UINT16_T_SWAP_ENDIAN(_v) ((((_v) & 0xff00) >> 8)\
                                    | (((_v) & 0x00ff) << 8))
    #define UINT32_T_SWAP_ENDIAN(_v) ((((_v) & 0xff000000) >> 24)\
                                    | (((_v) & 0x00ff0000) >> 8) \
                                    | (((_v) & 0x0000ff00) << 8) \
                                    | (((_v) & 0x000000ff) << 24))
#else
    #define UINT16_T_SWAP_ENDIAN(_v) (_v)
    #define UINT32_T_SWAP_ENDIAN(_v) (_v)
#endif

// BMP Header data
const uint32_t BMP_HEADER_SIZE      = 14;
const uint32_t DIB_HEADER_SIZE      = 40;
const uint32_t TOTAL_HEADER_SIZE    = BMP_HEADER_SIZE + DIB_HEADER_SIZE;

const uint16_t BITS_PER_PIXEL       = 24;
const uint16_t BYTES_PER_PIXEL      = BITS_PER_PIXEL / 8;

const uint16_t COLOR_PLANES         = 1;

const uint8_t PADDING[3] = { 0x00, 0x00, 0x00 }; // Max 3 bytes padding

struct Anders *Anders_Initialise(char *outputDir, uint32_t width, uint32_t height, uint8_t FPS)
{
    printf("Are you sure \"%s\" is your desired output directory? The directory's content will be cleared! (y/N) ", outputDir);
    char response;
    scanf("%c", &response);
    if(response != 'y')
    {
        return NULL;
    }

    struct Anders *a = (struct Anders *)malloc(1 * sizeof(struct Anders));
    if(NULL == a)
    {
        printf("Failed to allocate memory for Anders\n");
        goto fail_a;
    }

    char command[0xff];
    sprintf(command, "rm -r %s", outputDir);
    system(command);
    sprintf(command, "mkdir %s", outputDir);
    system(command);

    a->_FPS = FPS;
    a->_width = width;
    a->_height = height;
    a->_pixels = (struct _Anders_pixel *)malloc(a->_width * a->_height * sizeof(struct _Anders_pixel));
    if(NULL == a->_pixels)
    {
        printf("Failed to allocate memory for Anders' pixels\n");
        goto fail_pixels;
    }

    a->frame = 0;
    a->_shards = 0;
    a->_outputDir = outputDir;
    a->shardFrequency = 300;

    // Compute image header data
    uint32_t ROW_SIZE_IN_BYTES = a->_width * BYTES_PER_PIXEL;
    a->_PADDING_BYTES = (4 - (ROW_SIZE_IN_BYTES % 4)) % 4; // Row sizes must be a multiple of 4

    uint32_t PADDED_ROW_SIZE = ROW_SIZE_IN_BYTES + a->_PADDING_BYTES;

    a->_PIXEL_DATA_SIZE = PADDED_ROW_SIZE * a->_height; // Total pixel data size

    uint32_t FILE_SIZE = TOTAL_HEADER_SIZE + a->_PIXEL_DATA_SIZE;
    
    // Construct BMP header
    a->_BMPHeaderBytes = (uint8_t *)malloc(BMP_HEADER_SIZE * sizeof(uint8_t));
    if(NULL == a->_BMPHeaderBytes)
    {
        printf("Failed to allocate memory for BMP header data\n");
        goto fail_BMPHeaderBytes;
    }
    a->_BMPHeaderBytes[0] = 0x42;
    a->_BMPHeaderBytes[1] = 0x4d; // BMP file signature

    memcpy(&a->_BMPHeaderBytes[2], &FILE_SIZE, 4);
    memcpy(&a->_BMPHeaderBytes[10], &TOTAL_HEADER_SIZE, 4); // header size

    // Construct DIB header
    a->_DIBHeaderBytes = (uint8_t *)malloc(DIB_HEADER_SIZE * sizeof(uint8_t));
    if(NULL == a->_DIBHeaderBytes)
    {
        printf("Failed to allocate memory for DIB header data\n");
        goto fail_DIBHeaderBytes;
    }
    memcpy(&a->_DIBHeaderBytes[0], &DIB_HEADER_SIZE, 4);

    memcpy(&a->_DIBHeaderBytes[4], &a->_width, 4);
    int32_t signedHeight = (int32_t)a->_height; 
    memcpy(&a->_DIBHeaderBytes[8], &signedHeight, 4); // image sizing

    memcpy(&a->_DIBHeaderBytes[12], &COLOR_PLANES, 2);

    memcpy(&a->_DIBHeaderBytes[14], &BITS_PER_PIXEL, 2);

    memcpy(&a->_DIBHeaderBytes[20], &a->_PIXEL_DATA_SIZE, 4); // image size

    // Alocate memory for pixel buffer
    a->_rawPixelBuffer = (uint8_t *)malloc(a->_PIXEL_DATA_SIZE);
    if(NULL == a->_rawPixelBuffer)
    {
        printf("Failed to allocate pixel buffer.\n");
        goto fail_rawPixelBuffer;
    }

    return a;

fail_rawPixelBuffer:
    free(a->_DIBHeaderBytes);
fail_DIBHeaderBytes:
    free(a->_BMPHeaderBytes);
fail_BMPHeaderBytes:
    free(a->_pixels);
fail_pixels:
    free(a);
fail_a:
    return NULL;
};

void Anders_Destroy(struct Anders *a)
{
    free(a->_pixels);
    free(a->_BMPHeaderBytes);
    free(a->_DIBHeaderBytes);
    free(a->_rawPixelBuffer);
    free(a);
}

void Anders_Clear(struct Anders *a, uint8_t r, uint8_t g, uint8_t b)
{
    struct _Anders_pixel targetPixel = { .r = r, .g = g, .b = b };
    for(size_t i = 0; i < a->_width * a->_height; i++)
    {
        a->_pixels[i] = targetPixel;
    }
}

static void _Anders_Shard(struct Anders *a)
{
    char command[0xff << 2];
    sprintf(command, "ffmpeg -framerate %d -i %s%s.bmp -c:v libx264 -qp 0 %s_anders_shard%hu.mp4", a->_FPS, a->_outputDir, "%08d", a->_outputDir, a->_shards);
    system(command);
    sprintf(command, "rm %s*.bmp", a->_outputDir);
    system(command);

    a->_shards++;
}

/*
    https://en.wikipedia.org/wiki/BMP_file_format

    The BMP file architecture (little endian, bottom up)

    ------------------------------ BMP header -----------------
    0x42 0x4d                      file signature
    4 bytes                        file size
    2 bytes                        unused
    2 bytes                        unused
    4 bytes                        data start offset
    ------------------------------ DIB header -----------------
    4 byes                         header size
    4 bytes                        image width
    4 bytes                        image height
    2 bytes                        number of color planes
    2 bytes                        bits per pixel
    4 bytes                        no compression
    4 bytes                        size of raw bitmap
    4 bytes                        horizontal print resolution
    4 bytes                        vertical print resolution
    4 bytes                        colors in palette
    4 bytes                        important colors (0 for all)
    ------------------------------ Bitmap data ----------------
    PIXEL DATA
*/

void Anders_Frame(struct Anders *a)
{
    // Open file
    char filename[0xff];
    sprintf(filename, "%s%08d.bmp", a->_outputDir, a->frame % a->shardFrequency); 
    FILE *fptr = fopen(filename, "wb");

    if(NULL == fptr)
    {
        printf("Failed to open file \"%s\" for rendering.\n", filename);
        return;
    }

    // Write headers to file
    fwrite(a->_BMPHeaderBytes, 1, BMP_HEADER_SIZE, fptr);
    fwrite(a->_DIBHeaderBytes, 1, DIB_HEADER_SIZE, fptr);
    
    // Write pixel data
    uint8_t *pixelPointer = a->_rawPixelBuffer;
    for(int32_t y = a->_height - 1; y >= 0; y--) // BMP stores data bottom up
    {
        size_t rowStartIndex = y * a->_width;
        
        for(uint16_t x = 0; x < a->_width; x++)
        {
            struct _Anders_pixel *pixel = &a->_pixels[rowStartIndex + x];

            // Write individual byes for pixels
            *pixelPointer++ = pixel->b;
            *pixelPointer++ = pixel->g;
            *pixelPointer++ = pixel->r;
        }

        // Pad if needed
        if(a->_PADDING_BYTES > 0)
        {
            memcpy(pixelPointer, PADDING, a->_PADDING_BYTES);
            pixelPointer += a->_PADDING_BYTES;
        }
    }

    fwrite(a->_rawPixelBuffer, 1, a->_PIXEL_DATA_SIZE, fptr);

    fclose(fptr);

    a->frame++;
    if(a->frame % a->shardFrequency == 0) _Anders_Shard(a);
}

void Anders_Rectangle(struct Anders *a, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b)
{
    struct _Anders_pixel targetPixel = { .r = r, .g = g, .b = b };
    for(size_t _y = y; _y < y + height; _y++)
    {
        size_t startIndex = _y * a->_width;
        for(size_t _x = x; _x < x + width; _x++)
        {
            a->_pixels[startIndex + _x] = targetPixel;
        }   
    }
}

void Anders_Compose(struct Anders *a)
{
    if(a->frame % a->shardFrequency != 0) _Anders_Shard(a);

    char command[0xff << 2];
    for(size_t i = 0; i < a->_shards; i++)
    {
        sprintf(command, "echo \"file '_anders_shard%zu.mp4'\" >> %s_anders_shards.txt", i, a->_outputDir);
        system(command);
    }
    sprintf(command, "ffmpeg -f concat -i %s_anders_shards.txt -c copy -qp 0 %sanders.mp4", a->_outputDir, a->_outputDir);
    system(command);
    sprintf(command, "rm %s_anders_shard*", a->_outputDir);
    system(command);
    return;
}

void Anders_Circle(struct Anders *a, uint16_t x, uint16_t y, uint16_t radius, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t r2 = radius * radius;

    struct _Anders_pixel targetPixel = { .r = r, .g = g, .b = b };

    for(int32_t _x = -radius; _x <= radius; _x++)
    {
        uint32_t x2 = _x * _x;

        for(int32_t _y = -radius; _y <= radius; _y++)
        {
            int32_t px = x + _x;
            int32_t py = y + _y;

            if (x2 + (_y * _y) <= r2 &&
                px >= 0 && px < a->_width &&
                py >= 0 && py < a->_height)
            {
                a->_pixels[px + py * a->_width] = targetPixel;
            }
        }
    }
}