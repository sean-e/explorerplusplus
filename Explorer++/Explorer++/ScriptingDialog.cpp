// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "ScriptingDialog.h"
#include "MainResource.h"
#include "Plugins/Manifest.h"
#include "../Helper/StringHelper.h"
#include "../Helper/WindowHelper.h"
#include <boost/algorithm/string.hpp>

ScriptingDialog::ScriptingDialog(
	HINSTANCE hInstance, HWND hParent, PluginInterface *pluginInterface) :
	DarkModeDialogBase(hInstance, IDD_SCRIPTING, hParent, true),
	m_luaPlugin(L"", Plugins::Manifest(), pluginInterface)
{
	m_luaPlugin.GetLuaState().open_libraries(sol::lib::base);
}

INT_PTR ScriptingDialog::OnInitDialog()
{
	HWND commandControl = GetDlgItem(m_hDlg, IDC_COMMAND);
	SendMessage(m_hDlg, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(commandControl), TRUE);

	AllowDarkModeForControls({ ID_RUN });

	return FALSE;
}

void ScriptingDialog::GetResizableControlInformation(
	BaseDialog::DialogSizeConstraint &dsc, std::list<ResizableDialog::Control> &ControlList)
{
	dsc = BaseDialog::DialogSizeConstraint::None;

	ResizableDialog::Control control;

	control.iID = IDC_LOG;
	control.Type = ResizableDialog::ControlType::Resize;
	control.Constraint = ResizableDialog::ControlConstraint::None;
	ControlList.push_back(control);

	control.iID = IDC_COMMAND;
	control.Type = ResizableDialog::ControlType::Resize;
	control.Constraint = ResizableDialog::ControlConstraint::X;
	ControlList.push_back(control);

	control.iID = IDC_COMMAND;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::Y;
	ControlList.push_back(control);

	control.iID = IDC_STATIC_COMMAND_LABEL;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::Y;
	ControlList.push_back(control);

	control.iID = ID_RUN;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::None;
	ControlList.push_back(control);
}

INT_PTR ScriptingDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	if (HIWORD(wParam) == 0)
	{
		switch (LOWORD(wParam))
		{
		case ID_RUN:
			OnRun();
			break;
		}
	}

	return 0;
}

void ScriptingDialog::OnRun()
{
	HWND commandControl = GetDlgItem(m_hDlg, IDC_COMMAND);

	std::wstring command = GetWindowString(commandControl);

	std::wstring result;

	try
	{
		std::string convertedCommand = wstrToStr(command);
		auto protectedResult = m_luaPlugin.GetLuaState().safe_script(convertedCommand);

		result = FormatResult(protectedResult);
	}
	catch (const sol::error &e)
	{
		result = strToWstr(e.what());
	}

	boost::replace_all(result, _T("\n"), _T("\r\n"));
	AppendToLog(command, result);

	SetWindowText(commandControl, _T(""));
}

std::wstring ScriptingDialog::FormatResult(const sol::protected_function_result &result)
{
	switch (result.get_type())
	{
	case sol::type::boolean:
	{
		bool boolResult = result.get<bool>();
		return std::to_wstring(boolResult);
	}

	default:
		return result.get<std::wstring>();
	}
}

void ScriptingDialog::AppendToLog(const std::wstring &command, const std::wstring &result)
{
	HWND logControl = GetDlgItem(m_hDlg, IDC_LOG);

	int logLength = GetWindowTextLength(logControl);
	SendMessage(logControl, EM_SETSEL, logLength, logLength);

	std::wstring log;

	if (logLength > 0)
	{
		log += _T("\r\n");
	}

	log += command;

	if (!result.empty())
	{
		log += _T("\r\n") + result;
	}

	SendMessage(logControl, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(log.c_str()));
}

INT_PTR ScriptingDialog::OnClose()
{
	DestroyWindow(m_hDlg);
	return 0;
}

INT_PTR ScriptingDialog::OnNcDestroy()
{
	delete this;

	return 0;
}