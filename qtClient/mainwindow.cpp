#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QImage>
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
    old_i=0;
    n=0;
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
        int i, j, np, ps;
        memcpy(&i, datagram.constData(), sizeof(int));
        memcpy(&j, datagram.constData()+4, sizeof(int));
        memcpy(&np, datagram.constData()+8, sizeof(int));
        memcpy(&ps, datagram.constData()+12, sizeof(int));
        qDebug()<<"i: "<<i<<", j: "<<j<<", np: "<<np<<", ps: "<<ps;

        ui->label->setText(tr("Received i: %1, j: %2, np: %3, ps: %4")
            .arg(i).arg(j).arg(np).arg(ps));

        QByteArray byteArray((const char*)(datagram.constData()+16), ps);

        if(j==0 || i!=old_i) {
            old_i=i;
            data = QByteArray((const char*)(datagram.constData()+16), ps);
            n=1;
        } else {
            data.append((const char*)(datagram.constData()+16), ps);
            n++;
        }
        if (j==np-1 && n==np) {
            QImage img;
            img.loadFromData(data);
            ui->imageLabel->setPixmap(QPixmap::fromImage(img, Qt::AutoColor));
            ui->imageLabel->show();
        }
    }

}
