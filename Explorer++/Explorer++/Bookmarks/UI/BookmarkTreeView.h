// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "Bookmarks/BookmarkHelper.h"
#include "Bookmarks/BookmarkItem.h"
#include "Bookmarks/UI/BookmarkDropTargetWindow.h"
#include "ResourceHelper.h"
#include "SignalWrapper.h"
#include "../Helper/DpiCompatibility.h"
#include "../Helper/WindowSubclassWrapper.h"
#include <boost/signals2.hpp>
#include <wil/resource.h>
#include <optional>
#include <unordered_map>
#include <unordered_set>

class BookmarkTree;
__interface IExplorerplusplus;

class BookmarkTreeView : private BookmarkDropTargetWindow
{
public:
	BookmarkTreeView(HWND hTreeView, HINSTANCE hInstance, IExplorerplusplus *expp,
		BookmarkTree *bookmarkTree, const std::unordered_set<std::wstring> &setExpansion,
		std::optional<std::wstring> guidSelected = std::nullopt);

	BookmarkItem *GetBookmarkFolderFromTreeView(HTREEITEM hItem);

	void CreateNewFolder();
	void SelectFolder(const std::wstring &guid);
	bool CanDelete();
	void DeleteSelection();

	// Signals
	SignalWrapper<BookmarkTreeView, void(BookmarkItem *bookmarkFolder)> selectionChangedSignal;

private:
	using ItemMap_t = std::unordered_map<std::wstring, HTREEITEM>;

	static inline const UINT_PTR SUBCLASS_ID = 0;
	static inline const UINT_PTR PARENT_SUBCLASS_ID = 0;

	static inline const double FOLDER_CENTRAL_RECT_INDENT_PERCENTAGE = 0.2;

	static LRESULT CALLBACK BookmarkTreeViewProcStub(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	LRESULT CALLBACK TreeViewProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK BookmarkTreeViewParentProcStub(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	LRESULT CALLBACK TreeViewParentProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK TreeViewEditProcStub(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	LRESULT CALLBACK TreeViewEditProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	void SetupTreeView(const std::unordered_set<std::wstring> &setExpansion,
		std::optional<std::wstring> guidSelected);

	void InsertFoldersIntoTreeViewRecursive(HTREEITEM hParent, BookmarkItem *bookmarkItem);
	HTREEITEM InsertFolderIntoTreeView(
		HTREEITEM hParent, BookmarkItem *bookmarkFolder, int position);

	void OnKeyDown(const NMTVKEYDOWN *pnmtvkd);
	void OnTreeViewRename();
	BOOL OnBeginLabelEdit(const NMTVDISPINFO *dispInfo);
	BOOL OnEndLabelEdit(const NMTVDISPINFO *dispInfo);
	void OnSelChanged(const NMTREEVIEW *treeView);
	void OnBeginDrag(const NMTREEVIEW *treeView);

	void OnRClick(const NMHDR *pnmhdr);

	void OnBookmarkItemAdded(BookmarkItem &bookmarkItem, size_t index);
	void OnBookmarkItemUpdated(BookmarkItem &bookmarkItem, BookmarkItem::PropertyType propertyType);
	void OnBookmarkItemMoved(BookmarkItem *bookmarkItem, const BookmarkItem *oldParent,
		size_t oldIndex, const BookmarkItem *newParent, size_t newIndex);
	void OnBookmarkItemPreRemoval(BookmarkItem &bookmarkItem);

	HTREEITEM AddNewFolderToTreeView(BookmarkItem *bookmarkFolder);
	size_t GetFolderRelativeIndex(BookmarkItem *bookmarkFolder) const;
	void RemoveBookmarkItem(const BookmarkItem *bookmarkItem);

	DropLocation GetDropLocation(const POINT &pt) override;
	HTREEITEM FindNextItem(const POINT &ptClient) const;
	void UpdateUiForDropLocation(const DropLocation &dropLocation) override;
	void ResetDropUiState() override;
	void RemoveInsertionMark();
	void RemoveDropHighlight();

	HWND m_hTreeView;
	DpiCompatibility m_dpiCompat;
	wil::unique_himagelist m_imageList;
	IconImageListMapping m_imageListMappings;

	HINSTANCE m_instance;

	BookmarkTree *m_bookmarkTree;

	ItemMap_t m_mapItem;

	bool m_bNewFolderCreated;
	std::wstring m_NewFolderGUID;

	std::optional<HTREEITEM> m_previousDropItem;

	std::vector<std::unique_ptr<WindowSubclassWrapper>> m_windowSubclasses;
	std::vector<boost::signals2::scoped_connection> m_connections;
};