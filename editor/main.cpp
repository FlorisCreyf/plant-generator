#include "widgets/window.h"
#include <QApplication>
#include <QStyleFactory>

#define PG_SERIALIZE

int main(int argc, char *argv[])
{
	QSurfaceFormat format;
	format.setVersion(4, 3);
	format.setSamples(4);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setAlphaBufferSize(4);
	QSurfaceFormat::setDefaultFormat(format);

	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication app(argc, argv);

	Window window(argc, argv);
	window.show();

	return app.exec();
}
