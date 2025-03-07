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

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadSettingsFromHomeFolder();
private:
    Ui::MainWindow *ui;
    ScreenSampler *sampler;
    MulticastStreamer *mCastStreamer;
    bool isSampling;
    qint32 x,y,w,h,s,q;
    int nh, nw;
    QList<QNetworkInterface> interfaces;
    QList<QString> mCastIps;

    int iTest=0, jTest=0;

private slots:
    void startStopButtonClick();
    void updateScreenPreview();
    void paramsUpdated();
    //void on_comboBox_currentIndexChanged(int index);
    //void on_mCastComboBox_currentIndexChanged(int index);
    void on_sendTextPushButton_clicked();
    void on_nicComboBox_currentIndexChanged(int index);
    void on_streamComboBox_currentIndexChanged(int index);
};
#endif // MAINWINDOW_H
