#ifndef PROGRESSMESSAGE_H
#define PROGRESSMESSAGE_H

#include <QDialog>
#include <QFontMetrics>
#include <math.h>
#include "constants.h"

namespace Ui {
class ProgressMessage;
}

class ProgressMessage : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressMessage(QWidget *parent = nullptr);
    ~ProgressMessage();

    void setType(QWidget *parent, const int windowType);
    void setText(const QString &text);
    void setPercent(const int &percent);

signals:
    void saveAborted();

private:
    Ui::ProgressMessage *ui;

private slots:
    void onCancelClicked();
};

#endif // PROGRESSMESSAGE_H
