#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGlobal>
#include <QShowEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsOpacityEffect>
#include <QWheelEvent>
#include <QScrollBar>
#include <QFileDialog>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QDataStream>
#include <iostream>
#include <math.h>
#include <ctime>
#include "progressmessage.h"
#include "generator.h"
#include "constants.h"

#if defined (Q_OS_UNIX)
    #include <unistd.h>
#elif defined(Q_OS_WIN64)
    #include <windows.h>
#endif

using std::cout;


QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    friend QDataStream& (operator<<)(QDataStream &out, const NumPaths &path);
    friend QDataStream& (operator>>)(QDataStream &in, NumPaths &path);

private slots:

    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void onAddFolderClicked();
    void onSaveCollectionClicked();
    void onSaveJsonClicked();
    void onSaveProjectAsClicked();
    void onLoadProjectClicked();
    void setOutputFolder();
    void onLockARClicked();

    QString openFileDialog(const QString &title, int dialogType);
    QString styleCreator(const QString &list);

private:

    QList<QString> layers;
    QList<QList<NumPaths>> generatedPathsList;

    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    Generator *generator;
    ProgressMessage *ptr_progress;

    bool window_activated_flag,
         lock_ar_flag,
         folder_added_flag;

    int  current_image,
         maxPossibleCount;

    float aspectRatio;

    QString input_folder,
            output_folder,
            project_folder;

    void setConnections();
    void setParameters();
    void fillLayersTable(const QStringList &fullPaths);
    void fillValuesTable(const QList<NumPaths> &numFiles);
    void showScene(const QList<NumPaths> &numFiles);
    void showMessage(const QString &message);
    void renewFolder(const QString &folder,
                     QList<QList<NumPaths>> &totalPathsList,
                     QStringList &paths,
                     bool &correctSequenceFlag);
    void renewData(const QString &folder, const QStringList &paths);
    void regenerateCollection(const QString &folder);
    void scrollImages(const int direction);
};

#endif // MAINWINDOW_H
