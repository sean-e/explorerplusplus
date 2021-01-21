// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "Tab.h"
#include "Config.h"
#include "CoreInterface.h"
#include "PreservedTab.h"
#include "ShellBrowser/FolderSettings.h"
#include "ShellBrowser/ShellBrowser.h"
#include "ShellBrowser/ShellNavigationController.h"
#include <wil/resource.h>

int Tab::idCounter = 1;

Tab::Tab(IExplorerplusplus *expp, TabNavigationInterface *tabNavigation,
	FileActionHandler *fileActionHandler, const FolderSettings *folderSettings,
	std::optional<FolderColumns> initialColumns) :
	m_id(idCounter++),
	m_useCustomName(false),
	m_lockState(LockState::NotLocked)
{
	FolderSettings folderSettingsFinal;

	if (folderSettings)
	{
		folderSettingsFinal = *folderSettings;
	}
	else
	{
		folderSettingsFinal = expp->GetConfig()->defaultFolderSettings;
	}

	m_shellBrowser = ShellBrowser::CreateNew(m_id, expp->GetMainWindow(), expp, tabNavigation,
		fileActionHandler, folderSettingsFinal, initialColumns);
}

Tab::Tab(const PreservedTab &preservedTab, IExplorerplusplus *expp,
	TabNavigationInterface *tabNavigation, FileActionHandler *fileActionHandler) :
	m_id(idCounter++),
	m_useCustomName(preservedTab.useCustomName),
	m_customName(preservedTab.customName),
	m_lockState(preservedTab.lockState)
{
	m_shellBrowser = ShellBrowser::CreateFromPreserved(m_id, expp->GetMainWindow(), expp,
		tabNavigation, fileActionHandler, preservedTab.history, preservedTab.currentEntry,
		preservedTab.preservedFolderState);
}

int Tab::GetId() const
{
	return m_id;
}

ShellBrowser *Tab::GetShellBrowser() const
{
	return m_shellBrowser;
}

// If a custom name has been set, that will be returned. Otherwise, the
// display name of the current directory will be returned.
std::wstring Tab::GetName() const
{
	if (m_useCustomName)
	{
		return m_customName;
	}

	auto pidlDirectory = m_shellBrowser->GetDirectoryIdl();

	std::wstring name;
	HRESULT hr = GetDisplayName(pidlDirectory.get(), SHGDN_INFOLDER, name);

	if (FAILED(hr))
	{
		return L"(Unknown)";
	}

	return name;
}

bool Tab::GetUseCustomName() const
{
	return m_useCustomName;
}

void Tab::SetCustomName(const std::wstring &name)
{
	m_useCustomName = true;
	m_customName = name;

	m_tabUpdatedSignal(*this, PropertyType::Name);
}

void Tab::ClearCustomName()
{
	m_useCustomName = false;
	m_customName.erase();

	m_tabUpdatedSignal(*this, PropertyType::Name);
}

Tab::LockState Tab::GetLockState() const
{
	return m_lockState;
}

void Tab::SetLockState(LockState lockState)
{
	if (lockState == m_lockState)
	{
		return;
	}

	m_lockState = lockState;

	switch (lockState)
	{
	case Tab::LockState::NotLocked:
		m_shellBrowser->GetNavigationController()->SetNavigationMode(
			ShellNavigationController::NavigationMode::Normal);
		break;

	case Tab::LockState::Locked:
	case Tab::LockState::AddressLocked:
		m_shellBrowser->GetNavigationController()->SetNavigationMode(
			ShellNavigationController::NavigationMode::ForceNewTab);
		break;
	}

	m_tabUpdatedSignal(*this, PropertyType::LockState);
}

boost::signals2::connection Tab::AddTabUpdatedObserver(const TabUpdatedSignal::slot_type &observer)
{
	return m_tabUpdatedSignal.connect(observer);
}