#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QImage>
#include <QPainter>

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
    /*old_i=0;
    n=0;*/
    image=NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}
/*
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
}*/


void MainWindow::processPendingDatagrams()
{
    QByteArray datagram;
    // using QUdpSocket::readDatagram (API since Qt 4)
    while (udpSocket4->hasPendingDatagrams()) {
        datagram.resize(qsizetype(udpSocket4->pendingDatagramSize()));
        qDebug()<<"received packet of size "<<datagram.size();
        udpSocket4->readDatagram(datagram.data(), datagram.size());
        int w,h,x,y,ps;
        memcpy(&w, datagram.constData(), sizeof(int));
        memcpy(&h, datagram.constData()+4, sizeof(int));
        memcpy(&x, datagram.constData()+8, sizeof(int));
        memcpy(&y, datagram.constData()+12, sizeof(int));
        memcpy(&ps, datagram.constData()+16, sizeof(int));
        //qDebug()<<"i: "<<i<<", j: "<<j<<", np: "<<np<<", ps: "<<ps;

        ui->label->setText(tr("Received w: %1, h: %2, x: %3, y: %4, ps: %5")
            .arg(w).arg(h).arg(x).arg(y).arg(ps));

        QByteArray byteArray((const char*)(datagram.constData()+20), ps);

        if(image==NULL || image->size().width()!=w || image->size().height()!=h) {
            image = new QImage(w,h,QImage::Format_RGB32);
        }
        QImage img;
        img.loadFromData(byteArray);
        QPainter p(image);
        int xs=w/4, ys=h/4;
        QRect rect(x*xs,y*ys,xs,ys);
        p.drawImage(rect,img);
        ui->imageLabel->setPixmap(QPixmap::fromImage(*image, Qt::AutoColor));
        ui->imageLabel->resize(w,h);
        //ui->imageLabel->show();
    }
}
