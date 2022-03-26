#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>


namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:

    explicit Dialog(QWidget *parent = nullptr);

    ~Dialog();

    void setParameters();

private slots:

    void on_pushButton_Apply_clicked();
    void on_pushButton_Cancel_clicked();
    void on_pushButton_AddPair_clicked();
    void on_pushButton_RemovePair_clicked();

private:

    Ui::Dialog *ui;

    void showMessage(const QString &message);
};

#endif // DIALOG_H
