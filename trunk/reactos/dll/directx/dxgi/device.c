/*
 * Copyright 2008 Henri Verbeet for CodeWeavers
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
 *
 */

#include "config.h"
#include "wine/port.h"

#include "dxgi_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(dxgi);

static inline struct dxgi_device *impl_from_IWineDXGIDevice(IWineDXGIDevice *iface)
{
    return CONTAINING_RECORD(iface, struct dxgi_device, IWineDXGIDevice_iface);
}

/* IUnknown methods */

static HRESULT STDMETHODCALLTYPE dxgi_device_QueryInterface(IWineDXGIDevice *iface, REFIID riid, void **object)
{
    struct dxgi_device *This = impl_from_IWineDXGIDevice(iface);

    TRACE("iface %p, riid %s, object %p\n", iface, debugstr_guid(riid), object);

    if (IsEqualGUID(riid, &IID_IUnknown)
            || IsEqualGUID(riid, &IID_IDXGIObject)
            || IsEqualGUID(riid, &IID_IDXGIDevice)
            || IsEqualGUID(riid, &IID_IWineDXGIDevice))
    {
        IUnknown_AddRef(iface);
        *object = iface;
        return S_OK;
    }

    if (This->child_layer)
    {
        TRACE("forwarding to child layer %p\n", This->child_layer);
        return IUnknown_QueryInterface(This->child_layer, riid, object);
    }

    WARN("%s not implemented, returning E_NOINTERFACE\n", debugstr_guid(riid));

    *object = NULL;
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE dxgi_device_AddRef(IWineDXGIDevice *iface)
{
    struct dxgi_device *This = impl_from_IWineDXGIDevice(iface);
    ULONG refcount = InterlockedIncrement(&This->refcount);

    TRACE("%p increasing refcount to %u\n", This, refcount);

    return refcount;
}

static ULONG STDMETHODCALLTYPE dxgi_device_Release(IWineDXGIDevice *iface)
{
    struct dxgi_device *This = impl_from_IWineDXGIDevice(iface);
    ULONG refcount = InterlockedDecrement(&This->refcount);

    TRACE("%p decreasing refcount to %u\n", This, refcount);

    if (!refcount)
    {
        if (This->child_layer) IUnknown_Release(This->child_layer);
        EnterCriticalSection(&dxgi_cs);
        wined3d_device_decref(This->wined3d_device);
        LeaveCriticalSection(&dxgi_cs);
        IWineDXGIFactory_Release(This->factory);
        HeapFree(GetProcessHeap(), 0, This);
    }

    return refcount;
}

/* IDXGIObject methods */

static HRESULT STDMETHODCALLTYPE dxgi_device_SetPrivateData(IWineDXGIDevice *iface,
        REFGUID guid, UINT data_size, const void *data)
{
    FIXME("iface %p, guid %s, data_size %u, data %p stub!\n", iface, debugstr_guid(guid), data_size, data);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_SetPrivateDataInterface(IWineDXGIDevice *iface,
        REFGUID guid, const IUnknown *object)
{
    FIXME("iface %p, guid %s, object %p stub!\n", iface, debugstr_guid(guid), object);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_GetPrivateData(IWineDXGIDevice *iface,
        REFGUID guid, UINT *data_size, void *data)
{
    FIXME("iface %p, guid %s, data_size %p, data %p stub!\n", iface, debugstr_guid(guid), data_size, data);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_GetParent(IWineDXGIDevice *iface, REFIID riid, void **parent)
{
    IDXGIAdapter *adapter;
    HRESULT hr;

    TRACE("iface %p, riid %s, parent %p.\n", iface, debugstr_guid(riid), parent);

    hr = IWineDXGIDevice_GetAdapter(iface, &adapter);
    if (FAILED(hr))
    {
        ERR("Failed to get adapter, hr %#x.\n", hr);
        return hr;
    }

    hr = IDXGIAdapter_QueryInterface(adapter, riid, parent);
    IDXGIAdapter_Release(adapter);

    return hr;
}

/* IDXGIDevice methods */

static HRESULT STDMETHODCALLTYPE dxgi_device_GetAdapter(IWineDXGIDevice *iface, IDXGIAdapter **adapter)
{
    struct dxgi_device *This = impl_from_IWineDXGIDevice(iface);
    struct wined3d_device_creation_parameters create_parameters;

    TRACE("iface %p, adapter %p\n", iface, adapter);

    EnterCriticalSection(&dxgi_cs);
    wined3d_device_get_creation_parameters(This->wined3d_device, &create_parameters);
    LeaveCriticalSection(&dxgi_cs);

    return IWineDXGIFactory_EnumAdapters(This->factory, create_parameters.adapter_idx, adapter);
}

static HRESULT STDMETHODCALLTYPE dxgi_device_CreateSurface(IWineDXGIDevice *iface,
        const DXGI_SURFACE_DESC *desc, UINT surface_count, DXGI_USAGE usage,
        const DXGI_SHARED_RESOURCE *shared_resource, IDXGISurface **surface)
{
    struct wined3d_device_parent *device_parent;
    IWineDXGIDeviceParent *dxgi_device_parent;
    HRESULT hr;
    UINT i;
    UINT j;

    TRACE("iface %p, desc %p, surface_count %u, usage %#x, shared_resource %p, surface %p\n",
            iface, desc, surface_count, usage, shared_resource, surface);

    hr = IWineDXGIDevice_QueryInterface(iface, &IID_IWineDXGIDeviceParent, (void **)&dxgi_device_parent);
    if (FAILED(hr))
    {
        ERR("Device should implement IWineD3DDeviceParent\n");
        return E_FAIL;
    }

    device_parent = IWineDXGIDeviceParent_get_wined3d_device_parent(dxgi_device_parent);

    FIXME("Implement DXGI<->wined3d usage conversion\n");

    memset(surface, 0, surface_count * sizeof(*surface));
    for (i = 0; i < surface_count; ++i)
    {
        struct wined3d_surface *wined3d_surface;
        IUnknown *parent;

        hr = device_parent->ops->create_swapchain_surface(device_parent, NULL,
                desc->Width, desc->Height, wined3dformat_from_dxgi_format(desc->Format), usage,
                desc->SampleDesc.Count > 1 ? desc->SampleDesc.Count : WINED3D_MULTISAMPLE_NONE,
                desc->SampleDesc.Quality, &wined3d_surface);
        if (FAILED(hr))
        {
            ERR("CreateSurface failed, returning %#x\n", hr);
            goto fail;
        }

        parent = wined3d_surface_get_parent(wined3d_surface);
        hr = IUnknown_QueryInterface(parent, &IID_IDXGISurface, (void **)&surface[i]);
        wined3d_surface_decref(wined3d_surface);
        if (FAILED(hr))
        {
            ERR("Surface should implement IDXGISurface\n");
            goto fail;
        }

        TRACE("Created IDXGISurface %p (%u/%u)\n", surface[i], i + 1, surface_count);
    }
    IWineDXGIDeviceParent_Release(dxgi_device_parent);

    return S_OK;

fail:
    for (j = 0; j < i; ++j)
    {
        IDXGISurface_Release(surface[i]);
    }
    IWineDXGIDeviceParent_Release(dxgi_device_parent);
    return hr;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_QueryResourceResidency(IWineDXGIDevice *iface,
        IUnknown *const *resources, DXGI_RESIDENCY *residency, UINT resource_count)
{
    FIXME("iface %p, resources %p, residency %p, resource_count %u stub!\n",
            iface, resources, residency, resource_count);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_SetGPUThreadPriority(IWineDXGIDevice *iface, INT priority)
{
    FIXME("iface %p, priority %d stub!\n", iface, priority);

    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_GetGPUThreadPriority(IWineDXGIDevice *iface, INT *priority)
{
    FIXME("iface %p, priority %p stub!\n", iface, priority);

    return E_NOTIMPL;
}

/* IWineDXGIDevice methods */

static struct wined3d_device * STDMETHODCALLTYPE dxgi_device_get_wined3d_device(IWineDXGIDevice *iface)
{
    struct dxgi_device *This = impl_from_IWineDXGIDevice(iface);

    TRACE("iface %p\n", iface);

    EnterCriticalSection(&dxgi_cs);
    wined3d_device_incref(This->wined3d_device);
    LeaveCriticalSection(&dxgi_cs);
    return This->wined3d_device;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_create_surface(IWineDXGIDevice *iface, const DXGI_SURFACE_DESC *desc,
        DXGI_USAGE usage, const DXGI_SHARED_RESOURCE *shared_resource, IUnknown *outer, void **surface)
{
    struct dxgi_surface *object;
    HRESULT hr;

    FIXME("iface %p, desc %p, usage %#x, shared_resource %p, outer %p, surface %p partial stub!\n",
            iface, desc, usage, shared_resource, outer, surface);

    object = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*object));
    if (!object)
    {
        ERR("Failed to allocate DXGI surface object memory\n");
        return E_OUTOFMEMORY;
    }

    hr = dxgi_surface_init(object, (IDXGIDevice *)iface, outer);
    if (FAILED(hr))
    {
        WARN("Failed to initialize surface, hr %#x.\n", hr);
        HeapFree(GetProcessHeap(), 0, object);
        return hr;
    }

    TRACE("Created IDXGISurface %p\n", object);
    *surface = outer ? &object->IUnknown_iface : (IUnknown *)&object->IDXGISurface_iface;

    return S_OK;
}

static HRESULT STDMETHODCALLTYPE dxgi_device_create_swapchain(IWineDXGIDevice *iface,
        struct wined3d_swapchain_desc *desc, struct wined3d_swapchain **wined3d_swapchain)
{
    struct dxgi_device *This = impl_from_IWineDXGIDevice(iface);
    struct dxgi_swapchain *object;
    HRESULT hr;

    TRACE("iface %p, desc %p, wined3d_swapchain %p.\n",
            iface, desc, wined3d_swapchain);

    object = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*object));
    if (!object)
    {
        ERR("Failed to allocate DXGI swapchain object memory\n");
        return E_OUTOFMEMORY;
    }

    hr = dxgi_swapchain_init(object, This, desc);
    if (FAILED(hr))
    {
        WARN("Failed to initialize swapchain, hr %#x.\n", hr);
        HeapFree(GetProcessHeap(), 0, object);
        return hr;
    }

    TRACE("Created IDXGISwapChain %p\n", object);
    *wined3d_swapchain = object->wined3d_swapchain;

    return S_OK;
}

static const struct IWineDXGIDeviceVtbl dxgi_device_vtbl =
{
    /* IUnknown methods */
    dxgi_device_QueryInterface,
    dxgi_device_AddRef,
    dxgi_device_Release,
    /* IDXGIObject methods */
    dxgi_device_SetPrivateData,
    dxgi_device_SetPrivateDataInterface,
    dxgi_device_GetPrivateData,
    dxgi_device_GetParent,
    /* IDXGIDevice methods */
    dxgi_device_GetAdapter,
    dxgi_device_CreateSurface,
    dxgi_device_QueryResourceResidency,
    dxgi_device_SetGPUThreadPriority,
    dxgi_device_GetGPUThreadPriority,
    /* IWineDXGIAdapter methods */
    dxgi_device_get_wined3d_device,
    dxgi_device_create_surface,
    dxgi_device_create_swapchain,
};

HRESULT dxgi_device_init(struct dxgi_device *device, struct dxgi_device_layer *layer,
        IDXGIFactory *factory, IDXGIAdapter *adapter)
{
    struct wined3d_device_parent *wined3d_device_parent;
    IWineDXGIDeviceParent *dxgi_device_parent;
    IWineDXGIAdapter *wine_adapter;
    UINT adapter_ordinal;
    struct wined3d *wined3d;
    void *layer_base;
    HRESULT hr;
    WINED3DCAPS caps;

    device->IWineDXGIDevice_iface.lpVtbl = &dxgi_device_vtbl;
    device->refcount = 1;

    layer_base = device + 1;

    hr = layer->create(layer->id, &layer_base, 0,
            device, &IID_IUnknown, (void **)&device->child_layer);
    if (FAILED(hr))
    {
        WARN("Failed to create device, returning %#x.\n", hr);
        goto fail;
    }

    hr = IDXGIFactory_QueryInterface(factory, &IID_IWineDXGIFactory, (void **)&device->factory);
    if (FAILED(hr))
    {
        WARN("This is not the factory we're looking for, returning %#x.\n", hr);
        goto fail;
    }
    wined3d = IWineDXGIFactory_get_wined3d(device->factory);

    hr = IDXGIAdapter_QueryInterface(adapter, &IID_IWineDXGIAdapter, (void **)&wine_adapter);
    if (FAILED(hr))
    {
        WARN("This is not the adapter we're looking for, returning %#x.\n", hr);
        EnterCriticalSection(&dxgi_cs);
        wined3d_decref(wined3d);
        LeaveCriticalSection(&dxgi_cs);
        goto fail;
    }
    adapter_ordinal = IWineDXGIAdapter_get_ordinal(wine_adapter);
    IWineDXGIAdapter_Release(wine_adapter);

    hr = IWineDXGIDevice_QueryInterface(&device->IWineDXGIDevice_iface, &IID_IWineDXGIDeviceParent,
            (void **)&dxgi_device_parent);
    if (FAILED(hr))
    {
        ERR("DXGI device should implement IWineD3DDeviceParent.\n");
        goto fail;
    }

    wined3d_device_parent = IWineDXGIDeviceParent_get_wined3d_device_parent(dxgi_device_parent);

    FIXME("Ignoring adapter type.\n");

    hr = wined3d_get_device_caps(wined3d, adapter_ordinal, WINED3D_DEVICE_TYPE_HAL, &caps);
    if (FAILED(hr) || caps.VertexShaderVersion < 4 || caps.PixelShaderVersion < 4)
    {
        WARN("Direct3D 10 is not supported on this GPU with the current shader backend.\n");
        if (SUCCEEDED(hr))
            hr = E_FAIL;
        goto fail;
    }

    EnterCriticalSection(&dxgi_cs);
    hr = wined3d_device_create(wined3d, adapter_ordinal, WINED3D_DEVICE_TYPE_HAL, NULL, 0, 4,
            wined3d_device_parent, &device->wined3d_device);
    IWineDXGIDeviceParent_Release(dxgi_device_parent);
    wined3d_decref(wined3d);
    LeaveCriticalSection(&dxgi_cs);
    if (FAILED(hr))
    {
        WARN("Failed to create a wined3d device, returning %#x.\n", hr);
        goto fail;
    }

    return S_OK;

fail:
    if (device->wined3d_device)
    {
        EnterCriticalSection(&dxgi_cs);
        wined3d_device_decref(device->wined3d_device);
        LeaveCriticalSection(&dxgi_cs);
    }
    if (device->factory) IWineDXGIFactory_Release(device->factory);
    if (device->child_layer) IUnknown_Release(device->child_layer);
    return hr;
}
