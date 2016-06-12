/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include "window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QSurfaceFormat format;
	format.setVersion(3, 3);
	format.setSamples(4);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
	QSurfaceFormat::setDefaultFormat(format);

	QApplication app(argc, argv);

	Window window;
	window.show();

	return app.exec();
}
