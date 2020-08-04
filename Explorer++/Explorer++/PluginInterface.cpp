// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "Explorer++.h"
#include "UiTheming.h"

TabContainer *Explorerplusplus::GetTabContainer()
{
	return m_tabContainer;
}

Navigation *Explorerplusplus::GetNavigation()
{
	return m_navigation.get();
}

Plugins::PluginMenuManager *Explorerplusplus::GetPluginMenuManager()
{
	return &m_pluginMenuManager;
}

UiTheming *Explorerplusplus::GetUiTheming()
{
	return m_uiTheming.get();
}

AcceleratorUpdater *Explorerplusplus::GetAccleratorUpdater()
{
	return &m_acceleratorUpdater;
}

Plugins::PluginCommandManager *Explorerplusplus::GetPluginCommandManager()
{
	return &m_pluginCommandManager;
}