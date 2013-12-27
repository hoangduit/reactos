/*
 * Copyright (C) 2007 Google (Evan Stade)
 * Copyright (C) 2012 Dmitry Timoshkov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "gdiplus_private.h"

#include <assert.h>
#include <olectl.h>

#define PIXELFORMATBPP(x) ((x) ? ((x) >> 8) & 255 : 24)

static const struct
{
    const WICPixelFormatGUID *wic_format;
    PixelFormat gdip_format;
    /* predefined palette type to use for pixel format conversions */
    WICBitmapPaletteType palette_type;
} pixel_formats[] =
{
    { &GUID_WICPixelFormatBlackWhite, PixelFormat1bppIndexed, WICBitmapPaletteTypeFixedBW },
    { &GUID_WICPixelFormat1bppIndexed, PixelFormat1bppIndexed, WICBitmapPaletteTypeFixedBW },
    { &GUID_WICPixelFormat8bppGray, PixelFormat8bppIndexed, WICBitmapPaletteTypeFixedGray256 },
    { &GUID_WICPixelFormat8bppIndexed, PixelFormat8bppIndexed, WICBitmapPaletteTypeFixedHalftone256 },
    { &GUID_WICPixelFormat16bppBGR555, PixelFormat16bppRGB555, WICBitmapPaletteTypeFixedHalftone256 },
    { &GUID_WICPixelFormat24bppBGR, PixelFormat24bppRGB, WICBitmapPaletteTypeFixedHalftone256 },
    { &GUID_WICPixelFormat32bppBGR, PixelFormat32bppRGB, WICBitmapPaletteTypeFixedHalftone256 },
    { &GUID_WICPixelFormat32bppBGRA, PixelFormat32bppARGB, WICBitmapPaletteTypeFixedHalftone256 },
    { &GUID_WICPixelFormat32bppPBGRA, PixelFormat32bppPARGB, WICBitmapPaletteTypeFixedHalftone256 },
    { NULL }
};

static ColorPalette *get_palette(IWICBitmapFrameDecode *frame, WICBitmapPaletteType palette_type)
{
    HRESULT hr;
    IWICImagingFactory *factory;
    IWICPalette *wic_palette;
    ColorPalette *palette = NULL;

    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IWICImagingFactory, (void **)&factory);
    if (hr != S_OK) return NULL;

    hr = IWICImagingFactory_CreatePalette(factory, &wic_palette);
    if (hr == S_OK)
    {
        hr = WINCODEC_ERR_PALETTEUNAVAILABLE;
        if (frame)
            hr = IWICBitmapFrameDecode_CopyPalette(frame, wic_palette);
        if (hr != S_OK)
        {
            TRACE("using predefined palette %#x\n", palette_type);
            hr = IWICPalette_InitializePredefined(wic_palette, palette_type, FALSE);
        }
        if (hr == S_OK)
        {
            UINT count;
            BOOL mono, gray;

            IWICPalette_IsBlackWhite(wic_palette, &mono);
            IWICPalette_IsGrayscale(wic_palette, &gray);

            IWICPalette_GetColorCount(wic_palette, &count);
            palette = HeapAlloc(GetProcessHeap(), 0, 2 * sizeof(UINT) + count * sizeof(ARGB));
            IWICPalette_GetColors(wic_palette, count, palette->Entries, &palette->Count);

            if (mono)
                palette->Flags = 0;
            else if (gray)
                palette->Flags = PaletteFlagsGrayScale;
            else
                palette->Flags = PaletteFlagsHalftone;
        }
        IWICPalette_Release(wic_palette);
    }
    IWICImagingFactory_Release(factory);
    return palette;
}

static INT ipicture_pixel_height(IPicture *pic)
{
    HDC hdcref;
    OLE_YSIZE_HIMETRIC y;

    IPicture_get_Height(pic, &y);

    hdcref = CreateCompatibleDC(0);
    y = MulDiv(y, GetDeviceCaps(hdcref, LOGPIXELSY), INCH_HIMETRIC);
    DeleteDC(hdcref);

    return y;
}

static INT ipicture_pixel_width(IPicture *pic)
{
    HDC hdcref;
    OLE_XSIZE_HIMETRIC x;

    IPicture_get_Width(pic, &x);

    hdcref = CreateCompatibleDC(0);
    x = MulDiv(x, GetDeviceCaps(hdcref, LOGPIXELSX), INCH_HIMETRIC);
    DeleteDC(hdcref);

    return x;
}

GpStatus WINGDIPAPI GdipBitmapApplyEffect(GpBitmap* bitmap, CGpEffect* effect,
    RECT* roi, BOOL useAuxData, VOID** auxData, INT* auxDataSize)
{
    FIXME("(%p %p %p %d %p %p): stub\n", bitmap, effect, roi, useAuxData, auxData, auxDataSize);
    /*
     * Note: According to Jose Roca's GDI+ docs, this function is not
     * implemented in Windows's GDI+.
     */
    return NotImplemented;
}

GpStatus WINGDIPAPI GdipBitmapCreateApplyEffect(GpBitmap** inputBitmaps,
    INT numInputs, CGpEffect* effect, RECT* roi, RECT* outputRect,
    GpBitmap** outputBitmap, BOOL useAuxData, VOID** auxData, INT* auxDataSize)
{
    FIXME("(%p %d %p %p %p %p %d %p %p): stub\n", inputBitmaps, numInputs, effect, roi, outputRect, outputBitmap, useAuxData, auxData, auxDataSize);
    /*
     * Note: According to Jose Roca's GDI+ docs, this function is not
     * implemented in Windows's GDI+.
     */
    return NotImplemented;
}

static inline void getpixel_1bppIndexed(BYTE *index, const BYTE *row, UINT x)
{
    *index = (row[x/8]>>(7-x%8)) & 1;
}

static inline void getpixel_4bppIndexed(BYTE *index, const BYTE *row, UINT x)
{
    if (x & 1)
        *index = row[x/2]&0xf;
    else
        *index = row[x/2]>>4;
}

static inline void getpixel_8bppIndexed(BYTE *index, const BYTE *row, UINT x)
{
    *index = row[x];
}

static inline void getpixel_16bppGrayScale(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *r = *g = *b = row[x*2+1];
    *a = 255;
}

static inline void getpixel_16bppRGB555(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    WORD pixel = *((const WORD*)(row)+x);
    *r = (pixel>>7&0xf8)|(pixel>>12&0x7);
    *g = (pixel>>2&0xf8)|(pixel>>6&0x7);
    *b = (pixel<<3&0xf8)|(pixel>>2&0x7);
    *a = 255;
}

static inline void getpixel_16bppRGB565(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    WORD pixel = *((const WORD*)(row)+x);
    *r = (pixel>>8&0xf8)|(pixel>>13&0x7);
    *g = (pixel>>3&0xfc)|(pixel>>9&0x3);
    *b = (pixel<<3&0xf8)|(pixel>>2&0x7);
    *a = 255;
}

static inline void getpixel_16bppARGB1555(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    WORD pixel = *((const WORD*)(row)+x);
    *r = (pixel>>7&0xf8)|(pixel>>12&0x7);
    *g = (pixel>>2&0xf8)|(pixel>>6&0x7);
    *b = (pixel<<3&0xf8)|(pixel>>2&0x7);
    if ((pixel&0x8000) == 0x8000)
        *a = 255;
    else
        *a = 0;
}

static inline void getpixel_24bppRGB(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *r = row[x*3+2];
    *g = row[x*3+1];
    *b = row[x*3];
    *a = 255;
}

static inline void getpixel_32bppRGB(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *r = row[x*4+2];
    *g = row[x*4+1];
    *b = row[x*4];
    *a = 255;
}

static inline void getpixel_32bppARGB(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *r = row[x*4+2];
    *g = row[x*4+1];
    *b = row[x*4];
    *a = row[x*4+3];
}

static inline void getpixel_32bppPARGB(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *a = row[x*4+3];
    if (*a == 0)
        *r = *g = *b = 0;
    else
    {
        *r = row[x*4+2] * 255 / *a;
        *g = row[x*4+1] * 255 / *a;
        *b = row[x*4] * 255 / *a;
    }
}

static inline void getpixel_48bppRGB(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *r = row[x*6+5];
    *g = row[x*6+3];
    *b = row[x*6+1];
    *a = 255;
}

static inline void getpixel_64bppARGB(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *r = row[x*8+5];
    *g = row[x*8+3];
    *b = row[x*8+1];
    *a = row[x*8+7];
}

static inline void getpixel_64bppPARGB(BYTE *r, BYTE *g, BYTE *b, BYTE *a,
    const BYTE *row, UINT x)
{
    *a = row[x*8+7];
    if (*a == 0)
        *r = *g = *b = 0;
    else
    {
        *r = row[x*8+5] * 255 / *a;
        *g = row[x*8+3] * 255 / *a;
        *b = row[x*8+1] * 255 / *a;
    }
}

GpStatus WINGDIPAPI GdipBitmapGetPixel(GpBitmap* bitmap, INT x, INT y,
    ARGB *color)
{
    BYTE r, g, b, a;
    BYTE index;
    BYTE *row;

    if(!bitmap || !color ||
       x < 0 || y < 0 || x >= bitmap->width || y >= bitmap->height)
        return InvalidParameter;

    row = bitmap->bits+bitmap->stride*y;

    switch (bitmap->format)
    {
        case PixelFormat1bppIndexed:
            getpixel_1bppIndexed(&index,row,x);
            break;
        case PixelFormat4bppIndexed:
            getpixel_4bppIndexed(&index,row,x);
            break;
        case PixelFormat8bppIndexed:
            getpixel_8bppIndexed(&index,row,x);
            break;
        case PixelFormat16bppGrayScale:
            getpixel_16bppGrayScale(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat16bppRGB555:
            getpixel_16bppRGB555(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat16bppRGB565:
            getpixel_16bppRGB565(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat16bppARGB1555:
            getpixel_16bppARGB1555(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat24bppRGB:
            getpixel_24bppRGB(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat32bppRGB:
            getpixel_32bppRGB(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat32bppARGB:
            getpixel_32bppARGB(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat32bppPARGB:
            getpixel_32bppPARGB(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat48bppRGB:
            getpixel_48bppRGB(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat64bppARGB:
            getpixel_64bppARGB(&r,&g,&b,&a,row,x);
            break;
        case PixelFormat64bppPARGB:
            getpixel_64bppPARGB(&r,&g,&b,&a,row,x);
            break;
        default:
            FIXME("not implemented for format 0x%x\n", bitmap->format);
            return NotImplemented;
    }

    if (bitmap->format & PixelFormatIndexed)
        *color = bitmap->image.palette->Entries[index];
    else
        *color = a<<24|r<<16|g<<8|b;

    return Ok;
}

static inline UINT get_palette_index(BYTE r, BYTE g, BYTE b, BYTE a, ColorPalette *palette)
{
    BYTE index = 0;
    int best_distance = 0x7fff;
    int distance;
    UINT i;

    if (!palette) return 0;
    /* This algorithm scans entire palette,
       computes difference from desired color (all color components have equal weight)
       and returns the index of color with least difference.

       Note: Maybe it could be replaced with a better algorithm for better image quality
       and performance, though better algorithm would probably need some pre-built lookup
       tables and thus may actually be slower if this method is called only few times per
       every image.
    */
    for(i=0;i<palette->Count;i++) {
        ARGB color=palette->Entries[i];
        distance=abs(b-(color & 0xff)) + abs(g-(color>>8 & 0xff)) + abs(r-(color>>16 & 0xff)) + abs(a-(color>>24 & 0xff));
        if (distance<best_distance) {
            best_distance=distance;
            index=i;
        }
    }
    return index;
}

static inline void setpixel_8bppIndexed(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x, ColorPalette *palette)
{
     BYTE index = get_palette_index(r,g,b,a,palette);
     row[x]=index;
}

static inline void setpixel_1bppIndexed(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x, ColorPalette *palette)
{
    row[x/8]  = (row[x/8] & ~(1<<(7-x%8))) | (get_palette_index(r,g,b,a,palette)<<(7-x%8));
}

static inline void setpixel_4bppIndexed(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x, ColorPalette *palette)
{
    if (x & 1)
        row[x/2] = (row[x/2] & 0xf0) | get_palette_index(r,g,b,a,palette);
    else
        row[x/2] = (row[x/2] & 0x0f) | get_palette_index(r,g,b,a,palette)<<4;
}

static inline void setpixel_16bppGrayScale(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    *((WORD*)(row)+x) = (r+g+b)*85;
}

static inline void setpixel_16bppRGB555(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    *((WORD*)(row)+x) = (r<<7&0x7c00)|
                        (g<<2&0x03e0)|
                        (b>>3&0x001f);
}

static inline void setpixel_16bppRGB565(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    *((WORD*)(row)+x) = (r<<8&0xf800)|
                         (g<<3&0x07e0)|
                         (b>>3&0x001f);
}

static inline void setpixel_16bppARGB1555(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    *((WORD*)(row)+x) = (a<<8&0x8000)|
                        (r<<7&0x7c00)|
                        (g<<2&0x03e0)|
                        (b>>3&0x001f);
}

static inline void setpixel_24bppRGB(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    row[x*3+2] = r;
    row[x*3+1] = g;
    row[x*3] = b;
}

static inline void setpixel_32bppRGB(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    *((DWORD*)(row)+x) = (r<<16)|(g<<8)|b;
}

static inline void setpixel_32bppARGB(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    *((DWORD*)(row)+x) = (a<<24)|(r<<16)|(g<<8)|b;
}

static inline void setpixel_32bppPARGB(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    r = r * a / 255;
    g = g * a / 255;
    b = b * a / 255;
    *((DWORD*)(row)+x) = (a<<24)|(r<<16)|(g<<8)|b;
}

static inline void setpixel_48bppRGB(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    row[x*6+5] = row[x*6+4] = r;
    row[x*6+3] = row[x*6+2] = g;
    row[x*6+1] = row[x*6] = b;
}

static inline void setpixel_64bppARGB(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    UINT64 a64=a, r64=r, g64=g, b64=b;
    *((UINT64*)(row)+x) = (a64<<56)|(a64<<48)|(r64<<40)|(r64<<32)|(g64<<24)|(g64<<16)|(b64<<8)|b64;
}

static inline void setpixel_64bppPARGB(BYTE r, BYTE g, BYTE b, BYTE a,
    BYTE *row, UINT x)
{
    UINT64 a64, r64, g64, b64;
    a64 = a * 257;
    r64 = r * a / 255;
    g64 = g * a / 255;
    b64 = b * a / 255;
    *((UINT64*)(row)+x) = (a64<<48)|(r64<<32)|(g64<<16)|b64;
}

GpStatus WINGDIPAPI GdipBitmapSetPixel(GpBitmap* bitmap, INT x, INT y,
    ARGB color)
{
    BYTE a, r, g, b;
    BYTE *row;

    if(!bitmap || x < 0 || y < 0 || x >= bitmap->width || y >= bitmap->height)
        return InvalidParameter;

    a = color>>24;
    r = color>>16;
    g = color>>8;
    b = color;

    row = bitmap->bits + bitmap->stride * y;

    switch (bitmap->format)
    {
        case PixelFormat16bppGrayScale:
            setpixel_16bppGrayScale(r,g,b,a,row,x);
            break;
        case PixelFormat16bppRGB555:
            setpixel_16bppRGB555(r,g,b,a,row,x);
            break;
        case PixelFormat16bppRGB565:
            setpixel_16bppRGB565(r,g,b,a,row,x);
            break;
        case PixelFormat16bppARGB1555:
            setpixel_16bppARGB1555(r,g,b,a,row,x);
            break;
        case PixelFormat24bppRGB:
            setpixel_24bppRGB(r,g,b,a,row,x);
            break;
        case PixelFormat32bppRGB:
            setpixel_32bppRGB(r,g,b,a,row,x);
            break;
        case PixelFormat32bppARGB:
            setpixel_32bppARGB(r,g,b,a,row,x);
            break;
        case PixelFormat32bppPARGB:
            setpixel_32bppPARGB(r,g,b,a,row,x);
            break;
        case PixelFormat48bppRGB:
            setpixel_48bppRGB(r,g,b,a,row,x);
            break;
        case PixelFormat64bppARGB:
            setpixel_64bppARGB(r,g,b,a,row,x);
            break;
        case PixelFormat64bppPARGB:
            setpixel_64bppPARGB(r,g,b,a,row,x);
            break;
        case PixelFormat8bppIndexed:
            setpixel_8bppIndexed(r,g,b,a,row,x,bitmap->image.palette);
            break;
        case PixelFormat4bppIndexed:
            setpixel_4bppIndexed(r,g,b,a,row,x,bitmap->image.palette);
            break;
        case PixelFormat1bppIndexed:
            setpixel_1bppIndexed(r,g,b,a,row,x,bitmap->image.palette);
            break;
        default:
            FIXME("not implemented for format 0x%x\n", bitmap->format);
            return NotImplemented;
    }

    return Ok;
}

GpStatus convert_pixels(INT width, INT height,
    INT dst_stride, BYTE *dst_bits, PixelFormat dst_format,
    INT src_stride, const BYTE *src_bits, PixelFormat src_format,
    ColorPalette *palette)
{
    INT x, y;

    if (src_format == dst_format ||
        (dst_format == PixelFormat32bppRGB && PIXELFORMATBPP(src_format) == 32))
    {
        UINT widthbytes = PIXELFORMATBPP(src_format) * width / 8;
        for (y=0; y<height; y++)
            memcpy(dst_bits+dst_stride*y, src_bits+src_stride*y, widthbytes);
        return Ok;
    }

#define convert_indexed_to_rgb(getpixel_function, setpixel_function) do { \
    for (x=0; x<width; x++) \
        for (y=0; y<height; y++) { \
            BYTE index; \
            ARGB argb; \
            BYTE *color = (BYTE *)&argb; \
            getpixel_function(&index, src_bits+src_stride*y, x); \
            argb = (palette && index < palette->Count) ? palette->Entries[index] : 0; \
            setpixel_function(color[2], color[1], color[0], color[3], dst_bits+dst_stride*y, x); \
        } \
    return Ok; \
} while (0);

#define convert_rgb_to_rgb(getpixel_function, setpixel_function) do { \
    for (x=0; x<width; x++) \
        for (y=0; y<height; y++) { \
            BYTE r, g, b, a; \
            getpixel_function(&r, &g, &b, &a, src_bits+src_stride*y, x); \
            setpixel_function(r, g, b, a, dst_bits+dst_stride*y, x); \
        } \
    return Ok; \
} while (0);

#define convert_rgb_to_indexed(getpixel_function, setpixel_function) do { \
    for (x=0; x<width; x++) \
        for (y=0; y<height; y++) { \
            BYTE r, g, b, a; \
            getpixel_function(&r, &g, &b, &a, src_bits+src_stride*y, x); \
            setpixel_function(r, g, b, a, dst_bits+dst_stride*y, x, palette); \
        } \
    return Ok; \
} while (0);

    switch (src_format)
    {
    case PixelFormat1bppIndexed:
        switch (dst_format)
        {
        case PixelFormat16bppGrayScale:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_indexed_to_rgb(getpixel_1bppIndexed, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat4bppIndexed:
        switch (dst_format)
        {
        case PixelFormat16bppGrayScale:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_indexed_to_rgb(getpixel_4bppIndexed, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat8bppIndexed:
        switch (dst_format)
        {
        case PixelFormat16bppGrayScale:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_indexed_to_rgb(getpixel_8bppIndexed, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat16bppGrayScale:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppGrayScale, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppGrayScale, setpixel_8bppIndexed);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_16bppGrayScale, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat16bppRGB555:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppRGB555, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppRGB555, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_16bppRGB555, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat16bppRGB565:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppRGB565, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppRGB565, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_16bppRGB555);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_16bppRGB565, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat16bppARGB1555:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppARGB1555, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_16bppARGB1555, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_16bppRGB565);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_16bppARGB1555, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat24bppRGB:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_24bppRGB, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_24bppRGB, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_16bppARGB1555);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_24bppRGB, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat32bppRGB:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_32bppRGB, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_32bppRGB, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_24bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_32bppRGB, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat32bppARGB:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_32bppARGB, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_32bppARGB, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_32bppARGB, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_32bppARGB, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_32bppARGB, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_32bppARGB, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_32bppARGB, setpixel_24bppRGB);
        case PixelFormat32bppPARGB:
            convert_32bppARGB_to_32bppPARGB(width, height, dst_bits, dst_stride, src_bits, src_stride);
            return Ok;
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_32bppARGB, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_32bppARGB, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat32bppPARGB:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_32bppPARGB, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_32bppPARGB, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_32bppARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_32bppPARGB, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat48bppRGB:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_48bppRGB, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_48bppRGB, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_32bppPARGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_48bppRGB, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    case PixelFormat64bppARGB:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_64bppARGB, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_64bppARGB, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_64bppARGB, setpixel_48bppRGB);
        default:
            break;
        }
        break;
    case PixelFormat64bppPARGB:
        switch (dst_format)
        {
        case PixelFormat1bppIndexed:
            convert_rgb_to_indexed(getpixel_64bppPARGB, setpixel_1bppIndexed);
        case PixelFormat8bppIndexed:
            convert_rgb_to_indexed(getpixel_64bppPARGB, setpixel_8bppIndexed);
        case PixelFormat16bppGrayScale:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_16bppGrayScale);
        case PixelFormat16bppRGB555:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_16bppRGB555);
        case PixelFormat16bppRGB565:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_16bppRGB565);
        case PixelFormat16bppARGB1555:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_16bppARGB1555);
        case PixelFormat24bppRGB:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_24bppRGB);
        case PixelFormat32bppRGB:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_32bppRGB);
        case PixelFormat32bppARGB:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_32bppARGB);
        case PixelFormat32bppPARGB:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_32bppPARGB);
        case PixelFormat48bppRGB:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_48bppRGB);
        case PixelFormat64bppARGB:
            convert_rgb_to_rgb(getpixel_64bppPARGB, setpixel_64bppARGB);
        default:
            break;
        }
        break;
    default:
        break;
    }

