/* TreeMaker: 3D tree model editor
 * Copyright (C) 2016-2017  Floris Creyf
 *
 * TreeMaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TreeMaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window.h"
#include "property_box.h"
#include "curve_editor.h"
#include <QFileDialog>

Window::Window()
{
	editor = new Editor(&shared, this);
	this->setCentralWidget(editor);

	auto areas = Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea;

	QDockWidget *dockWidget = new QDockWidget(tr("Properties"), this);
        dockWidget->setAllowedAreas(areas);
        PropertyBox *propertyBox = new PropertyBox(&shared, this);
        dockWidget->setWidget(propertyBox);
        this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);

	dockWidget = new QDockWidget(tr("Curve"), this);
	dockWidget->setAllowedAreas(areas);
	CurveEditor *curveEditor = new CurveEditor(&shared, this);
	dockWidget->setWidget(curveEditor);
	this->addDockWidget(static_cast<Qt::DockWidgetArea>(1), dockWidget);

	propertyBox->bind(editor, curveEditor);
	widget.setupUi(this);

	QMenu *menu = createPopupMenu();
	menu->setTitle("Window");
	menuBar()->insertMenu(widget.menuHelp->menuAction(), menu);
	connect(widget.actionReportIssue, SIGNAL(triggered()), this,
			SLOT(reportIssue()));
}

void Window::openDialogBox()
{
	QString filename;
	filename = QFileDialog::getOpenFileName(this, tr("Open File"), "",
		tr("TreeMaker (*.tree)"));
}

void Window::saveDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Save File"),
		"untitled.tree", tr("TreeMaker (*.tree)"));
}

void Window::exportDialogBox()
{
	QString filename;
	filename = QFileDialog::getSaveFileName(this, tr("Export File"),
		"untitled.obj", tr("Wavefront (*.obj)"));
	QByteArray b = filename.toLatin1();
	editor->exportObject(b.data());
}

void Window::reportIssue()
{
	QString link = "https://github.com/FlorisCreyf/treemaker/issues";
	QDesktopServices::openUrl(QUrl(link));
}
