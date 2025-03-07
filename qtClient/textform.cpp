#include "textform.h"
#include "ui_textform.h"

TextForm::TextForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TextForm)
{
    ui->setupUi(this);
}

TextForm::~TextForm()
{
    delete ui;
}

void TextForm::setText(QString s)
{
    ui->textEdit->setPlainText(s);
}
