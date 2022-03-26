#ifndef GENERATOR_H
#define GENERATOR_H

#include <QApplication>
#include <QObject>
#include <QList>
#include <QString>
#include <QDirIterator>
#include <iostream>
#include <random>
#include "constants.h"

using std::sort;
using std::cout;

struct NumPaths {
    int num;
    QString fullPath;
};

class Generator: public QObject
{
    Q_OBJECT

public:
    explicit Generator(QObject *parent = nullptr);
    ~Generator();

    QList<QList<NumPaths>> generateCollection(const int generatedCount,
                                              const QList<QList<NumPaths>> &totalPathsList);

    QList<NumPaths> getFilesOrFolders(const QString &folder,
                                      const int typeOfObject);
signals:
    void progressChanged(const int value, const int total);
};

#endif // GENERATOR_H
