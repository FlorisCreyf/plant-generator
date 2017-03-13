#include "window.h"
#include <QApplication>
#include <QtGui/QTabBar>

int main(int argc, char *argv[])
{
	QSurfaceFormat format;
	format.setVersion(4, 3);
	format.setSamples(4);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
	format.setAlphaBufferSize(8);
	QSurfaceFormat::setDefaultFormat(format);

	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);

	Window window;
	window.show();

	return app.exec();
}
