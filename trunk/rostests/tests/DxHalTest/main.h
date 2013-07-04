#include <windows.h>
#include <ddrawi.h>
#include <d3dhal.h>


int StartupHAL (VOID);
extern HDC hdc;
extern   DDRAWI_DIRECTDRAW_GBL mDDrawGlobal;
  extern DDRAWI_DIRECTDRAW_LCL mDDrawLocal;
  extern DDHALINFO mHALInfo;

  extern DDHAL_CALLBACKS mCallbacks;
  extern DDHAL_DDEXEBUFCALLBACKS mD3dBufferCallbacks;
  extern D3DHAL_CALLBACKS mD3dCallbacks;
  extern D3DHAL_GLOBALDRIVERDATA mD3dDriverData;

  extern UINT mcModeInfos;
  extern DDHALMODEINFO *mpModeInfos;

  extern UINT mcvmList;
  extern VIDMEM *mpvmList;

  extern UINT mcFourCC;
  extern DWORD *mpFourCC;

  extern UINT mcTextures;
  extern DDSURFACEDESC *mpTextures;



BOOL (APIENTRY *DdCreateDirectDrawObject) (
    LPDDRAWI_DIRECTDRAW_GBL pDirectDrawGlobal,
    HDC hdc
);

BOOL (APIENTRY *DdQueryDirectDrawObject) (
    LPDDRAWI_DIRECTDRAW_GBL pDirectDrawGlobal,
    LPDDHALINFO pHalInfo,
    LPDDHAL_DDCALLBACKS pDDCallbacks,
    LPDDHAL_DDSURFACECALLBACKS pDDSurfaceCallbacks,
    LPDDHAL_DDPALETTECALLBACKS pDDPaletteCallbacks,
    LPD3DHAL_CALLBACKS pD3dCallbacks,
    LPD3DHAL_GLOBALDRIVERDATA pD3dDriverData,
    LPDDHAL_DDEXEBUFCALLBACKS pD3dBufferCallbacks,
    LPDDSURFACEDESC pD3dTextureFormats,
    LPDWORD pdwFourCC,
    LPVIDMEM pvmList
);

BOOL
APIENTRY
DdDeleteDirectDrawObject(
    LPDDRAWI_DIRECTDRAW_GBL pDirectDrawGlobal
);

BOOL
APIENTRY
DdCreateSurfaceObject(
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal,
    BOOL bPrimarySurface
);

BOOL
APIENTRY
DdDeleteSurfaceObject(
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal
);

//BOOL
//APIENTRY
//DdResetVisrgn(
//    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal,
//    HWND hWnd
//);
BOOL (APIENTRY *DdResetVisrgn) (
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal,
    HWND hWnd
);

HDC
APIENTRY
DdGetDC(
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal,
    LPPALETTEENTRY pColorTable
);

BOOL
APIENTRY
DdReleaseDC(
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal
);

HBITMAP
APIENTRY
DdCreateDIBSection(
    HDC hdc,
    CONST BITMAPINFO *pbmi,
    UINT iUsage,
    VOID **ppvBits,
    HANDLE hSectionApp,
    DWORD dwOffset
);

BOOL
APIENTRY
DdReenableDirectDrawObject(
    LPDDRAWI_DIRECTDRAW_GBL pDirectDrawGlobal,
    BOOL *pbNewMode
);

//BOOL
//APIENTRY
//DdAttachSurface(
//    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceFrom,
//    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceTo
//);

BOOL (APIENTRY *DdAttachSurface) (
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceFrom,
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceTo
);

VOID
APIENTRY
DdUnattachSurface(
    LPDDRAWI_DDRAWSURFACE_LCL pSurface,
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceAttached
);

ULONG
APIENTRY
DdQueryDisplaySettingsUniqueness(VOID);

HANDLE
APIENTRY
DdGetDxHandle(
    LPDDRAWI_DIRECTDRAW_LCL pDDraw,
    LPDDRAWI_DDRAWSURFACE_LCL pSurface,
    BOOL bRelease
);

BOOL
APIENTRY
DdSetGammaRamp(
    LPDDRAWI_DIRECTDRAW_LCL pDDraw,
    HDC hdc,
    LPVOID lpGammaRamp
);

DWORD
APIENTRY
DdSwapTextureHandles(
    LPDDRAWI_DIRECTDRAW_LCL pDDraw,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl1,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl2
);

