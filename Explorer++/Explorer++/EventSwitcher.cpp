// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

/*
 * Switches events based on the currently selected window
 * (principally the listview and treeview).
 */

#include "stdafx.h"
#include "Explorer++.h"
#include "Explorer++_internal.h"
#include "ShellBrowser/ShellBrowser.h"
#include "ShellTreeView/ShellTreeView.h"
#include "TabContainer.h"

void Explorerplusplus::OnCopyItemPath() const
{
	HWND hFocus;

	hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		OnListViewCopyItemPath();
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		OnTreeViewCopyItemPath();
	}
}

void Explorerplusplus::OnCopyUniversalPaths() const
{
	HWND hFocus;

	hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		OnListViewCopyUniversalPaths();
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		OnTreeViewCopyUniversalPaths();
	}
}

void Explorerplusplus::OnCopy(BOOL bCopy)
{
	HWND hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		Tab &selectedTab = m_tabContainer->GetSelectedTab();
		selectedTab.GetShellBrowser()->CopySelectedItemToClipboard(bCopy);
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		m_shellTreeView->CopySelectedItemToClipboard(bCopy);
	}
}

void Explorerplusplus::OnFileRename()
{
	HWND hFocus;

	if (m_bListViewRenaming)
	{
		SendMessage(ListView_GetEditControl(m_hActiveListView), WM_APP_KEYDOWN, VK_F2, 0);
	}
	else
	{
		hFocus = GetFocus();

		if (hFocus == m_hActiveListView)
		{
			Tab &selectedTab = m_tabContainer->GetSelectedTab();
			selectedTab.GetShellBrowser()->StartRenamingSelectedItems();
		}
		else if (hFocus == m_shellTreeView->GetHWND())
		{
			m_shellTreeView->StartRenamingSelectedItem();
		}
	}
}

void Explorerplusplus::OnFileDelete(bool permanent)
{
	HWND hFocus;

	hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		Tab &tab = m_tabContainer->GetSelectedTab();
		tab.GetShellBrowser()->DeleteSelectedItems(permanent);
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		m_shellTreeView->DeleteSelectedItem(permanent);
	}
}

void Explorerplusplus::OnSetFileAttributes() const
{
	HWND hFocus;

	hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		OnListViewSetFileAttributes();
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		OnTreeViewSetFileAttributes();
	}
}

void Explorerplusplus::OnShowFileProperties() const
{
	HWND hFocus;

	hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		const Tab &selectedTab = m_tabContainer->GetSelectedTab();
		selectedTab.GetShellBrowser()->ShowPropertiesForSelectedFiles();
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		m_shellTreeView->ShowPropertiesOfSelectedItem();
	}
}

void Explorerplusplus::OnRightClick(NMHDR *nmhdr)
{
	if (nmhdr->hwndFrom == m_hActiveListView)
	{
		POINT cursorPos;
		DWORD dwPos;

		dwPos = GetMessagePos();
		cursorPos.x = GET_X_LPARAM(dwPos);
		cursorPos.y = GET_Y_LPARAM(dwPos);

		OnListViewRClick(&cursorPos);
	}
}

void Explorerplusplus::OnPaste()
{
	HWND hFocus;

	hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		OnListViewPaste();
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		m_shellTreeView->PasteClipboardData();
	}
}

BOOL Explorerplusplus::OnMouseWheel(MousewheelSource mousewheelSource, WPARAM wParam, LPARAM lParam)
{
	short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
	m_zDeltaTotal += zDelta;

	DWORD dwCursorPos = GetMessagePos();
	POINTS pts = MAKEPOINTS(dwCursorPos);

	POINT pt;
	pt.x = pts.x;
	pt.y = pts.y;

	HWND hwnd = WindowFromPoint(pt);

	BOOL bMessageHandled = FALSE;

	/* Normally, mouse wheel messages will be sent
	to the window with focus. We want to be able to
	scroll windows even if they do not have focus,
	so we'll capture the mouse wheel message and
	and forward it to the window currently underneath
	the mouse. */
	if (hwnd == m_hActiveListView)
	{
		if (wParam & MK_CONTROL)
		{
			Tab &selectedTab = m_tabContainer->GetSelectedTab();

			/* Switch listview views. For each wheel delta
			(notch) the wheel is scrolled through, switch
			the view once. */
			for (int i = 0; i < abs(m_zDeltaTotal / WHEEL_DELTA); i++)
			{
				selectedTab.GetShellBrowser()->CycleViewMode((m_zDeltaTotal > 0));
			}
		}
		else if (wParam & MK_SHIFT)
		{
			if (m_zDeltaTotal < 0)
			{
				for (int i = 0; i < abs(m_zDeltaTotal / WHEEL_DELTA); i++)
				{
					// TODO: Navigate directly to offset.
					OnGoBack();
				}
			}
			else
			{
				for (int i = 0; i < abs(m_zDeltaTotal / WHEEL_DELTA); i++)
				{
					OnGoForward();
				}
			}
		}
		else
		{
			if (mousewheelSource != MousewheelSource::ListView)
			{
				bMessageHandled = TRUE;
				SendMessage(m_hActiveListView, WM_MOUSEWHEEL, wParam, lParam);
			}
		}
	}
	else if (hwnd == m_shellTreeView->GetHWND())
	{
		if (mousewheelSource != MousewheelSource::TreeView)
		{
			bMessageHandled = TRUE;
			SendMessage(m_shellTreeView->GetHWND(), WM_MOUSEWHEEL, wParam, lParam);
		}
	}
	else if (hwnd == m_tabContainer->GetHWND())
	{
		bMessageHandled = TRUE;

		HWND hUpDown = FindWindowEx(m_tabContainer->GetHWND(), nullptr, UPDOWN_CLASS, nullptr);

		if (hUpDown != nullptr)
		{
			BOOL bSuccess;
			int iPos = static_cast<int>(
				SendMessage(hUpDown, UDM_GETPOS32, 0, reinterpret_cast<LPARAM>(&bSuccess)));

			if (bSuccess)
			{
				int iScrollPos = iPos;

				int iLow;
				int iHigh;
				SendMessage(hUpDown, UDM_GETRANGE32, reinterpret_cast<WPARAM>(&iLow),
					reinterpret_cast<LPARAM>(&iHigh));

				if (m_zDeltaTotal < 0)
				{
					if (iScrollPos < iHigh)
					{
						iScrollPos++;
					}
				}
				else
				{
					if (iScrollPos > iLow)
					{
						iScrollPos--;
					}
				}

				SendMessage(m_tabContainer->GetHWND(), WM_HSCROLL,
					MAKEWPARAM(SB_THUMBPOSITION, iScrollPos), NULL);
			}
		}
	}

	if (abs(m_zDeltaTotal) >= WHEEL_DELTA)
	{
		m_zDeltaTotal = m_zDeltaTotal % WHEEL_DELTA;
	}

	return bMessageHandled;
}