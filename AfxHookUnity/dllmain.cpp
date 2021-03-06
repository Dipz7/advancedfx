#include "stdafx.h"

#include <windows.h>

#include <shared/Detours/src/detours.h>

#include <d3d11.h>

#include <tchar.h>


LONG error = NO_ERROR;
HMODULE hD3d11Dll = NULL;

typedef HRESULT(STDMETHODCALLTYPE * CreateRenderTargetView_t)(
	ID3D11Device * This,
	_In_  ID3D11Resource *pResource,
	_In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
	_COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView);

CreateRenderTargetView_t TrueCreateRenderTargetView = NULL;

HRESULT STDMETHODCALLTYPE MyCreateRenderTargetView(
	ID3D11Device * This,
	_In_  ID3D11Resource *pResource,
	_In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
	_COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView)
{
	MessageBox(0, _T(";-)"), _T(":-O"), MB_OK);

	return TrueCreateRenderTargetView(This, pResource, pDesc, ppRTView);
}



typedef HRESULT(WINAPI * D3D11CreateDevice_t)(
	_In_opt_        IDXGIAdapter        *pAdapter,
	D3D_DRIVER_TYPE     DriverType,
	HMODULE             Software,
	UINT                Flags,
	_In_opt_  const D3D_FEATURE_LEVEL   *pFeatureLevels,
	UINT                FeatureLevels,
	UINT                SDKVersion,
	_Out_opt_       ID3D11Device        **ppDevice,
	_Out_opt_       D3D_FEATURE_LEVEL   *pFeatureLevel,
	_Out_opt_       ID3D11DeviceContext **ppImmediateContext
);

D3D11CreateDevice_t TrueD3D11CreateDevice = NULL;

HRESULT WINAPI MyD3D11CreateDevice(
	_In_opt_        IDXGIAdapter        *pAdapter,
	D3D_DRIVER_TYPE     DriverType,
	HMODULE             Software,
	UINT                Flags,
	_In_opt_  const D3D_FEATURE_LEVEL   *pFeatureLevels,
	UINT                FeatureLevels,
	UINT                SDKVersion,
	_Out_opt_       ID3D11Device        **ppDevice,
	_Out_opt_       D3D_FEATURE_LEVEL   *pFeatureLevel,
	_Out_opt_       ID3D11DeviceContext **ppImmediateContext
) {
	HRESULT result = TrueD3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	if (IS_ERROR(result) || NULL == ppDevice)
	{
		error = E_FAIL;
		return result;
	}

	if (NULL != TrueCreateRenderTargetView)
		return result;

	void ** pCreateRenderTargetView = (void **)((*(char **)(*ppDevice)) + sizeof(void *)*9);

	TrueCreateRenderTargetView = (CreateRenderTargetView_t)*pCreateRenderTargetView; // poi(poi(poi(ppDevice))+48)

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)TrueCreateRenderTargetView, MyCreateRenderTargetView);
	error = DetourTransactionCommit();	

	/*
	DWORD oldProtect;

	if (!VirtualProtect(pCreateRenderTargetView, sizeof(void *), PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		error = E_FAIL;
		return result;
	}

	if (NULL != ppImmediateContext) MessageBox(0, _T(";-)"), _T(":-O"), MB_OK | MB_ICONERROR);

	*pCreateRenderTargetView = MyCreateRenderTargetView;

	VirtualProtect(pCreateRenderTargetView, sizeof(void *), oldProtect, NULL);
	*/

	return result;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		{
			hD3d11Dll = LoadLibrary(_T("d3d11.dll"));

			TrueD3D11CreateDevice = (D3D11CreateDevice_t)GetProcAddress(hD3d11Dll,"D3D11CreateDevice");

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)TrueD3D11CreateDevice, MyD3D11CreateDevice);
			error = DetourTransactionCommit();

		}
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(PVOID&)TrueD3D11CreateDevice, MyD3D11CreateDevice);
			error = DetourTransactionCommit();

			FreeLibrary(hD3d11Dll);
		}
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) bool AfxHookUnityStatus(void) {
	return NO_ERROR == error;
}
