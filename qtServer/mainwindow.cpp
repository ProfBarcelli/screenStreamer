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
#include <QDir>
#include "queuedpacket.h"
#include <QtNetwork/QNetworkInterface>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mCastStreamer=NULL;
    qDebug()<<QDir::homePath();

    connect(ui->pushButton, &QPushButton::clicked,
            this,           &MainWindow::startStopButtonClick);

    sampler = NULL;
    isSampling = false;


    mCastIps.append("225.1.1.1");
    mCastIps.append("225.1.1.2");
    mCastIps.append("225.1.1.3");
    mCastIps.append("225.1.1.4");
    ui->streamComboBox ->addItem("Stream 1");
    ui->streamComboBox->addItem("Stream 2");
    ui->streamComboBox->addItem("Stream 3");
    ui->streamComboBox->addItem("Stream 4");

    loadSettingsFromHomeFolder();

    QVarLengthArray sliders = {ui->xSlider,ui->ySlider,ui->wSlider,ui->hSlider,ui->sSlider,ui->qSlider};
    for(auto it = sliders.begin(); it < sliders.end(); it++)
        connect( *it, &QSlider::valueChanged, this, &MainWindow::paramsUpdated);
    paramsUpdated();


    //nh=4, nw=4;
    mCastStreamer = new MulticastStreamer(nh,nw);
    connect(mCastStreamer, &MulticastStreamer::finished, mCastStreamer, &QObject::deleteLater);
    mCastStreamer->start();

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        if(!(interface.flags() & QNetworkInterface::IsUp && interface.name().contains("eth", Qt::CaseInsensitive)))
            continue;
        //qDebug()<<interface;
        for (const QNetworkAddressEntry &addressEntry: interface.addressEntries()) {
            if (addressEntry.ip().protocol() == QAbstractSocket::IPv4Protocol && addressEntry.ip() != localhost) {
                ui->nicComboBox->addItem(addressEntry.ip().toString());
                //qDebug()<<"   "<<addressEntry.ip().toString();
                interfaces.append(interface);
                break;
            }
            //qDebug()<<"   "<<addressEntry.ip()<<addressEntry.ip().protocol();
        }
    }
}

