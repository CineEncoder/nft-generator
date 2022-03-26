#include "progressmessage.h"
#include "ui_progressmessage.h"
#include "message.h"

ProgressMessage::ProgressMessage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressMessage)
{
    ui->setupUi(this);
    setModal(true);
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::SubWindow);
    connect(ui->buttonCancel, &QPushButton::clicked, this, &ProgressMessage::onCancelClicked);   
}

ProgressMessage::~ProgressMessage()
{
    delete ui;
}

void ProgressMessage::setType(QWidget *parent, const int windowType)
{
    if (windowType == ProgressMode::SAVE) {
        ui->label_title->setWindowTitle("Save Files");
        ui->label_title->setText("Save Files");
        ui->label_name->setText("Filename:");
        ui->buttonCancel->show();
    } else {
        ui->label_title->setWindowTitle("Generating");
        ui->label_title->setText("Generating");
        ui->label_name->setText("Number:");
        ui->buttonCancel->hide();
    }
    const QPoint posMainWindow = parent->pos();
    const QSize sizeMainWindow = parent->size();
    const int position_x = posMainWindow.x() + static_cast<int>(round(static_cast<float>(sizeMainWindow.width())/2));
    const int position_y = posMainWindow.y() + static_cast<int>(round(static_cast<float>(sizeMainWindow.height())/2));
    const QSize sizeWindow = this->size();
    const int x_pos = position_x - static_cast<int>(round(static_cast<float>(sizeWindow.width())/2));
    const int y_pos = position_y - static_cast<int>(round(static_cast<float>(sizeWindow.height())/2));
    this->move(x_pos, y_pos);
}

void ProgressMessage::setText(const QString &text)
{
    QFontMetrics fm = ui->label_filename->fontMetrics();
#if (QT_VERSION < QT_VERSION_CHECK(5,11,0))
    const int fwidth = fm.width(text);
#else
    const int fwidth = fm.horizontalAdvance(text);
#endif
    const int width = ui->label_filename->width();
    const QString elidedText = (fwidth > width) ? fm.elidedText(text, Qt::ElideMiddle, width, 0) : text;
    ui->label_filename->setText(elidedText);
}

void ProgressMessage::setPercent(const int &percent)
{
    ui->progressBar->setValue(percent);
}

void ProgressMessage::onCancelClicked()
{
    emit saveAborted();
}