#undef convert_indexed_to_rgb
#undef convert_rgb_to_rgb

    return NotImplemented;
}

/* This function returns a pointer to an array of pixels that represents the
 * bitmap. The *entire* bitmap is locked according to the lock mode specified by
 * flags.  It is correct behavior that a user who calls this function with write
 * privileges can write to the whole bitmap (not just the area in rect).
 *
 * FIXME: only used portion of format is bits per pixel. */
GpStatus WINGDIPAPI GdipBitmapLockBits(GpBitmap* bitmap, GDIPCONST GpRect* rect,
    UINT flags, PixelFormat format, BitmapData* lockeddata)
{
    INT bitspp = PIXELFORMATBPP(format);
    GpRect act_rect; /* actual rect to be used */
    GpStatus stat;

    TRACE("%p %p %d 0x%x %p\n", bitmap, rect, flags, format, lockeddata);

    if(!lockeddata || !bitmap)
        return InvalidParameter;

    if(rect){
        if(rect->X < 0 || rect->Y < 0 || (rect->X + rect->Width > bitmap->width) ||
          (rect->Y + rect->Height > bitmap->height) || !flags)
            return InvalidParameter;

        act_rect = *rect;
    }
    else{
        act_rect.X = act_rect.Y = 0;
        act_rect.Width  = bitmap->width;
        act_rect.Height = bitmap->height;
    }

    if(bitmap->lockmode)
    {
        WARN("bitmap is already locked and cannot be locked again\n");
        return WrongState;
    }

    if (bitmap->bits && bitmap->format == format && !(flags & ImageLockModeUserInputBuf))
    {
        /* no conversion is necessary; just use the bits directly */
        lockeddata->Width = act_rect.Width;
        lockeddata->Height = act_rect.Height;
        lockeddata->PixelFormat = format;
        lockeddata->Reserved = flags;
        lockeddata->Stride = bitmap->stride;
        lockeddata->Scan0 = bitmap->bits + (bitspp / 8) * act_rect.X +
                            bitmap->stride * act_rect.Y;

        bitmap->lockmode = flags | ImageLockModeRead;
        bitmap->numlocks++;

        return Ok;
    }

    /* Make sure we can convert to the requested format. */
    if (flags & ImageLockModeRead)
    {
        stat = convert_pixels(0, 0, 0, NULL, format, 0, NULL, bitmap->format, NULL);
        if (stat == NotImplemented)
        {
            FIXME("cannot read bitmap from %x to %x\n", bitmap->format, format);
            return NotImplemented;
        }
    }

    /* If we're opening for writing, make sure we'll be able to write back in
     * the original format. */
    if (flags & ImageLockModeWrite)
    {
        stat = convert_pixels(0, 0, 0, NULL, bitmap->format, 0, NULL, format, NULL);
        if (stat == NotImplemented)
        {
            FIXME("cannot write bitmap from %x to %x\n", format, bitmap->format);
            return NotImplemented;
        }
    }

    lockeddata->Width  = act_rect.Width;
    lockeddata->Height = act_rect.Height;
    lockeddata->PixelFormat = format;
    lockeddata->Reserved = flags;

    if(!(flags & ImageLockModeUserInputBuf))
    {
        lockeddata->Stride = (((act_rect.Width * bitspp + 7) / 8) + 3) & ~3;

        bitmap->bitmapbits = GdipAlloc(lockeddata->Stride * act_rect.Height);

        if (!bitmap->bitmapbits) return OutOfMemory;

        lockeddata->Scan0  = bitmap->bitmapbits;
    }

    if (flags & ImageLockModeRead)
    {
        static int fixme=0;

        if (!fixme && (PIXELFORMATBPP(bitmap->format) * act_rect.X) % 8 != 0)
        {
            FIXME("Cannot copy rows that don't start at a whole byte.\n");
            fixme = 1;
        }

        stat = convert_pixels(act_rect.Width, act_rect.Height,
            lockeddata->Stride, lockeddata->Scan0, format,
            bitmap->stride,
            bitmap->bits + bitmap->stride * act_rect.Y + PIXELFORMATBPP(bitmap->format) * act_rect.X / 8,
            bitmap->format, bitmap->image.palette);

        if (stat != Ok)
        {
            GdipFree(bitmap->bitmapbits);
            bitmap->bitmapbits = NULL;
            return stat;
        }
    }

    bitmap->lockmode = flags | ImageLockModeRead;
    bitmap->numlocks++;
    bitmap->lockx = act_rect.X;
    bitmap->locky = act_rect.Y;

    return Ok;
}

GpStatus WINGDIPAPI GdipBitmapSetResolution(GpBitmap* bitmap, REAL xdpi, REAL ydpi)
{
    TRACE("(%p, %.2f, %.2f)\n", bitmap, xdpi, ydpi);

    if (!bitmap || xdpi == 0.0 || ydpi == 0.0)
        return InvalidParameter;

    bitmap->image.xres = xdpi;
    bitmap->image.yres = ydpi;

    return Ok;
}

GpStatus WINGDIPAPI GdipBitmapUnlockBits(GpBitmap* bitmap,
    BitmapData* lockeddata)
{
    GpStatus stat;
    static int fixme=0;

    TRACE("(%p,%p)\n", bitmap, lockeddata);

    if(!bitmap || !lockeddata)
        return InvalidParameter;

    if(!bitmap->lockmode)
        return WrongState;

    if(!(lockeddata->Reserved & ImageLockModeWrite)){
        if(!(--bitmap->numlocks))
            bitmap->lockmode = 0;

        GdipFree(bitmap->bitmapbits);
        bitmap->bitmapbits = NULL;
        return Ok;
    }

    if (!bitmap->bitmapbits && !(lockeddata->Reserved & ImageLockModeUserInputBuf))
    {
        /* we passed a direct reference; no need to do anything */
        bitmap->lockmode = 0;
        bitmap->numlocks = 0;
        return Ok;
    }

    if (!fixme && (PIXELFORMATBPP(bitmap->format) * bitmap->lockx) % 8 != 0)
    {
        FIXME("Cannot copy rows that don't start at a whole byte.\n");
        fixme = 1;
    }

    stat = convert_pixels(lockeddata->Width, lockeddata->Height,
        bitmap->stride,
        bitmap->bits + bitmap->stride * bitmap->locky + PIXELFORMATBPP(bitmap->format) * bitmap->lockx / 8,
        bitmap->format,
        lockeddata->Stride, lockeddata->Scan0, lockeddata->PixelFormat, NULL);

    if (stat != Ok)
    {
        ERR("failed to convert pixels; this should never happen\n");
    }

    GdipFree(bitmap->bitmapbits);
    bitmap->bitmapbits = NULL;
    bitmap->lockmode = 0;
    bitmap->numlocks = 0;

    return stat;
}

GpStatus WINGDIPAPI GdipCloneBitmapArea(REAL x, REAL y, REAL width, REAL height,
    PixelFormat format, GpBitmap* srcBitmap, GpBitmap** dstBitmap)
{
    BitmapData lockeddata_src, lockeddata_dst;
    int i;
    UINT row_size;
    Rect area;
    GpStatus stat;

    TRACE("(%f,%f,%f,%f,0x%x,%p,%p)\n", x, y, width, height, format, srcBitmap, dstBitmap);

    if (!srcBitmap || !dstBitmap || srcBitmap->image.type != ImageTypeBitmap ||
        x < 0 || y < 0 ||
        x + width > srcBitmap->width || y + height > srcBitmap->height)
    {
        TRACE("<-- InvalidParameter\n");
        return InvalidParameter;
    }

    if (format == PixelFormatDontCare)
        format = srcBitmap->format;

    area.X = gdip_round(x);
    area.Y = gdip_round(y);
    area.Width = gdip_round(width);
    area.Height = gdip_round(height);

    stat = GdipBitmapLockBits(srcBitmap, &area, ImageLockModeRead, format,
        &lockeddata_src);
    if (stat != Ok) return stat;

    stat = GdipCreateBitmapFromScan0(lockeddata_src.Width, lockeddata_src.Height,
        0, lockeddata_src.PixelFormat, NULL, dstBitmap);
    if (stat == Ok)
    {
        stat = GdipBitmapLockBits(*dstBitmap, NULL, ImageLockModeWrite,
            lockeddata_src.PixelFormat, &lockeddata_dst);

        if (stat == Ok)
        {
            /* copy the image data */
            row_size = (lockeddata_src.Width * PIXELFORMATBPP(lockeddata_src.PixelFormat) +7)/8;
            for (i=0; i<lockeddata_src.Height; i++)
                memcpy((BYTE*)lockeddata_dst.Scan0+lockeddata_dst.Stride*i,
                       (BYTE*)lockeddata_src.Scan0+lockeddata_src.Stride*i,
                       row_size);

            GdipBitmapUnlockBits(*dstBitmap, &lockeddata_dst);
        }

        if (stat != Ok)
            GdipDisposeImage((GpImage*)*dstBitmap);
    }

    GdipBitmapUnlockBits(srcBitmap, &lockeddata_src);

    if (stat != Ok)
    {
        *dstBitmap = NULL;
    }

    return stat;
}

GpStatus WINGDIPAPI GdipCloneBitmapAreaI(INT x, INT y, INT width, INT height,
    PixelFormat format, GpBitmap* srcBitmap, GpBitmap** dstBitmap)
{
    TRACE("(%i,%i,%i,%i,0x%x,%p,%p)\n", x, y, width, height, format, srcBitmap, dstBitmap);

    return GdipCloneBitmapArea(x, y, width, height, format, srcBitmap, dstBitmap);
}

GpStatus WINGDIPAPI GdipCloneImage(GpImage *image, GpImage **cloneImage)
{
    GpStatus stat = GenericError;

    TRACE("%p, %p\n", image, cloneImage);

    if (!image || !cloneImage)
        return InvalidParameter;

    if (image->picture)
    {
        IStream* stream;
        HRESULT hr;
        INT size;
        LARGE_INTEGER move;

        hr = CreateStreamOnHGlobal(0, TRUE, &stream);
        if (FAILED(hr))
            return GenericError;

        hr = IPicture_SaveAsFile(image->picture, stream, FALSE, &size);
        if(FAILED(hr))
        {
            WARN("Failed to save image on stream\n");
            goto out;
        }

        /* Set seek pointer back to the beginning of the picture */
        move.QuadPart = 0;
        hr = IStream_Seek(stream, move, STREAM_SEEK_SET, NULL);
        if (FAILED(hr))
            goto out;

        stat = GdipLoadImageFromStream(stream, cloneImage);
        if (stat != Ok) WARN("Failed to load image from stream\n");

    out:
        IStream_Release(stream);
        return stat;
    }
    else if (image->type == ImageTypeBitmap)
    {
        GpBitmap *bitmap = (GpBitmap*)image;
        BitmapData lockeddata_src, lockeddata_dst;
        int i;
        UINT row_size;

        stat = GdipBitmapLockBits(bitmap, NULL, ImageLockModeRead, bitmap->format,
            &lockeddata_src);
        if (stat != Ok) return stat;

        stat = GdipCreateBitmapFromScan0(lockeddata_src.Width, lockeddata_src.Height,
            0, lockeddata_src.PixelFormat, NULL, (GpBitmap**)cloneImage);
        if (stat == Ok)
        {
            stat = GdipBitmapLockBits((GpBitmap*)*cloneImage, NULL, ImageLockModeWrite,
                lockeddata_src.PixelFormat, &lockeddata_dst);

            if (stat == Ok)
            {
                /* copy the image data */
                row_size = (lockeddata_src.Width * PIXELFORMATBPP(lockeddata_src.PixelFormat) +7)/8;
                for (i=0; i<lockeddata_src.Height; i++)
                    memcpy((BYTE*)lockeddata_dst.Scan0+lockeddata_dst.Stride*i,
                           (BYTE*)lockeddata_src.Scan0+lockeddata_src.Stride*i,
                           row_size);

                GdipBitmapUnlockBits((GpBitmap*)*cloneImage, &lockeddata_dst);
            }

            if (stat != Ok)
                GdipDisposeImage(*cloneImage);
        }

        GdipBitmapUnlockBits(bitmap, &lockeddata_src);

        if (stat != Ok)
        {
            *cloneImage = NULL;
        }
        else memcpy(&(*cloneImage)->format, &image->format, sizeof(GUID));

        return stat;
    }
    else if (image->type == ImageTypeMetafile && ((GpMetafile*)image)->hemf)
    {
        GpMetafile *result, *metafile;

        metafile = (GpMetafile*)image;

        result = GdipAlloc(sizeof(*result));
        if (!result)
            return OutOfMemory;

        result->image.type = ImageTypeMetafile;
        result->image.format = image->format;
        result->image.flags = image->flags;
        result->image.frame_count = 1;
        result->image.xres = image->xres;
        result->image.yres = image->yres;
        result->bounds = metafile->bounds;
        result->unit = metafile->unit;
        result->metafile_type = metafile->metafile_type;
        result->hemf = CopyEnhMetaFileW(metafile->hemf, NULL);

        if (!result->hemf)
        {
            GdipFree(result);
            return OutOfMemory;
        }

        *cloneImage = &result->image;
        return Ok;
    }
    else
    {
        WARN("GpImage with no image data (metafile in wrong state?)\n");
        return InvalidParameter;
    }
}

GpStatus WINGDIPAPI GdipCreateBitmapFromFile(GDIPCONST WCHAR* filename,
    GpBitmap **bitmap)
{
    GpStatus stat;
    IStream *stream;

    TRACE("(%s) %p\n", debugstr_w(filename), bitmap);

    if(!filename || !bitmap)
        return InvalidParameter;

    stat = GdipCreateStreamOnFile(filename, GENERIC_READ, &stream);

    if(stat != Ok)
        return stat;

    stat = GdipCreateBitmapFromStream(stream, bitmap);

    IStream_Release(stream);

    return stat;
}

GpStatus WINGDIPAPI GdipCreateBitmapFromGdiDib(GDIPCONST BITMAPINFO* info,
                                               VOID *bits, GpBitmap **bitmap)
{
    DWORD height, stride;
    PixelFormat format;

    FIXME("(%p, %p, %p) - partially implemented\n", info, bits, bitmap);

    if (!info || !bits || !bitmap)
        return InvalidParameter;

    height = abs(info->bmiHeader.biHeight);
    stride = ((info->bmiHeader.biWidth * info->bmiHeader.biBitCount + 31) >> 3) & ~3;

    if(info->bmiHeader.biHeight > 0) /* bottom-up */
    {
        bits = (BYTE*)bits + (height - 1) * stride;
        stride = -stride;
    }

    switch(info->bmiHeader.biBitCount) {
    case 1:
        format = PixelFormat1bppIndexed;
        break;
    case 4:
        format = PixelFormat4bppIndexed;
        break;
    case 8:
        format = PixelFormat8bppIndexed;
        break;
    case 16:
        format = PixelFormat16bppRGB555;
        break;
    case 24:
        format = PixelFormat24bppRGB;
        break;
    case 32:
        format = PixelFormat32bppRGB;
        break;
    default:
        FIXME("don't know how to handle %d bpp\n", info->bmiHeader.biBitCount);
        *bitmap = NULL;
        return InvalidParameter;
    }

    return GdipCreateBitmapFromScan0(info->bmiHeader.biWidth, height, stride, format,
                                     bits, bitmap);

}

