// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "Explorer++.h"
#include "ShellBrowser/ShellBrowser.h"
#include "ShellTreeView/ShellTreeView.h"
#include "TabContainer.h"

BOOL Explorerplusplus::AnyItemsSelected() const
{
	HWND hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		const Tab &selectedTab = m_tabContainer->GetSelectedTab();

		if (ListView_GetSelectedCount(selectedTab.GetShellBrowser()->GetListView()) > 0)
		{
			return TRUE;
		}
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		if (TreeView_GetSelection(m_shellTreeView->GetHWND()) != nullptr)
		{
			return TRUE;
		}
	}

	return FALSE;
}

bool Explorerplusplus::CanCreate() const
{
	const Tab &selectedTab = m_tabContainer->GetSelectedTab();
	auto pidlDirectory = selectedTab.GetShellBrowser()->GetDirectoryIdl();

	SFGAOF attributes = SFGAO_FILESYSTEM;
	HRESULT hr = GetItemAttributes(pidlDirectory.get(), &attributes);

	if (FAILED(hr))
	{
		return false;
	}

	if ((attributes & SFGAO_FILESYSTEM) == SFGAO_FILESYSTEM)
	{
		return true;
	}

	// Library folders aren't filesystem folders, but they act like them
	// (e.g. they allow items to be created, copied and moved) and
	// ultimately they're backed by filesystem folders. If this is a
	// library folder, file creation will be allowed.
	return IsChildOfLibrariesFolder(pidlDirectory.get());
}

BOOL Explorerplusplus::CanCut() const
{
	return TestItemAttributes(SFGAO_CANMOVE);
}

BOOL Explorerplusplus::CanCopy() const
{
	return TestItemAttributes(SFGAO_CANCOPY);
}

BOOL Explorerplusplus::CanRename() const
{
	return TestItemAttributes(SFGAO_CANRENAME);
}

BOOL Explorerplusplus::CanDelete() const
{
	return TestItemAttributes(SFGAO_CANDELETE);
}

BOOL Explorerplusplus::CanShowFileProperties() const
{
	return TestItemAttributes(SFGAO_HASPROPSHEET);
}

/* Returns TRUE if all the specified attributes are set on the selected items. */
BOOL Explorerplusplus::TestItemAttributes(SFGAOF attributes) const
{
	SFGAOF commonAttributes = attributes;
	HRESULT hr = GetSelectionAttributes(&commonAttributes);

	if (SUCCEEDED(hr))
	{
		return (commonAttributes & attributes) == attributes;
	}

	return FALSE;
}

HRESULT Explorerplusplus::GetSelectionAttributes(SFGAOF *pItemAttributes) const
{
	HWND hFocus;
	HRESULT hr = E_FAIL;

	hFocus = GetFocus();

	if (hFocus == m_hActiveListView)
	{
		const Tab &selectedTab = m_tabContainer->GetSelectedTab();
		hr = selectedTab.GetShellBrowser()->GetListViewSelectionAttributes(pItemAttributes);
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		hr = GetTreeViewSelectionAttributes(pItemAttributes);
	}

	return hr;
}

HRESULT Explorerplusplus::GetTreeViewSelectionAttributes(SFGAOF *pItemAttributes) const
{
	HRESULT hr = E_FAIL;
	auto hItem = TreeView_GetSelection(m_shellTreeView->GetHWND());

	if (hItem != nullptr)
	{
		auto pidl = m_shellTreeView->GetItemPidl(hItem);
		hr = GetItemAttributes(pidl.get(), pItemAttributes);
	}

	return hr;
}

BOOL Explorerplusplus::CanPaste() const
{
	HWND hFocus = GetFocus();

	std::list<FORMATETC> ftcList;
	DropHandler::GetDropFormats(ftcList);

	BOOL bDataAvailable = FALSE;

	/* Check whether the drop source has the type of data
	that is needed for this drag operation. */
	for (const auto &ftc : ftcList)
	{
		if (IsClipboardFormatAvailable(ftc.cfFormat))
		{
			bDataAvailable = TRUE;
			break;
		}
	}

	if (hFocus == m_hActiveListView)
	{
		return bDataAvailable && CanCreate();
	}
	else if (hFocus == m_shellTreeView->GetHWND())
	{
		auto hItem = TreeView_GetSelection(m_shellTreeView->GetHWND());

		if (hItem != nullptr)
		{
			auto pidl = m_shellTreeView->GetItemPidl(hItem);

			SFGAOF attributes = SFGAO_FILESYSTEM;
			HRESULT hr = GetItemAttributes(pidl.get(), &attributes);

			if (hr == S_OK)
			{
				return bDataAvailable;
			}
		}
	}

	return FALSE;
}