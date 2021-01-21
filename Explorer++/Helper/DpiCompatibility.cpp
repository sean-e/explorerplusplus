// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "DpiCompatibility.h"
#include <wil/win32_helpers.h>

DpiCompatibility &DpiCompatibility::GetInstance()
{
	static DpiCompatibility dpiCompatibility;
	return dpiCompatibility;
}

DpiCompatibility::DpiCompatibility() :
	m_SystemParametersInfoForDpi(nullptr),
	m_GetSystemMetricsForDpi(nullptr),
	m_GetDpiForWindow(nullptr)
{
	m_user32.reset(LoadLibrary(L"user32.dll"));

	if (m_user32)
	{
		m_SystemParametersInfoForDpi =
			GetProcAddressByFunctionDeclaration(m_user32.get(), SystemParametersInfoForDpi);
		m_GetSystemMetricsForDpi =
			GetProcAddressByFunctionDeclaration(m_user32.get(), GetSystemMetricsForDpi);
		m_GetDpiForWindow = GetProcAddressByFunctionDeclaration(m_user32.get(), GetDpiForWindow);
	}
}

BOOL DpiCompatibility::SystemParametersInfoForDpi(
	UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi)
{
	if (m_SystemParametersInfoForDpi)
	{
		return m_SystemParametersInfoForDpi(uiAction, uiParam, pvParam, fWinIni, dpi);
	}

	return SystemParametersInfo(uiAction, uiParam, pvParam, fWinIni);
}

int DpiCompatibility::GetSystemMetricsForDpi(int nIndex, UINT dpi)
{
	if (m_GetSystemMetricsForDpi)
	{
		return m_GetSystemMetricsForDpi(nIndex, dpi);
	}

	return GetSystemMetrics(nIndex);
}

UINT DpiCompatibility::GetDpiForWindow(HWND hwnd)
{
	if (m_GetDpiForWindow)
	{
		return m_GetDpiForWindow(hwnd);
	}

	auto hdc = wil::GetDC(hwnd);

	if (hdc)
	{
		return GetDeviceCaps(hdc.get(), LOGPIXELSX);
	}

	return USER_DEFAULT_SCREEN_DPI;
}