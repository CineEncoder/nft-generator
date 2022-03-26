#include "dialog.h"
#include "ui_dialog.h"
#include "message.h"



Dialog::Dialog(QWidget *parent):
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QHeaderView *header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setParameters()
{

}

void Dialog::on_pushButton_Apply_clicked()
{

    this->accept();
}

void Dialog::on_pushButton_Cancel_clicked()
{
    this->close();
}

void Dialog::on_pushButton_AddPair_clicked()
{

}

void Dialog::on_pushButton_RemovePair_clicked()
{

}

void Dialog::showMessage(const QString &message)
{
    Message messageWindow(this);
    messageWindow.setMessage(message);
    messageWindow.setModal(true);
    messageWindow.exec();
}
