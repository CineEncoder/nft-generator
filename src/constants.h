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

enum Theme {
    GRAY, DARK, WAVE, DEFAULT
};

enum ToolBarAction {
    AddFolder, Renew, Previous, Next, ExportImages, ExportJson,
    Settings
};

#endif // CONSTANTS_H
