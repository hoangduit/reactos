#ifndef _MSDMO_PCH_
#define _MSDMO_PCH_

#include <stdarg.h>

#define WIN32_NO_STATUS
#define _INC_WINDOWS
#define COM_NO_WINDOWS_H

#define COBJMACROS

#include <windef.h>
#include <winbase.h>
#include <objbase.h>

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(msdmo);

#endif /* _MSDMO_PCH_ */
