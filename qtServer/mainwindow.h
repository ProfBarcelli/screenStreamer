#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "screensampler.h"
#include "multicaststreamer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    int nh, nw;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ScreenSampler *sampler;
    MulticastStreamer *mCastStreamer;
    bool isSampling;
    qint32 x,y,w,h,s,q;

private slots:
    void startStopButtonClick();
    void updateScreenPreview();
    void paramsUpdated();
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H