/*
 * Copyright 2006 Stefan Dösinger
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

#include "ddraw_private.h"

/*****************************************************************************
 * IDirectDrawPalette::QueryInterface
 *
 * A usual QueryInterface implementation. Can only Query IUnknown and
 * IDirectDrawPalette
 *
 * Params:
 *  refiid: The interface id queried for
 *  obj: Address to return the interface pointer at
 *
 * Returns:
 *  S_OK on success
 *  E_NOINTERFACE if the requested interface wasn't found
 *****************************************************************************/
static HRESULT WINAPI ddraw_palette_QueryInterface(IDirectDrawPalette *iface, REFIID refiid, void **obj)
{
    TRACE("iface %p, riid %s, object %p.\n", iface, debugstr_guid(refiid), obj);

    if (IsEqualGUID(refiid, &IID_IUnknown)
        || IsEqualGUID(refiid, &IID_IDirectDrawPalette))
    {
        *obj = iface;
        IDirectDrawPalette_AddRef(iface);
        return S_OK;
    }
    else
    {
        *obj = NULL;
        return E_NOINTERFACE;
    }
}

/*****************************************************************************
 * IDirectDrawPaletteImpl::AddRef
 *
 * Increases the refcount.
 *
 * Returns:
 *  The new refcount
 *
 *****************************************************************************/
static ULONG WINAPI ddraw_palette_AddRef(IDirectDrawPalette *iface)
{
    struct ddraw_palette *This = impl_from_IDirectDrawPalette(iface);
    ULONG ref = InterlockedIncrement(&This->ref);

    TRACE("%p increasing refcount to %u.\n", This, ref);

    return ref;
}

/*****************************************************************************
 * IDirectDrawPaletteImpl::Release
 *
 * Reduces the refcount. If the refcount falls to 0, the object is destroyed
 *
 * Returns:
 *  The new refcount
 *
 *****************************************************************************/
static ULONG WINAPI ddraw_palette_Release(IDirectDrawPalette *iface)
{
    struct ddraw_palette *This = impl_from_IDirectDrawPalette(iface);
    ULONG ref = InterlockedDecrement(&This->ref);

    TRACE("%p decreasing refcount to %u.\n", This, ref);

    if (ref == 0)
    {
        wined3d_mutex_lock();
        wined3d_palette_decref(This->wineD3DPalette);
        if(This->ifaceToRelease)
        {
            IUnknown_Release(This->ifaceToRelease);
        }
        wined3d_mutex_unlock();

        HeapFree(GetProcessHeap(), 0, This);
    }

    return ref;
}

/*****************************************************************************
 * IDirectDrawPalette::Initialize
 *
 * Initializes the palette. As we start initialized, return
 * DDERR_ALREADYINITIALIZED
 *
 * Params:
 *  DD: DirectDraw interface this palette is assigned to
 *  Flags: Some flags, as usual
 *  ColorTable: The startup color table
 *
 * Returns:
 *  DDERR_ALREADYINITIALIZED
 *
 *****************************************************************************/
static HRESULT WINAPI ddraw_palette_Initialize(IDirectDrawPalette *iface,
        IDirectDraw *ddraw, DWORD flags, PALETTEENTRY *entries)
{
    TRACE("iface %p, ddraw %p, flags %#x, entries %p.\n",
            iface, ddraw, flags, entries);

    return DDERR_ALREADYINITIALIZED;
}

/*****************************************************************************
 * IDirectDrawPalette::GetCaps
 *
 * Returns the palette description
 *
 * Params:
 *  Caps: Address to store the caps at
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS if Caps is NULL
 *  For more details, see IWineD3DPalette::GetCaps
 *
 *****************************************************************************/
static HRESULT WINAPI ddraw_palette_GetCaps(IDirectDrawPalette *iface, DWORD *caps)
{
    struct ddraw_palette *palette = impl_from_IDirectDrawPalette(iface);

    TRACE("iface %p, caps %p.\n", iface, caps);

    wined3d_mutex_lock();
    *caps = wined3d_palette_get_flags(palette->wineD3DPalette);
    wined3d_mutex_unlock();

    return D3D_OK;
}

