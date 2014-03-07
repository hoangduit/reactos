/*
 *    IEnumIDList
 *
 *    Copyright 1998    Juergen Schmied <juergen.schmied@metronet.de>
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

#include "precomp.h"

WINE_DEFAULT_DEBUG_CHANNEL(shell);

IEnumIDListImpl::IEnumIDListImpl()
{
    mpFirst = NULL;
    mpLast = NULL;
    mpCurrent = NULL;
}

IEnumIDListImpl::~IEnumIDListImpl()
{
}

/**************************************************************************
 *  AddToEnumList()
 */
BOOL IEnumIDListImpl::AddToEnumList(LPITEMIDLIST pidl)
{
    ENUMLIST *pNew;

    TRACE("(%p)->(pidl=%p)\n", this, pidl);

    if (!pidl)
        return FALSE;

    pNew = (ENUMLIST *)SHAlloc(sizeof(ENUMLIST));
    if (pNew)
    {
      /*set the next pointer */
      pNew->pNext = NULL;
      pNew->pidl = pidl;

      /*is This the first item in the list? */
      if (!mpFirst)
      {
        mpFirst = pNew;
        mpCurrent = pNew;
      }

      if (mpLast)
      {
        /*add the new item to the end of the list */
        mpLast->pNext = pNew;
      }

      /*update the last item pointer */
      mpLast = pNew;
      TRACE("-- (%p)->(first=%p, last=%p)\n", this, mpFirst, mpLast);
      return TRUE;
    }
    return FALSE;
}

/**************************************************************************
*   DeleteList()
*/
BOOL IEnumIDListImpl::DeleteList()
{
    ENUMLIST                    *pDelete;

    TRACE("(%p)->()\n", this);

    while (mpFirst)
    {
        pDelete = mpFirst;
        mpFirst = pDelete->pNext;
        SHFree(pDelete->pidl);
        SHFree(pDelete);
    }
    mpFirst = NULL;
    mpLast = NULL;
    mpCurrent = NULL;
    return TRUE;
}

/**************************************************************************
 *  HasItemWithCLSID()
 */
BOOL IEnumIDListImpl::HasItemWithCLSID(LPITEMIDLIST pidl)
{
    ENUMLIST *pCur;
    IID *ptr = _ILGetGUIDPointer(pidl);

    if (ptr)
    {
        REFIID refid = *ptr;
        pCur = mpFirst;

        while(pCur)
        {
            LPGUID curid = _ILGetGUIDPointer(pCur->pidl);
            if (curid && IsEqualGUID(*curid, refid))
            {
                return TRUE;
            }
            pCur = pCur->pNext;
        }
    }

    return FALSE;
}


/**************************************************************************
 *  CreateFolderEnumList()
 */