void MainWindow::loadSettingsFromHomeFolder() {
    nh=2;
    nw=2;

    QFile *file = new QFile(QDir::homePath()+"/.qtServer.config");
    if(!file->exists()) {
        qDebug()<<"Il file "<<file->fileName()<<" non esiste, lo creo";
        file->open(QIODevice::WriteOnly);
        QTextStream out(file);
        out << "nw:2\n";
        out << "nh:2\n";
        out << "x:0\n";
        out << "y:0\n";
        out << "w:50\n";
        out << "h:50\n";
        out << "q:70\n";
        out << "s:100\n";
        out << "streamNumber:0\n";
        //file->flush();
        file->close();
        //file=new QFile (QDir::homePath()+"/.qtServer.config");
    }
    if(!file->open(QIODevice::ReadOnly)) {
        qDebug()<<"non posso leggere il file "<<file->fileName()<< "Error:" << file->errorString();
    }

    QTextStream in(file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        //qDebug()<<line;
        QStringList fields = line.split(":");
        if(fields[0]=="nw") nw=fields[1].toInt();//qDebug()<<"nw : "<<fields[1];
        if(fields[0]=="nh") nh=fields[1].toInt();
        if(fields[0]=="x") x=fields[1].toInt();
        if(fields[0]=="y") y=fields[1].toInt();
        if(fields[0]=="w") w=fields[1].toInt();
        if(fields[0]=="h") h=fields[1].toInt();
        if(fields[0]=="q") q=fields[1].toInt();
        if(fields[0]=="s") s=fields[1].toInt();
        if(fields[0]=="streamNumber") {
            ui->streamComboBox->setCurrentIndex(fields[1].toInt());
        }
    }
    file->close();

    ui->xSlider->setValue(x);
    ui->ySlider->setValue(y);
    ui->wSlider->setValue(w);
    ui->hSlider->setValue(h);
    ui->sSlider->setValue(s);
    ui->qSlider->setValue(q);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::startStopButtonClick() {
    /*
    //ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScreen* screen = QGuiApplication::primaryScreen();
    QSize ss = screen->size();
    int w = ss.width()*(100-ui->xSlider->value())*uiwidth->wSlider->value()/10000;
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
    //mCastStreamer->togglePause();
    ui->pushButton->setText( isSampling ? "STOP" : "Start" );
}

void MainWindow::updateScreenPreview() {
    if(!isSampling) return;
    QImage grabbedImage = sampler->getSampledImage();
    QImage small = grabbedImage.scaled( w*s/100, h*s/100, Qt::KeepAspectRatio);
    ui->label->setPixmap(QPixmap::fromImage(small, Qt::AutoColor));
    ui->label->resize(w,h);

    int w4 = w*s/100/nw, h4 = h*s/100/nh;
    //qDebug()<<"Ricordarsi di cambiare il client per ricevere anche i dati nh e nw";
    for(int i=0;i<nh;i++)
        for(int j=0;j<nw;j++) {
            QRect rect(i*w4,j*h4,w4,h4);
            QImage partImg = small.copy(rect);
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            partImg.save(&buffer,"JPEG",q);
            int ps = buffer.buffer().size();
            char data[ps+28];
            int *p=(int*)data;
            p[0]=w*s/100;
            p[1]=h*s/100;
            p[2]=i;
            p[3]=j;
            p[4]=nh;
            p[5]=nw;
            p[6]=ps;
            //qDebug()<<"Ricordarsi di cambiare il client per ricevere anche i dati nh e nw";
            const char *srcData = buffer.buffer().constData();
            for(int k=0;k<ps;k++)
                data[28+k] = srcData[k];
            //qDebug()<<"w: "<<w<<", h:"<<h<<", x:"<<x<<", y:"<<y<<", ps:"<<ps;
            mCastStreamer->updatePacket(i,j,new QueuedPacket( QByteArray( data,ps+28) ));
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

    QFile *file = new QFile(QDir::homePath()+"/.qtServer.config");
    file->open(QIODevice::WriteOnly);
    QTextStream out(file);
    out << "nw:"<<nw<<"\n";
    out << "nh:"<<nh<<"\n";
    out << "x:"<<x<<"\n";
    out << "y:"<<y<<"\n";
    out << "w:"<<(int)(100*w/ss.width())<<"\n";
    out << "h:"<<(int)(100*h/ss.height())<<"\n";
    out << "q:"<<q<<"\n";
    out << "s:"<<s<<"\n";
    out << "streamNumber:"<<ui->streamComboBox->currentIndex()<<"\n";
    file->close();

}

void MainWindow::on_sendTextPushButton_clicked()
{
    if(mCastStreamer==NULL) return;
    //qDebug()<<ui->textEdit->toPlainText();
    QString str = ui->textEdit->toPlainText()+"\0";
    if(str.length()>0) {
        mCastStreamer->sendText(str);
        ui->textEdit->clear();
    } else {
        QImage image(100, 100, QImage::Format_RGB32);
        if(jTest==0) image.fill(QColor(Qt::red));
        if(jTest==1) image.fill(QColor(Qt::green));
        if(jTest==2) image.fill(QColor(Qt::blue));
        jTest=(jTest+1)%3;

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer,"JPEG",q);
        int ps = buffer.buffer().size();
        //qDebug()<<"bin size: "<<ps;
        char data[ps+28];
        int *p=(int*)data;
        p[0]=200;
        p[1]=200;
        p[2]=iTest/2;
        p[3]=iTest%2;
        p[4]=2;
        p[5]=2;
        p[6]=ps;

        iTest = (iTest+1)%4;
        /*
        //qDebug()<<"Ricordarsi di cambiare il client per ricevere anche i dati nh e nw";
        const char *srcData = buffer.buffer().constData();
        for(int k=0;k<ps;k++)
            data[28+k] = srcData[k];*/
        memcpy(data+28,buffer.buffer().constData(),ps);
        //qDebug()<<"w: "<<w<<", h:"<<h<<", x:"<<x<<", y:"<<y<<", ps:"<<ps;
        mCastStreamer->sendTestPacket(data, ps+28);
    }
}


void MainWindow::on_nicComboBox_currentIndexChanged(int index)
{
    if(mCastStreamer==NULL) return;
    //qDebug()<<index;
    if(index<interfaces.size()) {
        qDebug()<<interfaces.at(index);
        QNetworkInterface interface =  interfaces.at(index);
        mCastStreamer->setInterface(interface);
    }
    paramsUpdated();
}


void MainWindow::on_streamComboBox_currentIndexChanged(int index)
{
    if(mCastStreamer==NULL) return;
    mCastStreamer->setMcastIp(mCastIps.at(index));
    paramsUpdated();
}

