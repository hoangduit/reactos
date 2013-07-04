/*
 * Unit test suite for graphics objects
 *
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

#include <math.h>
#include <assert.h>

#define WIN32_NO_STATUS
#define _INC_WINDOWS
#define COM_NO_WINDOWS_H

//#include "windows.h"
#include <wine/test.h>
#include <wingdi.h>
#include <objbase.h>
#include <gdiplus.h>

#define expect(expected, got) ok((got) == (expected), "Expected %d, got %d\n", (INT)(expected), (INT)(got))
#define expectf_(expected, got, precision) ok(fabs((expected) - (got)) <= (precision), "Expected %f, got %f\n", (expected), (got))
#define expectf(expected, got) expectf_((expected), (got), 0.001)
#define TABLE_LEN (23)

static const REAL mm_per_inch = 25.4;
static const REAL point_per_inch = 72.0;
static HWND hwnd;

static void set_rect_empty(RectF *rc)
{
    rc->X = 0.0;
    rc->Y = 0.0;
    rc->Width = 0.0;
    rc->Height = 0.0;
}

/* converts a given unit to its value in pixels */
static REAL units_to_pixels(REAL units, GpUnit unit, REAL dpi)
{
    switch (unit)
    {
    case UnitPixel:
    case UnitDisplay:
        return units;
    case UnitPoint:
        return units * dpi / point_per_inch;
    case UnitInch:
        return units * dpi;
    case UnitDocument:
        return units * dpi / 300.0; /* Per MSDN */
    case UnitMillimeter:
        return units * dpi / mm_per_inch;
    default:
        assert(0);
        return 0;
    }
}

/* converts value in pixels to a given unit */
static REAL pixels_to_units(REAL pixels, GpUnit unit, REAL dpi)
{
    switch (unit)
    {
    case UnitPixel:
    case UnitDisplay:
        return pixels;
    case UnitPoint:
        return pixels * point_per_inch / dpi;
    case UnitInch:
        return pixels / dpi;
    case UnitDocument:
        return pixels * 300.0 / dpi;
    case UnitMillimeter:
        return pixels * mm_per_inch / dpi;
    default:
        assert(0);
        return 0;
    }
}

static REAL units_scale(GpUnit from, GpUnit to, REAL dpi)
{
    REAL pixels = units_to_pixels(1.0, from, dpi);
    return pixels_to_units(pixels, to, dpi);
}

static GpGraphics *create_graphics(REAL res_x, REAL res_y, GpUnit unit, REAL scale)
{
    GpStatus status;
    union
    {
        GpBitmap *bitmap;
        GpImage *image;
    } u;
    GpGraphics *graphics = NULL;
    REAL res;

    status = GdipCreateBitmapFromScan0(1, 1, 4, PixelFormat24bppRGB, NULL, &u.bitmap);
    expect(Ok, status);

    status = GdipBitmapSetResolution(u.bitmap, res_x, res_y);
    expect(Ok, status);
    status = GdipGetImageHorizontalResolution(u.image, &res);
    expect(Ok, status);
    expectf(res_x, res);
    status = GdipGetImageVerticalResolution(u.image, &res);
    expect(Ok, status);
    expectf(res_y, res);

    status = GdipGetImageGraphicsContext(u.image, &graphics);
    expect(Ok, status);
    /* image is intentionally leaked to make sure that there is no
       side effects after its destruction.
    status = GdipDisposeImage(u.image);
    expect(Ok, status);
    */

    status = GdipGetDpiX(graphics, &res);
    expect(Ok, status);
    expectf(res_x, res);
    status = GdipGetDpiY(graphics, &res);
    expect(Ok, status);
    expectf(res_y, res);

    status = GdipSetPageUnit(graphics, unit);
    expect(Ok, status);
    status = GdipSetPageScale(graphics, scale);
    expect(Ok, status);

    return graphics;
}

static void test_constructor_destructor(void)
{
    GpStatus stat;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );

    stat = GdipCreateFromHDC(NULL, &graphics);
    expect(OutOfMemory, stat);
    stat = GdipDeleteGraphics(graphics);
    expect(InvalidParameter, stat);

    stat = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, stat);
    stat = GdipDeleteGraphics(graphics);
    expect(Ok, stat);

    stat = GdipCreateFromHWND(NULL, &graphics);
    expect(Ok, stat);
    stat = GdipDeleteGraphics(graphics);
    expect(Ok, stat);

    stat = GdipCreateFromHWNDICM(NULL, &graphics);
    expect(Ok, stat);
    stat = GdipDeleteGraphics(graphics);
    expect(Ok, stat);

    stat = GdipDeleteGraphics(NULL);
    expect(InvalidParameter, stat);
    ReleaseDC(hwnd, hdc);
}

typedef struct node{
    GraphicsState data;
    struct node * next;
} node;

/* Linked list prepend function. */
static void log_state(GraphicsState data, node ** log)
{
    node * new_entry = HeapAlloc(GetProcessHeap(), 0, sizeof(node));

    new_entry->data = data;
    new_entry->next = *log;
    *log = new_entry;
}

/* Checks if there are duplicates in the list, and frees it. */
static void check_no_duplicates(node * log)
{
    INT dups = 0;
    node * temp = NULL;
    node * temp2 = NULL;
    node * orig = log;

    if(!log)
        goto end;

    do{
        temp = log;
        while((temp = temp->next)){
            if(log->data == temp->data){
                dups++;
                break;
            }
            if(dups > 0)
                break;
        }
    }while((log = log->next));

    temp = orig;
    do{
        temp2 = temp->next;
        HeapFree(GetProcessHeap(), 0, temp);
        temp = temp2;
    }while(temp);

end:
    expect(0, dups);
}

