#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpSocket4 = new QUdpSocket(this);

    groupAddress4 = new QHostAddress(QStringLiteral("225.1.1.1"));
    udpSocket4->bind(QHostAddress::AnyIPv4, 5007, QUdpSocket::ShareAddress);
    udpSocket4->joinMulticastGroup(*groupAddress4);

    connect(udpSocket4, &QUdpSocket::readyRead,
            this, &MainWindow::processPendingDatagrams);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processPendingDatagrams()
{
    QByteArray datagram;
    // using QUdpSocket::readDatagram (API since Qt 4)
    while (udpSocket4->hasPendingDatagrams()) {
        datagram.resize(qsizetype(udpSocket4->pendingDatagramSize()));
        qDebug()<<"received packet of size "<<datagram.size();
        udpSocket4->readDatagram(datagram.data(), datagram.size());
        int n;
        memcpy(&n, datagram, sizeof(int));
        qDebug()<<"value: "<<n;

        ui->label->setText(tr("Received IPv4 datagram of size %1, value: %2")
                                 .arg(datagram.size()).arg(n));
    }

}
