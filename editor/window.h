/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "qt/ui_window.h"
#include "render_system.h"

class Window : public QMainWindow
{
	Q_OBJECT

public:
	Window();

public Q_SLOTS:
	void openDialogBox();
	void exportDialogBox();
	void saveDialogBox();
	void reportIssue();

private:
	Ui::Window widget;
	RenderSystem r;
};

#endif /* WINDOW_H */
