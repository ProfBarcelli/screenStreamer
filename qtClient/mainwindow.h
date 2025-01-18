#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QHostAddress>
#include <QImage>
#include <QList>

#define N_PARTS_X 10
#define N_PARTS_Y 10

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void processPendingDatagrams();

    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QUdpSocket *udpSocket4=NULL;
    QHostAddress *groupAddress4=NULL;
    /*QByteArray data;
    int old_i, n;*/
    QImage *wholeImage;
    QList<QString> mCastIps;
    int mCastIpIndex = 100;
    void initMcast();
};
#endif // MAINWINDOW_H
