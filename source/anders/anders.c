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

static const char *_Anders_CompressionStrings[] =
{
    "-c:v libx264 -preset medium -crf 23",      // Default
    "-c:v ffv1 -level 3 -threads 8",            // Uncompressed
    "-c:v libx264 -preset ultrafast -crf 28",   // Speed optimized
    "-c:v libx264 -preset veryslow -crf 18"     // Size optimized
};

struct Anders *Anders_Initialise(char *outputDir, uint32_t width, uint32_t height, uint8_t FPS, enum Anders_CompressionSetting compression)
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

    char command[0xff << 2];
    sprintf(command, "rm -r %s", outputDir);
    system(command);
    sprintf(command, "mkdir %s", outputDir);
    system(command);

    a->_FPS = FPS;
    a->_width = width;
    a->_height = height;
    a->_outputDir = outputDir;
    sprintf(command, "ffmpeg -f rawvideo -pix_fmt bgr24 -s %ux%u -r %d -i - %s %s/anders.mp4",
                     a->_width, a->_height, a->_FPS,
                     _Anders_CompressionStrings[compression],
                     a->_outputDir);
    a->_FFmpeg = popen(command, "w");
    if(NULL == a->_FFmpeg)
    {
        printf("Failed to open pipe to FFmpeg\n");
        goto fail_ffmpeg;
    }

    a->_pixels = (struct _Anders_pixel *)malloc(a->_width * a->_height * sizeof(struct _Anders_pixel));
    if(NULL == a->_pixels)
    {
        printf("Failed to allocate memory for Anders' pixels\n");
        goto fail_pixels;
    }

    a->BMPCount = 0;
    a->frame = 0;

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
fail_ffmpeg:
    free(a);
fail_a:
    return NULL;
};

void Anders_Destroy(struct Anders *a)
{
    if(NULL == a) return;

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

static void _Anders_PrepareRawPixelBuffer(struct Anders *a)
{
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
}

void Anders_Frame(struct Anders *a)
{   
    _Anders_PrepareRawPixelBuffer(a);

    size_t written = fwrite(a->_rawPixelBuffer, 1, a->_PIXEL_DATA_SIZE, a->_FFmpeg);
    if(written != a->_PIXEL_DATA_SIZE)
    {
        printf("Failed to pipe full data to FFmpeg on frame %u\n", a->frame);
    }

    a->frame++;
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

void Anders_SaveAsBMP(struct Anders *a)
{
    // Open file
    char filename[0xff];
    sprintf(filename, "%s%08d.bmp", a->_outputDir, a->BMPCount); 
    FILE *fptr = fopen(filename, "wb");

    if(NULL == fptr)
    {
        printf("Failed to open file \"%s\" for rendering.\n", filename);
        return;
    }

    // Write headers to file
    fwrite(a->_BMPHeaderBytes, 1, BMP_HEADER_SIZE, fptr);
    fwrite(a->_DIBHeaderBytes, 1, DIB_HEADER_SIZE, fptr);
    
    // Prepare pixel data
    _Anders_PrepareRawPixelBuffer(a);

    fwrite(a->_rawPixelBuffer, 1, a->_PIXEL_DATA_SIZE, fptr);

    fclose(fptr);

    a->BMPCount++;
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
    int status = pclose(a->_FFmpeg);
    if(status != 0)
    {
        printf("FFmpeg failed to compose video\n");
    }

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