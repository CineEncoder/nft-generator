#ifndef MESSAGE_H
#define MESSAGE_H

#include <QDialog>


namespace Ui {
    class Message;
}

class Message : public QDialog
{
    Q_OBJECT

public:

    explicit Message(QWidget *parent = nullptr);
    ~Message();

    void setMessage(const QString &message);

private slots:

    void onButtonApply_clicked();

private:

    Ui::Message *ui;
};



#endif // MESSAGE_H
