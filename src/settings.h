#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
    class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr, int *theme = nullptr);
    ~Settings();

private slots:
    void onApplyClicked();

private:
    Ui::Settings *ui;
    int *ptr_theme;
};

#endif // SETTINGS_H