/* FIXME: no icm */
GpStatus WINGDIPAPI GdipCreateBitmapFromFileICM(GDIPCONST WCHAR* filename,
    GpBitmap **bitmap)
{
    TRACE("(%s) %p\n", debugstr_w(filename), bitmap);

    return GdipCreateBitmapFromFile(filename, bitmap);
}

GpStatus WINGDIPAPI GdipCreateBitmapFromResource(HINSTANCE hInstance,
    GDIPCONST WCHAR* lpBitmapName, GpBitmap** bitmap)
{
    HBITMAP hbm;
    GpStatus stat = InvalidParameter;

    TRACE("%p (%s) %p\n", hInstance, debugstr_w(lpBitmapName), bitmap);

    if(!lpBitmapName || !bitmap)
        return InvalidParameter;

    /* load DIB */
    hbm = LoadImageW(hInstance, lpBitmapName, IMAGE_BITMAP, 0, 0,
                     LR_CREATEDIBSECTION);

    if(hbm){
        stat = GdipCreateBitmapFromHBITMAP(hbm, NULL, bitmap);
        DeleteObject(hbm);
    }

    return stat;
}

GpStatus WINGDIPAPI GdipCreateHBITMAPFromBitmap(GpBitmap* bitmap,
    HBITMAP* hbmReturn, ARGB background)
{
    GpStatus stat;
    HBITMAP result;
    UINT width, height;
    BITMAPINFOHEADER bih;
    LPBYTE bits;
    BitmapData lockeddata;
    TRACE("(%p,%p,%x)\n", bitmap, hbmReturn, background);

    if (!bitmap || !hbmReturn) return InvalidParameter;

    GdipGetImageWidth((GpImage*)bitmap, &width);
    GdipGetImageHeight((GpImage*)bitmap, &height);

    bih.biSize = sizeof(bih);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = 0;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    result = CreateDIBSection(0, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

    if (result)
    {
        lockeddata.Stride = -width * 4;
        lockeddata.Scan0 = bits + (width * 4 * (height - 1));

        stat = GdipBitmapLockBits(bitmap, NULL, ImageLockModeRead|ImageLockModeUserInputBuf,
            PixelFormat32bppPARGB, &lockeddata);

        if (stat == Ok)
            stat = GdipBitmapUnlockBits(bitmap, &lockeddata);
    }
    else
        stat = GenericError;

    if (stat != Ok && result)
    {
        DeleteObject(result);
        result = NULL;
    }

    *hbmReturn = result;

    return stat;
}

GpStatus WINGDIPAPI GdipConvertToEmfPlus(const GpGraphics* ref,
    GpMetafile* metafile, BOOL* succ, EmfType emfType,
    const WCHAR* description, GpMetafile** out_metafile)
{
    static int calls;

    TRACE("(%p,%p,%p,%u,%s,%p)\n", ref, metafile, succ, emfType,
        debugstr_w(description), out_metafile);

    if(!ref || !metafile || !out_metafile)
        return InvalidParameter;

    *succ = FALSE;
    *out_metafile = NULL;

    if(!(calls++))
        FIXME("not implemented\n");

    return NotImplemented;
}

GpStatus WINGDIPAPI GdipCreateBitmapFromGraphics(INT width, INT height,
    GpGraphics* target, GpBitmap** bitmap)
{
    GpStatus ret;

    TRACE("(%d, %d, %p, %p)\n", width, height, target, bitmap);

    if(!target || !bitmap)
        return InvalidParameter;

    ret = GdipCreateBitmapFromScan0(width, height, 0, PixelFormat32bppPARGB,
                                    NULL, bitmap);

    if (ret == Ok)
    {
        GdipGetDpiX(target, &(*bitmap)->image.xres);
        GdipGetDpiY(target, &(*bitmap)->image.yres);
    }

    return ret;
}

GpStatus WINGDIPAPI GdipCreateBitmapFromHICON(HICON hicon, GpBitmap** bitmap)
{
    GpStatus stat;
    ICONINFO iinfo;
    BITMAP bm;
    int ret;
    UINT width, height, stride;
    GpRect rect;
    BitmapData lockeddata;
    HDC screendc;
    BOOL has_alpha;
    int x, y;
    BITMAPINFOHEADER bih;
    DWORD *src;
    BYTE *dst_row;
    DWORD *dst;

    TRACE("%p, %p\n", hicon, bitmap);

    if(!bitmap || !GetIconInfo(hicon, &iinfo))
        return InvalidParameter;

    /* get the size of the icon */
    ret = GetObjectA(iinfo.hbmColor ? iinfo.hbmColor : iinfo.hbmMask, sizeof(bm), &bm);
    if (ret == 0) {
        DeleteObject(iinfo.hbmColor);
        DeleteObject(iinfo.hbmMask);
        return GenericError;
    }

    width = bm.bmWidth;
    height = iinfo.hbmColor ? abs(bm.bmHeight) : abs(bm.bmHeight) / 2;
    stride = width * 4;

    stat = GdipCreateBitmapFromScan0(width, height, stride, PixelFormat32bppARGB, NULL, bitmap);
    if (stat != Ok) {
        DeleteObject(iinfo.hbmColor);
        DeleteObject(iinfo.hbmMask);
        return stat;
    }

    rect.X = 0;
    rect.Y = 0;
    rect.Width = width;
    rect.Height = height;

    stat = GdipBitmapLockBits(*bitmap, &rect, ImageLockModeWrite, PixelFormat32bppARGB, &lockeddata);
    if (stat != Ok) {
        DeleteObject(iinfo.hbmColor);
        DeleteObject(iinfo.hbmMask);
        GdipDisposeImage((GpImage*)*bitmap);
        return stat;
    }

    bih.biSize = sizeof(bih);
    bih.biWidth = width;
    bih.biHeight = iinfo.hbmColor ? -height: -height * 2;
    bih.biPlanes = 1;
    bih.biBitCount = 32;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = 0;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    screendc = CreateCompatibleDC(0);
    if (iinfo.hbmColor)
    {
        GetDIBits(screendc, iinfo.hbmColor, 0, height, lockeddata.Scan0, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

        if (bm.bmBitsPixel == 32)
        {
            has_alpha = FALSE;

            /* If any pixel has a non-zero alpha, ignore hbmMask */
            src = (DWORD*)lockeddata.Scan0;
            for (x=0; x<width && !has_alpha; x++)
                for (y=0; y<height && !has_alpha; y++)
                    if ((*src++ & 0xff000000) != 0)
                        has_alpha = TRUE;
        }
        else has_alpha = FALSE;
    }
    else
    {
        GetDIBits(screendc, iinfo.hbmMask, 0, height, lockeddata.Scan0, (BITMAPINFO*)&bih, DIB_RGB_COLORS);
        has_alpha = FALSE;
    }

    if (!has_alpha)
    {
        if (iinfo.hbmMask)
        {
            BYTE *bits = HeapAlloc(GetProcessHeap(), 0, height * stride);

            /* read alpha data from the mask */
            if (iinfo.hbmColor)
                GetDIBits(screendc, iinfo.hbmMask, 0, height, bits, (BITMAPINFO*)&bih, DIB_RGB_COLORS);
            else
                GetDIBits(screendc, iinfo.hbmMask, height, height, bits, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

            src = (DWORD*)bits;
            dst_row = lockeddata.Scan0;
            for (y=0; y<height; y++)
            {
                dst = (DWORD*)dst_row;
                for (x=0; x<height; x++)
                {
                    DWORD src_value = *src++;
                    if (src_value)
                        *dst++ = 0;
                    else
                        *dst++ |= 0xff000000;
                }
                dst_row += lockeddata.Stride;
            }

            HeapFree(GetProcessHeap(), 0, bits);
        }
        else
        {
            /* set constant alpha of 255 */
            dst_row = lockeddata.Scan0;
            for (y=0; y<height; y++)
            {
                dst = (DWORD*)dst_row;
                for (x=0; x<height; x++)
                    *dst++ |= 0xff000000;
                dst_row += lockeddata.Stride;
            }
        }
    }

    DeleteDC(screendc);

    DeleteObject(iinfo.hbmColor);
    DeleteObject(iinfo.hbmMask);

    GdipBitmapUnlockBits(*bitmap, &lockeddata);

    return Ok;
}

static void generate_halftone_palette(ARGB *entries, UINT count)
{
    static const BYTE halftone_values[6]={0x00,0x33,0x66,0x99,0xcc,0xff};
    UINT i;

    for (i=0; i<8 && i<count; i++)
    {
        entries[i] = 0xff000000;
        if (i&1) entries[i] |= 0x800000;
        if (i&2) entries[i] |= 0x8000;
        if (i&4) entries[i] |= 0x80;
    }

    if (8 < count)
        entries[i] = 0xffc0c0c0;

    for (i=9; i<16 && i<count; i++)
    {
        entries[i] = 0xff000000;
        if (i&1) entries[i] |= 0xff0000;
        if (i&2) entries[i] |= 0xff00;
        if (i&4) entries[i] |= 0xff;
    }

    for (i=16; i<40 && i<count; i++)
    {
        entries[i] = 0;
    }

    for (i=40; i<256 && i<count; i++)
    {
        entries[i] = 0xff000000;
        entries[i] |= halftone_values[(i-40)%6];
        entries[i] |= halftone_values[((i-40)/6)%6] << 8;
        entries[i] |= halftone_values[((i-40)/36)%6] << 16;
    }
}

static GpStatus get_screen_resolution(REAL *xres, REAL *yres)
{
    HDC screendc = CreateCompatibleDC(0);

    if (!screendc) return GenericError;

    *xres = (REAL)GetDeviceCaps(screendc, LOGPIXELSX);
    *yres = (REAL)GetDeviceCaps(screendc, LOGPIXELSY);

    DeleteDC(screendc);

    return Ok;
}

GpStatus WINGDIPAPI GdipCreateBitmapFromScan0(INT width, INT height, INT stride,
    PixelFormat format, BYTE* scan0, GpBitmap** bitmap)
{
    BITMAPINFO* pbmi;
    HBITMAP hbitmap=NULL;
    INT row_size, dib_stride;
    BYTE *bits=NULL, *own_bits=NULL;
    REAL xres, yres;
    GpStatus stat;

    TRACE("%d %d %d 0x%x %p %p\n", width, height, stride, format, scan0, bitmap);

    if (!bitmap) return InvalidParameter;

    if(width <= 0 || height <= 0 || (scan0 && (stride % 4))){
        *bitmap = NULL;
        return InvalidParameter;
    }

    if(scan0 && !stride)
        return InvalidParameter;

    stat = get_screen_resolution(&xres, &yres);
    if (stat != Ok) return stat;

    row_size = (width * PIXELFORMATBPP(format)+7) / 8;
    dib_stride = (row_size + 3) & ~3;

    if(stride == 0)
        stride = dib_stride;

    if (format & PixelFormatGDI && !(format & (PixelFormatAlpha|PixelFormatIndexed)) && !scan0)
    {
        pbmi = GdipAlloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
        if (!pbmi)
            return OutOfMemory;

        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = width;
        pbmi->bmiHeader.biHeight = -height;
        pbmi->bmiHeader.biPlanes = 1;
        /* FIXME: use the rest of the data from format */
        pbmi->bmiHeader.biBitCount = PIXELFORMATBPP(format);
        pbmi->bmiHeader.biCompression = BI_RGB;
        pbmi->bmiHeader.biSizeImage = 0;
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed = 0;
        pbmi->bmiHeader.biClrImportant = 0;

        hbitmap = CreateDIBSection(0, pbmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

        GdipFree(pbmi);

        if (!hbitmap) return GenericError;

        stride = dib_stride;
    }
    else
    {
        /* Not a GDI format; don't try to make an HBITMAP. */
        if (scan0)
            bits = scan0;
        else
        {
            INT size = abs(stride) * height;

            own_bits = bits = GdipAlloc(size);
            if (!own_bits) return OutOfMemory;

            if (stride < 0)
                bits += stride * (1 - height);
        }
    }

    *bitmap = GdipAlloc(sizeof(GpBitmap));
    if(!*bitmap)
    {
        DeleteObject(hbitmap);
        GdipFree(own_bits);
        return OutOfMemory;
    }

    (*bitmap)->image.type = ImageTypeBitmap;
    memcpy(&(*bitmap)->image.format, &ImageFormatMemoryBMP, sizeof(GUID));
    (*bitmap)->image.flags = ImageFlagsNone;
    (*bitmap)->image.frame_count = 1;
    (*bitmap)->image.current_frame = 0;
    (*bitmap)->image.palette = NULL;
    (*bitmap)->image.xres = xres;
    (*bitmap)->image.yres = yres;
    (*bitmap)->width = width;
    (*bitmap)->height = height;
    (*bitmap)->format = format;
    (*bitmap)->image.picture = NULL;
    (*bitmap)->image.stream = NULL;
    (*bitmap)->hbitmap = hbitmap;
    (*bitmap)->hdc = NULL;
    (*bitmap)->bits = bits;
    (*bitmap)->stride = stride;
    (*bitmap)->own_bits = own_bits;
    (*bitmap)->metadata_reader = NULL;
    (*bitmap)->prop_count = 0;
    (*bitmap)->prop_item = NULL;

    /* set format-related flags */
    if (format & (PixelFormatAlpha|PixelFormatPAlpha|PixelFormatIndexed))
        (*bitmap)->image.flags |= ImageFlagsHasAlpha;

    if (format == PixelFormat1bppIndexed ||
        format == PixelFormat4bppIndexed ||
        format == PixelFormat8bppIndexed)
    {
        (*bitmap)->image.palette = GdipAlloc(sizeof(UINT) * 2 + sizeof(ARGB) * (1 << PIXELFORMATBPP(format)));

        if (!(*bitmap)->image.palette)
        {
            GdipDisposeImage(&(*bitmap)->image);
            *bitmap = NULL;
            return OutOfMemory;
        }

        (*bitmap)->image.palette->Count = 1 << PIXELFORMATBPP(format);

        if (format == PixelFormat1bppIndexed)
        {
            (*bitmap)->image.palette->Flags = PaletteFlagsGrayScale;
            (*bitmap)->image.palette->Entries[0] = 0xff000000;
            (*bitmap)->image.palette->Entries[1] = 0xffffffff;
        }
        else
        {
            if (format == PixelFormat8bppIndexed)
                (*bitmap)->image.palette->Flags = PaletteFlagsHalftone;

            generate_halftone_palette((*bitmap)->image.palette->Entries,
                (*bitmap)->image.palette->Count);
        }
    }

    TRACE("<-- %p\n", *bitmap);

    return Ok;
}

GpStatus WINGDIPAPI GdipCreateBitmapFromStream(IStream* stream,
    GpBitmap **bitmap)
{
    GpStatus stat;

    TRACE("%p %p\n", stream, bitmap);

    stat = GdipLoadImageFromStream(stream, (GpImage**) bitmap);

    if(stat != Ok)
        return stat;

    if((*bitmap)->image.type != ImageTypeBitmap){
        GdipDisposeImage(&(*bitmap)->image);
        *bitmap = NULL;
        return GenericError; /* FIXME: what error to return? */
    }

    return Ok;
}

/* FIXME: no icm */
GpStatus WINGDIPAPI GdipCreateBitmapFromStreamICM(IStream* stream,
    GpBitmap **bitmap)
{
    TRACE("%p %p\n", stream, bitmap);

    return GdipCreateBitmapFromStream(stream, bitmap);
}

GpStatus WINGDIPAPI GdipCreateCachedBitmap(GpBitmap *bitmap, GpGraphics *graphics,
    GpCachedBitmap **cachedbmp)
{
    GpStatus stat;

    TRACE("%p %p %p\n", bitmap, graphics, cachedbmp);

    if(!bitmap || !graphics || !cachedbmp)
        return InvalidParameter;

    *cachedbmp = GdipAlloc(sizeof(GpCachedBitmap));
    if(!*cachedbmp)
        return OutOfMemory;

    stat = GdipCloneImage(&(bitmap->image), &(*cachedbmp)->image);
    if(stat != Ok){
        GdipFree(*cachedbmp);
        return stat;
    }

    return Ok;
}

GpStatus WINGDIPAPI GdipCreateHICONFromBitmap(GpBitmap *bitmap, HICON *hicon)
{
    GpStatus stat;
    BitmapData lockeddata;
    ULONG andstride, xorstride, bitssize;
    LPBYTE andbits, xorbits, androw, xorrow, srcrow;
    UINT x, y;

    TRACE("(%p, %p)\n", bitmap, hicon);

    if (!bitmap || !hicon)
        return InvalidParameter;

    stat = GdipBitmapLockBits(bitmap, NULL, ImageLockModeRead,
        PixelFormat32bppPARGB, &lockeddata);
    if (stat == Ok)
    {
        andstride = ((lockeddata.Width+31)/32)*4;
        xorstride = lockeddata.Width*4;
        bitssize = (andstride + xorstride) * lockeddata.Height;

        andbits = GdipAlloc(bitssize);

        if (andbits)
        {
            xorbits = andbits + andstride * lockeddata.Height;

            for (y=0; y<lockeddata.Height; y++)
            {
                srcrow = ((LPBYTE)lockeddata.Scan0) + lockeddata.Stride * y;

                androw = andbits + andstride * y;
                for (x=0; x<lockeddata.Width; x++)
                    if (srcrow[3+4*x] >= 128)
                        androw[x/8] |= 1 << (7-x%8);

                xorrow = xorbits + xorstride * y;
                memcpy(xorrow, srcrow, xorstride);
            }

            *hicon = CreateIcon(NULL, lockeddata.Width, lockeddata.Height, 1, 32,
                andbits, xorbits);

            GdipFree(andbits);
        }
        else
            stat = OutOfMemory;

        GdipBitmapUnlockBits(bitmap, &lockeddata);
    }

    return stat;
}

GpStatus WINGDIPAPI GdipDeleteCachedBitmap(GpCachedBitmap *cachedbmp)
{
    TRACE("%p\n", cachedbmp);

    if(!cachedbmp)
        return InvalidParameter;

    GdipDisposeImage(cachedbmp->image);
    GdipFree(cachedbmp);

    return Ok;
}

GpStatus WINGDIPAPI GdipDrawCachedBitmap(GpGraphics *graphics,
    GpCachedBitmap *cachedbmp, INT x, INT y)
{
    TRACE("%p %p %d %d\n", graphics, cachedbmp, x, y);

    if(!graphics || !cachedbmp)
        return InvalidParameter;

    return GdipDrawImage(graphics, cachedbmp->image, (REAL)x, (REAL)y);
}

GpStatus WINGDIPAPI GdipEmfToWmfBits(HENHMETAFILE hemf, UINT cbData16,
    LPBYTE pData16, INT iMapMode, INT eFlags)
{
    FIXME("(%p, %d, %p, %d, %d): stub\n", hemf, cbData16, pData16, iMapMode, eFlags);
    return NotImplemented;
}

/* Internal utility function: Replace the image data of dst with that of src,
 * and free src. */
static void move_bitmap(GpBitmap *dst, GpBitmap *src, BOOL clobber_palette)
{
    assert(src->image.type == ImageTypeBitmap);
    assert(dst->image.type == ImageTypeBitmap);

    GdipFree(dst->bitmapbits);
    GdipFree(dst->own_bits);
    DeleteDC(dst->hdc);
    DeleteObject(dst->hbitmap);

    if (clobber_palette)
    {
        GdipFree(dst->image.palette);
        dst->image.palette = src->image.palette;
    }
    else
        GdipFree(src->image.palette);

    dst->image.xres = src->image.xres;
    dst->image.yres = src->image.yres;
    dst->width = src->width;
    dst->height = src->height;
    dst->format = src->format;
    dst->hbitmap = src->hbitmap;
    dst->hdc = src->hdc;
    dst->bits = src->bits;
    dst->stride = src->stride;
    dst->own_bits = src->own_bits;
    if (dst->metadata_reader)
        IWICMetadataReader_Release(dst->metadata_reader);
    dst->metadata_reader = src->metadata_reader;
    GdipFree(dst->prop_item);
    dst->prop_item = src->prop_item;
    dst->prop_count = src->prop_count;
    if (dst->image.stream)
        IStream_Release(dst->image.stream);
    dst->image.stream = src->image.stream;
    dst->image.frame_count = src->image.frame_count;
    dst->image.current_frame = src->image.current_frame;
    dst->image.format = src->image.format;

    src->image.type = ~0;
    GdipFree(src);
}

static GpStatus free_image_data(GpImage *image)
{
    if(!image)
        return InvalidParameter;

    if (image->type == ImageTypeBitmap)
    {
        GdipFree(((GpBitmap*)image)->bitmapbits);
        GdipFree(((GpBitmap*)image)->own_bits);
        DeleteDC(((GpBitmap*)image)->hdc);
        DeleteObject(((GpBitmap*)image)->hbitmap);
        if (((GpBitmap*)image)->metadata_reader)
            IWICMetadataReader_Release(((GpBitmap*)image)->metadata_reader);
        GdipFree(((GpBitmap*)image)->prop_item);
    }
    else if (image->type == ImageTypeMetafile)
    {
        GpMetafile *metafile = (GpMetafile*)image;
        GdipFree(metafile->comment_data);
        DeleteEnhMetaFile(CloseEnhMetaFile(metafile->record_dc));
        if (!metafile->preserve_hemf)
            DeleteEnhMetaFile(metafile->hemf);
        if (metafile->record_graphics)
        {
            WARN("metafile closed while recording\n");
            /* not sure what to do here; for now just prevent the graphics from functioning or using this object */
            metafile->record_graphics->image = NULL;
            metafile->record_graphics->busy = TRUE;
        }
    }
    else
    {
        WARN("invalid image: %p\n", image);
        return ObjectBusy;
    }
    if (image->picture)
        IPicture_Release(image->picture);
    if (image->stream)
        IStream_Release(image->stream);
    GdipFree(image->palette);

    return Ok;
}

GpStatus WINGDIPAPI GdipDisposeImage(GpImage *image)
{
    GpStatus status;

    TRACE("%p\n", image);

    status = free_image_data(image);
    if (status != Ok) return status;
    image->type = ~0;
    GdipFree(image);

    return Ok;
}

GpStatus WINGDIPAPI GdipFindFirstImageItem(GpImage *image, ImageItemData* item)
{
    static int calls;

    TRACE("(%p,%p)\n", image, item);

    if(!image || !item)
        return InvalidParameter;

    if (!(calls++))
        FIXME("not implemented\n");

    return NotImplemented;
}

GpStatus WINGDIPAPI GdipGetImageItemData(GpImage *image, ImageItemData *item)
{
    static int calls;

    TRACE("(%p,%p)\n", image, item);

    if (!(calls++))
        FIXME("not implemented\n");

    return NotImplemented;
}

GpStatus WINGDIPAPI GdipGetImageBounds(GpImage *image, GpRectF *srcRect,
    GpUnit *srcUnit)
{
    TRACE("%p %p %p\n", image, srcRect, srcUnit);

    if(!image || !srcRect || !srcUnit)
        return InvalidParameter;
    if(image->type == ImageTypeMetafile){
        *srcRect = ((GpMetafile*)image)->bounds;
        *srcUnit = ((GpMetafile*)image)->unit;
    }
    else if(image->type == ImageTypeBitmap){
        srcRect->X = srcRect->Y = 0.0;
        srcRect->Width = (REAL) ((GpBitmap*)image)->width;
        srcRect->Height = (REAL) ((GpBitmap*)image)->height;
        *srcUnit = UnitPixel;
    }
    else{
        srcRect->X = srcRect->Y = 0.0;
        srcRect->Width = ipicture_pixel_width(image->picture);
        srcRect->Height = ipicture_pixel_height(image->picture);
        *srcUnit = UnitPixel;
    }

    TRACE("returning (%f, %f) (%f, %f) unit type %d\n", srcRect->X, srcRect->Y,
          srcRect->Width, srcRect->Height, *srcUnit);

    return Ok;
}

GpStatus WINGDIPAPI GdipGetImageDimension(GpImage *image, REAL *width,
    REAL *height)
{
    TRACE("%p %p %p\n", image, width, height);

    if(!image || !height || !width)
        return InvalidParameter;

    if(image->type == ImageTypeMetafile){
        *height = units_to_pixels(((GpMetafile*)image)->bounds.Height, ((GpMetafile*)image)->unit, image->yres);
        *width = units_to_pixels(((GpMetafile*)image)->bounds.Width, ((GpMetafile*)image)->unit, image->xres);
    }
    else if(image->type == ImageTypeBitmap){
        *height = ((GpBitmap*)image)->height;
        *width = ((GpBitmap*)image)->width;
    }
    else{
        *height = ipicture_pixel_height(image->picture);
        *width = ipicture_pixel_width(image->picture);
    }

    TRACE("returning (%f, %f)\n", *height, *width);
    return Ok;
}

GpStatus WINGDIPAPI GdipGetImageGraphicsContext(GpImage *image,
    GpGraphics **graphics)
{
    HDC hdc;
    GpStatus stat;

    TRACE("%p %p\n", image, graphics);

    if(!image || !graphics)
        return InvalidParameter;

    if (image->type == ImageTypeBitmap && ((GpBitmap*)image)->hbitmap)
    {
        hdc = ((GpBitmap*)image)->hdc;

        if(!hdc){
            hdc = CreateCompatibleDC(0);
            SelectObject(hdc, ((GpBitmap*)image)->hbitmap);
            ((GpBitmap*)image)->hdc = hdc;
        }

        stat = GdipCreateFromHDC(hdc, graphics);

        if (stat == Ok)
        {
            (*graphics)->image = image;
            (*graphics)->xres = image->xres;
            (*graphics)->yres = image->yres;
        }
    }
    else if (image->type == ImageTypeMetafile)
        stat = METAFILE_GetGraphicsContext((GpMetafile*)image, graphics);
    else
        stat = graphics_from_image(image, graphics);

    return stat;
}

GpStatus WINGDIPAPI GdipGetImageHeight(GpImage *image, UINT *height)
{
    TRACE("%p %p\n", image, height);

    if(!image || !height)
        return InvalidParameter;

    if(image->type == ImageTypeMetafile)
        *height = units_to_pixels(((GpMetafile*)image)->bounds.Height, ((GpMetafile*)image)->unit, image->yres);
    else if(image->type == ImageTypeBitmap)
        *height = ((GpBitmap*)image)->height;
    else
        *height = ipicture_pixel_height(image->picture);

    TRACE("returning %d\n", *height);

    return Ok;
}

GpStatus WINGDIPAPI GdipGetImageHorizontalResolution(GpImage *image, REAL *res)
{
    if(!image || !res)
        return InvalidParameter;

    *res = image->xres;

    TRACE("(%p) <-- %0.2f\n", image, *res);

    return Ok;
}

GpStatus WINGDIPAPI GdipGetImagePaletteSize(GpImage *image, INT *size)
{
    TRACE("%p %p\n", image, size);

    if(!image || !size)
        return InvalidParameter;

    if (!image->palette || image->palette->Count == 0)
        *size = sizeof(ColorPalette);
    else
        *size = sizeof(UINT)*2 + sizeof(ARGB)*image->palette->Count;

    TRACE("<-- %u\n", *size);

    return Ok;
}

/* FIXME: test this function for non-bitmap types */
GpStatus WINGDIPAPI GdipGetImagePixelFormat(GpImage *image, PixelFormat *format)
{
    TRACE("%p %p\n", image, format);

    if(!image || !format)
        return InvalidParameter;

    if(image->type != ImageTypeBitmap)
        *format = PixelFormat24bppRGB;
    else
        *format = ((GpBitmap*) image)->format;

    return Ok;
}

GpStatus WINGDIPAPI GdipGetImageRawFormat(GpImage *image, GUID *format)
{
    TRACE("(%p, %p)\n", image, format);

    if(!image || !format)
        return InvalidParameter;

    memcpy(format, &image->format, sizeof(GUID));

    return Ok;
}

GpStatus WINGDIPAPI GdipGetImageType(GpImage *image, ImageType *type)
{
    TRACE("%p %p\n", image, type);

    if(!image || !type)
        return InvalidParameter;

    *type = image->type;

    return Ok;
}

GpStatus WINGDIPAPI GdipGetImageVerticalResolution(GpImage *image, REAL *res)
{
    if(!image || !res)
        return InvalidParameter;

    *res = image->yres;

    TRACE("(%p) <-- %0.2f\n", image, *res);

    return Ok;
}

GpStatus WINGDIPAPI GdipGetImageWidth(GpImage *image, UINT *width)
{
    TRACE("%p %p\n", image, width);

    if(!image || !width)
        return InvalidParameter;

    if(image->type == ImageTypeMetafile)
        *width = units_to_pixels(((GpMetafile*)image)->bounds.Width, ((GpMetafile*)image)->unit, image->xres);
    else if(image->type == ImageTypeBitmap)
        *width = ((GpBitmap*)image)->width;
    else
        *width = ipicture_pixel_width(image->picture);

    TRACE("returning %d\n", *width);

    return Ok;
}

GpStatus WINGDIPAPI GdipGetMetafileHeaderFromMetafile(GpMetafile * metafile,
    MetafileHeader * header)
{
    static int calls;

    TRACE("(%p, %p)\n", metafile, header);

    if(!metafile || !header)
        return InvalidParameter;

    if(!(calls++))
        FIXME("not implemented\n");

    memset(header, 0, sizeof(MetafileHeader));

    return Ok;
}

GpStatus WINGDIPAPI GdipGetMetafileHeaderFromEmf(HENHMETAFILE hEmf,
    MetafileHeader *header)
{
    static int calls;

    if(!hEmf || !header)
        return InvalidParameter;

    if(!(calls++))
        FIXME("not implemented\n");

    memset(header, 0, sizeof(MetafileHeader));

    return Ok;
}

GpStatus WINGDIPAPI GdipGetMetafileHeaderFromFile(GDIPCONST WCHAR *filename,
    MetafileHeader *header)
{
    static int calls;

    TRACE("(%s,%p)\n", debugstr_w(filename), header);

    if(!filename || !header)
        return InvalidParameter;

    if(!(calls++))
        FIXME("not implemented\n");

    memset(header, 0, sizeof(MetafileHeader));

    return Ok;
}

GpStatus WINGDIPAPI GdipGetMetafileHeaderFromStream(IStream *stream,
    MetafileHeader *header)
{
    static int calls;

    TRACE("(%p,%p)\n", stream, header);

    if(!stream || !header)
        return InvalidParameter;

    if(!(calls++))
        FIXME("not implemented\n");

    memset(header, 0, sizeof(MetafileHeader));

    return Ok;
}

GpStatus WINGDIPAPI GdipGetPropertyCount(GpImage *image, UINT *num)
{
    TRACE("(%p, %p)\n", image, num);

    if (!image || !num) return InvalidParameter;

    *num = 0;

    if (image->type == ImageTypeBitmap)
    {
        if (((GpBitmap *)image)->prop_item)
        {
            *num = ((GpBitmap *)image)->prop_count;
            return Ok;
        }

        if (((GpBitmap *)image)->metadata_reader)
            IWICMetadataReader_GetCount(((GpBitmap *)image)->metadata_reader, num);
    }

    return Ok;
}

GpStatus WINGDIPAPI GdipGetPropertyIdList(GpImage *image, UINT num, PROPID *list)
{
    HRESULT hr;
    IWICMetadataReader *reader;
    IWICEnumMetadataItem *enumerator;
    UINT prop_count, i, items_returned;

    TRACE("(%p, %u, %p)\n", image, num, list);

    if (!image || !list) return InvalidParameter;

    if (image->type != ImageTypeBitmap)
    {
        FIXME("Not implemented for type %d\n", image->type);
        return NotImplemented;
    }

    if (((GpBitmap *)image)->prop_item)
    {
        if (num != ((GpBitmap *)image)->prop_count) return InvalidParameter;

        for (i = 0; i < num; i++)
        {
            list[i] = ((GpBitmap *)image)->prop_item[i].id;
        }

        return Ok;
    }

    reader = ((GpBitmap *)image)->metadata_reader;
    if (!reader)
    {
        if (num != 0) return InvalidParameter;
        return Ok;
    }

    hr = IWICMetadataReader_GetCount(reader, &prop_count);
    if (FAILED(hr)) return hresult_to_status(hr);

    if (num != prop_count) return InvalidParameter;

    hr = IWICMetadataReader_GetEnumerator(reader, &enumerator);
    if (FAILED(hr)) return hresult_to_status(hr);

    IWICEnumMetadataItem_Reset(enumerator);

    for (i = 0; i < num; i++)
    {
        PROPVARIANT id;

        hr = IWICEnumMetadataItem_Next(enumerator, 1, NULL, &id, NULL, &items_returned);
        if (hr != S_OK) break;

        if (id.vt != VT_UI2)
        {
            FIXME("not supported propvariant type for id: %u\n", id.vt);
            list[i] = 0;
            continue;
        }
        list[i] = id.u.uiVal;
    }

    IWICEnumMetadataItem_Release(enumerator);

    return hr == S_OK ? Ok : hresult_to_status(hr);
}

static UINT propvariant_size(PROPVARIANT *value)
{
    switch (value->vt & ~VT_VECTOR)
    {
    case VT_EMPTY:
        return 0;
    case VT_I1:
    case VT_UI1:
        if (!(value->vt & VT_VECTOR)) return 1;
        return value->u.caub.cElems;
    case VT_I2:
    case VT_UI2:
        if (!(value->vt & VT_VECTOR)) return 2;
        return value->u.caui.cElems * 2;
    case VT_I4:
    case VT_UI4:
    case VT_R4:
        if (!(value->vt & VT_VECTOR)) return 4;
        return value->u.caul.cElems * 4;
    case VT_I8:
    case VT_UI8:
    case VT_R8:
        if (!(value->vt & VT_VECTOR)) return 8;
        return value->u.cauh.cElems * 8;
    case VT_LPSTR:
        return value->u.pszVal ? strlen(value->u.pszVal) + 1 : 0;
    case VT_BLOB:
        return value->u.blob.cbSize;
    default:
        FIXME("not supported variant type %d\n", value->vt);
        return 0;
    }
}

GpStatus WINGDIPAPI GdipGetPropertyItemSize(GpImage *image, PROPID propid, UINT *size)
{
    HRESULT hr;
    IWICMetadataReader *reader;
    PROPVARIANT id, value;

    TRACE("(%p,%#x,%p)\n", image, propid, size);

    if (!size || !image) return InvalidParameter;

    if (image->type != ImageTypeBitmap)
    {
        FIXME("Not implemented for type %d\n", image->type);
        return NotImplemented;
    }

    if (((GpBitmap *)image)->prop_item)
    {
        UINT i;

        for (i = 0; i < ((GpBitmap *)image)->prop_count; i++)
        {
            if (propid == ((GpBitmap *)image)->prop_item[i].id)
            {
                *size = sizeof(PropertyItem) + ((GpBitmap *)image)->prop_item[i].length;
                return Ok;
            }
        }

        return PropertyNotFound;
    }

    reader = ((GpBitmap *)image)->metadata_reader;
    if (!reader) return PropertyNotFound;

    id.vt = VT_UI2;
    id.u.uiVal = propid;
    hr = IWICMetadataReader_GetValue(reader, NULL, &id, &value);
    if (FAILED(hr)) return PropertyNotFound;

    *size = propvariant_size(&value);
    if (*size) *size += sizeof(PropertyItem);
    PropVariantClear(&value);

    return Ok;
}

#ifndef PropertyTagTypeSByte
#define PropertyTagTypeSByte  6
#define PropertyTagTypeSShort 8
#define PropertyTagTypeFloat  11
#define PropertyTagTypeDouble 12
#endif

static UINT vt_to_itemtype(UINT vt)
{
    static const struct
    {
        UINT vt, type;
    } vt2type[] =
    {
        { VT_I1, PropertyTagTypeSByte },
        { VT_UI1, PropertyTagTypeByte },
        { VT_I2, PropertyTagTypeSShort },
        { VT_UI2, PropertyTagTypeShort },
        { VT_I4, PropertyTagTypeSLONG },
        { VT_UI4, PropertyTagTypeLong },
        { VT_I8, PropertyTagTypeSRational },
        { VT_UI8, PropertyTagTypeRational },
        { VT_R4, PropertyTagTypeFloat },
        { VT_R8, PropertyTagTypeDouble },
        { VT_LPSTR, PropertyTagTypeASCII },
        { VT_BLOB, PropertyTagTypeUndefined }
    };
    UINT i;
    for (i = 0; i < sizeof(vt2type)/sizeof(vt2type[0]); i++)
    {
        if (vt2type[i].vt == vt) return vt2type[i].type;
    }
    FIXME("not supported variant type %u\n", vt);
    return 0;
}

static GpStatus propvariant_to_item(PROPVARIANT *value, PropertyItem *item,
                                    UINT size, PROPID id)
{
    UINT item_size, item_type;

    item_size = propvariant_size(value);
    if (size != item_size + sizeof(PropertyItem)) return InvalidParameter;

    item_type = vt_to_itemtype(value->vt & ~VT_VECTOR);
    if (!item_type) return InvalidParameter;

    item->value = item + 1;

    switch (value->vt & ~VT_VECTOR)
    {
    case VT_I1:
    case VT_UI1:
        if (!(value->vt & VT_VECTOR))
            *(BYTE *)item->value = value->u.bVal;
        else
            memcpy(item->value, value->u.caub.pElems, item_size);
        break;
    case VT_I2:
    case VT_UI2:
        if (!(value->vt & VT_VECTOR))
            *(USHORT *)item->value = value->u.uiVal;
        else
            memcpy(item->value, value->u.caui.pElems, item_size);
        break;
    case VT_I4:
    case VT_UI4:
    case VT_R4:
        if (!(value->vt & VT_VECTOR))
            *(ULONG *)item->value = value->u.ulVal;
        else
            memcpy(item->value, value->u.caul.pElems, item_size);
        break;
    case VT_I8:
    case VT_UI8:
    case VT_R8:
        if (!(value->vt & VT_VECTOR))
            *(ULONGLONG *)item->value = value->u.uhVal.QuadPart;
        else
            memcpy(item->value, value->u.cauh.pElems, item_size);
        break;
    case VT_LPSTR:
        memcpy(item->value, value->u.pszVal, item_size);
        break;
    case VT_BLOB:
        memcpy(item->value, value->u.blob.pBlobData, item_size);
        break;
    default:
        FIXME("not supported variant type %d\n", value->vt);
        return InvalidParameter;
    }

    item->length = item_size;
    item->type = item_type;
    item->id = id;

    return Ok;
}

GpStatus WINGDIPAPI GdipGetPropertyItem(GpImage *image, PROPID propid, UINT size,
                                        PropertyItem *buffer)
{
    GpStatus stat;
    HRESULT hr;
    IWICMetadataReader *reader;
    PROPVARIANT id, value;

    TRACE("(%p,%#x,%u,%p)\n", image, propid, size, buffer);

    if (!image || !buffer) return InvalidParameter;

    if (image->type != ImageTypeBitmap)
    {
        FIXME("Not implemented for type %d\n", image->type);
        return NotImplemented;
    }

    if (((GpBitmap *)image)->prop_item)
    {
        UINT i;

        for (i = 0; i < ((GpBitmap *)image)->prop_count; i++)
        {
            if (propid == ((GpBitmap *)image)->prop_item[i].id)
            {
                if (size != sizeof(PropertyItem) + ((GpBitmap *)image)->prop_item[i].length)
                    return InvalidParameter;

                *buffer = ((GpBitmap *)image)->prop_item[i];
                buffer->value = buffer + 1;
                memcpy(buffer->value, ((GpBitmap *)image)->prop_item[i].value, buffer->length);
                return Ok;
            }
        }

        return PropertyNotFound;
    }

    reader = ((GpBitmap *)image)->metadata_reader;
    if (!reader) return PropertyNotFound;

    id.vt = VT_UI2;
    id.u.uiVal = propid;
    hr = IWICMetadataReader_GetValue(reader, NULL, &id, &value);
    if (FAILED(hr)) return PropertyNotFound;

    stat = propvariant_to_item(&value, buffer, size, propid);
    PropVariantClear(&value);

    return stat;
}

GpStatus WINGDIPAPI GdipGetPropertySize(GpImage *image, UINT *size, UINT *count)
{
    HRESULT hr;
    IWICMetadataReader *reader;
    IWICEnumMetadataItem *enumerator;
    UINT prop_count, prop_size, i;
    PROPVARIANT id, value;

    TRACE("(%p,%p,%p)\n", image, size, count);

    if (!image || !size || !count) return InvalidParameter;

    if (image->type != ImageTypeBitmap)
    {
        FIXME("Not implemented for type %d\n", image->type);
        return NotImplemented;
    }

    if (((GpBitmap *)image)->prop_item)
    {
        *count = ((GpBitmap *)image)->prop_count;
        *size = 0;

        for (i = 0; i < ((GpBitmap *)image)->prop_count; i++)
        {
            *size += sizeof(PropertyItem) + ((GpBitmap *)image)->prop_item[i].length;
        }

        return Ok;
    }

    reader = ((GpBitmap *)image)->metadata_reader;
    if (!reader) return PropertyNotFound;

    hr = IWICMetadataReader_GetCount(reader, &prop_count);
    if (FAILED(hr)) return hresult_to_status(hr);

    hr = IWICMetadataReader_GetEnumerator(reader, &enumerator);
    if (FAILED(hr)) return hresult_to_status(hr);

    IWICEnumMetadataItem_Reset(enumerator);

    prop_size = 0;

    PropVariantInit(&id);
    PropVariantInit(&value);

    for (i = 0; i < prop_count; i++)
    {
        UINT items_returned, item_size;

        hr = IWICEnumMetadataItem_Next(enumerator, 1, NULL, &id, &value, &items_returned);
        if (hr != S_OK) break;

        item_size = propvariant_size(&value);
        if (item_size) prop_size += sizeof(PropertyItem) + item_size;

        PropVariantClear(&id);
        PropVariantClear(&value);
    }

    IWICEnumMetadataItem_Release(enumerator);

    if (hr != S_OK) return PropertyNotFound;

    *count = prop_count;
    *size = prop_size;
    return Ok;
}

GpStatus WINGDIPAPI GdipGetAllPropertyItems(GpImage *image, UINT size,
                                            UINT count, PropertyItem *buf)
{
    GpStatus status;
    HRESULT hr;
    IWICMetadataReader *reader;
    IWICEnumMetadataItem *enumerator;
    UINT prop_count, prop_size, i;
    PROPVARIANT id, value;
    char *item_value;

    TRACE("(%p,%u,%u,%p)\n", image, size, count, buf);

    if (!image || !buf) return InvalidParameter;

    if (image->type != ImageTypeBitmap)
    {
        FIXME("Not implemented for type %d\n", image->type);
        return NotImplemented;
    }

    status = GdipGetPropertySize(image, &prop_size, &prop_count);
    if (status != Ok) return status;

    if (prop_count != count || prop_size != size) return InvalidParameter;

    if (((GpBitmap *)image)->prop_item)
    {
        memcpy(buf, ((GpBitmap *)image)->prop_item, prop_size);

        item_value = (char *)(buf + prop_count);

        for (i = 0; i < prop_count; i++)
        {
            buf[i].value = item_value;
            item_value += buf[i].length;
        }

        return Ok;
    }

    reader = ((GpBitmap *)image)->metadata_reader;
    if (!reader) return PropertyNotFound;

    hr = IWICMetadataReader_GetEnumerator(reader, &enumerator);
    if (FAILED(hr)) return hresult_to_status(hr);

    IWICEnumMetadataItem_Reset(enumerator);

    item_value = (char *)(buf + prop_count);

    PropVariantInit(&id);
    PropVariantInit(&value);

    for (i = 0; i < prop_count; i++)
    {
        PropertyItem *item;
        UINT items_returned, item_size;

        hr = IWICEnumMetadataItem_Next(enumerator, 1, NULL, &id, &value, &items_returned);
        if (hr != S_OK) break;

        if (id.vt != VT_UI2)
        {
            FIXME("not supported propvariant type for id: %u\n", id.vt);
            continue;
        }

        item_size = propvariant_size(&value);
        if (item_size)
        {
            item = HeapAlloc(GetProcessHeap(), 0, item_size + sizeof(*item));

            propvariant_to_item(&value, item, item_size + sizeof(*item), id.u.uiVal);
            buf[i].id = item->id;
            buf[i].type = item->type;
            buf[i].length = item_size;
            buf[i].value = item_value;
            memcpy(item_value, item->value, item_size);
            item_value += item_size;

            HeapFree(GetProcessHeap(), 0, item);
        }

        PropVariantClear(&id);
        PropVariantClear(&value);
    }

    IWICEnumMetadataItem_Release(enumerator);

    if (hr != S_OK) return PropertyNotFound;

    return Ok;
}

struct image_format_dimension
{
    const GUID *format;
    const GUID *dimension;
};

static const struct image_format_dimension image_format_dimensions[] =
{
    {&ImageFormatGIF, &FrameDimensionTime},
    {&ImageFormatIcon, &FrameDimensionResolution},
    {NULL}
};

GpStatus WINGDIPAPI GdipImageGetFrameCount(GpImage *image,
    GDIPCONST GUID* dimensionID, UINT* count)
{
    TRACE("(%p,%s,%p)\n", image, debugstr_guid(dimensionID), count);

    if(!image || !count)
        return InvalidParameter;

    if (!dimensionID ||
        IsEqualGUID(dimensionID, &image->format) ||
        IsEqualGUID(dimensionID, &FrameDimensionPage) ||
        IsEqualGUID(dimensionID, &FrameDimensionTime))
    {
        *count = image->frame_count;
        return Ok;
    }

    return InvalidParameter;
}

GpStatus WINGDIPAPI GdipImageGetFrameDimensionsCount(GpImage *image,
    UINT* count)
{
    TRACE("(%p, %p)\n", image, count);

    /* Native gdiplus 1.1 does not yet support multiple frame dimensions. */

    if(!image || !count)
        return InvalidParameter;

    *count = 1;

    return Ok;
}

GpStatus WINGDIPAPI GdipImageGetFrameDimensionsList(GpImage* image,
    GUID* dimensionIDs, UINT count)
{
    int i;
    const GUID *result=NULL;

    TRACE("(%p,%p,%u)\n", image, dimensionIDs, count);

    if(!image || !dimensionIDs || count != 1)
        return InvalidParameter;

    for (i=0; image_format_dimensions[i].format; i++)
    {
        if (IsEqualGUID(&image->format, image_format_dimensions[i].format))
        {
            result = image_format_dimensions[i].dimension;
            break;
        }
    }

    if (!result)
        result = &FrameDimensionPage;

    memcpy(dimensionIDs, result, sizeof(GUID));

    return Ok;
}

GpStatus WINGDIPAPI GdipLoadImageFromFile(GDIPCONST WCHAR* filename,
                                          GpImage **image)
{
    GpStatus stat;
    IStream *stream;

    TRACE("(%s) %p\n", debugstr_w(filename), image);

    if (!filename || !image)
        return InvalidParameter;

    stat = GdipCreateStreamOnFile(filename, GENERIC_READ, &stream);

    if (stat != Ok)
        return stat;

    stat = GdipLoadImageFromStream(stream, image);

    IStream_Release(stream);

    return stat;
}

/* FIXME: no icm handling */
GpStatus WINGDIPAPI GdipLoadImageFromFileICM(GDIPCONST WCHAR* filename,GpImage **image)
{
    TRACE("(%s) %p\n", debugstr_w(filename), image);

    return GdipLoadImageFromFile(filename, image);
}

static void add_property(GpBitmap *bitmap, PropertyItem *item)
{
    UINT prop_size, prop_count;
    PropertyItem *prop_item;

    if (bitmap->prop_item == NULL)
    {
        prop_size = prop_count = 0;
        prop_item = GdipAlloc(item->length + sizeof(PropertyItem));
        if (!prop_item) return;
    }
    else
    {
        UINT i;
        char *item_value;

        GdipGetPropertySize((GpImage *)bitmap, &prop_size, &prop_count);

        prop_item = GdipAlloc(prop_size + item->length + sizeof(PropertyItem));
        if (!prop_item) return;
        memcpy(prop_item, bitmap->prop_item, sizeof(PropertyItem) * bitmap->prop_count);
        prop_size -= sizeof(PropertyItem) * bitmap->prop_count;
        memcpy(prop_item + prop_count + 1, bitmap->prop_item + prop_count, prop_size);

        item_value = (char *)(prop_item + prop_count + 1);

        for (i = 0; i < prop_count; i++)
        {
            prop_item[i].value = item_value;
            item_value += prop_item[i].length;
        }
    }

    prop_item[prop_count].id = item->id;
    prop_item[prop_count].type = item->type;
    prop_item[prop_count].length = item->length;
    prop_item[prop_count].value = (char *)(prop_item + prop_count + 1) + prop_size;
    memcpy(prop_item[prop_count].value, item->value, item->length);

    GdipFree(bitmap->prop_item);
    bitmap->prop_item = prop_item;
    bitmap->prop_count++;
}

static BOOL get_bool_property(IWICMetadataReader *reader, const GUID *guid, const WCHAR *prop_name)
{
    HRESULT hr;
    GUID format;
    PROPVARIANT id, value;
    BOOL ret = FALSE;

    IWICMetadataReader_GetMetadataFormat(reader, &format);
    if (!IsEqualGUID(&format, guid)) return FALSE;

    PropVariantInit(&id);
    PropVariantInit(&value);

    id.vt = VT_LPWSTR;
    id.u.pwszVal = HeapAlloc(GetProcessHeap(), 0, (lstrlenW(prop_name) + 1) * sizeof(WCHAR));
    if (!id.u.pwszVal) return FALSE;
    lstrcpyW(id.u.pwszVal, prop_name);
    hr = IWICMetadataReader_GetValue(reader, NULL, &id, &value);
    if (hr == S_OK && value.vt == VT_BOOL)
        ret = value.u.boolVal;

    PropVariantClear(&id);
    PropVariantClear(&value);

    return ret;
}

static PropertyItem *get_property(IWICMetadataReader *reader, const GUID *guid, const WCHAR *prop_name)
{
    HRESULT hr;
    GUID format;
    PROPVARIANT id, value;
    PropertyItem *item = NULL;

    IWICMetadataReader_GetMetadataFormat(reader, &format);
    if (!IsEqualGUID(&format, guid)) return NULL;

    PropVariantInit(&id);
    PropVariantInit(&value);

    id.vt = VT_LPWSTR;
    id.u.pwszVal = HeapAlloc(GetProcessHeap(), 0, (lstrlenW(prop_name) + 1) * sizeof(WCHAR));
    if (!id.u.pwszVal) return NULL;
    lstrcpyW(id.u.pwszVal, prop_name);
    hr = IWICMetadataReader_GetValue(reader, NULL, &id, &value);
    if (hr == S_OK)
    {
        UINT item_size = propvariant_size(&value);
        if (item_size)
        {
            item_size += sizeof(*item);
            item = GdipAlloc(item_size);
            if (propvariant_to_item(&value, item, item_size, 0) != Ok)
            {
                GdipFree(item);
                item = NULL;
            }
        }
    }

    PropVariantClear(&id);
    PropVariantClear(&value);

    return item;
}

static PropertyItem *get_gif_comment(IWICMetadataReader *reader)
{
    static const WCHAR textentryW[] = { 'T','e','x','t','E','n','t','r','y',0 };
    PropertyItem *comment;

    comment = get_property(reader, &GUID_MetadataFormatGifComment, textentryW);
    if (comment)
        comment->id = PropertyTagExifUserComment;

    return comment;
}

static PropertyItem *get_gif_loopcount(IWICMetadataReader *reader)
{
    static const WCHAR applicationW[] = { 'A','p','p','l','i','c','a','t','i','o','n',0 };
    static const WCHAR dataW[] = { 'D','a','t','a',0 };
    PropertyItem *appext = NULL, *appdata = NULL, *loop = NULL;

    appext = get_property(reader, &GUID_MetadataFormatAPE, applicationW);
    if (appext)
    {
        if (appext->type == PropertyTagTypeByte && appext->length == 11 &&
            (!memcmp(appext->value, "NETSCAPE2.0", 11) || !memcmp(appext->value, "ANIMEXTS1.0", 11)))
        {
            appdata = get_property(reader, &GUID_MetadataFormatAPE, dataW);
            if (appdata)
            {
                if (appdata->type == PropertyTagTypeByte && appdata->length == 4)
                {
                    BYTE *data = appdata->value;
                    if (data[0] == 3 && data[1] == 1)
                    {
                        loop = GdipAlloc(sizeof(*loop) + sizeof(SHORT));
                        if (loop)
                        {
                            loop->type = PropertyTagTypeShort;
                            loop->id = PropertyTagLoopCount;
                            loop->length = sizeof(SHORT);
                            loop->value = loop + 1;
                            *(SHORT *)loop->value = data[2] | (data[3] << 8);
                        }
                    }
                }
            }
        }
    }

    GdipFree(appext);
    GdipFree(appdata);

    return loop;
}

static PropertyItem *get_gif_background(IWICMetadataReader *reader)
{
    static const WCHAR backgroundW[] = { 'B','a','c','k','g','r','o','u','n','d','C','o','l','o','r','I','n','d','e','x',0 };
    PropertyItem *background;

    background = get_property(reader, &GUID_MetadataFormatLSD, backgroundW);
    if (background)
        background->id = PropertyTagIndexBackground;

    return background;
}

static PropertyItem *get_gif_palette(IWICBitmapDecoder *decoder, IWICMetadataReader *reader)
{
    static const WCHAR global_flagW[] = { 'G','l','o','b','a','l','C','o','l','o','r','T','a','b','l','e','F','l','a','g',0 };
    HRESULT hr;
    IWICImagingFactory *factory;
    IWICPalette *palette;
    UINT count = 0;
    WICColor colors[256];

    if (!get_bool_property(reader, &GUID_MetadataFormatLSD, global_flagW))
        return NULL;

    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IWICImagingFactory, (void **)&factory);
    if (hr != S_OK) return NULL;

    hr = IWICImagingFactory_CreatePalette(factory, &palette);
    if (hr == S_OK)
    {
        hr = IWICBitmapDecoder_CopyPalette(decoder, palette);
        if (hr == S_OK)
            IWICPalette_GetColors(palette, 256, colors, &count);

        IWICPalette_Release(palette);
    }

    IWICImagingFactory_Release(factory);

    if (count)
    {
        PropertyItem *pal;
        UINT i;
        BYTE *rgb;

        pal = GdipAlloc(sizeof(*pal) + count * 3);
        if (!pal) return NULL;
        pal->type = PropertyTagTypeByte;
        pal->id = PropertyTagGlobalPalette;
        pal->value = pal + 1;
        pal->length = count * 3;

        rgb = pal->value;

        for (i = 0; i < count; i++)
        {
            rgb[i*3] = (colors[i] >> 16) & 0xff;
            rgb[i*3 + 1] = (colors[i] >> 8) & 0xff;
            rgb[i*3 + 2] = colors[i] & 0xff;
        }

        return pal;
    }

    return NULL;
}

static PropertyItem *get_gif_transparent_idx(IWICMetadataReader *reader)
{
    static const WCHAR transparency_flagW[] = { 'T','r','a','n','s','p','a','r','e','n','c','y','F','l','a','g',0 };
    static const WCHAR colorW[] = { 'T','r','a','n','s','p','a','r','e','n','t','C','o','l','o','r','I','n','d','e','x',0 };
    PropertyItem *index = NULL;

    if (get_bool_property(reader, &GUID_MetadataFormatGCE, transparency_flagW))
    {
        index = get_property(reader, &GUID_MetadataFormatGCE, colorW);
        if (index)
            index->id = PropertyTagIndexTransparent;
    }
    return index;
}

static LONG get_gif_frame_delay(IWICBitmapFrameDecode *frame)
{
    static const WCHAR delayW[] = { 'D','e','l','a','y',0 };
    HRESULT hr;
    IWICMetadataBlockReader *block_reader;
    IWICMetadataReader *reader;
    UINT block_count, i;
    PropertyItem *delay;
    LONG value = 0;

    hr = IWICBitmapFrameDecode_QueryInterface(frame, &IID_IWICMetadataBlockReader, (void **)&block_reader);
    if (hr == S_OK)
    {
        hr = IWICMetadataBlockReader_GetCount(block_reader, &block_count);
        if (hr == S_OK)
        {
            for (i = 0; i < block_count; i++)
            {
                hr = IWICMetadataBlockReader_GetReaderByIndex(block_reader, i, &reader);
                if (hr == S_OK)
                {
                    delay = get_property(reader, &GUID_MetadataFormatGCE, delayW);
                    if (delay)
                    {
                        if (delay->type == PropertyTagTypeShort && delay->length == 2)
                            value = *(SHORT *)delay->value;

                        GdipFree(delay);
                    }
                    IWICMetadataReader_Release(reader);
                }
            }
        }
        IWICMetadataBlockReader_Release(block_reader);
    }

    return value;
}

static void gif_metadata_reader(GpBitmap *bitmap, IWICBitmapDecoder *decoder, UINT active_frame)
{
    HRESULT hr;
    IWICBitmapFrameDecode *frame;
    IWICMetadataBlockReader *block_reader;
    IWICMetadataReader *reader;
    UINT frame_count, block_count, i;
    PropertyItem *delay = NULL, *comment = NULL, *background = NULL;
    PropertyItem *transparent_idx = NULL, *loop = NULL, *palette = NULL;

    IWICBitmapDecoder_GetFrameCount(decoder, &frame_count);
    if (frame_count > 1)
    {
        delay = GdipAlloc(sizeof(*delay) + frame_count * sizeof(LONG));
        if (delay)
        {
            LONG *value;

            delay->type = PropertyTagTypeLong;
            delay->id = PropertyTagFrameDelay;
            delay->length = frame_count * sizeof(LONG);
            delay->value = delay + 1;

            value = delay->value;

            for (i = 0; i < frame_count; i++)
            {
                hr = IWICBitmapDecoder_GetFrame(decoder, i, &frame);
                if (hr == S_OK)
                {
                    value[i] = get_gif_frame_delay(frame);
                    IWICBitmapFrameDecode_Release(frame);
                }
                else value[i] = 0;
            }
        }
    }

    hr = IWICBitmapDecoder_QueryInterface(decoder, &IID_IWICMetadataBlockReader, (void **)&block_reader);
    if (hr == S_OK)
    {
        hr = IWICMetadataBlockReader_GetCount(block_reader, &block_count);
        if (hr == S_OK)
        {
            for (i = 0; i < block_count; i++)
            {
                hr = IWICMetadataBlockReader_GetReaderByIndex(block_reader, i, &reader);
                if (hr == S_OK)
                {
                    if (!comment)
                        comment = get_gif_comment(reader);

                    if (frame_count > 1 && !loop)
                        loop = get_gif_loopcount(reader);

                    if (!background)
                        background = get_gif_background(reader);

                    if (!palette)
                        palette = get_gif_palette(decoder, reader);

                    IWICMetadataReader_Release(reader);
                }
            }
        }
        IWICMetadataBlockReader_Release(block_reader);
    }

    if (frame_count > 1 && !loop)
    {
        loop = GdipAlloc(sizeof(*loop) + sizeof(SHORT));
        if (loop)
        {
            loop->type = PropertyTagTypeShort;
            loop->id = PropertyTagLoopCount;
            loop->length = sizeof(SHORT);
            loop->value = loop + 1;
            *(SHORT *)loop->value = 1;
        }
    }

    if (delay) add_property(bitmap, delay);
    if (comment) add_property(bitmap, comment);
    if (loop) add_property(bitmap, loop);
    if (palette) add_property(bitmap, palette);
    if (background) add_property(bitmap, background);

    GdipFree(delay);
    GdipFree(comment);
    GdipFree(loop);
    GdipFree(palette);
    GdipFree(background);

    /* Win7 gdiplus always returns transparent color index from frame 0 */
    hr = IWICBitmapDecoder_GetFrame(decoder, 0, &frame);
    if (hr != S_OK) return;

    hr = IWICBitmapFrameDecode_QueryInterface(frame, &IID_IWICMetadataBlockReader, (void **)&block_reader);
    if (hr == S_OK)
    {
        hr = IWICMetadataBlockReader_GetCount(block_reader, &block_count);
        if (hr == S_OK)
        {
            for (i = 0; i < block_count; i++)
            {
                hr = IWICMetadataBlockReader_GetReaderByIndex(block_reader, i, &reader);
                if (hr == S_OK)
                {
                    if (!transparent_idx)
                        transparent_idx = get_gif_transparent_idx(reader);

                    IWICMetadataReader_Release(reader);
                }
            }
        }
        IWICMetadataBlockReader_Release(block_reader);
    }

    if (transparent_idx) add_property(bitmap, transparent_idx);
    GdipFree(transparent_idx);

    IWICBitmapFrameDecode_Release(frame);
}

typedef void (*metadata_reader_func)(GpBitmap *bitmap, IWICBitmapDecoder *decoder, UINT frame);

static GpStatus decode_image_wic(IStream *stream, GDIPCONST CLSID *clsid,
    UINT active_frame, metadata_reader_func metadata_reader, GpImage **image)
{
    GpStatus status=Ok;
    GpBitmap *bitmap;
    HRESULT hr;
    IWICBitmapDecoder *decoder;
    IWICBitmapFrameDecode *frame;
    IWICBitmapSource *source=NULL;
    IWICMetadataBlockReader *block_reader;
    WICPixelFormatGUID wic_format;
    PixelFormat gdip_format=0;
    ColorPalette *palette = NULL;
    WICBitmapPaletteType palette_type = WICBitmapPaletteTypeFixedHalftone256;
    int i;
    UINT width, height, frame_count;
    BitmapData lockeddata;
    WICRect wrc;
    HRESULT initresult;

    TRACE("%p,%s,%u,%p\n", stream, wine_dbgstr_guid(clsid), active_frame, image);

    initresult = CoInitialize(NULL);

    hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICBitmapDecoder, (void**)&decoder);
    if (FAILED(hr)) goto end;

    hr = IWICBitmapDecoder_Initialize(decoder, stream, WICDecodeMetadataCacheOnLoad);
    if (SUCCEEDED(hr))
    {
        IWICBitmapDecoder_GetFrameCount(decoder, &frame_count);
        hr = IWICBitmapDecoder_GetFrame(decoder, active_frame, &frame);
    }

    if (SUCCEEDED(hr)) /* got frame */
    {
        hr = IWICBitmapFrameDecode_GetPixelFormat(frame, &wic_format);

        if (SUCCEEDED(hr))
        {
            IWICBitmapSource *bmp_source;
            IWICBitmapFrameDecode_QueryInterface(frame, &IID_IWICBitmapSource, (void **)&bmp_source);

            for (i=0; pixel_formats[i].wic_format; i++)
            {
                if (IsEqualGUID(&wic_format, pixel_formats[i].wic_format))
                {
                    source = bmp_source;
                    gdip_format = pixel_formats[i].gdip_format;
                    palette_type = pixel_formats[i].palette_type;
                    break;
                }
            }
            if (!source)
            {
                /* unknown format; fall back on 32bppARGB */
                hr = WICConvertBitmapSource(&GUID_WICPixelFormat32bppBGRA, bmp_source, &source);
                gdip_format = PixelFormat32bppARGB;
                IWICBitmapSource_Release(bmp_source);
            }
            TRACE("%s => %#x\n", wine_dbgstr_guid(&wic_format), gdip_format);
        }

        if (SUCCEEDED(hr)) /* got source */
        {
            hr = IWICBitmapSource_GetSize(source, &width, &height);

            if (SUCCEEDED(hr))
                status = GdipCreateBitmapFromScan0(width, height, 0, gdip_format,
                    NULL, &bitmap);

            if (SUCCEEDED(hr) && status == Ok) /* created bitmap */
            {
                status = GdipBitmapLockBits(bitmap, NULL, ImageLockModeWrite,
                    gdip_format, &lockeddata);
                if (status == Ok) /* locked bitmap */
                {
                    wrc.X = 0;
                    wrc.Width = width;
                    wrc.Height = 1;
                    for (i=0; i<height; i++)
                    {
                        wrc.Y = i;
                        hr = IWICBitmapSource_CopyPixels(source, &wrc, abs(lockeddata.Stride),
                            abs(lockeddata.Stride), (BYTE*)lockeddata.Scan0+lockeddata.Stride*i);
                        if (FAILED(hr)) break;
                    }

                    GdipBitmapUnlockBits(bitmap, &lockeddata);
                }

                if (SUCCEEDED(hr) && status == Ok)
                    *image = (GpImage*)bitmap;
                else
                {
                    *image = NULL;
                    GdipDisposeImage((GpImage*)bitmap);
                }

                if (SUCCEEDED(hr) && status == Ok)
                {
                    double dpix, dpiy;
                    hr = IWICBitmapSource_GetResolution(source, &dpix, &dpiy);
                    if (SUCCEEDED(hr))
                    {
                        bitmap->image.xres = dpix;
                        bitmap->image.yres = dpiy;
                    }
                    hr = S_OK;
                }
            }

            IWICBitmapSource_Release(source);
        }

        if (SUCCEEDED(hr)) {
            bitmap->metadata_reader = NULL;

            if (metadata_reader)
                metadata_reader(bitmap, decoder, active_frame);
            else if (IWICBitmapFrameDecode_QueryInterface(frame, &IID_IWICMetadataBlockReader, (void **)&block_reader) == S_OK)
            {
                UINT block_count = 0;
                if (IWICMetadataBlockReader_GetCount(block_reader, &block_count) == S_OK && block_count)
                    IWICMetadataBlockReader_GetReaderByIndex(block_reader, 0, &bitmap->metadata_reader);
                IWICMetadataBlockReader_Release(block_reader);
            }

            palette = get_palette(frame, palette_type);
            IWICBitmapFrameDecode_Release(frame);
        }
    }

    IWICBitmapDecoder_Release(decoder);

end:
    if (SUCCEEDED(initresult)) CoUninitialize();

    if (FAILED(hr) && status == Ok) status = hresult_to_status(hr);

    if (status == Ok)
    {
        /* Native GDI+ used to be smarter, but since Win7 it just sets these flags. */
        bitmap->image.flags |= ImageFlagsReadOnly|ImageFlagsHasRealPixelSize|ImageFlagsHasRealDPI|ImageFlagsColorSpaceRGB;
        bitmap->image.frame_count = frame_count;
        bitmap->image.current_frame = active_frame;
        bitmap->image.stream = stream;
        if (palette)
        {
            GdipFree(bitmap->image.palette);
            bitmap->image.palette = palette;
        }
        else
        {
            if (IsEqualGUID(&wic_format, &GUID_WICPixelFormatBlackWhite))
                bitmap->image.palette->Flags = 0;
        }
        /* Pin the source stream */
        IStream_AddRef(stream);
        TRACE("=> %p\n", *image);
    }

    return status;
}

static GpStatus decode_image_icon(IStream* stream, REFCLSID clsid, UINT active_frame, GpImage **image)
{
    return decode_image_wic(stream, &CLSID_WICIcoDecoder, active_frame, NULL, image);
}

static GpStatus decode_image_bmp(IStream* stream, REFCLSID clsid, UINT active_frame, GpImage **image)
{
    GpStatus status;
    GpBitmap* bitmap;

    status = decode_image_wic(stream, &CLSID_WICBmpDecoder, active_frame, NULL, image);

    bitmap = (GpBitmap*)*image;

    if (status == Ok && bitmap->format == PixelFormat32bppARGB)
    {
        /* WIC supports bmp files with alpha, but gdiplus does not */
        bitmap->format = PixelFormat32bppRGB;
    }

    return status;
}

static GpStatus decode_image_jpeg(IStream* stream, REFCLSID clsid, UINT active_frame, GpImage **image)
{
    return decode_image_wic(stream, &CLSID_WICJpegDecoder, active_frame, NULL, image);
}

static GpStatus decode_image_png(IStream* stream, REFCLSID clsid, UINT active_frame, GpImage **image)
{
    return decode_image_wic(stream, &CLSID_WICPngDecoder, active_frame, NULL, image);
}

static GpStatus decode_image_gif(IStream* stream, REFCLSID clsid, UINT active_frame, GpImage **image)
{
    return decode_image_wic(stream, &CLSID_WICGifDecoder, active_frame, gif_metadata_reader, image);
}

static GpStatus decode_image_tiff(IStream* stream, REFCLSID clsid, UINT active_frame, GpImage **image)
{
    return decode_image_wic(stream, &CLSID_WICTiffDecoder, active_frame, NULL, image);
}

static GpStatus decode_image_olepicture_metafile(IStream* stream, REFCLSID clsid, UINT active_frame, GpImage **image)
{
    IPicture *pic;

    TRACE("%p %p\n", stream, image);

    if(!stream || !image)
        return InvalidParameter;

    if(OleLoadPicture(stream, 0, FALSE, &IID_IPicture,
        (LPVOID*) &pic) != S_OK){
        TRACE("Could not load picture\n");
        return GenericError;
    }

    /* FIXME: missing initialization code */
    *image = GdipAlloc(sizeof(GpMetafile));
    if(!*image) return OutOfMemory;
    (*image)->type = ImageTypeMetafile;
    (*image)->stream = NULL;
    (*image)->picture = pic;
    (*image)->flags   = ImageFlagsNone;
    (*image)->frame_count = 1;
    (*image)->current_frame = 0;
    (*image)->palette = NULL;

    TRACE("<-- %p\n", *image);

    return Ok;
}

typedef GpStatus (*encode_image_func)(GpImage *image, IStream* stream,
    GDIPCONST CLSID* clsid, GDIPCONST EncoderParameters* params);

typedef GpStatus (*decode_image_func)(IStream *stream, REFCLSID clsid, UINT active_frame, GpImage **image);

typedef struct image_codec {
    ImageCodecInfo info;
    encode_image_func encode_func;
    decode_image_func decode_func;
} image_codec;

typedef enum {
    BMP,
    JPEG,
    GIF,
    TIFF,
    EMF,
    WMF,
    PNG,
    ICO,
    NUM_CODECS
} ImageFormat;

static const struct image_codec codecs[NUM_CODECS];

static GpStatus get_decoder_info(IStream* stream, const struct image_codec **result)
{
    BYTE signature[8];
    const BYTE *pattern, *mask;
    LARGE_INTEGER seek;
    HRESULT hr;
    UINT bytesread;
    int i;
    DWORD j, sig;

    /* seek to the start of the stream */
    seek.QuadPart = 0;
    hr = IStream_Seek(stream, seek, STREAM_SEEK_SET, NULL);
    if (FAILED(hr)) return hresult_to_status(hr);

    /* read the first 8 bytes */
    /* FIXME: This assumes all codecs have signatures <= 8 bytes in length */
    hr = IStream_Read(stream, signature, 8, &bytesread);
    if (FAILED(hr)) return hresult_to_status(hr);
    if (hr == S_FALSE || bytesread == 0) return GenericError;

    for (i = 0; i < NUM_CODECS; i++) {
        if ((codecs[i].info.Flags & ImageCodecFlagsDecoder) &&
            bytesread >= codecs[i].info.SigSize)
        {
            for (sig=0; sig<codecs[i].info.SigCount; sig++)
            {
                pattern = &codecs[i].info.SigPattern[codecs[i].info.SigSize*sig];
                mask = &codecs[i].info.SigMask[codecs[i].info.SigSize*sig];
                for (j=0; j<codecs[i].info.SigSize; j++)
                    if ((signature[j] & mask[j]) != pattern[j])
                        break;
                if (j == codecs[i].info.SigSize)
                {
                    *result = &codecs[i];
                    return Ok;
                }
            }
        }
    }

    TRACE("no match for %i byte signature %x %x %x %x %x %x %x %x\n", bytesread,
        signature[0],signature[1],signature[2],signature[3],
        signature[4],signature[5],signature[6],signature[7]);

    return GenericError;
}

GpStatus WINGDIPAPI GdipImageSelectActiveFrame(GpImage *image, GDIPCONST GUID *dimensionID,
                                               UINT frame)
{
    GpStatus stat;
    LARGE_INTEGER seek;
    HRESULT hr;
    const struct image_codec *codec = NULL;
    GpImage *new_image;

    TRACE("(%p,%s,%u)\n", image, debugstr_guid(dimensionID), frame);

    if (!image || !dimensionID)
        return InvalidParameter;

    if (frame >= image->frame_count)
    {
        WARN("requested frame %u, but image has only %u\n", frame, image->frame_count);
        return InvalidParameter;
    }

    if (image->type != ImageTypeBitmap && image->type != ImageTypeMetafile)
    {
        WARN("invalid image type %d\n", image->type);
        return InvalidParameter;
    }

    if (image->current_frame == frame)
        return Ok;

    if (!image->stream)
    {
        TRACE("image doesn't have an associated stream\n");
        return Ok;
    }

    /* choose an appropriate image decoder */
    stat = get_decoder_info(image->stream, &codec);
    if (stat != Ok)
    {
        WARN("can't find decoder info\n");
        return stat;
    }

    /* seek to the start of the stream */
    seek.QuadPart = 0;
    hr = IStream_Seek(image->stream, seek, STREAM_SEEK_SET, NULL);
    if (FAILED(hr))
        return hresult_to_status(hr);

    /* call on the image decoder to do the real work */
    stat = codec->decode_func(image->stream, &codec->info.Clsid, frame, &new_image);

    if (stat == Ok)
    {
        memcpy(&new_image->format, &codec->info.FormatID, sizeof(GUID));
        free_image_data(image);
        if (image->type == ImageTypeBitmap)
            *(GpBitmap *)image = *(GpBitmap *)new_image;
        else if (image->type == ImageTypeMetafile)
            *(GpMetafile *)image = *(GpMetafile *)new_image;
        new_image->type = ~0;
        GdipFree(new_image);
        return Ok;
    }

    return stat;
}

GpStatus WINGDIPAPI GdipLoadImageFromStream(IStream *stream, GpImage **image)
{
    GpStatus stat;
    LARGE_INTEGER seek;
    HRESULT hr;
    const struct image_codec *codec=NULL;

    /* choose an appropriate image decoder */
    stat = get_decoder_info(stream, &codec);
    if (stat != Ok) return stat;

    /* seek to the start of the stream */
    seek.QuadPart = 0;
    hr = IStream_Seek(stream, seek, STREAM_SEEK_SET, NULL);
    if (FAILED(hr)) return hresult_to_status(hr);

    /* call on the image decoder to do the real work */
    stat = codec->decode_func(stream, &codec->info.Clsid, 0, image);

    /* take note of the original data format */
    if (stat == Ok)
    {
        memcpy(&(*image)->format, &codec->info.FormatID, sizeof(GUID));
        return Ok;
    }

    return stat;
}

/* FIXME: no ICM */
GpStatus WINGDIPAPI GdipLoadImageFromStreamICM(IStream* stream, GpImage **image)
{
    TRACE("%p %p\n", stream, image);

    return GdipLoadImageFromStream(stream, image);
}

GpStatus WINGDIPAPI GdipRemovePropertyItem(GpImage *image, PROPID propId)
{
    static int calls;

    TRACE("(%p,%u)\n", image, propId);

    if(!image)
        return InvalidParameter;

    if(!(calls++))
        FIXME("not implemented\n");

    return NotImplemented;
}

GpStatus WINGDIPAPI GdipSetPropertyItem(GpImage *image, GDIPCONST PropertyItem* item)
{
    static int calls;

    if (!image || !item) return InvalidParameter;

    TRACE("(%p,%p:%#x,%u,%u,%p)\n", image, item, item->id, item->type, item->length, item->value);

    if(!(calls++))
        FIXME("not implemented\n");

    return Ok;
}

GpStatus WINGDIPAPI GdipSaveImageToFile(GpImage *image, GDIPCONST WCHAR* filename,
                                        GDIPCONST CLSID *clsidEncoder,
                                        GDIPCONST EncoderParameters *encoderParams)
{
    GpStatus stat;
    IStream *stream;

    TRACE("%p (%s) %p %p\n", image, debugstr_w(filename), clsidEncoder, encoderParams);

    if (!image || !filename|| !clsidEncoder)
        return InvalidParameter;

    stat = GdipCreateStreamOnFile(filename, GENERIC_WRITE, &stream);
    if (stat != Ok)
        return GenericError;

    stat = GdipSaveImageToStream(image, stream, clsidEncoder, encoderParams);

    IStream_Release(stream);
    return stat;
}

/*************************************************************************
 * Encoding functions -
 *   These functions encode an image in different image file formats.
 */
#define BITMAP_FORMAT_BMP   0x4d42 /* "BM" */
#define BITMAP_FORMAT_JPEG  0xd8ff
#define BITMAP_FORMAT_GIF   0x4947
#define BITMAP_FORMAT_PNG   0x5089
#define BITMAP_FORMAT_APM   0xcdd7

static GpStatus encode_image_WIC(GpImage *image, IStream* stream,
    GDIPCONST CLSID* clsid, GDIPCONST EncoderParameters* params)
{
    GpStatus stat;
    GpBitmap *bitmap;
    IWICBitmapEncoder *encoder;
    IWICBitmapFrameEncode *frameencode;
    IPropertyBag2 *encoderoptions;
    HRESULT hr;
    UINT width, height;
    PixelFormat gdipformat=0;
    WICPixelFormatGUID wicformat;
    GpRect rc;
    BitmapData lockeddata;
    HRESULT initresult;
    UINT i;

    if (image->type != ImageTypeBitmap)
        return GenericError;

    bitmap = (GpBitmap*)image;

    GdipGetImageWidth(image, &width);
    GdipGetImageHeight(image, &height);

    rc.X = 0;
    rc.Y = 0;
    rc.Width = width;
    rc.Height = height;

    initresult = CoInitialize(NULL);

    hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICBitmapEncoder, (void**)&encoder);
    if (FAILED(hr))
    {
        if (SUCCEEDED(initresult)) CoUninitialize();
        return hresult_to_status(hr);
    }

    hr = IWICBitmapEncoder_Initialize(encoder, stream, WICBitmapEncoderNoCache);

    if (SUCCEEDED(hr))
    {
        hr = IWICBitmapEncoder_CreateNewFrame(encoder, &frameencode, &encoderoptions);
    }

    if (SUCCEEDED(hr)) /* created frame */
    {
        hr = IWICBitmapFrameEncode_Initialize(frameencode, encoderoptions);

        if (SUCCEEDED(hr))
            hr = IWICBitmapFrameEncode_SetSize(frameencode, width, height);

        if (SUCCEEDED(hr))
            hr = IWICBitmapFrameEncode_SetResolution(frameencode, image->xres, image->yres);

        if (SUCCEEDED(hr))
        {
            for (i=0; pixel_formats[i].wic_format; i++)
            {
                if (pixel_formats[i].gdip_format == bitmap->format)
                {
                    memcpy(&wicformat, pixel_formats[i].wic_format, sizeof(GUID));
                    gdipformat = bitmap->format;
                    break;
                }
            }
            if (!gdipformat)
            {
                memcpy(&wicformat, &GUID_WICPixelFormat32bppBGRA, sizeof(GUID));
                gdipformat = PixelFormat32bppARGB;
            }

            hr = IWICBitmapFrameEncode_SetPixelFormat(frameencode, &wicformat);
        }

        if (SUCCEEDED(hr))
        {
            stat = GdipBitmapLockBits(bitmap, &rc, ImageLockModeRead, gdipformat,
                &lockeddata);

            if (stat == Ok)
            {
                UINT row_size = (lockeddata.Width * PIXELFORMATBPP(gdipformat) + 7)/8;
                BYTE *row;

                /* write one row at a time in case stride is negative */
                row = lockeddata.Scan0;
                for (i=0; i<lockeddata.Height; i++)
                {
                    hr = IWICBitmapFrameEncode_WritePixels(frameencode, 1, row_size, row_size, row);
                    if (FAILED(hr)) break;
                    row += lockeddata.Stride;
                }

                GdipBitmapUnlockBits(bitmap, &lockeddata);
            }
            else
                hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
            hr = IWICBitmapFrameEncode_Commit(frameencode);

        IWICBitmapFrameEncode_Release(frameencode);
        IPropertyBag2_Release(encoderoptions);
    }

    if (SUCCEEDED(hr))
        hr = IWICBitmapEncoder_Commit(encoder);

    IWICBitmapEncoder_Release(encoder);

    if (SUCCEEDED(initresult)) CoUninitialize();

    return hresult_to_status(hr);
}

static GpStatus encode_image_BMP(GpImage *image, IStream* stream,
    GDIPCONST CLSID* clsid, GDIPCONST EncoderParameters* params)
{
    return encode_image_WIC(image, stream, &CLSID_WICBmpEncoder, params);
}

static GpStatus encode_image_tiff(GpImage *image, IStream* stream,
    GDIPCONST CLSID* clsid, GDIPCONST EncoderParameters* params)
{
    return encode_image_WIC(image, stream, &CLSID_WICTiffEncoder, params);
}

static GpStatus encode_image_png(GpImage *image, IStream* stream,
    GDIPCONST CLSID* clsid, GDIPCONST EncoderParameters* params)
{
    return encode_image_WIC(image, stream, &CLSID_WICPngEncoder, params);
}

static GpStatus encode_image_jpeg(GpImage *image, IStream* stream,
    GDIPCONST CLSID* clsid, GDIPCONST EncoderParameters* params)
{
    return encode_image_WIC(image, stream, &CLSID_WICJpegEncoder, params);
}

/*****************************************************************************
 * GdipSaveImageToStream [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipSaveImageToStream(GpImage *image, IStream* stream,
    GDIPCONST CLSID* clsid, GDIPCONST EncoderParameters* params)
{
    GpStatus stat;
    encode_image_func encode_image;
    int i;

    TRACE("%p %p %p %p\n", image, stream, clsid, params);

    if(!image || !stream)
        return InvalidParameter;

    /* select correct encoder */
    encode_image = NULL;
    for (i = 0; i < NUM_CODECS; i++) {
        if ((codecs[i].info.Flags & ImageCodecFlagsEncoder) &&
            IsEqualCLSID(clsid, &codecs[i].info.Clsid))
            encode_image = codecs[i].encode_func;
    }
    if (encode_image == NULL)
        return UnknownImageFormat;

    stat = encode_image(image, stream, clsid, params);

    return stat;
}

/*****************************************************************************
 * GdipSaveAdd [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipSaveAdd(GpImage *image, GDIPCONST EncoderParameters *params)
{
    FIXME("(%p,%p): stub\n", image, params);
    return Ok;
}

/*****************************************************************************
 * GdipGetImagePalette [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipGetImagePalette(GpImage *image, ColorPalette *palette, INT size)
{
    INT count;

    TRACE("(%p,%p,%i)\n", image, palette, size);

    if (!image || !palette)
        return InvalidParameter;

    count = image->palette ? image->palette->Count : 0;

    if (size < (sizeof(UINT)*2+sizeof(ARGB)*count))
    {
        TRACE("<-- InsufficientBuffer\n");
        return InsufficientBuffer;
    }

    if (image->palette)
    {
        palette->Flags = image->palette->Flags;
        palette->Count = image->palette->Count;
        memcpy(palette->Entries, image->palette->Entries, sizeof(ARGB)*image->palette->Count);
    }
    else
    {
        palette->Flags = 0;
        palette->Count = 0;
    }
    return Ok;
}

/*****************************************************************************
 * GdipSetImagePalette [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipSetImagePalette(GpImage *image,
    GDIPCONST ColorPalette *palette)
{
    ColorPalette *new_palette;

    TRACE("(%p,%p)\n", image, palette);

    if(!image || !palette || palette->Count > 256)
        return InvalidParameter;

    new_palette = GdipAlloc(2 * sizeof(UINT) + palette->Count * sizeof(ARGB));
    if (!new_palette) return OutOfMemory;

    GdipFree(image->palette);
    image->palette = new_palette;
    image->palette->Flags = palette->Flags;
    image->palette->Count = palette->Count;
    memcpy(image->palette->Entries, palette->Entries, sizeof(ARGB)*palette->Count);

    return Ok;
}

/*************************************************************************
 * Encoders -
 *   Structures that represent which formats we support for encoding.
 */

/* ImageCodecInfo creation routines taken from libgdiplus */
static const WCHAR bmp_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'B', 'M', 'P', 0}; /* Built-in BMP */
static const WCHAR bmp_extension[] = {'*','.','B', 'M', 'P',';', '*','.', 'D','I', 'B',';', '*','.', 'R', 'L', 'E',0}; /* *.BMP;*.DIB;*.RLE */
static const WCHAR bmp_mimetype[] = {'i', 'm', 'a','g', 'e', '/', 'b', 'm', 'p', 0}; /* image/bmp */
static const WCHAR bmp_format[] = {'B', 'M', 'P', 0}; /* BMP */
static const BYTE bmp_sig_pattern[] = { 0x42, 0x4D };
static const BYTE bmp_sig_mask[] = { 0xFF, 0xFF };