static void test_save_restore(void)
{
    GpStatus stat;
    GraphicsState state_a, state_b, state_c;
    InterpolationMode mode;
    GpGraphics *graphics1, *graphics2;
    node * state_log = NULL;
    HDC hdc = GetDC( hwnd );
    state_a = state_b = state_c = 0xdeadbeef;

    /* Invalid saving. */
    GdipCreateFromHDC(hdc, &graphics1);
    stat = GdipSaveGraphics(graphics1, NULL);
    expect(InvalidParameter, stat);
    stat = GdipSaveGraphics(NULL, &state_a);
    expect(InvalidParameter, stat);
    GdipDeleteGraphics(graphics1);

    log_state(state_a, &state_log);

    /* Basic save/restore. */
    GdipCreateFromHDC(hdc, &graphics1);
    GdipSetInterpolationMode(graphics1, InterpolationModeBilinear);
    stat = GdipSaveGraphics(graphics1, &state_a);
    expect(Ok, stat);
    GdipSetInterpolationMode(graphics1, InterpolationModeBicubic);
    stat = GdipRestoreGraphics(graphics1, state_a);
    expect(Ok, stat);
    GdipGetInterpolationMode(graphics1, &mode);
    expect(InterpolationModeBilinear, mode);
    GdipDeleteGraphics(graphics1);

    log_state(state_a, &state_log);

    /* Restoring garbage doesn't affect saves. */
    GdipCreateFromHDC(hdc, &graphics1);
    GdipSetInterpolationMode(graphics1, InterpolationModeBilinear);
    GdipSaveGraphics(graphics1, &state_a);
    GdipSetInterpolationMode(graphics1, InterpolationModeBicubic);
    GdipSaveGraphics(graphics1, &state_b);
    GdipSetInterpolationMode(graphics1, InterpolationModeNearestNeighbor);
    stat = GdipRestoreGraphics(graphics1, 0xdeadbeef);
    expect(Ok, stat);
    GdipRestoreGraphics(graphics1, state_b);
    GdipGetInterpolationMode(graphics1, &mode);
    expect(InterpolationModeBicubic, mode);
    GdipRestoreGraphics(graphics1, state_a);
    GdipGetInterpolationMode(graphics1, &mode);
    expect(InterpolationModeBilinear, mode);
    GdipDeleteGraphics(graphics1);

    log_state(state_a, &state_log);
    log_state(state_b, &state_log);

    /* Restoring older state invalidates newer saves (but not older saves). */
    GdipCreateFromHDC(hdc, &graphics1);
    GdipSetInterpolationMode(graphics1, InterpolationModeBilinear);
    GdipSaveGraphics(graphics1, &state_a);
    GdipSetInterpolationMode(graphics1, InterpolationModeBicubic);
    GdipSaveGraphics(graphics1, &state_b);
    GdipSetInterpolationMode(graphics1, InterpolationModeNearestNeighbor);
    GdipSaveGraphics(graphics1, &state_c);
    GdipSetInterpolationMode(graphics1, InterpolationModeHighQualityBilinear);
    GdipRestoreGraphics(graphics1, state_b);
    GdipGetInterpolationMode(graphics1, &mode);
    expect(InterpolationModeBicubic, mode);
    GdipRestoreGraphics(graphics1, state_c);
    GdipGetInterpolationMode(graphics1, &mode);
    expect(InterpolationModeBicubic, mode);
    GdipRestoreGraphics(graphics1, state_a);
    GdipGetInterpolationMode(graphics1, &mode);
    expect(InterpolationModeBilinear, mode);
    GdipDeleteGraphics(graphics1);

    log_state(state_a, &state_log);
    log_state(state_b, &state_log);
    log_state(state_c, &state_log);

    /* Restoring older save from one graphics object does not invalidate
     * newer save from other graphics object. */
    GdipCreateFromHDC(hdc, &graphics1);
    GdipCreateFromHDC(hdc, &graphics2);
    GdipSetInterpolationMode(graphics1, InterpolationModeBilinear);
    GdipSaveGraphics(graphics1, &state_a);
    GdipSetInterpolationMode(graphics2, InterpolationModeBicubic);
    GdipSaveGraphics(graphics2, &state_b);
    GdipSetInterpolationMode(graphics1, InterpolationModeNearestNeighbor);
    GdipSetInterpolationMode(graphics2, InterpolationModeNearestNeighbor);
    GdipRestoreGraphics(graphics1, state_a);
    GdipGetInterpolationMode(graphics1, &mode);
    expect(InterpolationModeBilinear, mode);
    GdipRestoreGraphics(graphics2, state_b);
    GdipGetInterpolationMode(graphics2, &mode);
    expect(InterpolationModeBicubic, mode);
    GdipDeleteGraphics(graphics1);
    GdipDeleteGraphics(graphics2);

    /* You can't restore a state to a graphics object that didn't save it. */
    GdipCreateFromHDC(hdc, &graphics1);
    GdipCreateFromHDC(hdc, &graphics2);
    GdipSetInterpolationMode(graphics1, InterpolationModeBilinear);
    GdipSaveGraphics(graphics1, &state_a);
    GdipSetInterpolationMode(graphics1, InterpolationModeNearestNeighbor);
    GdipSetInterpolationMode(graphics2, InterpolationModeNearestNeighbor);
    GdipRestoreGraphics(graphics2, state_a);
    GdipGetInterpolationMode(graphics2, &mode);
    expect(InterpolationModeNearestNeighbor, mode);
    GdipDeleteGraphics(graphics1);
    GdipDeleteGraphics(graphics2);

    log_state(state_a, &state_log);

    /* The same state value should never be returned twice. */
    todo_wine
        check_no_duplicates(state_log);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipFillClosedCurve2(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpSolidFill *brush = NULL;
    HDC hdc = GetDC( hwnd );
    GpPointF points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and brush object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    GdipCreateSolidFill((ARGB)0xdeadbeef, &brush);

    /* InvalidParameter cases: null graphics, null brush, null points */
    status = GdipFillClosedCurve2(NULL, NULL, NULL, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2(graphics, NULL, NULL, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2(NULL, (GpBrush*)brush, NULL, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2(NULL, NULL, points, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2(graphics, (GpBrush*)brush, NULL, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2(graphics, NULL, points, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2(NULL, (GpBrush*)brush, points, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipFillClosedCurve2(graphics, (GpBrush*)brush, points, -1, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2(graphics, (GpBrush*)brush, points, 0, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipFillClosedCurve2(graphics, (GpBrush*)brush, points, 1, 0.5, FillModeAlternate);
    expect(Ok, status);

    status = GdipFillClosedCurve2(graphics, (GpBrush*)brush, points, 2, 0.5, FillModeAlternate);
    expect(Ok, status);

    status = GdipFillClosedCurve2(graphics, (GpBrush*)brush, points, 3, 0.5, FillModeAlternate);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);
    GdipDeleteBrush((GpBrush*)brush);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipFillClosedCurve2I(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpSolidFill *brush = NULL;
    HDC hdc = GetDC( hwnd );
    GpPoint points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and brush object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    GdipCreateSolidFill((ARGB)0xdeadbeef, &brush);

    /* InvalidParameter cases: null graphics, null brush */
    /* Note: GdipFillClosedCurveI and GdipFillClosedCurve2I hang in Windows
             when points == NULL, so don't test this condition */
    status = GdipFillClosedCurve2I(NULL, NULL, points, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2I(graphics, NULL, points, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve2I(NULL, (GpBrush*)brush, points, 3, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipFillClosedCurve2I(graphics, (GpBrush*)brush, points, 0, 0.5, FillModeAlternate);
    expect(InvalidParameter, status);

    /* OutOfMemory cases: large (unsigned) int */
    status = GdipFillClosedCurve2I(graphics, (GpBrush*)brush, points, -1, 0.5, FillModeAlternate);
    expect(OutOfMemory, status);

    /* Valid test cases */
    status = GdipFillClosedCurve2I(graphics, (GpBrush*)brush, points, 1, 0.5, FillModeAlternate);
    expect(Ok, status);

    status = GdipFillClosedCurve2I(graphics, (GpBrush*)brush, points, 2, 0.5, FillModeAlternate);
    expect(Ok, status);

    status = GdipFillClosedCurve2I(graphics, (GpBrush*)brush, points, 3, 0.5, FillModeAlternate);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);
    GdipDeleteBrush((GpBrush*)brush);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawArc(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen, non-positive width, non-positive height */
    status = GdipDrawArc(NULL, NULL, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    expect(InvalidParameter, status);

    status = GdipDrawArc(graphics, NULL, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0);
    expect(InvalidParameter, status);

    status = GdipDrawArc(NULL, pen, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0);
    expect(InvalidParameter, status);

    status = GdipDrawArc(graphics, pen, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    expect(InvalidParameter, status);

    status = GdipDrawArc(graphics, pen, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0);
    expect(InvalidParameter, status);

    /* successful case */
    status = GdipDrawArc(graphics, pen, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawArcI(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen, non-positive width, non-positive height */
    status = GdipDrawArcI(NULL, NULL, 0, 0, 0, 0, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawArcI(graphics, NULL, 0, 0, 1, 1, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawArcI(NULL, pen, 0, 0, 1, 1, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawArcI(graphics, pen, 0, 0, 1, 0, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawArcI(graphics, pen, 0, 0, 0, 1, 0, 0);
    expect(InvalidParameter, status);

    /* successful case */
    status = GdipDrawArcI(graphics, pen, 0, 0, 1, 1, 0, 0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_BeginContainer2(void)
{
    GpMatrix *transform;
    GpRectF clip;
    REAL defClip[] = {5, 10, 15, 20};
    REAL elems[6], defTrans[] = {1, 2, 3, 4, 5, 6};
    GraphicsContainer cont1, cont2, cont3, cont4;
    CompositingQuality compqual, defCompqual = CompositingQualityHighSpeed;
    CompositingMode compmode, defCompmode = CompositingModeSourceOver;
    InterpolationMode interp, defInterp = InterpolationModeHighQualityBicubic;
    REAL scale, defScale = 17;
    GpUnit unit, defUnit = UnitPixel;
    PixelOffsetMode offsetmode, defOffsetmode = PixelOffsetModeHighSpeed;
    SmoothingMode smoothmode, defSmoothmode = SmoothingModeAntiAlias;
    UINT contrast, defContrast = 5;
    TextRenderingHint texthint, defTexthint = TextRenderingHintAntiAlias;

    GpStatus status;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );

    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    /* null graphics, null container */
    status = GdipBeginContainer2(NULL, &cont1);
    expect(InvalidParameter, status);

    status = GdipBeginContainer2(graphics, NULL);
    expect(InvalidParameter, status);

    status = GdipEndContainer(NULL, cont1);
    expect(InvalidParameter, status);

    /* test all quality-related values */
    GdipSetCompositingMode(graphics, defCompmode);
    GdipSetCompositingQuality(graphics, defCompqual);
    GdipSetInterpolationMode(graphics, defInterp);
    GdipSetPageScale(graphics, defScale);
    GdipSetPageUnit(graphics, defUnit);
    GdipSetPixelOffsetMode(graphics, defOffsetmode);
    GdipSetSmoothingMode(graphics, defSmoothmode);
    GdipSetTextContrast(graphics, defContrast);
    GdipSetTextRenderingHint(graphics, defTexthint);

    status = GdipBeginContainer2(graphics, &cont1);
    expect(Ok, status);

    GdipSetCompositingMode(graphics, CompositingModeSourceCopy);
    GdipSetCompositingQuality(graphics, CompositingQualityHighQuality);
    GdipSetInterpolationMode(graphics, InterpolationModeBilinear);
    GdipSetPageScale(graphics, 10);
    GdipSetPageUnit(graphics, UnitDocument);
    GdipSetPixelOffsetMode(graphics, PixelOffsetModeHalf);
    GdipSetSmoothingMode(graphics, SmoothingModeNone);
    GdipSetTextContrast(graphics, 7);
    GdipSetTextRenderingHint(graphics, TextRenderingHintClearTypeGridFit);

    status = GdipEndContainer(graphics, cont1);
    expect(Ok, status);

    GdipGetCompositingMode(graphics, &compmode);
    ok(defCompmode == compmode, "Expected Compositing Mode to be restored to %d, got %d\n", defCompmode, compmode);

    GdipGetCompositingQuality(graphics, &compqual);
    ok(defCompqual == compqual, "Expected Compositing Quality to be restored to %d, got %d\n", defCompqual, compqual);

    GdipGetInterpolationMode(graphics, &interp);
    ok(defInterp == interp, "Expected Interpolation Mode to be restored to %d, got %d\n", defInterp, interp);

    GdipGetPageScale(graphics, &scale);
    ok(fabs(defScale - scale) < 0.0001, "Expected Page Scale to be restored to %f, got %f\n", defScale, scale);

    GdipGetPageUnit(graphics, &unit);
    ok(defUnit == unit, "Expected Page Unit to be restored to %d, got %d\n", defUnit, unit);

    GdipGetPixelOffsetMode(graphics, &offsetmode);
    ok(defOffsetmode == offsetmode, "Expected Pixel Offset Mode to be restored to %d, got %d\n", defOffsetmode, offsetmode);

    GdipGetSmoothingMode(graphics, &smoothmode);
    ok(defSmoothmode == smoothmode, "Expected Smoothing Mode to be restored to %d, got %d\n", defSmoothmode, smoothmode);

    GdipGetTextContrast(graphics, &contrast);
    ok(defContrast == contrast, "Expected Text Contrast to be restored to %d, got %d\n", defContrast, contrast);

    GdipGetTextRenderingHint(graphics, &texthint);
    ok(defTexthint == texthint, "Expected Text Hint to be restored to %d, got %d\n", defTexthint, texthint);

    /* test world transform */
    status = GdipBeginContainer2(graphics, &cont1);
    expect(Ok, status);

    status = GdipCreateMatrix2(defTrans[0], defTrans[1], defTrans[2], defTrans[3],
            defTrans[4], defTrans[5], &transform);
    expect(Ok, status);
    GdipSetWorldTransform(graphics, transform);
    GdipDeleteMatrix(transform);
    transform = NULL;

    status = GdipBeginContainer2(graphics, &cont2);
    expect(Ok, status);

    status = GdipCreateMatrix2(10, 20, 30, 40, 50, 60, &transform);
    expect(Ok, status);
    GdipSetWorldTransform(graphics, transform);
    GdipDeleteMatrix(transform);
    transform = NULL;

    status = GdipEndContainer(graphics, cont2);
    expect(Ok, status);

    status = GdipCreateMatrix(&transform);
    expect(Ok, status);
    GdipGetWorldTransform(graphics, transform);
    GdipGetMatrixElements(transform, elems);
    ok(fabs(defTrans[0] - elems[0]) < 0.0001 &&
            fabs(defTrans[1] - elems[1]) < 0.0001 &&
            fabs(defTrans[2] - elems[2]) < 0.0001 &&
            fabs(defTrans[3] - elems[3]) < 0.0001 &&
            fabs(defTrans[4] - elems[4]) < 0.0001 &&
            fabs(defTrans[5] - elems[5]) < 0.0001,
            "Expected World Transform Matrix to be restored to [%f, %f, %f, %f, %f, %f], got [%f, %f, %f, %f, %f, %f]\n",
            defTrans[0], defTrans[1], defTrans[2], defTrans[3], defTrans[4], defTrans[5],
            elems[0], elems[1], elems[2], elems[3], elems[4], elems[5]);
    GdipDeleteMatrix(transform);
    transform = NULL;

    status = GdipEndContainer(graphics, cont1);
    expect(Ok, status);

    /* test clipping */
    status = GdipBeginContainer2(graphics, &cont1);
    expect(Ok, status);

    GdipSetClipRect(graphics, defClip[0], defClip[1], defClip[2], defClip[3], CombineModeReplace);

    status = GdipBeginContainer2(graphics, &cont2);
    expect(Ok, status);

    GdipSetClipRect(graphics, 2, 4, 6, 8, CombineModeReplace);

    status = GdipEndContainer(graphics, cont2);
    expect(Ok, status);

    GdipGetClipBounds(graphics, &clip);
    ok(fabs(defClip[0] - clip.X) < 0.0001 &&
            fabs(defClip[1] - clip.Y) < 0.0001 &&
            fabs(defClip[2] - clip.Width) < 0.0001 &&
            fabs(defClip[3] - clip.Height) < 0.0001,
            "Expected Clipping Rectangle to be restored to [%f, %f, %f, %f], got [%f, %f, %f, %f]\n",
            defClip[0], defClip[1], defClip[2], defClip[3],
            clip.X, clip.Y, clip.Width, clip.Height);

    status = GdipEndContainer(graphics, cont1);
    expect(Ok, status);

    /* nesting */
    status = GdipBeginContainer2(graphics, &cont1);
    expect(Ok, status);

    status = GdipBeginContainer2(graphics, &cont2);
    expect(Ok, status);

    status = GdipBeginContainer2(graphics, &cont3);
    expect(Ok, status);

    status = GdipEndContainer(graphics, cont3);
    expect(Ok, status);

    status = GdipBeginContainer2(graphics, &cont4);
    expect(Ok, status);

    status = GdipEndContainer(graphics, cont4);
    expect(Ok, status);

    /* skip cont2 */
    status = GdipEndContainer(graphics, cont1);
    expect(Ok, status);

    /* end an already-ended container */
    status = GdipEndContainer(graphics, cont1);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawBezierI(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawBezierI(NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawBezierI(graphics, NULL, 0, 0, 0, 0, 0, 0, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawBezierI(NULL, pen, 0, 0, 0, 0, 0, 0, 0, 0);
    expect(InvalidParameter, status);

    /* successful case */
    status = GdipDrawBezierI(graphics, pen, 0, 0, 0, 0, 0, 0, 0, 0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawCurve3(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );
    GpPointF points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawCurve3(NULL, NULL, points, 3, 0, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3(graphics, NULL, points, 3, 0, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3(NULL, pen, points, 3, 0, 2, 1);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipDrawCurve3(graphics, pen, points, -1, 0, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3(graphics, pen, points, 0, 0, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3(graphics, pen, points, 1, 0, 0, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3(graphics, pen, points, 3, 4, 2, 1);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid number of segments */
    status = GdipDrawCurve3(graphics, pen, points, 3, 0, -1, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3(graphics, pen, points, 3, 1, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3(graphics, pen, points, 2, 0, 2, 1);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipDrawCurve3(graphics, pen, points, 2, 0, 1, 1);
    expect(Ok, status);

    status = GdipDrawCurve3(graphics, pen, points, 3, 0, 2, 2);
    expect(Ok, status);

    status = GdipDrawCurve3(graphics, pen, points, 2, 0, 1, -2);
    expect(Ok, status);

    status = GdipDrawCurve3(graphics, pen, points, 3, 1, 1, 0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawCurve3I(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );
    GpPoint points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawCurve3I(NULL, NULL, points, 3, 0, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3I(graphics, NULL, points, 3, 0, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3I(NULL, pen, points, 3, 0, 2, 1);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipDrawCurve3I(graphics, pen, points, -1, -1, -1, 1);
    expect(OutOfMemory, status);

    status = GdipDrawCurve3I(graphics, pen, points, 0, 0, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3I(graphics, pen, points, 1, 0, 0, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3I(graphics, pen, points, 3, 4, 2, 1);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid number of segments */
    status = GdipDrawCurve3I(graphics, pen, points, 3, 0, -1, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3I(graphics, pen, points, 3, 1, 2, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve3I(graphics, pen, points, 2, 0, 2, 1);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipDrawCurve3I(graphics, pen, points, 2, 0, 1, 1);
    expect(Ok, status);

    status = GdipDrawCurve3I(graphics, pen, points, 3, 0, 2, 2);
    expect(Ok, status);

    status = GdipDrawCurve3I(graphics, pen, points, 2, 0, 1, -2);
    expect(Ok, status);

    status = GdipDrawCurve3I(graphics, pen, points, 3, 1, 1, 0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawCurve2(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );
    GpPointF points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawCurve2(NULL, NULL, points, 3, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve2(graphics, NULL, points, 3, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve2(NULL, pen, points, 3, 1);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipDrawCurve2(graphics, pen, points, -1, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve2(graphics, pen, points, 0, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve2(graphics, pen, points, 1, 1);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipDrawCurve2(graphics, pen, points, 2, 1);
    expect(Ok, status);

    status = GdipDrawCurve2(graphics, pen, points, 3, 2);
    expect(Ok, status);

    status = GdipDrawCurve2(graphics, pen, points, 3, -2);
    expect(Ok, status);

    status = GdipDrawCurve2(graphics, pen, points, 3, 0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawCurve2I(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );
    GpPoint points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawCurve2I(NULL, NULL, points, 3, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve2I(graphics, NULL, points, 3, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve2I(NULL, pen, points, 3, 1);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipDrawCurve2I(graphics, pen, points, -1, 1);
    expect(OutOfMemory, status);

    status = GdipDrawCurve2I(graphics, pen, points, 0, 1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve2I(graphics, pen, points, 1, 1);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipDrawCurve2I(graphics, pen, points, 2, 1);
    expect(Ok, status);

    status = GdipDrawCurve2I(graphics, pen, points, 3, 2);
    expect(Ok, status);

    status = GdipDrawCurve2I(graphics, pen, points, 3, -2);
    expect(Ok, status);

    status = GdipDrawCurve2I(graphics, pen, points, 3, 0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawCurve(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );
    GpPointF points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawCurve(NULL, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipDrawCurve(graphics, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipDrawCurve(NULL, pen, points, 3);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipDrawCurve(graphics, pen, points, -1);
    expect(InvalidParameter, status);

    status = GdipDrawCurve(graphics, pen, points, 0);
    expect(InvalidParameter, status);

    status = GdipDrawCurve(graphics, pen, points, 1);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipDrawCurve(graphics, pen, points, 2);
    expect(Ok, status);

    status = GdipDrawCurve(graphics, pen, points, 3);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawCurveI(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );
    GpPoint points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawCurveI(NULL, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipDrawCurveI(graphics, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipDrawCurveI(NULL, pen, points, 3);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipDrawCurveI(graphics, pen, points, -1);
    expect(OutOfMemory, status);

    status = GdipDrawCurveI(graphics, pen, points, 0);
    expect(InvalidParameter, status);

    status = GdipDrawCurveI(graphics, pen, points, 1);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipDrawCurveI(graphics, pen, points, 2);
    expect(Ok, status);

    status = GdipDrawCurveI(graphics, pen, points, 3);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawLineI(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    HDC hdc = GetDC( hwnd );

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* InvalidParameter cases: null graphics, null pen */
    status = GdipDrawLineI(NULL, NULL, 0, 0, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawLineI(graphics, NULL, 0, 0, 0, 0);
    expect(InvalidParameter, status);

    status = GdipDrawLineI(NULL, pen, 0, 0, 0, 0);
    expect(InvalidParameter, status);

    /* successful case */
    status = GdipDrawLineI(graphics, pen, 0, 0, 0, 0);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawImagePointsRect(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPointF ptf[4];
    GpBitmap *bm = NULL;
    BYTE rbmi[sizeof(BITMAPINFOHEADER)];
    BYTE buff[400];
    BITMAPINFO *bmi = (BITMAPINFO*)rbmi;
    HDC hdc = GetDC( hwnd );
    if (!hdc)
        return;

    memset(rbmi, 0, sizeof(rbmi));
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = 10;
    bmi->bmiHeader.biHeight = 10;
    bmi->bmiHeader.biPlanes = 1;
    bmi->bmiHeader.biBitCount = 32;
    bmi->bmiHeader.biCompression = BI_RGB;
    status = GdipCreateBitmapFromGdiDib(bmi, buff, &bm);
    expect(Ok, status);
    ok(NULL != bm, "Expected bitmap to be initialized\n");
    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ptf[0].X = 0;
    ptf[0].Y = 0;
    ptf[1].X = 10;
    ptf[1].Y = 0;
    ptf[2].X = 0;
    ptf[2].Y = 10;
    ptf[3].X = 10;
    ptf[3].Y = 10;
    status = GdipDrawImagePointsRect(graphics, (GpImage*)bm, ptf, 4, 0, 0, 10, 10, UnitPixel, NULL, NULL, NULL);
    expect(NotImplemented, status);
    status = GdipDrawImagePointsRect(graphics, (GpImage*)bm, ptf, 2, 0, 0, 10, 10, UnitPixel, NULL, NULL, NULL);
    expect(InvalidParameter, status);
    status = GdipDrawImagePointsRect(graphics, (GpImage*)bm, ptf, 3, 0, 0, 10, 10, UnitPixel, NULL, NULL, NULL);
    expect(Ok, status);
    status = GdipDrawImagePointsRect(graphics, NULL, ptf, 3, 0, 0, 10, 10, UnitPixel, NULL, NULL, NULL);
    expect(InvalidParameter, status);
    status = GdipDrawImagePointsRect(graphics, (GpImage*)bm, NULL, 3, 0, 0, 10, 10, UnitPixel, NULL, NULL, NULL);
    expect(InvalidParameter, status);
    status = GdipDrawImagePointsRect(graphics, (GpImage*)bm, ptf, 3, 0, 0, 0, 0, UnitPixel, NULL, NULL, NULL);
    expect(Ok, status);
    memset(ptf, 0, sizeof(ptf));
    status = GdipDrawImagePointsRect(graphics, (GpImage*)bm, ptf, 3, 0, 0, 10, 10, UnitPixel, NULL, NULL, NULL);
    expect(Ok, status);

    GdipDisposeImage((GpImage*)bm);
    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawLinesI(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen = NULL;
    GpPoint *ptf = NULL;
    HDC hdc = GetDC( hwnd );

    /* make a graphics object and pen object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);
    ok(pen != NULL, "Expected pen to be initialized\n");

    /* make some arbitrary valid points*/
    ptf = GdipAlloc(2 * sizeof(GpPointF));

    ptf[0].X = 1;
    ptf[0].Y = 1;

    ptf[1].X = 2;
    ptf[1].Y = 2;

    /* InvalidParameter cases: null graphics, null pen, null points, count < 2*/
    status = GdipDrawLinesI(NULL, NULL, NULL, 0);
    expect(InvalidParameter, status);

    status = GdipDrawLinesI(graphics, pen, ptf, 0);
    expect(InvalidParameter, status);

    status = GdipDrawLinesI(graphics, NULL, ptf, 2);
    expect(InvalidParameter, status);

    status = GdipDrawLinesI(NULL, pen, ptf, 2);
    expect(InvalidParameter, status);

    /* successful case */
    status = GdipDrawLinesI(graphics, pen, ptf, 2);
    expect(Ok, status);

    GdipFree(ptf);
    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipFillClosedCurve(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpSolidFill *brush = NULL;
    HDC hdc = GetDC( hwnd );
    GpPointF points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and brush object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    GdipCreateSolidFill((ARGB)0xdeadbeef, &brush);

    /* InvalidParameter cases: null graphics, null brush, null points */
    status = GdipFillClosedCurve(NULL, NULL, NULL, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve(graphics, NULL, NULL, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve(NULL, (GpBrush*)brush, NULL, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve(NULL, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve(graphics, (GpBrush*)brush, NULL, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve(graphics, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve(NULL, (GpBrush*)brush, points, 3);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipFillClosedCurve(graphics, (GpBrush*)brush, points, -1);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurve(graphics, (GpBrush*)brush, points, 0);
    expect(InvalidParameter, status);

    /* Valid test cases */
    status = GdipFillClosedCurve(graphics, (GpBrush*)brush, points, 1);
    expect(Ok, status);

    status = GdipFillClosedCurve(graphics, (GpBrush*)brush, points, 2);
    expect(Ok, status);

    status = GdipFillClosedCurve(graphics, (GpBrush*)brush, points, 3);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);
    GdipDeleteBrush((GpBrush*)brush);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipFillClosedCurveI(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpSolidFill *brush = NULL;
    HDC hdc = GetDC( hwnd );
    GpPoint points[3];

    points[0].X = 0;
    points[0].Y = 0;

    points[1].X = 40;
    points[1].Y = 20;

    points[2].X = 10;
    points[2].Y = 40;

    /* make a graphics object and brush object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    GdipCreateSolidFill((ARGB)0xdeadbeef, &brush);

    /* InvalidParameter cases: null graphics, null brush */
    /* Note: GdipFillClosedCurveI and GdipFillClosedCurve2I hang in Windows
             when points == NULL, so don't test this condition */
    status = GdipFillClosedCurveI(NULL, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurveI(graphics, NULL, points, 3);
    expect(InvalidParameter, status);

    status = GdipFillClosedCurveI(NULL, (GpBrush*)brush, points, 3);
    expect(InvalidParameter, status);

    /* InvalidParameter cases: invalid count */
    status = GdipFillClosedCurveI(graphics, (GpBrush*)brush, points, 0);
    expect(InvalidParameter, status);

    /* OutOfMemory cases: large (unsigned) int */
    status = GdipFillClosedCurveI(graphics, (GpBrush*)brush, points, -1);
    expect(OutOfMemory, status);

    /* Valid test cases */
    status = GdipFillClosedCurveI(graphics, (GpBrush*)brush, points, 1);
    expect(Ok, status);

    status = GdipFillClosedCurveI(graphics, (GpBrush*)brush, points, 2);
    expect(Ok, status);

    status = GdipFillClosedCurveI(graphics, (GpBrush*)brush, points, 3);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);
    GdipDeleteBrush((GpBrush*)brush);

    ReleaseDC(hwnd, hdc);
}

static void test_Get_Release_DC(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpPen *pen;
    GpSolidFill *brush;
    GpPath *path;
    HDC hdc = GetDC( hwnd );
    HDC retdc;
    REAL r;
    CompositingQuality quality;
    CompositingMode compmode;
    InterpolationMode intmode;
    GpMatrix *m;
    GpRegion *region;
    GpUnit unit;
    PixelOffsetMode offsetmode;
    SmoothingMode smoothmode;
    TextRenderingHint texthint;
    GpPointF ptf[5];
    GpPoint  pt[5];
    GpRectF  rectf[2];
    GpRect   rect[2];
    GpRegion *clip;
    INT i;
    BOOL res;
    ARGB color = 0x00000000;
    HRGN hrgn = CreateRectRgn(0, 0, 10, 10);

    pt[0].X = 10;
    pt[0].Y = 10;
    pt[1].X = 20;
    pt[1].Y = 15;
    pt[2].X = 40;
    pt[2].Y = 80;
    pt[3].X = -20;
    pt[3].Y = 20;
    pt[4].X = 50;
    pt[4].Y = 110;

    for(i = 0; i < 5;i++){
        ptf[i].X = (REAL)pt[i].X;
        ptf[i].Y = (REAL)pt[i].Y;
    }

    rect[0].X = 0;
    rect[0].Y = 0;
    rect[0].Width  = 50;
    rect[0].Height = 70;
    rect[1].X = 0;
    rect[1].Y = 0;
    rect[1].Width  = 10;
    rect[1].Height = 20;

    for(i = 0; i < 2;i++){
        rectf[i].X = (REAL)rect[i].X;
        rectf[i].Y = (REAL)rect[i].Y;
        rectf[i].Height = (REAL)rect[i].Height;
        rectf[i].Width  = (REAL)rect[i].Width;
    }

    status = GdipCreateMatrix(&m);
    expect(Ok, status);
    GdipCreateRegion(&region);
    GdipCreateSolidFill((ARGB)0xdeadbeef, &brush);
    GdipCreatePath(FillModeAlternate, &path);
    GdipCreateRegion(&clip);

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");
    status = GdipCreatePen1((ARGB)0xffff00ff, 10.0f, UnitPixel, &pen);
    expect(Ok, status);

    /* NULL arguments */
    status = GdipGetDC(NULL, NULL);
    expect(InvalidParameter, status);
    status = GdipGetDC(graphics, NULL);
    expect(InvalidParameter, status);
    status = GdipGetDC(NULL, &retdc);
    expect(InvalidParameter, status);

    status = GdipReleaseDC(NULL, NULL);
    expect(InvalidParameter, status);
    status = GdipReleaseDC(graphics, NULL);
    expect(InvalidParameter, status);
    status = GdipReleaseDC(NULL, (HDC)0xdeadbeef);
    expect(InvalidParameter, status);

    /* Release without Get */
    status = GdipReleaseDC(graphics, hdc);
    expect(InvalidParameter, status);

    retdc = NULL;
    status = GdipGetDC(graphics, &retdc);
    expect(Ok, status);
    ok(retdc == hdc, "Invalid HDC returned\n");
    /* call it once more */
    status = GdipGetDC(graphics, &retdc);
    expect(ObjectBusy, status);

    /* try all Graphics calls here */
    status = GdipDrawArc(graphics, pen, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0);
    expect(ObjectBusy, status);
    status = GdipDrawArcI(graphics, pen, 0, 0, 1, 1, 0.0, 0.0);
    expect(ObjectBusy, status);
    status = GdipDrawBezier(graphics, pen, 0.0, 10.0, 20.0, 15.0, 35.0, -10.0, 10.0, 10.0);
    expect(ObjectBusy, status);
    status = GdipDrawBezierI(graphics, pen, 0, 0, 0, 0, 0, 0, 0, 0);
    expect(ObjectBusy, status);
    status = GdipDrawBeziers(graphics, pen, ptf, 5);
    expect(ObjectBusy, status);
    status = GdipDrawBeziersI(graphics, pen, pt, 5);
    expect(ObjectBusy, status);
    status = GdipDrawClosedCurve(graphics, pen, ptf, 5);
    expect(ObjectBusy, status);
    status = GdipDrawClosedCurveI(graphics, pen, pt, 5);
    expect(ObjectBusy, status);
    status = GdipDrawClosedCurve2(graphics, pen, ptf, 5, 1.0);
    expect(ObjectBusy, status);
    status = GdipDrawClosedCurve2I(graphics, pen, pt, 5, 1.0);
    expect(ObjectBusy, status);
    status = GdipDrawCurve(graphics, pen, ptf, 5);
    expect(ObjectBusy, status);
    status = GdipDrawCurveI(graphics, pen, pt, 5);
    expect(ObjectBusy, status);
    status = GdipDrawCurve2(graphics, pen, ptf, 5, 1.0);
    expect(ObjectBusy, status);
    status = GdipDrawCurve2I(graphics, pen, pt, 5, 1.0);
    expect(ObjectBusy, status);
    status = GdipDrawEllipse(graphics, pen, 0.0, 0.0, 100.0, 50.0);
    expect(ObjectBusy, status);
    status = GdipDrawEllipseI(graphics, pen, 0, 0, 100, 50);
    expect(ObjectBusy, status);
    /* GdipDrawImage/GdipDrawImageI */
    /* GdipDrawImagePointsRect/GdipDrawImagePointsRectI */
    /* GdipDrawImageRectRect/GdipDrawImageRectRectI */
    /* GdipDrawImageRect/GdipDrawImageRectI */
    status = GdipDrawLine(graphics, pen, 0.0, 0.0, 100.0, 200.0);
    expect(ObjectBusy, status);
    status = GdipDrawLineI(graphics, pen, 0, 0, 100, 200);
    expect(ObjectBusy, status);
    status = GdipDrawLines(graphics, pen, ptf, 5);
    expect(ObjectBusy, status);
    status = GdipDrawLinesI(graphics, pen, pt, 5);
    expect(ObjectBusy, status);
    status = GdipDrawPath(graphics, pen, path);
    expect(ObjectBusy, status);
    status = GdipDrawPie(graphics, pen, 0.0, 0.0, 100.0, 100.0, 0.0, 90.0);
    expect(ObjectBusy, status);
    status = GdipDrawPieI(graphics, pen, 0, 0, 100, 100, 0.0, 90.0);
    expect(ObjectBusy, status);
    status = GdipDrawRectangle(graphics, pen, 0.0, 0.0, 100.0, 300.0);
    expect(ObjectBusy, status);
    status = GdipDrawRectangleI(graphics, pen, 0, 0, 100, 300);
    expect(ObjectBusy, status);
    status = GdipDrawRectangles(graphics, pen, rectf, 2);
    expect(ObjectBusy, status);
    status = GdipDrawRectanglesI(graphics, pen, rect, 2);
    expect(ObjectBusy, status);
    /* GdipDrawString */
    status = GdipFillClosedCurve2(graphics, (GpBrush*)brush, ptf, 5, 1.0, FillModeAlternate);
    expect(ObjectBusy, status);
    status = GdipFillClosedCurve2I(graphics, (GpBrush*)brush, pt, 5, 1.0, FillModeAlternate);
    expect(ObjectBusy, status);
    status = GdipFillClosedCurve(graphics, (GpBrush*)brush, ptf, 5);
    expect(ObjectBusy, status);
    status = GdipFillClosedCurveI(graphics, (GpBrush*)brush, pt, 5);
    expect(ObjectBusy, status);
    status = GdipFillEllipse(graphics, (GpBrush*)brush, 0.0, 0.0, 100.0, 100.0);
    expect(ObjectBusy, status);
    status = GdipFillEllipseI(graphics, (GpBrush*)brush, 0, 0, 100, 100);
    expect(ObjectBusy, status);
    status = GdipFillPath(graphics, (GpBrush*)brush, path);
    expect(ObjectBusy, status);
    status = GdipFillPie(graphics, (GpBrush*)brush, 0.0, 0.0, 100.0, 100.0, 0.0, 15.0);
    expect(ObjectBusy, status);
    status = GdipFillPieI(graphics, (GpBrush*)brush, 0, 0, 100, 100, 0.0, 15.0);
    expect(ObjectBusy, status);
    status = GdipFillPolygon(graphics, (GpBrush*)brush, ptf, 5, FillModeAlternate);
    expect(ObjectBusy, status);
    status = GdipFillPolygonI(graphics, (GpBrush*)brush, pt, 5, FillModeAlternate);
    expect(ObjectBusy, status);
    status = GdipFillPolygon2(graphics, (GpBrush*)brush, ptf, 5);
    expect(ObjectBusy, status);
    status = GdipFillPolygon2I(graphics, (GpBrush*)brush, pt, 5);
    expect(ObjectBusy, status);
    status = GdipFillRectangle(graphics, (GpBrush*)brush, 0.0, 0.0, 100.0, 100.0);
    expect(ObjectBusy, status);
    status = GdipFillRectangleI(graphics, (GpBrush*)brush, 0, 0, 100, 100);
    expect(ObjectBusy, status);
    status = GdipFillRectangles(graphics, (GpBrush*)brush, rectf, 2);
    expect(ObjectBusy, status);
    status = GdipFillRectanglesI(graphics, (GpBrush*)brush, rect, 2);
    expect(ObjectBusy, status);
    status = GdipFillRegion(graphics, (GpBrush*)brush, region);
    expect(ObjectBusy, status);
    status = GdipFlush(graphics, FlushIntentionFlush);
    expect(ObjectBusy, status);
    status = GdipGetClipBounds(graphics, rectf);
    expect(ObjectBusy, status);
    status = GdipGetClipBoundsI(graphics, rect);
    expect(ObjectBusy, status);
    status = GdipGetCompositingMode(graphics, &compmode);
    expect(ObjectBusy, status);
    status = GdipGetCompositingQuality(graphics, &quality);
    expect(ObjectBusy, status);
    status = GdipGetInterpolationMode(graphics, &intmode);
    expect(ObjectBusy, status);
    status = GdipGetNearestColor(graphics, &color);
    expect(ObjectBusy, status);
    status = GdipGetPageScale(graphics, &r);
    expect(ObjectBusy, status);
    status = GdipGetPageUnit(graphics, &unit);
    expect(ObjectBusy, status);
    status = GdipGetPixelOffsetMode(graphics, &offsetmode);
    expect(ObjectBusy, status);
    status = GdipGetSmoothingMode(graphics, &smoothmode);
    expect(ObjectBusy, status);
    status = GdipGetTextRenderingHint(graphics, &texthint);
    expect(ObjectBusy, status);
    status = GdipGetWorldTransform(graphics, m);
    expect(ObjectBusy, status);
    status = GdipGraphicsClear(graphics, 0xdeadbeef);
    expect(ObjectBusy, status);
    status = GdipIsVisiblePoint(graphics, 0.0, 0.0, &res);
    expect(ObjectBusy, status);
    status = GdipIsVisiblePointI(graphics, 0, 0, &res);
    expect(ObjectBusy, status);
    /* GdipMeasureCharacterRanges */
    /* GdipMeasureString */
    status = GdipResetClip(graphics);
    expect(ObjectBusy, status);
    status = GdipResetWorldTransform(graphics);
    expect(ObjectBusy, status);
    /* GdipRestoreGraphics */
    status = GdipRotateWorldTransform(graphics, 15.0, MatrixOrderPrepend);
    expect(ObjectBusy, status);
    /*  GdipSaveGraphics */
    status = GdipScaleWorldTransform(graphics, 1.0, 1.0, MatrixOrderPrepend);
    expect(ObjectBusy, status);
    status = GdipSetCompositingMode(graphics, CompositingModeSourceOver);
    expect(ObjectBusy, status);
    status = GdipSetCompositingQuality(graphics, CompositingQualityDefault);
    expect(ObjectBusy, status);
    status = GdipSetInterpolationMode(graphics, InterpolationModeDefault);
    expect(ObjectBusy, status);
    status = GdipSetPageScale(graphics, 1.0);
    expect(ObjectBusy, status);
    status = GdipSetPageUnit(graphics, UnitWorld);
    expect(ObjectBusy, status);
    status = GdipSetPixelOffsetMode(graphics, PixelOffsetModeDefault);
    expect(ObjectBusy, status);
    status = GdipSetSmoothingMode(graphics, SmoothingModeDefault);
    expect(ObjectBusy, status);
    status = GdipSetTextRenderingHint(graphics, TextRenderingHintSystemDefault);
    expect(ObjectBusy, status);
    status = GdipSetWorldTransform(graphics, m);
    expect(ObjectBusy, status);
    status = GdipTranslateWorldTransform(graphics, 0.0, 0.0, MatrixOrderPrepend);
    expect(ObjectBusy, status);
    status = GdipSetClipHrgn(graphics, hrgn, CombineModeReplace);
    expect(ObjectBusy, status);
    status = GdipSetClipPath(graphics, path, CombineModeReplace);
    expect(ObjectBusy, status);
    status = GdipSetClipRect(graphics, 0.0, 0.0, 10.0, 10.0, CombineModeReplace);
    expect(ObjectBusy, status);
    status = GdipSetClipRectI(graphics, 0, 0, 10, 10, CombineModeReplace);
    expect(ObjectBusy, status);
    status = GdipSetClipRegion(graphics, clip, CombineModeReplace);
    expect(ObjectBusy, status);
    status = GdipTranslateClip(graphics, 0.0, 0.0);
    expect(ObjectBusy, status);
    status = GdipTranslateClipI(graphics, 0, 0);
    expect(ObjectBusy, status);
    status = GdipDrawPolygon(graphics, pen, ptf, 5);
    expect(ObjectBusy, status);
    status = GdipDrawPolygonI(graphics, pen, pt, 5);
    expect(ObjectBusy, status);
    status = GdipGetDpiX(graphics, &r);
    expect(ObjectBusy, status);
    status = GdipGetDpiY(graphics, &r);
    expect(ObjectBusy, status);
    status = GdipMultiplyWorldTransform(graphics, m, MatrixOrderPrepend);
    expect(ObjectBusy, status);
    status = GdipGetClip(graphics, region);
    expect(ObjectBusy, status);
    status = GdipTransformPoints(graphics, CoordinateSpacePage, CoordinateSpaceWorld, ptf, 5);
    expect(ObjectBusy, status);

    /* try to delete before release */
    status = GdipDeleteGraphics(graphics);
    expect(ObjectBusy, status);

    status = GdipReleaseDC(graphics, retdc);
    expect(Ok, status);

    GdipDeletePen(pen);
    GdipDeleteGraphics(graphics);

    GdipDeleteRegion(clip);
    GdipDeletePath(path);
    GdipDeleteBrush((GpBrush*)brush);
    GdipDeleteRegion(region);
    GdipDeleteMatrix(m);
    DeleteObject(hrgn);

    ReleaseDC(hwnd, hdc);
}

static void test_transformpoints(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );
    GpPointF ptf[2];
    GpPoint pt[2];

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);

    /* NULL arguments */
    status = GdipTransformPoints(NULL, CoordinateSpacePage, CoordinateSpaceWorld, NULL, 0);
    expect(InvalidParameter, status);
    status = GdipTransformPoints(graphics, CoordinateSpacePage, CoordinateSpaceWorld, NULL, 0);
    expect(InvalidParameter, status);
    status = GdipTransformPoints(graphics, CoordinateSpacePage, CoordinateSpaceWorld, ptf, 0);
    expect(InvalidParameter, status);
    status = GdipTransformPoints(graphics, CoordinateSpacePage, CoordinateSpaceWorld, ptf, -1);
    expect(InvalidParameter, status);

    ptf[0].X = 1.0;
    ptf[0].Y = 0.0;
    ptf[1].X = 0.0;
    ptf[1].Y = 1.0;
    status = GdipTransformPoints(graphics, CoordinateSpaceDevice, CoordinateSpaceWorld, ptf, 2);
    expect(Ok, status);
    expectf(1.0, ptf[0].X);
    expectf(0.0, ptf[0].Y);
    expectf(0.0, ptf[1].X);
    expectf(1.0, ptf[1].Y);

    status = GdipTranslateWorldTransform(graphics, 5.0, 5.0, MatrixOrderAppend);
    expect(Ok, status);
    status = GdipSetPageUnit(graphics, UnitPixel);
    expect(Ok, status);
    status = GdipSetPageScale(graphics, 3.0);
    expect(Ok, status);

    ptf[0].X = 1.0;
    ptf[0].Y = 0.0;
    ptf[1].X = 0.0;
    ptf[1].Y = 1.0;
    status = GdipTransformPoints(graphics, CoordinateSpaceDevice, CoordinateSpaceWorld, ptf, 2);
    expect(Ok, status);
    expectf(18.0, ptf[0].X);
    expectf(15.0, ptf[0].Y);
    expectf(15.0, ptf[1].X);
    expectf(18.0, ptf[1].Y);

    ptf[0].X = 1.0;
    ptf[0].Y = 0.0;
    ptf[1].X = 0.0;
    ptf[1].Y = 1.0;
    status = GdipTransformPoints(graphics, CoordinateSpacePage, CoordinateSpaceWorld, ptf, 2);
    expect(Ok, status);
    expectf(6.0, ptf[0].X);
    expectf(5.0, ptf[0].Y);
    expectf(5.0, ptf[1].X);
    expectf(6.0, ptf[1].Y);

    ptf[0].X = 1.0;
    ptf[0].Y = 0.0;
    ptf[1].X = 0.0;
    ptf[1].Y = 1.0;
    status = GdipTransformPoints(graphics, CoordinateSpaceDevice, CoordinateSpacePage, ptf, 2);
    expect(Ok, status);
    expectf(3.0, ptf[0].X);
    expectf(0.0, ptf[0].Y);
    expectf(0.0, ptf[1].X);
    expectf(3.0, ptf[1].Y);

    ptf[0].X = 18.0;
    ptf[0].Y = 15.0;
    ptf[1].X = 15.0;
    ptf[1].Y = 18.0;
    status = GdipTransformPoints(graphics, CoordinateSpaceWorld, CoordinateSpaceDevice, ptf, 2);
    expect(Ok, status);
    expectf(1.0, ptf[0].X);
    expectf(0.0, ptf[0].Y);
    expectf(0.0, ptf[1].X);
    expectf(1.0, ptf[1].Y);

    ptf[0].X = 6.0;
    ptf[0].Y = 5.0;
    ptf[1].X = 5.0;
    ptf[1].Y = 6.0;
    status = GdipTransformPoints(graphics, CoordinateSpaceWorld, CoordinateSpacePage, ptf, 2);
    expect(Ok, status);
    expectf(1.0, ptf[0].X);
    expectf(0.0, ptf[0].Y);
    expectf(0.0, ptf[1].X);
    expectf(1.0, ptf[1].Y);

    ptf[0].X = 3.0;
    ptf[0].Y = 0.0;
    ptf[1].X = 0.0;
    ptf[1].Y = 3.0;
    status = GdipTransformPoints(graphics, CoordinateSpacePage, CoordinateSpaceDevice, ptf, 2);
    expect(Ok, status);
    expectf(1.0, ptf[0].X);
    expectf(0.0, ptf[0].Y);
    expectf(0.0, ptf[1].X);
    expectf(1.0, ptf[1].Y);

    pt[0].X = 1;
    pt[0].Y = 0;
    pt[1].X = 0;
    pt[1].Y = 1;
    status = GdipTransformPointsI(graphics, CoordinateSpaceDevice, CoordinateSpaceWorld, pt, 2);
    expect(Ok, status);
    expect(18, pt[0].X);
    expect(15, pt[0].Y);
    expect(15, pt[1].X);
    expect(18, pt[1].Y);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_get_set_clip(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );
    GpRegion *clip;
    GpRectF rect;
    BOOL res;

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);

    rect.X = rect.Y = 0.0;
    rect.Height = rect.Width = 100.0;

    status = GdipCreateRegionRect(&rect, &clip);
    expect(Ok, status);

    /* NULL arguments */
    status = GdipGetClip(NULL, NULL);
    expect(InvalidParameter, status);
    status = GdipGetClip(graphics, NULL);
    expect(InvalidParameter, status);
    status = GdipGetClip(NULL, clip);
    expect(InvalidParameter, status);

    status = GdipSetClipRegion(NULL, NULL, CombineModeReplace);
    expect(InvalidParameter, status);
    status = GdipSetClipRegion(graphics, NULL, CombineModeReplace);
    expect(InvalidParameter, status);

    status = GdipSetClipPath(NULL, NULL, CombineModeReplace);
    expect(InvalidParameter, status);
    status = GdipSetClipPath(graphics, NULL, CombineModeReplace);
    expect(InvalidParameter, status);

    res = FALSE;
    status = GdipGetClip(graphics, clip);
    expect(Ok, status);
    status = GdipIsInfiniteRegion(clip, graphics, &res);
    expect(Ok, status);
    expect(TRUE, res);

    /* remains infinite after reset */
    res = FALSE;
    status = GdipResetClip(graphics);
    expect(Ok, status);
    status = GdipGetClip(graphics, clip);
    expect(Ok, status);
    status = GdipIsInfiniteRegion(clip, graphics, &res);
    expect(Ok, status);
    expect(TRUE, res);

    /* set to empty and then reset to infinite */
    status = GdipSetEmpty(clip);
    expect(Ok, status);
    status = GdipSetClipRegion(graphics, clip, CombineModeReplace);
    expect(Ok, status);

    status = GdipGetClip(graphics, clip);
    expect(Ok, status);
    res = FALSE;
    status = GdipIsEmptyRegion(clip, graphics, &res);
    expect(Ok, status);
    expect(TRUE, res);
    status = GdipResetClip(graphics);
    expect(Ok, status);
    status = GdipGetClip(graphics, clip);
    expect(Ok, status);
    res = FALSE;
    status = GdipIsInfiniteRegion(clip, graphics, &res);
    expect(Ok, status);
    expect(TRUE, res);

    GdipDeleteRegion(clip);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_isempty(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );
    GpRegion *clip;
    BOOL res;

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);

    status = GdipCreateRegion(&clip);
    expect(Ok, status);

    /* NULL */
    status = GdipIsClipEmpty(NULL, NULL);
    expect(InvalidParameter, status);
    status = GdipIsClipEmpty(graphics, NULL);
    expect(InvalidParameter, status);
    status = GdipIsClipEmpty(NULL, &res);
    expect(InvalidParameter, status);

    /* default is infinite */
    res = TRUE;
    status = GdipIsClipEmpty(graphics, &res);
    expect(Ok, status);
    expect(FALSE, res);

    GdipDeleteRegion(clip);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_clear(void)
{
    GpStatus status;

    status = GdipGraphicsClear(NULL, 0xdeadbeef);
    expect(InvalidParameter, status);
}

static void test_textcontrast(void)
{
    GpStatus status;
    HDC hdc = GetDC( hwnd );
    GpGraphics *graphics;
    UINT contrast;

    status = GdipGetTextContrast(NULL, NULL);
    expect(InvalidParameter, status);

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);

    status = GdipGetTextContrast(graphics, NULL);
    expect(InvalidParameter, status);
    status = GdipGetTextContrast(graphics, &contrast);
    expect(Ok, status);
    expect(4, contrast);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_GdipDrawString(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpFont *fnt = NULL;
    RectF  rect;
    GpStringFormat *format;
    GpBrush *brush;
    LOGFONTA logfont;
    HDC hdc = GetDC( hwnd );
    static const WCHAR string[] = {'T','e','s','t',0};
    static const PointF positions[4] = {{0,0}, {1,1}, {2,2}, {3,3}};
    GpMatrix *matrix;

    memset(&logfont,0,sizeof(logfont));
    strcpy(logfont.lfFaceName,"Arial");
    logfont.lfHeight = 12;
    logfont.lfCharSet = DEFAULT_CHARSET;

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);

    status = GdipCreateFontFromLogfontA(hdc, &logfont, &fnt);
    if (status == NotTrueTypeFont || status == FileNotFound)
    {
        skip("Arial not installed.\n");
        return;
    }
    expect(Ok, status);

    status = GdipCreateSolidFill((ARGB)0xdeadbeef, (GpSolidFill**)&brush);
    expect(Ok, status);

    status = GdipCreateStringFormat(0,0,&format);
    expect(Ok, status);

    rect.X = 0;
    rect.Y = 0;
    rect.Width = 0;
    rect.Height = 12;

    status = GdipDrawString(graphics, string, 4, fnt, &rect, format, brush);
    expect(Ok, status);

    status = GdipCreateMatrix(&matrix);
    expect(Ok, status);

    status = GdipDrawDriverString(NULL, string, 4, fnt, brush, positions, DriverStringOptionsCmapLookup, matrix);
    expect(InvalidParameter, status);

    status = GdipDrawDriverString(graphics, NULL, 4, fnt, brush, positions, DriverStringOptionsCmapLookup, matrix);
    expect(InvalidParameter, status);

    status = GdipDrawDriverString(graphics, string, 4, NULL, brush, positions, DriverStringOptionsCmapLookup, matrix);
    expect(InvalidParameter, status);

    status = GdipDrawDriverString(graphics, string, 4, fnt, NULL, positions, DriverStringOptionsCmapLookup, matrix);
    expect(InvalidParameter, status);

    status = GdipDrawDriverString(graphics, string, 4, fnt, brush, NULL, DriverStringOptionsCmapLookup, matrix);
    expect(InvalidParameter, status);

    status = GdipDrawDriverString(graphics, string, 4, fnt, brush, positions, DriverStringOptionsCmapLookup|0x10, matrix);
    expect(Ok, status);

    status = GdipDrawDriverString(graphics, string, 4, fnt, brush, positions, DriverStringOptionsCmapLookup, NULL);
    expect(Ok, status);

    status = GdipDrawDriverString(graphics, string, 4, fnt, brush, positions, DriverStringOptionsCmapLookup, matrix);
    expect(Ok, status);

    GdipDeleteMatrix(matrix);
    GdipDeleteGraphics(graphics);
    GdipDeleteBrush(brush);
    GdipDeleteFont(fnt);
    GdipDeleteStringFormat(format);

    ReleaseDC(hwnd, hdc);
}

static void test_GdipGetVisibleClipBounds_screen(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC(0);
    GpRectF rectf, exp, clipr;
    GpRect recti;

    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    /* no clipping rect */
    exp.X = 0;
    exp.Y = 0;
    exp.Width = GetDeviceCaps(hdc, HORZRES);
    exp.Height = GetDeviceCaps(hdc, VERTRES);

    status = GdipGetVisibleClipBounds(graphics, &rectf);
    expect(Ok, status);
    ok(rectf.X == exp.X &&
        rectf.Y == exp.Y &&
        rectf.Width == exp.Width &&
        rectf.Height == exp.Height,
        "Expected clip bounds (%0.f, %0.f, %0.f, %0.f) to be the size of "
        "the screen (%0.f, %0.f, %0.f, %0.f)\n",
        rectf.X, rectf.Y, rectf.Width, rectf.Height,
        exp.X, exp.Y, exp.Width, exp.Height);

    /* clipping rect entirely within window */
    exp.X = clipr.X = 10;
    exp.Y = clipr.Y = 12;
    exp.Width = clipr.Width = 14;
    exp.Height = clipr.Height = 16;

    status = GdipSetClipRect(graphics, clipr.X, clipr.Y, clipr.Width, clipr.Height, CombineModeReplace);
    expect(Ok, status);

    status = GdipGetVisibleClipBounds(graphics, &rectf);
    expect(Ok, status);
    ok(rectf.X == exp.X &&
        rectf.Y == exp.Y &&
        rectf.Width == exp.Width &&
        rectf.Height == exp.Height,
        "Expected clip bounds (%0.f, %0.f, %0.f, %0.f) to be the size of "
        "the clipping rect (%0.f, %0.f, %0.f, %0.f)\n",
        rectf.X, rectf.Y, rectf.Width, rectf.Height,
        exp.X, exp.Y, exp.Width, exp.Height);

    /* clipping rect partially outside of screen */
    clipr.X = -10;
    clipr.Y = -12;
    clipr.Width = 20;
    clipr.Height = 24;

    status = GdipSetClipRect(graphics, clipr.X, clipr.Y, clipr.Width, clipr.Height, CombineModeReplace);
    expect(Ok, status);

    exp.X = 0;
    exp.Y = 0;
    exp.Width = 10;
    exp.Height = 12;

    status = GdipGetVisibleClipBounds(graphics, &rectf);
    expect(Ok, status);
    ok(rectf.X == exp.X &&
        rectf.Y == exp.Y &&
        rectf.Width == exp.Width &&
        rectf.Height == exp.Height,
        "Expected clip bounds (%0.f, %0.f, %0.f, %0.f) to be the size of "
        "the visible clipping rect (%0.f, %0.f, %0.f, %0.f)\n",
        rectf.X, rectf.Y, rectf.Width, rectf.Height,
        exp.X, exp.Y, exp.Width, exp.Height);

    status = GdipGetVisibleClipBoundsI(graphics, &recti);
    expect(Ok, status);
    ok(recti.X == exp.X &&
        recti.Y == exp.Y &&
        recti.Width == exp.Width &&
        recti.Height == exp.Height,
        "Expected clip bounds (%d, %d, %d, %d) to be the size of "
        "the visible clipping rect (%0.f, %0.f, %0.f, %0.f)\n",
        recti.X, recti.Y, recti.Width, recti.Height,
        exp.X, exp.Y, exp.Width, exp.Height);

    GdipDeleteGraphics(graphics);
    ReleaseDC(0, hdc);
}

static void test_GdipGetVisibleClipBounds_window(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpRectF rectf, window, exp, clipr;
    GpRect recti;
    HDC hdc;
    PAINTSTRUCT ps;
    RECT wnd_rect;

    /* get client area size */
    ok(GetClientRect(hwnd, &wnd_rect), "GetClientRect should have succeeded\n");
    window.X = wnd_rect.left;
    window.Y = wnd_rect.top;
    window.Width = wnd_rect.right - wnd_rect.left;
    window.Height = wnd_rect.bottom - wnd_rect.top;

    hdc = BeginPaint(hwnd, &ps);

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipGetVisibleClipBounds(graphics, &rectf);
    expect(Ok, status);
    ok(rectf.X == window.X &&
        rectf.Y == window.Y &&
        rectf.Width == window.Width &&
        rectf.Height == window.Height,
        "Expected clip bounds (%0.f, %0.f, %0.f, %0.f) to be the size of "
        "the window (%0.f, %0.f, %0.f, %0.f)\n",
        rectf.X, rectf.Y, rectf.Width, rectf.Height,
        window.X, window.Y, window.Width, window.Height);

    /* clipping rect entirely within window */
    exp.X = clipr.X = 20;
    exp.Y = clipr.Y = 8;
    exp.Width = clipr.Width = 30;
    exp.Height = clipr.Height = 20;

    status = GdipSetClipRect(graphics, clipr.X, clipr.Y, clipr.Width, clipr.Height, CombineModeReplace);
    expect(Ok, status);

    status = GdipGetVisibleClipBounds(graphics, &rectf);
    expect(Ok, status);
    ok(rectf.X == exp.X &&
        rectf.Y == exp.Y &&
        rectf.Width == exp.Width &&
        rectf.Height == exp.Height,
        "Expected clip bounds (%0.f, %0.f, %0.f, %0.f) to be the size of "
        "the clipping rect (%0.f, %0.f, %0.f, %0.f)\n",
        rectf.X, rectf.Y, rectf.Width, rectf.Height,
        exp.X, exp.Y, exp.Width, exp.Height);

    /* clipping rect partially outside of window */
    clipr.X = window.Width - 10;
    clipr.Y = window.Height - 15;
    clipr.Width = 20;
    clipr.Height = 30;

    status = GdipSetClipRect(graphics, clipr.X, clipr.Y, clipr.Width, clipr.Height, CombineModeReplace);
    expect(Ok, status);

    exp.X = window.Width - 10;
    exp.Y = window.Height - 15;
    exp.Width = 10;
    exp.Height = 15;

    status = GdipGetVisibleClipBounds(graphics, &rectf);
    expect(Ok, status);
    ok(rectf.X == exp.X &&
        rectf.Y == exp.Y &&
        rectf.Width == exp.Width &&
        rectf.Height == exp.Height,
        "Expected clip bounds (%0.f, %0.f, %0.f, %0.f) to be the size of "
        "the visible clipping rect (%0.f, %0.f, %0.f, %0.f)\n",
        rectf.X, rectf.Y, rectf.Width, rectf.Height,
        exp.X, exp.Y, exp.Width, exp.Height);

    status = GdipGetVisibleClipBoundsI(graphics, &recti);
    expect(Ok, status);
    ok(recti.X == exp.X &&
        recti.Y == exp.Y &&
        recti.Width == exp.Width &&
        recti.Height == exp.Height,
        "Expected clip bounds (%d, %d, %d, %d) to be the size of "
        "the visible clipping rect (%0.f, %0.f, %0.f, %0.f)\n",
        recti.X, recti.Y, recti.Width, recti.Height,
        exp.X, exp.Y, exp.Width, exp.Height);

    GdipDeleteGraphics(graphics);
    EndPaint(hwnd, &ps);
}

static void test_GdipGetVisibleClipBounds(void)
{
    GpGraphics* graphics = NULL;
    GpRectF rectf;
    GpRect rect;
    HDC hdc = GetDC( hwnd );
    GpStatus status;

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    /* test null parameters */
    status = GdipGetVisibleClipBounds(graphics, NULL);
    expect(InvalidParameter, status);

    status = GdipGetVisibleClipBounds(NULL, &rectf);
    expect(InvalidParameter, status);

    status = GdipGetVisibleClipBoundsI(graphics, NULL);
    expect(InvalidParameter, status);

    status = GdipGetVisibleClipBoundsI(NULL, &rect);
    expect(InvalidParameter, status);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);

    test_GdipGetVisibleClipBounds_screen();
    test_GdipGetVisibleClipBounds_window();
}

static void test_fromMemoryBitmap(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpBitmap *bitmap = NULL;
    BYTE bits[48] = {0};
    HDC hdc=NULL;
    COLORREF color;

    status = GdipCreateBitmapFromScan0(4, 4, 12, PixelFormat24bppRGB, bits, &bitmap);
    expect(Ok, status);

    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);

    status = GdipGraphicsClear(graphics, 0xff686868);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);

    /* drawing writes to the memory provided */
    expect(0x68, bits[10]);

    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);

    status = GdipGetDC(graphics, &hdc);
    expect(Ok, status);
    ok(hdc != NULL, "got NULL hdc\n");

    color = GetPixel(hdc, 0, 0);
    /* The HDC is write-only, and native fills with a solid color to figure out
     * which pixels have changed. */
    todo_wine expect(0x0c0b0d, color);

    SetPixel(hdc, 0, 0, 0x797979);
    SetPixel(hdc, 1, 0, 0x0c0b0d);

    status = GdipReleaseDC(graphics, hdc);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);

    expect(0x79, bits[0]);
    todo_wine expect(0x68, bits[3]);

    GdipDisposeImage((GpImage*)bitmap);

    /* We get the same kind of write-only HDC for a "normal" bitmap */
    status = GdipCreateBitmapFromScan0(4, 4, 12, PixelFormat24bppRGB, NULL, &bitmap);
    expect(Ok, status);

    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);

    status = GdipGetDC(graphics, &hdc);
    expect(Ok, status);
    ok(hdc != NULL, "got NULL hdc\n");

    color = GetPixel(hdc, 0, 0);
    todo_wine expect(0x0c0b0d, color);

    status = GdipReleaseDC(graphics, hdc);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);

    GdipDisposeImage((GpImage*)bitmap);
}

static void test_GdipIsVisiblePoint(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );
    REAL x, y;
    BOOL val;

    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    /* null parameters */
    status = GdipIsVisiblePoint(NULL, 0, 0, &val);
    expect(InvalidParameter, status);

    status = GdipIsVisiblePoint(graphics, 0, 0, NULL);
    expect(InvalidParameter, status);

    status = GdipIsVisiblePointI(NULL, 0, 0, &val);
    expect(InvalidParameter, status);

    status = GdipIsVisiblePointI(graphics, 0, 0, NULL);
    expect(InvalidParameter, status);

    x = 0;
    y = 0;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f) to be visible\n", x, y);

    x = -10;
    y = 0;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 0;
    y = -5;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 1;
    y = 1;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f) to be visible\n", x, y);

    status = GdipSetClipRect(graphics, 10, 20, 30, 40, CombineModeReplace);
    expect(Ok, status);

    x = 1;
    y = 1;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 15.5;
    y = 40.5;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "After clipping, expected (%.2f, %.2f) to be visible\n", x, y);

    /* translate into the center of the rect */
    GdipTranslateWorldTransform(graphics, 25, 40, MatrixOrderAppend);

    x = 0;
    y = 0;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f) to be visible\n", x, y);

    x = 25;
    y = 40;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f) not to be visible\n", x, y);

    GdipTranslateWorldTransform(graphics, -25, -40, MatrixOrderAppend);

    /* corner cases */
    x = 9;
    y = 19;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 9.25;
    y = 19.25;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 9.5;
    y = 19.5;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "After clipping, expected (%.2f, %.2f) to be visible\n", x, y);

    x = 9.75;
    y = 19.75;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "After clipping, expected (%.2f, %.2f) to be visible\n", x, y);

    x = 10;
    y = 20;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "After clipping, expected (%.2f, %.2f) to be visible\n", x, y);

    x = 40;
    y = 20;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 39;
    y = 59;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "After clipping, expected (%.2f, %.2f) to be visible\n", x, y);

    x = 39.25;
    y = 59.25;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == TRUE, "After clipping, expected (%.2f, %.2f) to be visible\n", x, y);

    x = 39.5;
    y = 39.5;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 39.75;
    y = 59.75;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 40;
    y = 60;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 40.15;
    y = 60.15;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    x = 10;
    y = 60;
    status = GdipIsVisiblePoint(graphics, x, y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    /* integer version */
    x = 25;
    y = 30;
    status = GdipIsVisiblePointI(graphics, (INT)x, (INT)y, &val);
    expect(Ok, status);
    ok(val == TRUE, "After clipping, expected (%.2f, %.2f) to be visible\n", x, y);

    x = 50;
    y = 100;
    status = GdipIsVisiblePointI(graphics, (INT)x, (INT)y, &val);
    expect(Ok, status);
    ok(val == FALSE, "After clipping, expected (%.2f, %.2f) not to be visible\n", x, y);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_GdipIsVisibleRect(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );
    REAL x, y, width, height;
    BOOL val;

    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipIsVisibleRect(NULL, 0, 0, 0, 0, &val);
    expect(InvalidParameter, status);

    status = GdipIsVisibleRect(graphics, 0, 0, 0, 0, NULL);
    expect(InvalidParameter, status);

    status = GdipIsVisibleRectI(NULL, 0, 0, 0, 0, &val);
    expect(InvalidParameter, status);

    status = GdipIsVisibleRectI(graphics, 0, 0, 0, 0, NULL);
    expect(InvalidParameter, status);

    /* entirely within the visible region */
    x = 0; width = 10;
    y = 0; height = 10;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    /* partially outside */
    x = -10; width = 20;
    y = -10; height = 20;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    /* entirely outside */
    x = -10; width = 5;
    y = -10; height = 5;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f, %.2f, %.2f) not to be visible\n", x, y, width, height);

    status = GdipSetClipRect(graphics, 10, 20, 30, 40, CombineModeReplace);
    expect(Ok, status);

    /* entirely within the visible region */
    x = 12; width = 10;
    y = 22; height = 10;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    /* partially outside */
    x = 35; width = 10;
    y = 55; height = 10;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    /* entirely outside */
    x = 45; width = 5;
    y = 65; height = 5;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f, %.2f, %.2f) not to be visible\n", x, y, width, height);

    /* translate into center of clipping rect */
    GdipTranslateWorldTransform(graphics, 25, 40, MatrixOrderAppend);

    x = 0; width = 10;
    y = 0; height = 10;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    x = 25; width = 5;
    y = 40; height = 5;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f, %.2f, %.2f) not to be visible\n", x, y, width, height);

    GdipTranslateWorldTransform(graphics, -25, -40, MatrixOrderAppend);

    /* corners entirely outside, but some intersections */
    x = 0; width = 70;
    y = 0; height = 90;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    x = 0; width = 70;
    y = 0; height = 30;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    x = 0; width = 30;
    y = 0; height = 90;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    /* edge cases */
    x = 0; width = 10;
    y = 20; height = 40;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f, %.2f, %.2f) not to be visible\n", x, y, width, height);

    x = 10; width = 30;
    y = 0; height = 20;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f, %.2f, %.2f) not to be visible\n", x, y, width, height);

    x = 40; width = 10;
    y = 20; height = 40;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f, %.2f, %.2f) not to be visible\n", x, y, width, height);

    x = 10; width = 30;
    y = 60; height = 10;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == FALSE, "Expected (%.2f, %.2f, %.2f, %.2f) not to be visible\n", x, y, width, height);

    /* rounding tests */
    x = 0.4; width = 10.4;
    y = 20; height = 40;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    x = 10; width = 30;
    y = 0.4; height = 20.4;
    status = GdipIsVisibleRect(graphics, x, y, width, height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    /* integer version */
    x = 0; width = 30;
    y = 0; height = 90;
    status = GdipIsVisibleRectI(graphics, (INT)x, (INT)y, (INT)width, (INT)height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    x = 12; width = 10;
    y = 22; height = 10;
    status = GdipIsVisibleRectI(graphics, (INT)x, (INT)y, (INT)width, (INT)height, &val);
    expect(Ok, status);
    ok(val == TRUE, "Expected (%.2f, %.2f, %.2f, %.2f) to be visible\n", x, y, width, height);

    GdipDeleteGraphics(graphics);
    ReleaseDC(hwnd, hdc);
}

static void test_GdipGetNearestColor(void)
{
    GpStatus status;
    GpGraphics *graphics;
    GpBitmap *bitmap;
    ARGB color = 0xdeadbeef;
    HDC hdc = GetDC( hwnd );

    /* create a graphics object */
    ok(hdc != NULL, "Expected HDC to be initialized\n");

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipGetNearestColor(graphics, NULL);
    expect(InvalidParameter, status);

    status = GdipGetNearestColor(NULL, &color);
    expect(InvalidParameter, status);
    GdipDeleteGraphics(graphics);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat1bppIndexed, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    ok(broken(status == OutOfMemory) /* winver < Win7 */ || status == Ok, "status=%u\n", status);
    if (status == Ok)
    {
        status = GdipGetNearestColor(graphics, &color);
        expect(Ok, status);
        expect(0xdeadbeef, color);
        GdipDeleteGraphics(graphics);
    }
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat4bppIndexed, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    ok(broken(status == OutOfMemory) /* winver < Win7 */ || status == Ok, "status=%u\n", status);
    if (status == Ok)
    {
        status = GdipGetNearestColor(graphics, &color);
        expect(Ok, status);
        expect(0xdeadbeef, color);
        GdipDeleteGraphics(graphics);
    }
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat8bppIndexed, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    ok(broken(status == OutOfMemory) /* winver < Win7 */ || status == Ok, "status=%u\n", status);
    if (status == Ok)
    {
        status = GdipGetNearestColor(graphics, &color);
        expect(Ok, status);
        expect(0xdeadbeef, color);
        GdipDeleteGraphics(graphics);
    }
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat16bppGrayScale, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    todo_wine expect(OutOfMemory, status);
    if (status == Ok)
        GdipDeleteGraphics(graphics);
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat24bppRGB, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);
    status = GdipGetNearestColor(graphics, &color);
    expect(Ok, status);
    expect(0xdeadbeef, color);
    GdipDeleteGraphics(graphics);
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat32bppRGB, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);
    status = GdipGetNearestColor(graphics, &color);
    expect(Ok, status);
    expect(0xdeadbeef, color);
    GdipDeleteGraphics(graphics);
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat32bppARGB, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);
    status = GdipGetNearestColor(graphics, &color);
    expect(Ok, status);
    expect(0xdeadbeef, color);
    GdipDeleteGraphics(graphics);
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat48bppRGB, NULL, &bitmap);
    expect(Ok, status);
    if (status == Ok)
    {
        status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
        expect(Ok, status);
        status = GdipGetNearestColor(graphics, &color);
        expect(Ok, status);
        expect(0xdeadbeef, color);
        GdipDeleteGraphics(graphics);
        GdipDisposeImage((GpImage*)bitmap);
    }

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat64bppARGB, NULL, &bitmap);
    expect(Ok, status);
    if (status == Ok)
    {
        status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
        expect(Ok, status);
        status = GdipGetNearestColor(graphics, &color);
        expect(Ok, status);
        expect(0xdeadbeef, color);
        GdipDeleteGraphics(graphics);
        GdipDisposeImage((GpImage*)bitmap);
    }

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat64bppPARGB, NULL, &bitmap);
    expect(Ok, status);
    if (status == Ok)
    {
        status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
        expect(Ok, status);
        status = GdipGetNearestColor(graphics, &color);
        expect(Ok, status);
        expect(0xdeadbeef, color);
        GdipDeleteGraphics(graphics);
        GdipDisposeImage((GpImage*)bitmap);
    }

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat16bppRGB565, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);
    status = GdipGetNearestColor(graphics, &color);
    expect(Ok, status);
    todo_wine expect(0xffa8bce8, color);
    GdipDeleteGraphics(graphics);
    GdipDisposeImage((GpImage*)bitmap);

    status = GdipCreateBitmapFromScan0(10, 10, 10, PixelFormat16bppRGB555, NULL, &bitmap);
    expect(Ok, status);
    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);
    status = GdipGetNearestColor(graphics, &color);
    expect(Ok, status);
    todo_wine
    ok(color == 0xffa8b8e8 ||
       broken(color == 0xffa0b8e0), /* Win98/WinMe */
       "Expected ffa8b8e8, got %.8x\n", color);
    GdipDeleteGraphics(graphics);
    GdipDisposeImage((GpImage*)bitmap);

    ReleaseDC(hwnd, hdc);
}

#if CORE_6659_IS_FIXED
static void test_string_functions(void)
{
    GpStatus status;
    GpGraphics *graphics;
    GpFontFamily *family;
    GpFont *font;
    RectF rc, char_bounds, bounds;
    GpBrush *brush;
    ARGB color = 0xff000000;
    HDC hdc = GetDC( hwnd );
    const WCHAR fontname[] = {'T','a','h','o','m','a',0};
    const WCHAR teststring[] = {'M','M',' ','M','\n','M',0};
    const WCHAR teststring2[] = {'j',0};
    REAL char_width, char_height;
    INT codepointsfitted, linesfilled;
    GpStringFormat *format;
    CharacterRange ranges[3] = {{0, 1}, {1, 3}, {5, 1}};
    GpRegion *regions[4];
    BOOL region_isempty[4];
    int i;
    PointF positions[8];
    GpMatrix *identity;

    ok(hdc != NULL, "Expected HDC to be initialized\n");
    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipCreateFontFamilyFromName(fontname, NULL, &family);
    expect(Ok, status);

    status = GdipCreateFont(family, 10.0, FontStyleRegular, UnitPixel, &font);
    expect(Ok, status);

    status = GdipCreateSolidFill(color, (GpSolidFill**)&brush);
    expect(Ok, status);

    status = GdipCreateStringFormat(0, LANG_NEUTRAL, &format);
    expect(Ok, status);

    rc.X = 0;
    rc.Y = 0;
    rc.Width = 100.0;
    rc.Height = 100.0;

    status = GdipDrawString(NULL, teststring, 6, font, &rc, NULL, brush);
    expect(InvalidParameter, status);

    status = GdipDrawString(graphics, NULL, 6, font, &rc, NULL, brush);
    expect(InvalidParameter, status);

    status = GdipDrawString(graphics, teststring, 6, NULL, &rc, NULL, brush);
    expect(InvalidParameter, status);

    status = GdipDrawString(graphics, teststring, 6, font, NULL, NULL, brush);
    expect(InvalidParameter, status);

    status = GdipDrawString(graphics, teststring, 6, font, &rc, NULL, NULL);
    expect(InvalidParameter, status);

    status = GdipDrawString(graphics, teststring, 6, font, &rc, NULL, brush);
    expect(Ok, status);

    status = GdipMeasureString(NULL, teststring, 6, font, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(InvalidParameter, status);

    status = GdipMeasureString(graphics, NULL, 6, font, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(InvalidParameter, status);

    status = GdipMeasureString(graphics, teststring, 6, NULL, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(InvalidParameter, status);

    status = GdipMeasureString(graphics, teststring, 6, font, NULL, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(InvalidParameter, status);

    status = GdipMeasureString(graphics, teststring, 6, font, &rc, NULL, NULL, &codepointsfitted, &linesfilled);
    expect(InvalidParameter, status);

    status = GdipMeasureString(graphics, teststring, 6, font, &rc, NULL, &bounds, NULL, &linesfilled);
    expect(Ok, status);

    status = GdipMeasureString(graphics, teststring, 6, font, &rc, NULL, &bounds, &codepointsfitted, NULL);
    expect(Ok, status);

    status = GdipMeasureString(graphics, teststring, 1, font, &rc, NULL, &char_bounds, &codepointsfitted, &linesfilled);
    expect(Ok, status);
    expectf(0.0, char_bounds.X);
    expectf(0.0, char_bounds.Y);
    ok(char_bounds.Width > 0, "got %0.2f\n", bounds.Width);
    ok(char_bounds.Height > 0, "got %0.2f\n", bounds.Height);
    expect(1, codepointsfitted);
    expect(1, linesfilled);

    status = GdipMeasureString(graphics, teststring, 2, font, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(Ok, status);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    ok(bounds.Width > char_bounds.Width, "got %0.2f, expected at least %0.2f\n", bounds.Width, char_bounds.Width);
    expectf(char_bounds.Height, bounds.Height);
    expect(2, codepointsfitted);
    expect(1, linesfilled);
    char_width = bounds.Width - char_bounds.Width;

    status = GdipMeasureString(graphics, teststring, 6, font, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(Ok, status);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    ok(bounds.Width > char_bounds.Width + char_width * 2, "got %0.2f, expected at least %0.2f\n",
       bounds.Width, char_bounds.Width + char_width * 2);
    ok(bounds.Height > char_bounds.Height, "got %0.2f, expected at least %0.2f\n", bounds.Height, char_bounds.Height);
    expect(6, codepointsfitted);
    expect(2, linesfilled);
    char_height = bounds.Height - char_bounds.Height;

    /* Cut off everything after the first space. */
    rc.Width = char_bounds.Width + char_width * 2.1;

    status = GdipMeasureString(graphics, teststring, 6, font, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(Ok, status);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf_(char_bounds.Width + char_width, bounds.Width, 0.01);
    expectf_(char_bounds.Height + char_height * 2, bounds.Height, 0.01);
    expect(6, codepointsfitted);
    expect(3, linesfilled);

    /* Cut off everything including the first space. */
    rc.Width = char_bounds.Width + char_width * 1.7;

    status = GdipMeasureString(graphics, teststring, 6, font, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(Ok, status);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf_(char_bounds.Width + char_width, bounds.Width, 0.01);
    expectf_(char_bounds.Height + char_height * 2, bounds.Height, 0.01);
    expect(6, codepointsfitted);
    expect(3, linesfilled);

    /* Cut off everything after the first character. */
    rc.Width = char_bounds.Width + char_width * 0.8;

    status = GdipMeasureString(graphics, teststring, 6, font, &rc, NULL, &bounds, &codepointsfitted, &linesfilled);
    expect(Ok, status);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf_(char_bounds.Width, bounds.Width, 0.01);
    expectf_(char_bounds.Height + char_height * 3, bounds.Height, 0.05);
    expect(6, codepointsfitted);
    todo_wine expect(4, linesfilled);

    for (i = 0; i < 4; i++)
        regions[i] = (GpRegion *)0xdeadbeef;

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, format, 0, regions);
    expect(Ok, status);

    for (i = 0; i < 4; i++)
        ok(regions[i] == (GpRegion *)0xdeadbeef, "expected 0xdeadbeef, got %p\n", regions[i]);

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, format, 3, regions);
    expect(Ok, status);

    for (i = 0; i < 4; i++)
        ok(regions[i] == (GpRegion *)0xdeadbeef, "expected 0xdeadbeef, got %p\n", regions[i]);

    status = GdipSetStringFormatMeasurableCharacterRanges(format, 3, ranges);
    expect(Ok, status);

    set_rect_empty(&rc);

    for (i=0; i<4; i++)
    {
        status = GdipCreateRegion(&regions[i]);
        expect(Ok, status);
        status = GdipSetEmpty(regions[i]);
        expect(Ok, status);
    }

    status = GdipMeasureCharacterRanges(NULL, teststring, 6, font, &rc, format, 3, regions);
    expect(InvalidParameter, status);

    status = GdipMeasureCharacterRanges(graphics, NULL, 6, font, &rc, format, 3, regions);
    expect(InvalidParameter, status);

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, NULL, &rc, format, 3, regions);
    expect(InvalidParameter, status);

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, NULL, format, 3, regions);
    expect(InvalidParameter, status);

    if (0)
    {
        /* Crashes on Windows XP */
        status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, NULL, 3, regions);
        expect(InvalidParameter, status);
    }

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, format, 3, NULL);
    expect(InvalidParameter, status);

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, format, 2, regions);
    expect(InvalidParameter, status);

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, format, 3, regions);
    expect(Ok, status);

    for (i = 0; i < 4; i++)
    {
        status = GdipIsEmptyRegion(regions[i], graphics, &region_isempty[i]);
        expect(Ok, status);
    }

    ok(region_isempty[0], "region should be empty\n");
    ok(region_isempty[1], "region should be empty\n");
    ok(region_isempty[2], "region should be empty\n");
    ok(region_isempty[3], "region should be empty\n");

    rc.Width = 100.0;
    rc.Height = 100.0;

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, format, 4, regions);
    expect(Ok, status);

    for (i=0; i<4; i++)
    {
        status = GdipIsEmptyRegion(regions[i], graphics, &region_isempty[i]);
        expect(Ok, status);
    }

    ok(!region_isempty[0], "region shouldn't be empty\n");
    ok(!region_isempty[1], "region shouldn't be empty\n");
    ok(!region_isempty[2], "region shouldn't be empty\n");
    ok(region_isempty[3], "region should be empty\n");

    /* Cut off everything after the first space, and the second line. */
    rc.Width = char_bounds.Width + char_width * 2.1;
    rc.Height = char_bounds.Height + char_height * 0.5;

    status = GdipMeasureCharacterRanges(graphics, teststring, 6, font, &rc, format, 3, regions);
    expect(Ok, status);

    for (i=0; i<4; i++)
    {
        status = GdipIsEmptyRegion(regions[i], graphics, &region_isempty[i]);
        expect(Ok, status);
    }

    ok(!region_isempty[0], "region shouldn't be empty\n");
    ok(!region_isempty[1], "region shouldn't be empty\n");
    ok(region_isempty[2], "region should be empty\n");
    ok(region_isempty[3], "region should be empty\n");

    for (i=0; i<4; i++)
        GdipDeleteRegion(regions[i]);

    status = GdipCreateMatrix(&identity);
    expect(Ok, status);

    rc.X = 0;
    rc.Y = 0;
    rc.Width = 0;
    rc.Height = 0;
    memset(positions, 0, sizeof(positions));
    status = GdipMeasureDriverString(NULL, teststring, 6, font, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, &rc);
    expect(InvalidParameter, status);

    status = GdipMeasureDriverString(graphics, NULL, 6, font, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, &rc);
    expect(InvalidParameter, status);

    status = GdipMeasureDriverString(graphics, teststring, 6, NULL, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, &rc);
    expect(InvalidParameter, status);

    status = GdipMeasureDriverString(graphics, teststring, 6, font, NULL,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, &rc);
    expect(InvalidParameter, status);

    status = GdipMeasureDriverString(graphics, teststring, 6, font, positions,
        0x100, identity, &rc);
    expect(Ok, status);

    status = GdipMeasureDriverString(graphics, teststring, 6, font, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        NULL, &rc);
    expect(Ok, status);

    status = GdipMeasureDriverString(graphics, teststring, 6, font, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, NULL);
    expect(InvalidParameter, status);

    rc.X = 0;
    rc.Y = 0;
    rc.Width = 0;
    rc.Height = 0;
    status = GdipMeasureDriverString(graphics, teststring, 6, font, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, &rc);
    expect(Ok, status);

    expectf(0.0, rc.X);
    ok(rc.Y < 0.0, "unexpected Y %0.2f\n", rc.Y);
    ok(rc.Width > 0.0, "unexpected Width %0.2f\n", rc.Width);
    ok(rc.Height > 0.0, "unexpected Y %0.2f\n", rc.Y);

    char_width = rc.Width;
    char_height = rc.Height;

    rc.X = 0;
    rc.Y = 0;
    rc.Width = 0;
    rc.Height = 0;
    status = GdipMeasureDriverString(graphics, teststring, 4, font, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, &rc);
    expect(Ok, status);

    expectf(0.0, rc.X);
    ok(rc.Y < 0.0, "unexpected Y %0.2f\n", rc.Y);
    ok(rc.Width < char_width, "got Width %0.2f, expecting less than %0.2f\n", rc.Width, char_width);
    expectf(char_height, rc.Height);

    rc.X = 0;
    rc.Y = 0;
    rc.Width = 0;
    rc.Height = 0;
    status = GdipMeasureDriverString(graphics, teststring2, 1, font, positions,
        DriverStringOptionsCmapLookup|DriverStringOptionsRealizedAdvance,
        identity, &rc);
    expect(Ok, status);

    expectf(rc.X, 0.0);
    ok(rc.Y < 0.0, "unexpected Y %0.2f\n", rc.Y);
    ok(rc.Width > 0, "unexpected Width %0.2f\n", rc.Width);
    expectf(rc.Height, char_height);

    GdipDeleteMatrix(identity);
    GdipDeleteStringFormat(format);
    GdipDeleteBrush(brush);
    GdipDeleteFont(font);
    GdipDeleteFontFamily(family);
    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}
#endif /* CORE_6659_IS_FIXED */

static void test_get_set_interpolation(void)
{
    GpGraphics *graphics;
    HDC hdc = GetDC( hwnd );
    GpStatus status;
    InterpolationMode mode;

    ok(hdc != NULL, "Expected HDC to be initialized\n");
    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipGetInterpolationMode(NULL, &mode);
    expect(InvalidParameter, status);

    if (0)
    {
        /* Crashes on Windows XP */
        status = GdipGetInterpolationMode(graphics, NULL);
        expect(InvalidParameter, status);
    }

    status = GdipSetInterpolationMode(NULL, InterpolationModeNearestNeighbor);
    expect(InvalidParameter, status);

    /* out of range */
    status = GdipSetInterpolationMode(graphics, InterpolationModeHighQualityBicubic+1);
    expect(InvalidParameter, status);

    status = GdipSetInterpolationMode(graphics, InterpolationModeInvalid);
    expect(InvalidParameter, status);

    status = GdipGetInterpolationMode(graphics, &mode);
    expect(Ok, status);
    expect(InterpolationModeBilinear, mode);

    status = GdipSetInterpolationMode(graphics, InterpolationModeNearestNeighbor);
    expect(Ok, status);

    status = GdipGetInterpolationMode(graphics, &mode);
    expect(Ok, status);
    expect(InterpolationModeNearestNeighbor, mode);

    status = GdipSetInterpolationMode(graphics, InterpolationModeDefault);
    expect(Ok, status);

    status = GdipGetInterpolationMode(graphics, &mode);
    expect(Ok, status);
    expect(InterpolationModeBilinear, mode);

    status = GdipSetInterpolationMode(graphics, InterpolationModeLowQuality);
    expect(Ok, status);

    status = GdipGetInterpolationMode(graphics, &mode);
    expect(Ok, status);
    expect(InterpolationModeBilinear, mode);

    status = GdipSetInterpolationMode(graphics, InterpolationModeHighQuality);
    expect(Ok, status);

    status = GdipGetInterpolationMode(graphics, &mode);
    expect(Ok, status);
    expect(InterpolationModeHighQualityBicubic, mode);

    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_get_set_textrenderinghint(void)
{
    GpGraphics *graphics;
    HDC hdc = GetDC( hwnd );
    GpStatus status;
    TextRenderingHint hint;

    ok(hdc != NULL, "Expected HDC to be initialized\n");
    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);
    ok(graphics != NULL, "Expected graphics to be initialized\n");

    status = GdipGetTextRenderingHint(NULL, &hint);
    expect(InvalidParameter, status);

    status = GdipGetTextRenderingHint(graphics, NULL);
    expect(InvalidParameter, status);

    status = GdipSetTextRenderingHint(NULL, TextRenderingHintAntiAlias);
    expect(InvalidParameter, status);

    /* out of range */
    status = GdipSetTextRenderingHint(graphics, TextRenderingHintClearTypeGridFit+1);
    expect(InvalidParameter, status);

    status = GdipGetTextRenderingHint(graphics, &hint);
    expect(Ok, status);
    expect(TextRenderingHintSystemDefault, hint);

    status = GdipSetTextRenderingHint(graphics, TextRenderingHintSystemDefault);
    expect(Ok, status);

    status = GdipGetTextRenderingHint(graphics, &hint);
    expect(Ok, status);
    expect(TextRenderingHintSystemDefault, hint);

    status = GdipSetTextRenderingHint(graphics, TextRenderingHintAntiAliasGridFit);
    expect(Ok, status);

    status = GdipGetTextRenderingHint(graphics, &hint);
    expect(Ok, status);
    expect(TextRenderingHintAntiAliasGridFit, hint);

    GdipDeleteGraphics(graphics);

    ReleaseDC(hwnd, hdc);
}

static void test_getdc_scaled(void)
{
    GpStatus status;
    GpGraphics *graphics = NULL;
    GpBitmap *bitmap = NULL;
    HDC hdc=NULL;
    HBRUSH hbrush, holdbrush;
    ARGB color;

    status = GdipCreateBitmapFromScan0(10, 10, 12, PixelFormat24bppRGB, NULL, &bitmap);
    expect(Ok, status);

    status = GdipGetImageGraphicsContext((GpImage*)bitmap, &graphics);
    expect(Ok, status);

    status = GdipScaleWorldTransform(graphics, 2.0, 2.0, MatrixOrderPrepend);
    expect(Ok, status);

    status = GdipGetDC(graphics, &hdc);
    expect(Ok, status);
    ok(hdc != NULL, "got NULL hdc\n");

    hbrush = CreateSolidBrush(RGB(255, 0, 0));

    holdbrush = SelectObject(hdc, hbrush);

    Rectangle(hdc, 2, 2, 6, 6);

    SelectObject(hdc, holdbrush);

    DeleteObject(hbrush);

    status = GdipReleaseDC(graphics, hdc);
    expect(Ok, status);

    GdipDeleteGraphics(graphics);

    status = GdipBitmapGetPixel(bitmap, 3, 3, &color);
    expect(Ok, status);
    expect(0xffff0000, color);

    status = GdipBitmapGetPixel(bitmap, 8, 8, &color);
    expect(Ok, status);
    expect(0xff000000, color);

    GdipDisposeImage((GpImage*)bitmap);
}

static void test_GdipMeasureString(void)
{
    static const struct test_data
    {
        REAL res_x, res_y, page_scale;
        GpUnit unit;
    } td[] =
    {
        { 200.0, 200.0, 1.0, UnitPixel }, /* base */
        { 200.0, 200.0, 2.0, UnitPixel },
        { 200.0, 200.0, 1.0, UnitDisplay },
        { 200.0, 200.0, 2.0, UnitDisplay },
        { 200.0, 200.0, 1.0, UnitInch },
        { 200.0, 200.0, 2.0, UnitInch },
        { 200.0, 600.0, 1.0, UnitPoint },
        { 200.0, 600.0, 2.0, UnitPoint },
        { 200.0, 600.0, 1.0, UnitDocument },
        { 200.0, 600.0, 2.0, UnitDocument },
        { 200.0, 600.0, 1.0, UnitMillimeter },
        { 200.0, 600.0, 2.0, UnitMillimeter },
        { 200.0, 600.0, 1.0, UnitDisplay },
        { 200.0, 600.0, 2.0, UnitDisplay },
        { 200.0, 600.0, 1.0, UnitPixel },
        { 200.0, 600.0, 2.0, UnitPixel },
    };
    static const WCHAR tahomaW[] = { 'T','a','h','o','m','a',0 };
    static const WCHAR string[] = { '1','2','3','4','5','6','7',0 };
    GpStatus status;
    GpGraphics *graphics;
    GpFontFamily *family;
    GpFont *font;
    GpStringFormat *format;
    RectF bounds, rc;
    REAL base_cx = 0, base_cy = 0, height;
    INT chars, lines;
    LOGFONTW lf;
    UINT i;
    REAL font_size;
    GpUnit font_unit, unit;

    status = GdipCreateStringFormat(0, LANG_NEUTRAL, &format);
    expect(Ok, status);
    status = GdipCreateFontFamilyFromName(tahomaW, NULL, &family);
    expect(Ok, status);

    /* font size in pixels */
    status = GdipCreateFont(family, 100.0, FontStyleRegular, UnitPixel, &font);
    expect(Ok, status);
    status = GdipGetFontSize(font, &font_size);
    expect(Ok, status);
    expectf(100.0, font_size);
    status = GdipGetFontUnit(font, &font_unit);
    expect(Ok, status);
    expect(UnitPixel, font_unit);

    for (i = 0; i < sizeof(td)/sizeof(td[0]); i++)
    {
        graphics = create_graphics(td[i].res_x, td[i].res_y, td[i].unit, td[i].page_scale);

        lf.lfHeight = 0xdeadbeef;
        status = GdipGetLogFontW(font, graphics, &lf);
        expect(Ok, status);
        height = units_to_pixels(font_size, td[i].unit, td[i].res_y);
        if (td[i].unit != UnitDisplay)
            height *= td[i].page_scale;
        ok(-lf.lfHeight == (LONG)(height + 0.5), "%u: expected %d (%f), got %d\n",
           i, (LONG)(height + 0.5), height, lf.lfHeight);

        height = font_size + 2.0 * font_size / 6.0;

        set_rect_empty(&rc);
        set_rect_empty(&bounds);
        status = GdipMeasureString(graphics, string, -1, font, &rc, format, &bounds, &chars, &lines);
        expect(Ok, status);

        if (i == 0)
        {
            base_cx = bounds.Width;
            base_cy = bounds.Height;
        }

        expectf(0.0, bounds.X);
        expectf(0.0, bounds.Y);
todo_wine
        expectf_(height, bounds.Height, height / 100.0);
        expectf_(bounds.Height / base_cy, bounds.Width / base_cx, 0.1);
        expect(7, chars);
        expect(1, lines);

        /* make sure it really fits */
        bounds.Width += 1.0;
        bounds.Height += 1.0;
        rc = bounds;
        rc.X = 50.0;
        rc.Y = 50.0;
        set_rect_empty(&bounds);
        status = GdipMeasureString(graphics, string, -1, font, &rc, format, &bounds, &chars, &lines);
        expect(Ok, status);
        expectf(50.0, bounds.X);
        expectf(50.0, bounds.Y);
todo_wine
        expectf_(height, bounds.Height, height / 100.0);
        expectf_(bounds.Height / base_cy, bounds.Width / base_cx, 0.1);
        expect(7, chars);
        expect(1, lines);

        status = GdipDeleteGraphics(graphics);
        expect(Ok, status);
    }

    GdipDeleteFont(font);

    /* font size in logical units */
    /* UnitPoint = 3, UnitInch = 4, UnitDocument = 5, UnitMillimeter = 6 */
    for (unit = 3; unit <= 6; unit++)
    {
        /* create a font which final height is 100.0 pixels with 200 dpi device */
        /* height + 2 * (height/6) = 100 => height = 100 * 3 / 4 => 75 */
        height = pixels_to_units(75.0, unit, 200.0);
        status = GdipCreateFont(family, height, FontStyleRegular, unit, &font);
        expect(Ok, status);
        status = GdipGetFontSize(font, &font_size);
        expect(Ok, status);
        expectf(height, font_size);
        status = GdipGetFontUnit(font, &font_unit);
        expect(Ok, status);
        expect(unit, font_unit);

        for (i = 0; i < sizeof(td)/sizeof(td[0]); i++)
        {
            REAL unit_scale;

            graphics = create_graphics(td[i].res_x, td[i].res_y, td[i].unit, td[i].page_scale);

            lf.lfHeight = 0xdeadbeef;
            status = GdipGetLogFontW(font, graphics, &lf);
            expect(Ok, status);
            if (td[i].unit == UnitDisplay || td[i].unit == UnitPixel)
                height = units_to_pixels(font_size, font_unit, td[i].res_x);
            else
                height = units_to_pixels(font_size, font_unit, td[i].res_y);
            /*trace("%.1f font units = %f pixels with %.1f dpi, page_scale %.1f\n", font_size, height, td[i].res_y, td[i].page_scale);*/
            ok(-lf.lfHeight == (LONG)(height + 0.5), "%u: expected %d (%f), got %d\n",
               i, (LONG)(height + 0.5), height, lf.lfHeight);

            if (td[i].unit == UnitDisplay || td[i].unit == UnitPixel)
                unit_scale = units_scale(font_unit, td[i].unit, td[i].res_x);
            else
                unit_scale = units_scale(font_unit, td[i].unit, td[i].res_y);
            /*trace("%u: %d to %d, %.1f dpi => unit_scale %f\n", i, font_unit, td[i].unit, td[i].res_y, unit_scale);*/
            height = (font_size + 2.0 * font_size / 6.0) * unit_scale;
            if (td[i].unit != UnitDisplay)
                height /= td[i].page_scale;
            /*trace("%u: %.1f font units = %f units with %.1f dpi, page_scale %.1f\n", i, font_size, height, td[i].res_y, td[i].page_scale);*/

            set_rect_empty(&rc);
            set_rect_empty(&bounds);
            status = GdipMeasureString(graphics, string, -1, font, &rc, format, &bounds, &chars, &lines);
            expect(Ok, status);

            if (i == 0)
            {
                base_cx = bounds.Width;
                base_cy = bounds.Height;
            }

            expectf(0.0, bounds.X);
            expectf(0.0, bounds.Y);
todo_wine
            expectf_(height, bounds.Height, height / 85.0);
            expectf_(bounds.Height / base_cy, bounds.Width / base_cx, 0.1);
            expect(7, chars);
            expect(1, lines);

            /* make sure it really fits */
            bounds.Width += 1.0;
            bounds.Height += 1.0;
            rc = bounds;
            rc.X = 50.0;
            rc.Y = 50.0;
            set_rect_empty(&bounds);
            status = GdipMeasureString(graphics, string, -1, font, &rc, format, &bounds, &chars, &lines);
            expect(Ok, status);
            expectf(50.0, bounds.X);
            expectf(50.0, bounds.Y);
todo_wine
            expectf_(height, bounds.Height, height / 85.0);
            expectf_(bounds.Height / base_cy, bounds.Width / base_cx, 0.1);
            expect(7, chars);
            expect(1, lines);

            /* verify the result */
            height = units_to_pixels(bounds.Height, td[i].unit, td[i].res_x);
            if (td[i].unit != UnitDisplay)
                height *= td[i].page_scale;
            /*trace("%u: unit %u, %.1fx%.1f dpi, scale %.1f, height %f, pixels %f\n",
                  i, td[i].unit, td[i].res_x, td[i].res_y, td[i].page_scale, bounds.Height, height);*/
todo_wine
            expectf_(100.0, height, 1.1);

            status = GdipDeleteGraphics(graphics);
            expect(Ok, status);
        }

        GdipDeleteFont(font);
    }

    GdipDeleteFontFamily(family);
    GdipDeleteStringFormat(format);
}

static void test_transform(void)
{
    static const struct test_data
    {
        REAL res_x, res_y, scale;
        GpUnit unit;
        GpPointF in[2], out[2];
    } td[] =
    {
        { 96.0, 96.0, 1.0, UnitPixel,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 100.0, 0.0 }, { 0.0, 100.0 } } },
        { 96.0, 96.0, 1.0, UnitDisplay,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 100.0, 0.0 }, { 0.0, 100.0 } } },
        { 96.0, 96.0, 1.0, UnitInch,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 9600.0, 0.0 }, { 0.0, 9600.0 } } },
        { 123.0, 456.0, 1.0, UnitPoint,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 170.833313, 0.0 }, { 0.0, 633.333252 } } },
        { 123.0, 456.0, 1.0, UnitDocument,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 40.999996, 0.0 }, { 0.0, 151.999985 } } },
        { 123.0, 456.0, 2.0, UnitMillimeter,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 968.503845, 0.0 }, { 0.0, 3590.550781 } } },
        { 196.0, 296.0, 1.0, UnitDisplay,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 100.0, 0.0 }, { 0.0, 100.0 } } },
        { 196.0, 296.0, 1.0, UnitPixel,
          { { 100.0, 0.0 }, { 0.0, 100.0 } }, { { 100.0, 0.0 }, { 0.0, 100.0 } } },
    };
    GpStatus status;
    GpGraphics *graphics;
    GpPointF ptf[2];
    UINT i;

    for (i = 0; i < sizeof(td)/sizeof(td[0]); i++)
    {
        graphics = create_graphics(td[i].res_x, td[i].res_y, td[i].unit, td[i].scale);
        ptf[0].X = td[i].in[0].X;
        ptf[0].Y = td[i].in[0].Y;
        ptf[1].X = td[i].in[1].X;
        ptf[1].Y = td[i].in[1].Y;
        status = GdipTransformPoints(graphics, CoordinateSpaceDevice, CoordinateSpaceWorld, ptf, 2);
        expect(Ok, status);
        expectf(td[i].out[0].X, ptf[0].X);
        expectf(td[i].out[0].Y, ptf[0].Y);
        expectf(td[i].out[1].X, ptf[1].X);
        expectf(td[i].out[1].Y, ptf[1].Y);
        status = GdipTransformPoints(graphics, CoordinateSpaceWorld, CoordinateSpaceDevice, ptf, 2);
        expect(Ok, status);
        expectf(td[i].in[0].X, ptf[0].X);
        expectf(td[i].in[0].Y, ptf[0].Y);
        expectf(td[i].in[1].X, ptf[1].X);
        expectf(td[i].in[1].Y, ptf[1].Y);
        status = GdipDeleteGraphics(graphics);
        expect(Ok, status);
    }
}

/* Many people on the net ask why there is so much difference in rendered
 * text height between gdiplus and gdi32, this test suggests an answer to
 * that question. Important: this test assumes that font dpi == device dpi.
 */
static void test_font_height_scaling(void)
{
    static const WCHAR tahomaW[] = { 'T','a','h','o','m','a',0 };
    static const WCHAR string[] = { '1','2','3','4','5','6','7',0 };
    HDC hdc;
    GpStringFormat *format;
    CharacterRange range = { 0, 7 };
    GpRegion *region;
    GpGraphics *graphics;
    GpFontFamily *family;
    GpFont *font;
    GpStatus status;
    RectF bounds, rect;
    REAL height, dpi, scale;
    PointF ptf;
    GpUnit gfx_unit, font_unit;

    status = GdipCreateStringFormat(StringFormatFlagsNoWrap, LANG_NEUTRAL, &format);
    expect(Ok, status);
    status = GdipSetStringFormatMeasurableCharacterRanges(format, 1, &range);
    expect(Ok, status);
    status = GdipCreateRegion(&region);
    expect(Ok, status);

    status = GdipCreateFontFamilyFromName(tahomaW, NULL, &family);
    expect(Ok, status);

    hdc = CreateCompatibleDC(0);
    status = GdipCreateFromHDC(hdc, &graphics);

    status = GdipGetDpiY(graphics, &dpi);
    expect(Ok, status);

    /* First check if tested functionality works:
     * under XP if font and graphics units differ then GdipTransformPoints
     * followed by GdipSetPageUnit to change the graphics units breaks region
     * scaling in GdipMeasureCharacterRanges called later.
     */
    status = GdipSetPageUnit(graphics, UnitDocument);
    expect(Ok, status);

    ptf.X = 0.0;
    ptf.Y = 0.0;
    status = GdipTransformPoints(graphics, CoordinateSpaceWorld, CoordinateSpaceDevice, &ptf, 1);
    expect(Ok, status);

    status = GdipSetPageUnit(graphics, UnitInch);
    expect(Ok, status);

    status = GdipCreateFont(family, 720.0, FontStyleRegular, UnitPoint, &font);
    expect(Ok, status);

    set_rect_empty(&rect);
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, NULL, NULL);
    expect(Ok, status);
    trace("test bounds: %f,%f,%f,%f\n", bounds.X, bounds.Y, bounds.Width, bounds.Height);

    set_rect_empty(&rect);
    rect.Width = 32000.0;
    rect.Height = 32000.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);

    set_rect_empty(&rect);
    status = GdipGetRegionBounds(region, graphics, &rect);
    expect(Ok, status);
    trace("test region: %f,%f,%f,%f\n", rect.X, rect.Y, rect.Width, rect.Height);

    GdipDeleteFont(font);

    scale = rect.Height / bounds.Height;
    if (fabs(scale - 1.0) > 0.1)
    {
        win_skip("GdipGetRegionBounds is broken, scale %f (should be near 1.0)\n", scale);
        goto cleanup;
    }

    status = GdipScaleWorldTransform(graphics, 0.01, 0.01, MatrixOrderAppend);
    expect(Ok, status);

    /* UnitPixel = 2, UnitPoint = 3, UnitInch = 4, UnitDocument = 5, UnitMillimeter = 6 */
    /* UnitPixel as a font base unit is not tested because it drastically
       differs in behaviour */
    for (font_unit = 3; font_unit <= 6; font_unit++)
    {
        /* create a font for the final text height of 100 pixels */
        /* height + 2 * (height/6) = 100 => height = 100 * 3 / 4 => 75 */
        status = GdipSetPageUnit(graphics, font_unit);
        expect(Ok, status);
        ptf.X = 0;
        ptf.Y = 75.0;
        status = GdipTransformPoints(graphics, CoordinateSpaceWorld, CoordinateSpaceDevice, &ptf, 1);
        expect(Ok, status);
        height = ptf.Y;
        /*trace("height %f units\n", height);*/
        status = GdipCreateFont(family, height, FontStyleRegular, font_unit, &font);
        expect(Ok, status);

        /* UnitPixel = 2, UnitPoint = 3, UnitInch = 4, UnitDocument = 5, UnitMillimeter = 6 */
        for (gfx_unit = 2; gfx_unit <= 6; gfx_unit++)
        {
            static const WCHAR doubleW[2] = { 'W','W' };
            RectF bounds_1, bounds_2;
            REAL margin, margin_y, font_height;
            int match;

            status = GdipSetPageUnit(graphics, gfx_unit);
            expect(Ok, status);

            margin_y = units_to_pixels(height / 8.0, font_unit, dpi);
            margin_y = pixels_to_units(margin_y, gfx_unit, dpi);

            status = GdipGetFontHeight(font, graphics, &font_height);
            expect(Ok, status);

            set_rect_empty(&rect);
            set_rect_empty(&bounds);
            status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, NULL, NULL);
            expect(Ok, status);
            /*trace("bounds: %f,%f,%f,%f\n", bounds.X, bounds.Y, bounds.Width, bounds.Height);*/
todo_wine
            expectf_(font_height + margin_y, bounds.Height, 0.005);

            ptf.X = 0;
            ptf.Y = bounds.Height;
            status = GdipTransformPoints(graphics, CoordinateSpaceDevice, CoordinateSpaceWorld, &ptf, 1);
            expect(Ok, status);
            match = fabs(100.0 - ptf.Y) <= 1.0;
todo_wine
            ok(match, "Expected 100.0, got %f\n", ptf.Y);

            /* verify the result */
            ptf.Y = units_to_pixels(bounds.Height, gfx_unit, dpi);
            ptf.Y /= 100.0;
            match = fabs(100.0 - ptf.Y) <= 1.0;
todo_wine
            ok(match, "Expected 100.0, got %f\n", ptf.Y);

            /* bounds.width of 1 glyph: [margin]+[width]+[margin] */
            set_rect_empty(&rect);
            set_rect_empty(&bounds_1);
            status = GdipMeasureString(graphics, doubleW, 1, font, &rect, format, &bounds_1, NULL, NULL);
            expect(Ok, status);
            /* bounds.width of 2 identical glyphs: [margin]+[width]+[width]+[margin] */
            set_rect_empty(&rect);
            set_rect_empty(&bounds_2);
            status = GdipMeasureString(graphics, doubleW, 2, font, &rect, format, &bounds_2, NULL, NULL);
            expect(Ok, status);

            /* margin = [bounds.width of 1] - [bounds.width of 2] / 2*/
            margin = bounds_1.Width - bounds_2.Width / 2.0;
            /*trace("margin %f\n", margin);*/
            ok(margin > 0.0, "wrong margin %f\n", margin);

            set_rect_empty(&rect);
            rect.Width = 320000.0;
            rect.Height = 320000.0;
            status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
            expect(Ok, status);
            set_rect_empty(&rect);
            status = GdipGetRegionBounds(region, graphics, &rect);
            expect(Ok, status);
            /*trace("region: %f,%f,%f,%f\n", rect.X, rect.Y, rect.Width, rect.Height);*/
            ok(rect.X > 0.0, "wrong rect.X %f\n", rect.X);
            expectf(0.0, rect.Y);
            match = fabs(1.0 - margin / rect.X) <= 0.05;
            ok(match, "Expected %f, got %f\n", margin, rect.X);
            match = fabs(1.0 - font_height / rect.Height) <= 0.1;
todo_wine
            ok(match, "Expected %f, got %f\n", font_height, rect.Height);
            match = fabs(1.0 - bounds.Width / (rect.Width + margin * 2.0)) <= 0.05;
            ok(match, "Expected %f, got %f\n", bounds.Width, rect.Width + margin * 2.0);
        }

        GdipDeleteFont(font);
    }

cleanup:
    status = GdipDeleteGraphics(graphics);
    expect(Ok, status);
    DeleteDC(hdc);

    GdipDeleteFontFamily(family);
    GdipDeleteRegion(region);
    GdipDeleteStringFormat(format);
}

static void test_measure_string(void)
{
    static const WCHAR tahomaW[] = { 'T','a','h','o','m','a',0 };
    static const WCHAR string[] = { 'A','0','1',0 };
    HDC hdc;
    GpStringFormat *format;
    CharacterRange range;
    GpRegion *region;
    GpGraphics *graphics;
    GpFontFamily *family;
    GpFont *font;
    GpStatus status;
    RectF bounds, rect;
    REAL width, height, width_1, width_2;
    REAL margin_x, margin_y, width_rgn, height_rgn;
    int lines, glyphs;

    status = GdipCreateStringFormat(StringFormatFlagsNoWrap, LANG_NEUTRAL, &format);
    expect(Ok, status);
    expect(Ok, status);

    status = GdipCreateRegion(&region);
    expect(Ok, status);

    status = GdipCreateFontFamilyFromName(tahomaW, NULL, &family);
    expect(Ok, status);

    hdc = CreateCompatibleDC(0);
    status = GdipCreateFromHDC(hdc, &graphics);

    status = GdipCreateFont(family, 20, FontStyleRegular, UnitPixel, &font);
    expect(Ok, status);

    margin_x = 20.0 / 6.0;
    margin_y = 20.0 / 8.0;

    set_rect_empty(&rect);
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    width = bounds.Width;
    height = bounds.Height;

    set_rect_empty(&rect);
    rect.Height = height / 2.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf(width, bounds.Width);
todo_wine
    expectf(height / 2.0, bounds.Height);

    range.First = 0;
    range.Length = lstrlenW(string);
    status = GdipSetStringFormatMeasurableCharacterRanges(format, 1, &range);
    expect(Ok, status);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 32000.0;
    rect.Height = 32000.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
    expectf_(5.0 + margin_x, bounds.X, 1.0);
    expectf(5.0, bounds.Y);
    expectf_(width - margin_x*2.0, bounds.Width, 1.0);
todo_wine
    expectf_(height - margin_y, bounds.Height, 1.0);

    width_rgn = bounds.Width;
    height_rgn = bounds.Height;

    range.First = 0;
    range.Length = 1;
    status = GdipSetStringFormatMeasurableCharacterRanges(format, 1, &range);
    expect(Ok, status);

    set_rect_empty(&rect);
    rect.Width = 32000.0;
    rect.Height = 32000.0;
    status = GdipMeasureCharacterRanges(graphics, string, 1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
    expectf_(margin_x, bounds.X, 1.0);
    expectf(0.0, bounds.Y);
    ok(bounds.Width < width_rgn / 2.0, "width of 1 glyph is wrong\n");
    expectf(height_rgn, bounds.Height);
    width_1 = bounds.Width;

    range.First = 0;
    range.Length = lstrlenW(string);
    status = GdipSetStringFormatMeasurableCharacterRanges(format, 1, &range);
    expect(Ok, status);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 0.0;
    rect.Height = 0.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf(0.0, bounds.Width);
    expectf(0.0, bounds.Height);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = width_rgn / 2.0;
    rect.Height = 32000.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
    expectf_(5.0 + margin_x, bounds.X, 1.0);
    expectf(5.0, bounds.Y);
    expectf_(width_1, bounds.Width, 1.0);
todo_wine
    expectf_(height - margin_y, bounds.Height, 1.0);

    status = GdipSetStringFormatFlags(format, StringFormatFlagsNoWrap | StringFormatFlagsNoClip);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 0.0;
    rect.Height = 0.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
    expectf_(5.0 + margin_x, bounds.X, 1.0);
    expectf(5.0, bounds.Y);
    expectf(width_rgn, bounds.Width);
    expectf(height_rgn, bounds.Height);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = width_rgn / 2.0;
    rect.Height = 32000.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
    expectf_(5.0 + margin_x, bounds.X, 1.0);
    expectf(5.0, bounds.Y);
    expectf_(width_1, bounds.Width, 1.0);
    expectf(height_rgn, bounds.Height);

    set_rect_empty(&rect);
    rect.Height = height / 2.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf_(width, bounds.Width, 0.01);
todo_wine
    expectf(height, bounds.Height);

    set_rect_empty(&rect);
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, 1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(1, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    ok(bounds.Width < width / 2.0, "width of 1 glyph is wrong\n");
    expectf(height, bounds.Height);
    width_1 = bounds.Width;

    set_rect_empty(&rect);
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, 2, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(2, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    ok(bounds.Width < width, "width of 2 glyphs is wrong\n");
    ok(bounds.Width > width_1, "width of 2 glyphs is wrong\n");
    expectf(height, bounds.Height);
    width_2 = bounds.Width;

    set_rect_empty(&rect);
    rect.Width = width / 2.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(1, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf_(width_1, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    set_rect_empty(&rect);
    rect.Height = height;
    rect.Width = width - 0.05;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(2, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf_(width_2, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    set_rect_empty(&rect);
    rect.Height = height;
    rect.Width = width_2 - 0.05;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(1, glyphs);
    expect(1, lines);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf_(width_1, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    /* Default (Near) alignment */
    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = width * 2.0;
    rect.Height = height * 2.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
    expectf(5.0, bounds.X);
    expectf(5.0, bounds.Y);
    expectf_(width, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 32000.0;
    rect.Height = 32000.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
    expectf_(5.0 + margin_x, bounds.X, 1.0);
    expectf(5.0, bounds.Y);
    expectf_(width - margin_x*2.0, bounds.Width, 1.0);
todo_wine
    expectf_(height - margin_y, bounds.Height, 1.0);

    width_rgn = bounds.Width;
    height_rgn = bounds.Height;

    /* Center alignment */
    GdipSetStringFormatAlign(format, StringAlignmentCenter);
    GdipSetStringFormatLineAlign(format, StringAlignmentCenter);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = width * 2.0;
    rect.Height = height * 2.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
todo_wine
    expectf_(5.0 + width/2.0, bounds.X, 0.01);
todo_wine
    expectf(5.0 + height/2.0, bounds.Y);
    expectf_(width, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 0.0;
    rect.Height = 0.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
todo_wine
    expectf_(5.0 - width/2.0, bounds.X, 0.01);
todo_wine
    expectf(5.0 - height/2.0, bounds.Y);
    expectf_(width, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = width_rgn * 2.0;
    rect.Height = height_rgn * 2.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
todo_wine
    expectf_(5.0 + width_rgn/2.0, bounds.X, 1.0);
todo_wine
    expectf_(5.0 + height_rgn/2.0, bounds.Y, 1.0);
    expectf_(width_rgn, bounds.Width, 1.0);
    expectf_(height_rgn, bounds.Height, 1.0);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 0.0;
    rect.Height = 0.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
todo_wine
    expectf_(5.0 - width_rgn/2.0, bounds.X, 1.0);
todo_wine
    expectf_(5.0 - height_rgn/2.0, bounds.Y, 1.0);
    expectf_(width_rgn, bounds.Width, 1.0);
    expectf_(height_rgn, bounds.Height, 1.0);

    /* Far alignment */
    GdipSetStringFormatAlign(format, StringAlignmentFar);
    GdipSetStringFormatLineAlign(format, StringAlignmentFar);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = width * 2.0;
    rect.Height = height * 2.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
todo_wine
    expectf_(5.0 + width, bounds.X, 0.01);
todo_wine
    expectf(5.0 + height, bounds.Y);
    expectf_(width, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 0.0;
    rect.Height = 0.0;
    set_rect_empty(&bounds);
    status = GdipMeasureString(graphics, string, -1, font, &rect, format, &bounds, &glyphs, &lines);
    expect(Ok, status);
    expect(3, glyphs);
    expect(1, lines);
todo_wine
    expectf_(5.0 - width, bounds.X, 0.01);
todo_wine
    expectf(5.0 - height, bounds.Y);
    expectf_(width, bounds.Width, 0.01);
    expectf(height, bounds.Height);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = width_rgn * 2.0;
    rect.Height = height_rgn * 2.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
todo_wine
    expectf_(5.0 + width_rgn, bounds.X, 2.0);
todo_wine
    expectf_(5.0 + height_rgn, bounds.Y, 1.0);
    expectf_(width_rgn, bounds.Width, 1.0);
    expectf_(height_rgn, bounds.Height, 1.0);

    rect.X = 5.0;
    rect.Y = 5.0;
    rect.Width = 0.0;
    rect.Height = 0.0;
    status = GdipMeasureCharacterRanges(graphics, string, -1, font, &rect, format, 1, &region);
    expect(Ok, status);
    set_rect_empty(&bounds);
    status = GdipGetRegionBounds(region, graphics, &bounds);
    expect(Ok, status);
todo_wine
    expectf_(5.0 - width_rgn, bounds.X, 2.0);
todo_wine
    expectf_(5.0 - height_rgn, bounds.Y, 1.0);
    expectf_(width_rgn, bounds.Width, 1.0);
    expectf_(height_rgn, bounds.Height, 1.0);

    status = GdipDeleteFont(font);
    expect(Ok, status);

    status = GdipDeleteGraphics(graphics);
    expect(Ok, status);
    DeleteDC(hdc);

    GdipDeleteFontFamily(family);
    GdipDeleteRegion(region);
    GdipDeleteStringFormat(format);
}

static void test_measured_extra_space(void)
{
    static const WCHAR tahomaW[] = { 'T','a','h','o','m','a',0 };
    static const WCHAR string[2] = { 'W','W' };
    GpStringFormat *format;
    HDC hdc;
    GpGraphics *graphics;
    GpFontFamily *family;
    GpFont *font;
    GpStatus status;
    GpUnit gfx_unit, font_unit;
    RectF bounds_1, bounds_2, rect;
    REAL margin, font_size, dpi;

    status = GdipCreateStringFormat(0, LANG_NEUTRAL, &format);
    expect(Ok, status);

    status = GdipCreateFontFamilyFromName(tahomaW, NULL, &family);
    expect(Ok, status);
    hdc = CreateCompatibleDC(0);
    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);

    status = GdipGetDpiX(graphics, &dpi);
    expect(Ok, status);

    /* UnitPixel = 2, UnitPoint = 3, UnitInch = 4, UnitDocument = 5, UnitMillimeter = 6 */
    /* UnitPixel as a font base unit is not tested because it differs in behaviour */
    for (font_unit = 3; font_unit <= 6; font_unit++)
    {
        status = GdipCreateFont(family, 1234.0, FontStyleRegular, font_unit, &font);
        expect(Ok, status);

        status = GdipGetFontSize(font, &font_size);
        expect(Ok, status);
        font_size = units_to_pixels(font_size, font_unit, dpi);
        /*trace("font size/6 = %f pixels\n", font_size / 6.0);*/

        /* UnitPixel = 2, UnitPoint = 3, UnitInch = 4, UnitDocument = 5, UnitMillimeter = 6 */
        for (gfx_unit = 2; gfx_unit <= 6; gfx_unit++)
        {
            status = GdipSetPageUnit(graphics, gfx_unit);
            expect(Ok, status);

            /* bounds.width of 1 glyph: [margin]+[width]+[margin] */
            set_rect_empty(&rect);
            set_rect_empty(&bounds_1);
            status = GdipMeasureString(graphics, string, 1, font, &rect, format, &bounds_1, NULL, NULL);
            expect(Ok, status);
            /* bounds.width of 2 identical glyphs: [margin]+[width]+[width]+[margin] */
            set_rect_empty(&rect);
            set_rect_empty(&bounds_2);
            status = GdipMeasureString(graphics, string, 2, font, &rect, format, &bounds_2, NULL, NULL);
            expect(Ok, status);

            /* margin = [bounds.width of 1] - [bounds.width of 2] / 2*/
            margin = units_to_pixels(bounds_1.Width - bounds_2.Width / 2.0, gfx_unit, dpi);
            /*trace("margin %f pixels\n", margin);*/
            expectf_(font_size / 6.0, margin, font_size / 100.0);
        }

        GdipDeleteFont(font);
    }

    GdipDeleteGraphics(graphics);
    DeleteDC(hdc);
    GdipDeleteFontFamily(family);
    GdipDeleteStringFormat(format);
}

static void test_alpha_hdc(void)
{
    GpStatus status;
    HDC hdc;
    HBITMAP hbm, old_hbm;
    GpGraphics *graphics;
    ULONG *bits;
    BITMAPINFO bmi;
    GpRectF bounds;

    hdc = CreateCompatibleDC(0);
    ok(hdc != NULL, "CreateCompatibleDC failed\n");
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biHeight = 5;
    bmi.bmiHeader.biWidth = 5;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biClrUsed = 0;

    hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
    ok(hbm != NULL, "CreateDIBSection failed\n");

    old_hbm = SelectObject(hdc, hbm);

    status = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, status);

    status = GdipGetVisibleClipBounds(graphics, &bounds);
    expect(Ok, status);
    expectf(0.0, bounds.X);
    expectf(0.0, bounds.Y);
    expectf(5.0, bounds.Width);
    expectf(5.0, bounds.Height);

    bits[0] = 0xdeadbeef;

    status = GdipGraphicsClear(graphics, 0xffaaaaaa);
    expect(Ok, status);

    expect(0xffaaaaaa, bits[0]);

    SelectObject(hdc, old_hbm);

    bits[0] = 0xdeadbeef;

    status = GdipGraphicsClear(graphics, 0xffbbbbbb);
    expect(Ok, status);

    todo_wine expect(0xffbbbbbb, bits[0]);

    GdipDeleteGraphics(graphics);

    DeleteObject(hbm);
    DeleteDC(hdc);
}

static void test_bitmapfromgraphics(void)
{
    GpStatus stat;
    GpGraphics *graphics = NULL;
    HDC hdc = GetDC( hwnd );
    GpBitmap *bitmap = NULL;
    PixelFormat format;
    REAL imageres, graphicsres;
    UINT width, height;

    stat = GdipCreateFromHDC(hdc, &graphics);
    expect(Ok, stat);

    stat = GdipCreateBitmapFromGraphics(12, 13, NULL, &bitmap);
    expect(InvalidParameter, stat);

    stat = GdipCreateBitmapFromGraphics(12, 13, graphics, NULL);
    expect(InvalidParameter, stat);

    stat = GdipCreateBitmapFromGraphics(12, 13, graphics, &bitmap);
    expect(Ok, stat);

    stat = GdipGetImagePixelFormat((GpImage*)bitmap, &format);
    expect(Ok, stat);
    expect(PixelFormat32bppPARGB, format);

    stat = GdipGetDpiX(graphics, &graphicsres);
    expect(Ok, stat);

    stat = GdipGetImageHorizontalResolution((GpImage*)bitmap, &imageres);
    expect(Ok, stat);
    expectf(graphicsres, imageres);

    stat = GdipGetDpiY(graphics, &graphicsres);
    expect(Ok, stat);

    stat = GdipGetImageVerticalResolution((GpImage*)bitmap, &imageres);
    expect(Ok, stat);
    expectf(graphicsres, imageres);

    stat = GdipGetImageWidth((GpImage*)bitmap, &width);
    expect(Ok, stat);
    expect(12, width);

    stat = GdipGetImageHeight((GpImage*)bitmap, &height);
    expect(Ok, stat);
    expect(13, height);

    GdipDeleteGraphics(graphics);
    GdipDisposeImage((GpImage*)bitmap);
}

START_TEST(graphics)
{
    struct GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    WNDCLASSA class;

    memset( &class, 0, sizeof(class) );
    class.lpszClassName = "gdiplus_test";
    class.style = CS_HREDRAW | CS_VREDRAW;
    class.lpfnWndProc = DefWindowProcA;
    class.hInstance = GetModuleHandleA(0);
    class.hIcon = LoadIcon(0, IDI_APPLICATION);
    class.hCursor = LoadCursor(NULL, IDC_ARROW);
    class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassA( &class );
    hwnd = CreateWindowA( "gdiplus_test", "graphics test", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                          CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, 0, 0, GetModuleHandleA(0), 0 );
    ok(hwnd != NULL, "Expected window to be created\n");

    gdiplusStartupInput.GdiplusVersion              = 1;
    gdiplusStartupInput.DebugEventCallback          = NULL;
    gdiplusStartupInput.SuppressBackgroundThread    = 0;
    gdiplusStartupInput.SuppressExternalCodecs      = 0;

    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    test_measured_extra_space();
    test_measure_string();
    test_font_height_scaling();
    test_transform();
    test_GdipMeasureString();
    test_constructor_destructor();
    test_save_restore();
    test_GdipFillClosedCurve2();
    test_GdipFillClosedCurve2I();
    test_GdipDrawBezierI();
    test_GdipDrawArc();
    test_GdipDrawArcI();
    test_GdipDrawCurve();
    test_GdipDrawCurveI();
    test_GdipDrawCurve2();
    test_GdipDrawCurve2I();
    test_GdipDrawCurve3();
    test_GdipDrawCurve3I();
    test_GdipDrawLineI();
    test_GdipDrawLinesI();
    test_GdipDrawImagePointsRect();
    test_GdipFillClosedCurve();
    test_GdipFillClosedCurveI();
    test_GdipDrawString();
    test_GdipGetNearestColor();
    test_GdipGetVisibleClipBounds();
    test_GdipIsVisiblePoint();
    test_GdipIsVisibleRect();
    test_Get_Release_DC();
    test_BeginContainer2();
    test_transformpoints();
    test_get_set_clip();
    test_isempty();
    test_clear();
    test_textcontrast();
    test_fromMemoryBitmap();
#if CORE_6659_IS_FIXED
    test_string_functions();
#endif
    test_get_set_interpolation();
    test_get_set_textrenderinghint();
    test_getdc_scaled();
    test_alpha_hdc();
    test_bitmapfromgraphics();

    GdiplusShutdown(gdiplusToken);
    DestroyWindow( hwnd );
}
