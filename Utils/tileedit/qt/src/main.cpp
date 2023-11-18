#include "tileedit.h"
#include <QtWidgets/QApplication>
#include <QtPlugin>

#ifdef STATIC
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	tileedit w {argv[0]};
	w.show();
	return a.exec();
}