/*****************************************************************************
 * IDirectDrawPalette::SetEntries
 *
 * Sets the palette entries from a PALETTEENTRY structure. WineD3D takes
 * care for updating the surface.
 *
 * Params:
 *  Flags: Flags, as usual
 *  Start: First palette entry to set
 *  Count: Number of entries to set
 *  PalEnt: Source entries
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS if PalEnt is NULL
 *  For details, see IWineD3DDevice::SetEntries
 *
 *****************************************************************************/
static HRESULT WINAPI ddraw_palette_SetEntries(IDirectDrawPalette *iface,
        DWORD flags, DWORD start, DWORD count, PALETTEENTRY *entries)
{
    struct ddraw_palette *palette = impl_from_IDirectDrawPalette(iface);
    HRESULT hr;

    TRACE("iface %p, flags %#x, start %u, count %u, entries %p.\n",
            iface, flags, start, count, entries);

    if (!entries)
        return DDERR_INVALIDPARAMS;

    wined3d_mutex_lock();
    hr = wined3d_palette_set_entries(palette->wineD3DPalette, flags, start, count, entries);
    wined3d_mutex_unlock();

    return hr;
}

/*****************************************************************************
 * IDirectDrawPalette::GetEntries
 *
 * Returns the entries stored in this interface.
 *
 * Params:
 *  Flags: Flags :)
 *  Start: First entry to return
 *  Count: The number of entries to return
 *  PalEnt: PALETTEENTRY structure to write the entries to
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS if PalEnt is NULL
 *  For details, see IWineD3DDevice::SetEntries
 *
 *****************************************************************************/
static HRESULT WINAPI ddraw_palette_GetEntries(IDirectDrawPalette *iface,
        DWORD flags, DWORD start, DWORD count, PALETTEENTRY *entries)
{
    struct ddraw_palette *palette = impl_from_IDirectDrawPalette(iface);
    HRESULT hr;

    TRACE("iface %p, flags %#x, start %u, count %u, entries %p.\n",
            iface, flags, start, count, entries);

    if (!entries)
        return DDERR_INVALIDPARAMS;

    wined3d_mutex_lock();
    hr = wined3d_palette_get_entries(palette->wineD3DPalette, flags, start, count, entries);
    wined3d_mutex_unlock();

    return hr;
}

static const struct IDirectDrawPaletteVtbl ddraw_palette_vtbl =
{
    /*** IUnknown ***/
    ddraw_palette_QueryInterface,
    ddraw_palette_AddRef,
    ddraw_palette_Release,
    /*** IDirectDrawPalette ***/
    ddraw_palette_GetCaps,
    ddraw_palette_GetEntries,
    ddraw_palette_Initialize,
    ddraw_palette_SetEntries
};

struct ddraw_palette *unsafe_impl_from_IDirectDrawPalette(IDirectDrawPalette *iface)
{
    if (!iface) return NULL;
    assert(iface->lpVtbl == &ddraw_palette_vtbl);
    return CONTAINING_RECORD(iface, struct ddraw_palette, IDirectDrawPalette_iface);
}

HRESULT ddraw_palette_init(struct ddraw_palette *palette,
        struct ddraw *ddraw, DWORD flags, PALETTEENTRY *entries)
{
    HRESULT hr;

    palette->IDirectDrawPalette_iface.lpVtbl = &ddraw_palette_vtbl;
    palette->ref = 1;

    hr = wined3d_palette_create(ddraw->wined3d_device, flags,
            entries, palette, &palette->wineD3DPalette);
    if (FAILED(hr))
    {
        WARN("Failed to create wined3d palette, hr %#x.\n", hr);
        return hr;
    }

    palette->ifaceToRelease = (IUnknown *)&ddraw->IDirectDraw7_iface;
    IUnknown_AddRef(palette->ifaceToRelease);

    return DD_OK;
}
