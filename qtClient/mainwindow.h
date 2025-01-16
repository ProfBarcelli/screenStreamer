#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QHostAddress>
#include <QImage>

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

private:
    Ui::MainWindow *ui;
    QUdpSocket *udpSocket4;
    QHostAddress *groupAddress4;
    /*QByteArray data;
    int old_i, n;*/
    QImage *wholeImage;
};
#endif // MAINWINDOW_H
