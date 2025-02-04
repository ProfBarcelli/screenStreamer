#ifndef TEXTFORM_H
#define TEXTFORM_H

#include <QWidget>

namespace Ui {
class TextForm;
}

class TextForm : public QWidget
{
    Q_OBJECT

public:
    explicit TextForm(QWidget *parent = nullptr);
    ~TextForm();

    void setText(QString);
private:
    Ui::TextForm *ui;
};

#endif // TEXTFORM_H
