#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    wholeImage=NULL;

    mCastIps.append("225.1.1.1");
    mCastIps.append("225.1.1.2");
    mCastIps.append("225.1.1.3");
    mCastIps.append("225.1.1.4");
    ui->comboBox->addItem("Stream 1");
    ui->comboBox->addItem("Stream 2");
    ui->comboBox->addItem("Stream 3");
    ui->comboBox->addItem("Stream 4");

    //initMcast();
}

void MainWindow::initMcast() {
    if(mCastIpIndex>=mCastIps.size())
        return;

    if(udpSocket4!=NULL) {
        disconnect(udpSocket4,&QUdpSocket::readyRead,
                   this, &MainWindow::processPendingDatagrams);
        delete udpSocket4;
    }

    udpSocket4 = new QUdpSocket(this);

    QString ip = mCastIps.at(mCastIpIndex);
    qDebug()<<ip;
    groupAddress4 = new QHostAddress(ip);
    udpSocket4->bind(QHostAddress::AnyIPv4, 5007, QUdpSocket::ShareAddress);
    udpSocket4->joinMulticastGroup(*groupAddress4);

    connect(udpSocket4, &QUdpSocket::readyRead,
            this, &MainWindow::processPendingDatagrams);


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
        //qDebug()<<"received packet of size "<<datagram.size();
        udpSocket4->readDatagram(datagram.data(), datagram.size());
        int w,h,x,y,nh,nw,ps;
        memcpy(&w, datagram.constData(), sizeof(int));
        memcpy(&h, datagram.constData()+4, sizeof(int));
        memcpy(&x, datagram.constData()+8, sizeof(int));
        memcpy(&y, datagram.constData()+12, sizeof(int));
        memcpy(&nh, datagram.constData()+16, sizeof(int));
        memcpy(&nw, datagram.constData()+20, sizeof(int));
        memcpy(&ps, datagram.constData()+24, sizeof(int));
        //qDebug()<<"i: "<<i<<", j: "<<j<<", np: "<<np<<", ps: "<<ps;
        //qDebug()<<"w:"<<w<<", h: "<<h<<", x:"<<x<<", y:"<<y<<", nh:"<<nh<<", nw:"<<nw<<", ps:"<<ps;

        ui->label->setText(tr("Received w: %1, h: %2, x: %3, y: %4, nh: %5, nw: %6, ps: %7")
            .arg(w).arg(h).arg(x).arg(y).arg(nh).arg(nw).arg(ps));

        QByteArray sectionImageByteArray((const char*)(datagram.constData()+28), ps);

        if(wholeImage==NULL ||
            wholeImage->size().width()!=w ||
            wholeImage->size().height()!=h) {
            wholeImage = new QImage(w,h,QImage::Format_RGB32);
        }
        QImage sectionImage;
        sectionImage.loadFromData(sectionImageByteArray);
        QPainter p(wholeImage);
        int xs=w/nw, ys=h/nh;
        QRect rect(x*xs,y*ys,xs,ys);
        //qDebug()<<rect;
        p.drawImage(rect,sectionImage);
        //ui->imageLabel->setPixmap(QPixmap::fromImage(sectionImage, Qt::AutoColor));
        ui->imageLabel->setPixmap(QPixmap::fromImage(*wholeImage, Qt::AutoColor));
        ui->imageLabel->resize(w,h);
        //ui->imageLabel->show();

    }
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    mCastIpIndex = index;
    initMcast();
}

