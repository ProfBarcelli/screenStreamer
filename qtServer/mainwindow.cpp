#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QScreen>
#include <QImage>
#include <QRect>
#include <QDebug>
#include <QBuffer>
#include <QByteArray>
#include <QVarLengthArray>
#include <QSlider>
#include "queuedpacket.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked,
            this,           &MainWindow::testClick);

    sampler = NULL;
    isSampling = false;

    QVarLengthArray sliders = {ui->xSlider,ui->ySlider,ui->wSlider,ui->hSlider,ui->sSlider,ui->qSlider};
    for(auto it = sliders.begin(); it < sliders.end(); it++)
        connect( *it, &QSlider::valueChanged, this, &MainWindow::paramsUpdated);
    paramsUpdated();

    mCastStreamer = new MulticastStreamer();
    connect(mCastStreamer, &MulticastStreamer::finished, mCastStreamer, &QObject::deleteLater);
    mCastStreamer->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::testClick() {
    /*
    //ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScreen* screen = QGuiApplication::primaryScreen();
    QSize ss = screen->size();
    int w = ss.width()*(100-ui->xSlider->value())*ui->wSlider->value()/10000;
    int h = ss.height()*(100-ui->ySlider->value())*ui->hSlider->value()/10000;
    QPixmap pixmap = screen->grabWindow(0);
    int x = ss.width()*ui->xSlider->value()/100;
    int y = ss.height()*ui->ySlider->value()/100;
    int s = ui->sSlider->value();
    qDebug()<<"x:"<<x<<", y:"<<y<<", w:"<<w<<", h:"<<h;
    QRect rect(x,y,w,h);
    QImage grabbedImage = pixmap.toImage().copy(rect);
    QImage small = grabbedImage.scaled( w*s/100, w*h/100, Qt::KeepAspectRatio);
    ui->label->setPixmap(QPixmap::fromImage(small, Qt::AutoColor));
    ui->label->resize(w,h);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    small.save(&buffer,"JPEG",ui->qSlider->value());
    qDebug()<<"compressed size :"<<byteArray.size();*/
    if(isSampling) {
        if(sampler!=NULL) {
            sampler->stop();
            sampler->quit();
            sampler->wait();
            disconnect(sampler, &ScreenSampler::finished, sampler, &QObject::deleteLater);
            disconnect(sampler, &ScreenSampler::sampledNewimage, this, &MainWindow::updateScreenPreview);
            delete sampler;
        }
    } else {
        sampler = new ScreenSampler(x,y,w,h);
        //connect(sampler, &ScreenSampler::resultReady, this, &MainWindow::handleResults);
        connect(sampler, &ScreenSampler::sampledNewimage, this, &MainWindow::updateScreenPreview);
        connect(sampler, &ScreenSampler::finished, sampler, &QObject::deleteLater);
        sampler->start();
    }
    isSampling = !isSampling;
    ui->pushButton->setText( isSampling ? "STOP sampling" : "Start sampling" );
}

void MainWindow::updateScreenPreview() {
    if(!isSampling) return;
    QImage grabbedImage = sampler->getSampledImage();
    QImage small = grabbedImage.scaled( w*s/100, w*h/100, Qt::KeepAspectRatio);
    ui->label->setPixmap(QPixmap::fromImage(small, Qt::AutoColor));
    ui->label->resize(w,h);

    int w4 = w*s/100/4, h4 = h*s/100/4;
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++) {
            QRect rect(i*w4,j*h4,w4,h4);
            QImage partImg = small.copy(rect);
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            partImg.save(&buffer,"JPEG",q);
            int ps = buffer.buffer().size();
            char data[ps+20];
            int *p=(int*)data;
            p[0]=w*s/100;
            p[1]=h*s/100;
            p[2]=i;
            p[3]=j;
            p[4]=ps;
            const char *srcData = buffer.buffer().constData();
            for(int k=0;k<ps;k++)
                data[20+k] = srcData[k];
            //qDebug()<<"w: "<<w<<", h:"<<h<<", x:"<<x<<", y:"<<y<<", ps:"<<ps;
            mCastStreamer->updatePacket(i,j,new QueuedPacket( QByteArray( data,ps+20) ));
        }
}

void MainWindow::paramsUpdated() {
    QScreen* screen = QGuiApplication::primaryScreen();
    QSize ss = screen->size();
    w = ss.width()*(100-ui->xSlider->value())*ui->wSlider->value()/10000;
    h = ss.height()*(100-ui->ySlider->value())*ui->hSlider->value()/10000;
    x = ss.width()*ui->xSlider->value()/100;
    y = ss.height()*ui->ySlider->value()/100;
    s = ui->sSlider->value();
    q = ui->qSlider->value();
    if(sampler!=NULL)
        sampler->setRectangle(x,y,w,h);
    qDebug()<<"x:"<<x<<", y:"<<y<<", w:"<<w<<", h:"<<h;
}