static const WCHAR jpeg_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'J','P','E','G', 0};
static const WCHAR jpeg_extension[] = {'*','.','J','P','G',';', '*','.','J','P','E','G',';', '*','.','J','P','E',';', '*','.','J','F','I','F',0};
static const WCHAR jpeg_mimetype[] = {'i','m','a','g','e','/','j','p','e','g', 0};
static const WCHAR jpeg_format[] = {'J','P','E','G',0};
static const BYTE jpeg_sig_pattern[] = { 0xFF, 0xD8 };
static const BYTE jpeg_sig_mask[] = { 0xFF, 0xFF };

static const WCHAR gif_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'G','I','F', 0};
static const WCHAR gif_extension[] = {'*','.','G','I','F',0};
static const WCHAR gif_mimetype[] = {'i','m','a','g','e','/','g','i','f', 0};
static const WCHAR gif_format[] = {'G','I','F',0};
static const BYTE gif_sig_pattern[12] = "GIF87aGIF89a";
static const BYTE gif_sig_mask[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static const WCHAR tiff_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'T','I','F','F', 0};
static const WCHAR tiff_extension[] = {'*','.','T','I','F','F',';','*','.','T','I','F',0};
static const WCHAR tiff_mimetype[] = {'i','m','a','g','e','/','t','i','f','f', 0};
static const WCHAR tiff_format[] = {'T','I','F','F',0};
static const BYTE tiff_sig_pattern[] = {0x49,0x49,42,0,0x4d,0x4d,0,42};
static const BYTE tiff_sig_mask[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static const WCHAR emf_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'E','M','F', 0};
static const WCHAR emf_extension[] = {'*','.','E','M','F',0};
static const WCHAR emf_mimetype[] = {'i','m','a','g','e','/','x','-','e','m','f', 0};
static const WCHAR emf_format[] = {'E','M','F',0};
static const BYTE emf_sig_pattern[] = { 0x01, 0x00, 0x00, 0x00 };
static const BYTE emf_sig_mask[] = { 0xFF, 0xFF, 0xFF, 0xFF };

