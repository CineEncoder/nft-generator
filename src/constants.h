#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QSettings>
#define APP "NFTGenerator"
#define NAME "NFTGenerator"

#define SETTINGS(settings) QSettings settings(QSettings::NativeFormat, QSettings::UserScope, APP, NAME);


enum ProgressMode {
    SAVE, GENERATE
};

enum TypeOfObject {
    FILES, FOLDERS
};

enum Direction {
    PREVIOUS, NEXT
};

enum FileDialogType {
    LOADFILE, SAVEFILE, OPENFOLDER
};

#endif // CONSTANTS_H