BOOL IEnumIDListImpl::CreateFolderEnumList(
    LPCWSTR lpszPath,
    DWORD dwFlags)
{
    WIN32_FIND_DATAW stffile;
    HANDLE hFile;
    WCHAR  szPath[MAX_PATH];
    BOOL succeeded = TRUE;
    static const WCHAR stars[] = { '*','.','*',0 };
    static const WCHAR dot[] = { '.',0 };
    static const WCHAR dotdot[] = { '.','.',0 };

    TRACE("(%p)->(path=%s flags=0x%08x)\n", this, debugstr_w(lpszPath), dwFlags);

    if(!lpszPath || !lpszPath[0]) return FALSE;

    wcscpy(szPath, lpszPath);
    PathAddBackslashW(szPath);
    wcscat(szPath,stars);

    hFile = FindFirstFileW(szPath,&stffile);
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        BOOL findFinished = FALSE;

        do
        {
            if ( !(stffile.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
             || (dwFlags & SHCONTF_INCLUDEHIDDEN) )
            {
                LPITEMIDLIST pidl = NULL;

                if ( (stffile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                 dwFlags & SHCONTF_FOLDERS &&
                 strcmpW(stffile.cFileName, dot) && strcmpW(stffile.cFileName, dotdot))
                {
                    pidl = _ILCreateFromFindDataW(&stffile);
                    succeeded = succeeded && AddToEnumList(pidl);
                }
                else if (!(stffile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                 && dwFlags & SHCONTF_NONFOLDERS)
                {
                    pidl = _ILCreateFromFindDataW(&stffile);
                    succeeded = succeeded && AddToEnumList(pidl);
                }
            }
            if (succeeded)
            {
                if (!FindNextFileW(hFile, &stffile))
                {
                    if (GetLastError() == ERROR_NO_MORE_FILES)
                        findFinished = TRUE;
                    else
                        succeeded = FALSE;
                }
            }
        } while (succeeded && !findFinished);
        FindClose(hFile);
    }

    return succeeded;
}

/**************************************************************************
 *  IEnumIDList_fnNext
 */

HRESULT WINAPI IEnumIDListImpl::Next(
    ULONG celt,
    LPITEMIDLIST * rgelt,
    ULONG *pceltFetched)
{
    ULONG    i;
    HRESULT  hr = S_OK;
    LPITEMIDLIST  temp;

    TRACE("(%p)->(%d,%p, %p)\n", this, celt, rgelt, pceltFetched);

/* It is valid to leave pceltFetched NULL when celt is 1. Some of explorer's
 * subsystems actually use it (and so may a third party browser)
 */
    if(pceltFetched)
      *pceltFetched = 0;

    *rgelt=0;

    if(celt > 1 && !pceltFetched)
    { return E_INVALIDARG;
    }

    if(celt > 0 && !mpCurrent)
    { return S_FALSE;
    }

    for(i = 0; i < celt; i++)
    { if(!mpCurrent)
        break;

      temp = ILClone(mpCurrent->pidl);
      rgelt[i] = temp;
      mpCurrent = mpCurrent->pNext;
    }
    if(pceltFetched)
    {  *pceltFetched = i;
    }

    return hr;
}

/**************************************************************************
*  IEnumIDList_fnSkip
*/
HRESULT WINAPI IEnumIDListImpl::Skip(
    ULONG celt)
{
    DWORD    dwIndex;
    HRESULT  hr = S_OK;

    TRACE("(%p)->(%u)\n", this, celt);

    for(dwIndex = 0; dwIndex < celt; dwIndex++)
    { if(!mpCurrent)
      { hr = S_FALSE;
        break;
      }
      mpCurrent = mpCurrent->pNext;
    }
    return hr;
}

/**************************************************************************
*  IEnumIDList_fnReset
*/
HRESULT WINAPI IEnumIDListImpl::Reset()
{
    TRACE("(%p)\n", this);
    mpCurrent = mpFirst;
    return S_OK;
}

/**************************************************************************
*  IEnumIDList_fnClone
*/
HRESULT WINAPI IEnumIDListImpl::Clone(LPENUMIDLIST *ppenum)
{
    TRACE("(%p)->() to (%p)->() E_NOTIMPL\n", this, ppenum);
    return E_NOTIMPL;
}

/**************************************************************************
 *  IEnumIDList_Folder_Constructor
 *
 */
HRESULT IEnumIDList_Constructor(IEnumIDList **enumerator)
{
    CComObject<IEnumIDListImpl>                *theEnumerator;
    CComPtr<IEnumIDList>                    result;
    HRESULT                                    hResult;

    if (enumerator == NULL)
        return E_POINTER;
    *enumerator = NULL;
    ATLTRY (theEnumerator = new CComObject<IEnumIDListImpl>);
    if (theEnumerator == NULL)
        return E_OUTOFMEMORY;
    hResult = theEnumerator->QueryInterface(IID_PPV_ARG(IEnumIDList, &result));
    if (FAILED (hResult))
    {
        delete theEnumerator;
        return hResult;
    }
    *enumerator = result.Detach ();
    return S_OK;
}
