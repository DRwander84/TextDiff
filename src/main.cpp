#include "MainWindow.h"

#include <QApplication>
#include <QIcon>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("TextDiff");
    QApplication::setOrganizationName("TextDiff");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication::setWindowIcon(QIcon(":/icons/logo-256.png"));

    MainWindow window;
    window.show();

    return app.exec();
}
