#include "message.h"
#include "ui_message.h"


Message::Message(QWidget *parent):
    QDialog(parent),
    ui(new Ui::Message)
{
    ui->setupUi(this);
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->buttonApply, &QPushButton::clicked, this, &Message::onButtonApply_clicked);
}

Message::~Message()
{
    delete ui;
}

void Message::onButtonApply_clicked()
{
    this->close();
}

void Message::setMessage(const QString &message)
{
    ui->textBrowser_message->clear();
    ui->textBrowser_message->setAlignment(Qt::AlignCenter);
    ui->textBrowser_message->append(message);
    QTextCursor textCursor = ui->textBrowser_message->textCursor();
    textCursor.movePosition(QTextCursor::Start);
    ui->textBrowser_message->setTextCursor(textCursor);
}