static const WCHAR wmf_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'W','M','F', 0};
static const WCHAR wmf_extension[] = {'*','.','W','M','F',0};
static const WCHAR wmf_mimetype[] = {'i','m','a','g','e','/','x','-','w','m','f', 0};
static const WCHAR wmf_format[] = {'W','M','F',0};
static const BYTE wmf_sig_pattern[] = { 0xd7, 0xcd };
static const BYTE wmf_sig_mask[] = { 0xFF, 0xFF };

static const WCHAR png_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'P','N','G', 0};
static const WCHAR png_extension[] = {'*','.','P','N','G',0};
static const WCHAR png_mimetype[] = {'i','m','a','g','e','/','p','n','g', 0};
static const WCHAR png_format[] = {'P','N','G',0};
static const BYTE png_sig_pattern[] = { 137, 80, 78, 71, 13, 10, 26, 10, };
static const BYTE png_sig_mask[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static const WCHAR ico_codecname[] = {'B', 'u', 'i','l', 't', '-','i', 'n', ' ', 'I','C','O', 0};
static const WCHAR ico_extension[] = {'*','.','I','C','O',0};
static const WCHAR ico_mimetype[] = {'i','m','a','g','e','/','x','-','i','c','o','n', 0};
static const WCHAR ico_format[] = {'I','C','O',0};
static const BYTE ico_sig_pattern[] = { 0x00, 0x00, 0x01, 0x00 };
static const BYTE ico_sig_mask[] = { 0xFF, 0xFF, 0xFF, 0xFF };

static const struct image_codec codecs[NUM_CODECS] = {
    {
        { /* BMP */
            /* Clsid */              { 0x557cf400, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3cabU, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          bmp_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  bmp_format,
            /* FilenameExtension */  bmp_extension,
            /* MimeType */           bmp_mimetype,
            /* Flags */              ImageCodecFlagsEncoder | ImageCodecFlagsDecoder | ImageCodecFlagsSupportBitmap | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           1,
            /* SigSize */            2,
            /* SigPattern */         bmp_sig_pattern,
            /* SigMask */            bmp_sig_mask,
        },
        encode_image_BMP,
        decode_image_bmp
    },
    {
        { /* JPEG */
            /* Clsid */              { 0x557cf401, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3caeU, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          jpeg_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  jpeg_format,
            /* FilenameExtension */  jpeg_extension,
            /* MimeType */           jpeg_mimetype,
            /* Flags */              ImageCodecFlagsEncoder | ImageCodecFlagsDecoder | ImageCodecFlagsSupportBitmap | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           1,
            /* SigSize */            2,
            /* SigPattern */         jpeg_sig_pattern,
            /* SigMask */            jpeg_sig_mask,
        },
        encode_image_jpeg,
        decode_image_jpeg
    },
    {
        { /* GIF */
            /* Clsid */              { 0x557cf402, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3cb0U, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          gif_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  gif_format,
            /* FilenameExtension */  gif_extension,
            /* MimeType */           gif_mimetype,
            /* Flags */              ImageCodecFlagsDecoder | ImageCodecFlagsSupportBitmap | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           2,
            /* SigSize */            6,
            /* SigPattern */         gif_sig_pattern,
            /* SigMask */            gif_sig_mask,
        },
        NULL,
        decode_image_gif
    },
    {
        { /* TIFF */
            /* Clsid */              { 0x557cf405, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3cb1U, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          tiff_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  tiff_format,
            /* FilenameExtension */  tiff_extension,
            /* MimeType */           tiff_mimetype,
            /* Flags */              ImageCodecFlagsDecoder | ImageCodecFlagsEncoder | ImageCodecFlagsSupportBitmap | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           2,
            /* SigSize */            4,
            /* SigPattern */         tiff_sig_pattern,
            /* SigMask */            tiff_sig_mask,
        },
        encode_image_tiff,
        decode_image_tiff
    },
    {
        { /* EMF */
            /* Clsid */              { 0x557cf403, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3cacU, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          emf_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  emf_format,
            /* FilenameExtension */  emf_extension,
            /* MimeType */           emf_mimetype,
            /* Flags */              ImageCodecFlagsDecoder | ImageCodecFlagsSupportVector | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           1,
            /* SigSize */            4,
            /* SigPattern */         emf_sig_pattern,
            /* SigMask */            emf_sig_mask,
        },
        NULL,
        decode_image_olepicture_metafile
    },
    {
        { /* WMF */
            /* Clsid */              { 0x557cf404, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3cadU, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          wmf_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  wmf_format,
            /* FilenameExtension */  wmf_extension,
            /* MimeType */           wmf_mimetype,
            /* Flags */              ImageCodecFlagsDecoder | ImageCodecFlagsSupportVector | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           1,
            /* SigSize */            2,
            /* SigPattern */         wmf_sig_pattern,
            /* SigMask */            wmf_sig_mask,
        },
        NULL,
        decode_image_olepicture_metafile
    },
    {
        { /* PNG */
            /* Clsid */              { 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3cafU, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          png_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  png_format,
            /* FilenameExtension */  png_extension,
            /* MimeType */           png_mimetype,
            /* Flags */              ImageCodecFlagsEncoder | ImageCodecFlagsDecoder | ImageCodecFlagsSupportBitmap | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           1,
            /* SigSize */            8,
            /* SigPattern */         png_sig_pattern,
            /* SigMask */            png_sig_mask,
        },
        encode_image_png,
        decode_image_png
    },
    {
        { /* ICO */
            /* Clsid */              { 0x557cf407, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } },
            /* FormatID */           { 0xb96b3cabU, 0x0728U, 0x11d3U, {0x9d, 0x7b, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e} },
            /* CodecName */          ico_codecname,
            /* DllName */            NULL,
            /* FormatDescription */  ico_format,
            /* FilenameExtension */  ico_extension,
            /* MimeType */           ico_mimetype,
            /* Flags */              ImageCodecFlagsDecoder | ImageCodecFlagsSupportBitmap | ImageCodecFlagsBuiltin,
            /* Version */            1,
            /* SigCount */           1,
            /* SigSize */            4,
            /* SigPattern */         ico_sig_pattern,
            /* SigMask */            ico_sig_mask,
        },
        NULL,
        decode_image_icon
    },
};

/*****************************************************************************
 * GdipGetImageDecodersSize [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipGetImageDecodersSize(UINT *numDecoders, UINT *size)
{
    int decoder_count=0;
    int i;
    TRACE("%p %p\n", numDecoders, size);

    if (!numDecoders || !size)
        return InvalidParameter;

    for (i=0; i<NUM_CODECS; i++)
    {
        if (codecs[i].info.Flags & ImageCodecFlagsDecoder)
            decoder_count++;
    }

    *numDecoders = decoder_count;
    *size = decoder_count * sizeof(ImageCodecInfo);

    return Ok;
}

/*****************************************************************************
 * GdipGetImageDecoders [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipGetImageDecoders(UINT numDecoders, UINT size, ImageCodecInfo *decoders)
{
    int i, decoder_count=0;
    TRACE("%u %u %p\n", numDecoders, size, decoders);

    if (!decoders ||
        size != numDecoders * sizeof(ImageCodecInfo))
        return GenericError;

    for (i=0; i<NUM_CODECS; i++)
    {
        if (codecs[i].info.Flags & ImageCodecFlagsDecoder)
        {
            if (decoder_count == numDecoders) return GenericError;
            memcpy(&decoders[decoder_count], &codecs[i].info, sizeof(ImageCodecInfo));
            decoder_count++;
        }
    }

    if (decoder_count < numDecoders) return GenericError;

    return Ok;
}

/*****************************************************************************
 * GdipGetImageEncodersSize [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipGetImageEncodersSize(UINT *numEncoders, UINT *size)
{
    int encoder_count=0;
    int i;
    TRACE("%p %p\n", numEncoders, size);

    if (!numEncoders || !size)
        return InvalidParameter;

    for (i=0; i<NUM_CODECS; i++)
    {
        if (codecs[i].info.Flags & ImageCodecFlagsEncoder)
            encoder_count++;
    }

    *numEncoders = encoder_count;
    *size = encoder_count * sizeof(ImageCodecInfo);

    return Ok;
}

/*****************************************************************************
 * GdipGetImageEncoders [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipGetImageEncoders(UINT numEncoders, UINT size, ImageCodecInfo *encoders)
{
    int i, encoder_count=0;
    TRACE("%u %u %p\n", numEncoders, size, encoders);

    if (!encoders ||
        size != numEncoders * sizeof(ImageCodecInfo))
        return GenericError;

    for (i=0; i<NUM_CODECS; i++)
    {
        if (codecs[i].info.Flags & ImageCodecFlagsEncoder)
        {
            if (encoder_count == numEncoders) return GenericError;
            memcpy(&encoders[encoder_count], &codecs[i].info, sizeof(ImageCodecInfo));
            encoder_count++;
        }
    }

    if (encoder_count < numEncoders) return GenericError;

    return Ok;
}

GpStatus WINGDIPAPI GdipGetEncoderParameterListSize(GpImage *image,
    GDIPCONST CLSID* clsidEncoder, UINT *size)
{
    static int calls;

    TRACE("(%p,%s,%p)\n", image, debugstr_guid(clsidEncoder), size);

    if(!(calls++))
        FIXME("not implemented\n");

    *size = 0;

    return NotImplemented;
}

static PixelFormat get_16bpp_format(HBITMAP hbm)
{
    BITMAPV4HEADER bmh;
    HDC hdc;
    PixelFormat result;

    hdc = CreateCompatibleDC(NULL);

    memset(&bmh, 0, sizeof(bmh));
    bmh.bV4Size = sizeof(bmh);
    bmh.bV4Width = 1;
    bmh.bV4Height = 1;
    bmh.bV4V4Compression = BI_BITFIELDS;
    bmh.bV4BitCount = 16;

    GetDIBits(hdc, hbm, 0, 0, NULL, (BITMAPINFO*)&bmh, DIB_RGB_COLORS);

    if (bmh.bV4RedMask == 0x7c00 &&
        bmh.bV4GreenMask == 0x3e0 &&
        bmh.bV4BlueMask == 0x1f)
    {
        result = PixelFormat16bppRGB555;
    }
    else if (bmh.bV4RedMask == 0xf800 &&
        bmh.bV4GreenMask == 0x7e0 &&
        bmh.bV4BlueMask == 0x1f)
    {
        result = PixelFormat16bppRGB565;
    }
    else
    {
        FIXME("unrecognized bitfields %x,%x,%x\n", bmh.bV4RedMask,
            bmh.bV4GreenMask, bmh.bV4BlueMask);
        result = PixelFormatUndefined;
    }

    DeleteDC(hdc);

    return result;
}

/*****************************************************************************
 * GdipCreateBitmapFromHBITMAP [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipCreateBitmapFromHBITMAP(HBITMAP hbm, HPALETTE hpal, GpBitmap** bitmap)
{
    BITMAP bm;
    GpStatus retval;
    PixelFormat format;
    BitmapData lockeddata;

    TRACE("%p %p %p\n", hbm, hpal, bitmap);

    if(!hbm || !bitmap)
        return InvalidParameter;

    if (GetObjectA(hbm, sizeof(bm), &bm) != sizeof(bm))
            return InvalidParameter;

    /* TODO: Figure out the correct format for 16, 32, 64 bpp */
    switch(bm.bmBitsPixel) {
        case 1:
            format = PixelFormat1bppIndexed;
            break;
        case 4:
            format = PixelFormat4bppIndexed;
            break;
        case 8:
            format = PixelFormat8bppIndexed;
            break;
        case 16:
            format = get_16bpp_format(hbm);
            if (format == PixelFormatUndefined)
                return InvalidParameter;
            break;
        case 24:
            format = PixelFormat24bppRGB;
            break;
        case 32:
            format = PixelFormat32bppRGB;
            break;
        case 48:
            format = PixelFormat48bppRGB;
            break;
        default:
            FIXME("don't know how to handle %d bpp\n", bm.bmBitsPixel);
            return InvalidParameter;
    }

    retval = GdipCreateBitmapFromScan0(bm.bmWidth, bm.bmHeight, 0,
        format, NULL, bitmap);

    if (retval == Ok)
    {
        retval = GdipBitmapLockBits(*bitmap, NULL, ImageLockModeWrite,
            format, &lockeddata);
        if (retval == Ok)
        {
            HDC hdc;
            BITMAPINFO *pbmi;
            INT src_height;

            hdc = CreateCompatibleDC(NULL);
            pbmi = GdipAlloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

            if (pbmi)
            {
                pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                pbmi->bmiHeader.biBitCount = 0;

                GetDIBits(hdc, hbm, 0, 0, NULL, pbmi, DIB_RGB_COLORS);

                src_height = abs(pbmi->bmiHeader.biHeight);
                pbmi->bmiHeader.biHeight = -src_height;

                GetDIBits(hdc, hbm, 0, src_height, lockeddata.Scan0, pbmi, DIB_RGB_COLORS);

                GdipFree(pbmi);
            }
            else
                retval = OutOfMemory;

            DeleteDC(hdc);

            GdipBitmapUnlockBits(*bitmap, &lockeddata);
        }

        if (retval == Ok && hpal)
        {
            PALETTEENTRY entry[256];
            ColorPalette *palette=NULL;
            int i, num_palette_entries;

            num_palette_entries = GetPaletteEntries(hpal, 0, 256, entry);
            if (!num_palette_entries)
                retval = GenericError;

            palette = GdipAlloc(sizeof(ColorPalette) + sizeof(ARGB) * (num_palette_entries-1));
            if (!palette)
                retval = OutOfMemory;

            if (retval == Ok)
            {
                palette->Flags = 0;
                palette->Count = num_palette_entries;

                for (i=0; i<num_palette_entries; i++)
                {
                    palette->Entries[i] = 0xff000000 | entry[i].peRed << 16 |
                                          entry[i].peGreen << 8 | entry[i].peBlue;
                }

                retval = GdipSetImagePalette((GpImage*)*bitmap, palette);
            }

            GdipFree(palette);
        }

        if (retval != Ok)
        {
            GdipDisposeImage((GpImage*)*bitmap);
            *bitmap = NULL;
        }
    }

    return retval;
}

GpStatus WINGDIPAPI GdipDeleteEffect(CGpEffect *effect)
{
    FIXME("(%p): stub\n", effect);
    /* note: According to Jose Roca's GDI+ Docs, this is not implemented
     * in Windows's gdiplus */
    return NotImplemented;
}

/*****************************************************************************
 * GdipSetEffectParameters [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipSetEffectParameters(CGpEffect *effect,
    const VOID *params, const UINT size)
{
    static int calls;

    TRACE("(%p,%p,%u)\n", effect, params, size);

    if(!(calls++))
        FIXME("not implemented\n");

    return NotImplemented;
}

/*****************************************************************************
 * GdipGetImageFlags [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipGetImageFlags(GpImage *image, UINT *flags)
{
    TRACE("%p %p\n", image, flags);

    if(!image || !flags)
        return InvalidParameter;

    *flags = image->flags;

    return Ok;
}

GpStatus WINGDIPAPI GdipTestControl(GpTestControlEnum control, void *param)
{
    TRACE("(%d, %p)\n", control, param);

    switch(control){
        case TestControlForceBilinear:
            if(param)
                FIXME("TestControlForceBilinear not handled\n");
            break;
        case TestControlNoICM:
            if(param)
                FIXME("TestControlNoICM not handled\n");
            break;
        case TestControlGetBuildNumber:
            *((DWORD*)param) = 3102;
            break;
    }

    return Ok;
}

GpStatus WINGDIPAPI GdipRecordMetafileFileName(GDIPCONST WCHAR* fileName,
                            HDC hdc, EmfType type, GDIPCONST GpRectF *pFrameRect,
                            MetafileFrameUnit frameUnit, GDIPCONST WCHAR *desc,
                            GpMetafile **metafile)
{
    FIXME("%s %p %d %p %d %s %p stub!\n", debugstr_w(fileName), hdc, type, pFrameRect,
                                 frameUnit, debugstr_w(desc), metafile);

    return NotImplemented;
}

GpStatus WINGDIPAPI GdipRecordMetafileFileNameI(GDIPCONST WCHAR* fileName, HDC hdc, EmfType type,
                            GDIPCONST GpRect *pFrameRect, MetafileFrameUnit frameUnit,
                            GDIPCONST WCHAR *desc, GpMetafile **metafile)
{
    FIXME("%s %p %d %p %d %s %p stub!\n", debugstr_w(fileName), hdc, type, pFrameRect,
                                 frameUnit, debugstr_w(desc), metafile);

    return NotImplemented;
}

GpStatus WINGDIPAPI GdipImageForceValidation(GpImage *image)
{
    TRACE("%p\n", image);

    return Ok;
}

/*****************************************************************************
 * GdipGetImageThumbnail [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipGetImageThumbnail(GpImage *image, UINT width, UINT height,
                            GpImage **ret_image, GetThumbnailImageAbort cb,
                            VOID * cb_data)
{
    GpStatus stat;
    GpGraphics *graphics;
    UINT srcwidth, srcheight;

    TRACE("(%p %u %u %p %p %p)\n",
        image, width, height, ret_image, cb, cb_data);

    if (!image || !ret_image)
        return InvalidParameter;

    if (!width) width = 120;
    if (!height) height = 120;

    GdipGetImageWidth(image, &srcwidth);
    GdipGetImageHeight(image, &srcheight);

    stat = GdipCreateBitmapFromScan0(width, height, 0, PixelFormat32bppPARGB,
        NULL, (GpBitmap**)ret_image);

    if (stat == Ok)
    {
        stat = GdipGetImageGraphicsContext(*ret_image, &graphics);

        if (stat == Ok)
        {
            stat = GdipDrawImageRectRectI(graphics, image,
                0, 0, width, height, 0, 0, srcwidth, srcheight, UnitPixel,
                NULL, NULL, NULL);

            GdipDeleteGraphics(graphics);
        }

        if (stat != Ok)
        {
            GdipDisposeImage(*ret_image);
            *ret_image = NULL;
        }
    }

    return stat;
}

/*****************************************************************************
 * GdipImageRotateFlip [GDIPLUS.@]
 */
GpStatus WINGDIPAPI GdipImageRotateFlip(GpImage *image, RotateFlipType type)
{
    GpBitmap *new_bitmap;
    GpBitmap *bitmap;
    int bpp, bytesperpixel;
    int rotate_90, flip_x, flip_y;
    int src_x_offset, src_y_offset;
    LPBYTE src_origin;
    UINT x, y, width, height;
    BitmapData src_lock, dst_lock;
    GpStatus stat;

    TRACE("(%p, %u)\n", image, type);

    if (!image)
        return InvalidParameter;

    rotate_90 = type&1;
    flip_x = (type&6) == 2 || (type&6) == 4;
    flip_y = (type&3) == 1 || (type&3) == 2;

    if (image->type != ImageTypeBitmap)
    {
        FIXME("Not implemented for type %i\n", image->type);
        return NotImplemented;
    }

    bitmap = (GpBitmap*)image;
    bpp = PIXELFORMATBPP(bitmap->format);

    if (bpp < 8)
    {
        FIXME("Not implemented for %i bit images\n", bpp);
        return NotImplemented;
    }

    if (rotate_90)
    {
        width = bitmap->height;
        height = bitmap->width;
    }
    else
    {
        width = bitmap->width;
        height = bitmap->height;
    }

    bytesperpixel = bpp/8;

    stat = GdipCreateBitmapFromScan0(width, height, 0, bitmap->format, NULL, &new_bitmap);

    if (stat != Ok)
        return stat;

    stat = GdipBitmapLockBits(bitmap, NULL, ImageLockModeRead, bitmap->format, &src_lock);

    if (stat == Ok)
    {
        stat = GdipBitmapLockBits(new_bitmap, NULL, ImageLockModeWrite, bitmap->format, &dst_lock);

        if (stat == Ok)
        {
            LPBYTE src_row, src_pixel;
            LPBYTE dst_row, dst_pixel;

            src_origin = src_lock.Scan0;
            if (flip_x) src_origin += bytesperpixel * (bitmap->width - 1);
            if (flip_y) src_origin += src_lock.Stride * (bitmap->height - 1);

            if (rotate_90)
            {
                if (flip_y) src_x_offset = -src_lock.Stride;
                else src_x_offset = src_lock.Stride;
                if (flip_x) src_y_offset = -bytesperpixel;
                else src_y_offset = bytesperpixel;
            }
            else
            {
                if (flip_x) src_x_offset = -bytesperpixel;
                else src_x_offset = bytesperpixel;
                if (flip_y) src_y_offset = -src_lock.Stride;
                else src_y_offset = src_lock.Stride;
            }

            src_row = src_origin;
            dst_row = dst_lock.Scan0;
            for (y=0; y<height; y++)
            {
                src_pixel = src_row;
                dst_pixel = dst_row;
                for (x=0; x<width; x++)
                {
                    /* FIXME: This could probably be faster without memcpy. */
                    memcpy(dst_pixel, src_pixel, bytesperpixel);
                    dst_pixel += bytesperpixel;
                    src_pixel += src_x_offset;
                }
                src_row += src_y_offset;
                dst_row += dst_lock.Stride;
            }

            GdipBitmapUnlockBits(new_bitmap, &dst_lock);
        }

        GdipBitmapUnlockBits(bitmap, &src_lock);
    }

    if (stat == Ok)
        move_bitmap(bitmap, new_bitmap, FALSE);
    else
        GdipDisposeImage((GpImage*)new_bitmap);

    return stat;
}

/*****************************************************************************
 * GdipConvertToEmfPlusToFile [GDIPLUS.@]
 */

GpStatus WINGDIPAPI GdipConvertToEmfPlusToFile(const GpGraphics* refGraphics,
                                               GpMetafile* metafile, BOOL* conversionSuccess,
                                               const WCHAR* filename, EmfType emfType,
                                               const WCHAR* description, GpMetafile** out_metafile)
{
    FIXME("stub: %p, %p, %p, %p, %u, %p, %p\n", refGraphics, metafile, conversionSuccess, filename, emfType, description, out_metafile);
    return NotImplemented;
}