char* DDErrorString (HRESULT hr)
{
	switch (hr)
	{
		case DD_OK:								 return "DD_OK";
		case DDERR_ALREADYINITIALIZED:           return "DDERR_ALREADYINITIALIZED";
		case DDERR_CANNOTATTACHSURFACE:          return "DDERR_CANNOTATTACHSURFACE";
		case DDERR_CANNOTDETACHSURFACE:          return "DDERR_CANNOTDETACHSURFACE";
		case DDERR_CURRENTLYNOTAVAIL:            return "DDERR_CURRENTLYNOTAVAIL";
		case DDERR_EXCEPTION:                    return "DDERR_EXCEPTION";
		case DDERR_GENERIC:                      return "DDERR_GENERIC";
		case DDERR_HEIGHTALIGN:                  return "DDERR_HEIGHTALIGN";
		case DDERR_INCOMPATIBLEPRIMARY:          return "DDERR_INCOMPATIBLEPRIMARY";
		case DDERR_INVALIDCAPS:                  return "DDERR_INVALIDCAPS";
		case DDERR_INVALIDCLIPLIST:              return "DDERR_INVALIDCLIPLIST";
		case DDERR_INVALIDMODE:                  return "DDERR_INVALIDMODE";
		case DDERR_INVALIDOBJECT:                return "DDERR_INVALIDOBJECT";
		case DDERR_INVALIDPARAMS:                return "DDERR_INVALIDPARAMS";
		case DDERR_INVALIDPIXELFORMAT:           return "DDERR_INVALIDPIXELFORMAT";
		case DDERR_INVALIDRECT:                  return "DDERR_INVALIDRECT";
		case DDERR_LOCKEDSURFACES:               return "DDERR_LOCKEDSURFACES";
		case DDERR_NO3D:                         return "DDERR_NO3D";
		case DDERR_NOALPHAHW:                    return "DDERR_NOALPHAHW";
		case DDERR_NOCLIPLIST:                   return "DDERR_NOCLIPLIST";
		case DDERR_NOCOLORCONVHW:                return "DDERR_NOCOLORCONVHW";
		case DDERR_NOCOOPERATIVELEVELSET:        return "DDERR_NOCOOPERATIVELEVELSET";
		case DDERR_NOCOLORKEY:                   return "DDERR_NOCOLORKEY";
		case DDERR_NOCOLORKEYHW:                 return "DDERR_NOCOLORKEYHW";
		case DDERR_NODIRECTDRAWSUPPORT:          return "DDERR_NODIRECTDRAWSUPPORT";
		case DDERR_NOEXCLUSIVEMODE:              return "DDERR_NOEXCLUSIVEMODE";
		case DDERR_NOFLIPHW:                     return "DDERR_NOFLIPHW";
		case DDERR_NOGDI:                        return "DDERR_NOGDI";
		case DDERR_NOMIRRORHW:                   return "DDERR_NOMIRRORHW";
		case DDERR_NOTFOUND:                     return "DDERR_NOTFOUND";
		case DDERR_NOOVERLAYHW:                  return "DDERR_NOOVERLAYHW";
		case DDERR_NORASTEROPHW:                 return "DDERR_NORASTEROPHW";
		case DDERR_NOROTATIONHW:                 return "DDERR_NOROTATIONHW";
		case DDERR_NOSTRETCHHW:                  return "DDERR_NOSTRETCHHW";
		case DDERR_NOT4BITCOLOR:                 return "DDERR_NOT4BITCOLOR";
		case DDERR_NOT4BITCOLORINDEX:            return "DDERR_NOT4BITCOLORINDEX";
		case DDERR_NOT8BITCOLOR:                 return "DDERR_NOT8BITCOLOR";
		case DDERR_NOTEXTUREHW:                  return "DDERR_NOTEXTUREHW";
		case DDERR_NOVSYNCHW:                    return "DDERR_NOVSYNCHW";
		case DDERR_NOZBUFFERHW:                  return "DDERR_NOZBUFFERHW";
		case DDERR_NOZOVERLAYHW:                 return "DDERR_NOZOVERLAYHW";
		case DDERR_OUTOFCAPS:                    return "DDERR_OUTOFCAPS";
		case DDERR_OUTOFMEMORY:                  return "DDERR_OUTOFMEMORY";
		case DDERR_OUTOFVIDEOMEMORY:             return "DDERR_OUTOFVIDEOMEMORY";
		case DDERR_OVERLAYCANTCLIP:              return "DDERR_OVERLAYCANTCLIP";
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
		case DDERR_PALETTEBUSY:                  return "DDERR_PALETTEBUSY";
		case DDERR_COLORKEYNOTSET:               return "DDERR_COLORKEYNOTSET";
		case DDERR_SURFACEALREADYATTACHED:       return "DDERR_SURFACEALREADYATTACHED";
		case DDERR_SURFACEALREADYDEPENDENT:      return "DDERR_SURFACEALREADYDEPENDENT";
		case DDERR_SURFACEBUSY:                  return "DDERR_SURFACEBUSY";
		case DDERR_CANTLOCKSURFACE:              return "DDERR_CANTLOCKSURFACE";
		case DDERR_SURFACEISOBSCURED:            return "DDERR_SURFACEISOBSCURED";
		case DDERR_SURFACELOST:                  return "DDERR_SURFACELOST";
		case DDERR_SURFACENOTATTACHED:           return "DDERR_SURFACENOTATTACHED";
		case DDERR_TOOBIGHEIGHT:                 return "DDERR_TOOBIGHEIGHT";
		case DDERR_TOOBIGSIZE:                   return "DDERR_TOOBIGSIZE";
		case DDERR_TOOBIGWIDTH:                  return "DDERR_TOOBIGWIDTH";
		case DDERR_UNSUPPORTED:                  return "DDERR_UNSUPPORTED";
		case DDERR_UNSUPPORTEDFORMAT:            return "DDERR_UNSUPPORTEDFORMAT";
		case DDERR_UNSUPPORTEDMASK:              return "DDERR_UNSUPPORTEDMASK";
		case DDERR_VERTICALBLANKINPROGRESS:      return "DDERR_VERTICALBLANKINPROGRESS";
		case DDERR_WASSTILLDRAWING:              return "DDERR_WASSTILLDRAWING";
		case DDERR_XALIGN:                       return "DDERR_XALIGN";
		case DDERR_INVALIDDIRECTDRAWGUID:        return "DDERR_INVALIDDIRECTDRAWGUID";
		case DDERR_DIRECTDRAWALREADYCREATED:     return "DDERR_DIRECTDRAWALREADYCREATED";
		case DDERR_NODIRECTDRAWHW:               return "DDERR_NODIRECTDRAWHW";
		case DDERR_PRIMARYSURFACEALREADYEXISTS:  return "DDERR_PRIMARYSURFACEALREADYEXISTS";
		case DDERR_NOEMULATION:                  return "DDERR_NOEMULATION";
		case DDERR_REGIONTOOSMALL:               return "DDERR_REGIONTOOSMALL";
		case DDERR_CLIPPERISUSINGHWND:           return "DDERR_CLIPPERISUSINGHWND";
		case DDERR_NOCLIPPERATTACHED:            return "DDERR_NOCLIPPERATTACHED";
		case DDERR_NOHWND:                       return "DDERR_NOHWND";
		case DDERR_HWNDSUBCLASSED:               return "DDERR_HWNDSUBCLASSED";
		case DDERR_HWNDALREADYSET:               return "DDERR_HWNDALREADYSET";
		case DDERR_NOPALETTEATTACHED:            return "DDERR_NOPALETTEATTACHED";
		case DDERR_NOPALETTEHW:                  return "DDERR_NOPALETTEHW";
		case DDERR_BLTFASTCANTCLIP:              return "DDERR_BLTFASTCANTCLIP";
		case DDERR_NOBLTHW:                      return "DDERR_NOBLTHW";
		case DDERR_NODDROPSHW:                   return "DDERR_NODDROPSHW";
		case DDERR_OVERLAYNOTVISIBLE:            return "DDERR_OVERLAYNOTVISIBLE";
		case DDERR_NOOVERLAYDEST:                return "DDERR_NOOVERLAYDEST";
		case DDERR_INVALIDPOSITION:              return "DDERR_INVALIDPOSITION";
		case DDERR_NOTAOVERLAYSURFACE:           return "DDERR_NOTAOVERLAYSURFACE";
		case DDERR_EXCLUSIVEMODEALREADYSET:      return "DDERR_EXCLUSIVEMODEALREADYSET";
		case DDERR_NOTFLIPPABLE:                 return "DDERR_NOTFLIPPABLE";
		case DDERR_CANTDUPLICATE:                return "DDERR_CANTDUPLICATE";
		case DDERR_NOTLOCKED:                    return "DDERR_NOTLOCKED";
		case DDERR_CANTCREATEDC:                 return "DDERR_CANTCREATEDC";
		case DDERR_NODC:                         return "DDERR_NODC";
		case DDERR_WRONGMODE:                    return "DDERR_WRONGMODE";
		case DDERR_IMPLICITLYCREATED:            return "DDERR_IMPLICITLYCREATED";
		case DDERR_NOTPALETTIZED:                return "DDERR_NOTPALETTIZED";
		case DDERR_UNSUPPORTEDMODE:              return "DDERR_UNSUPPORTEDMODE";
		case DDERR_NOMIPMAPHW:                   return "DDERR_NOMIPMAPHW";
		case DDERR_INVALIDSURFACETYPE:           return "DDERR_INVALIDSURFACETYPE";
		case DDERR_DCALREADYCREATED:             return "DDERR_DCALREADYCREATED";
		case DDERR_CANTPAGELOCK:                 return "DDERR_CANTPAGELOCK";
		case DDERR_CANTPAGEUNLOCK:               return "DDERR_CANTPAGEUNLOCK";
		case DDERR_NOTPAGELOCKED:                return "DDERR_NOTPAGELOCKED";
		case DDERR_NOTINITIALIZED:               return "DDERR_NOTINITIALIZED";
	}

	return "Unknown Error ";
}

