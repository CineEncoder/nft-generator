#include "mainwindow.h"
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    //QCoreApplication::setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles, true);
    QFont font = app.font();
    font.setPointSize(9);
    app.setFont(font);

    MainWindow window;
    window.show();
    return app.exec();
}
