#pragma once
#include <comdef.h>
#include "serialize.h"


DECLARE_INTERFACE_IID_(IWzProperty, IWzSerialize, "986515d9-0a0b-4929-8b4f-718682177b92") {
	BEGIN_INTERFACE;

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppv) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	/*** IWzSerialize methods ***/
	STDMETHOD(get_persistentUOL)(THIS_ const wchar_t**) PURE;
	STDMETHOD(raw_Serialize)(THIS_ IWzArchive*) PURE;

	/*** IWzProperty methods ***/
	STDMETHOD(get_item)(THIS_ const wchar_t*, tagVARIANT*) PURE;
	STDMETHOD(put_item)(THIS_ const wchar_t*, tagVARIANT) PURE;
	STDMETHOD(get__NewEnum)(THIS_ IUnknown**) PURE;
	STDMETHOD(get_count)(THIS_ unsigned int*) PURE;
	STDMETHOD(raw_Add)(THIS_ const wchar_t*, tagVARIANT, tagVARIANT) PURE;
	STDMETHOD(raw_Remove)(THIS_ const wchar_t*) PURE;
	STDMETHOD(raw_Import)(THIS_ const wchar_t*) PURE;

	END_INTERFACE;
};
_COM_SMARTPTR_TYPEDEF(IWzProperty, __uuidof(IWzProperty));