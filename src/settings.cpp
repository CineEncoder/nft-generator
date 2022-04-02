#include "settings.h"
#include "ui_settings.h"


Settings::Settings(QWidget *parent, int *theme) :
    QDialog(parent),
    ui(new Ui::Settings),
    ptr_theme(theme)
{
    ui->setupUi(this);
    connect(ui->buttonApply, &QPushButton::clicked, this, &Settings::onApplyClicked);
    ui->comboBox_theme->setCurrentIndex(*ptr_theme);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::onApplyClicked()
{
    *ptr_theme = ui->comboBox_theme->currentIndex();
    this->accept();
}
