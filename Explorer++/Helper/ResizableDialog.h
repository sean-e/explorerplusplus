// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "Macros.h"
#include <list>

/* Within a resizable dialog, controls
either: move, resize, or hold the same
size and position.
For controls that do move/resize, they
may be constrained along one axis.
For example, a particular control may
resize/move horizontally, but not vertically. */
class ResizableDialog
{
public:
	enum class ControlType
	{
		Move,
		Resize
	};

	enum class ControlConstraint
	{
		None,
		X,
		Y
	};

	struct Control
	{
		int iID;
		ControlType Type;
		ControlConstraint Constraint;
	};

	ResizableDialog(HWND hDlg, const std::list<Control> &controlList);

	void UpdateControls(int iWidth, int iHeight);

private:
	DISALLOW_COPY_AND_ASSIGN(ResizableDialog);

	struct ControlInternal
	{
		int iID;
		ControlType Type;
		ControlConstraint Constraint;

		int iWidthDelta;
		int iHeightDelta;
	};

	const HWND m_hDlg;
	std::list<ControlInternal> m_ControlList;
};