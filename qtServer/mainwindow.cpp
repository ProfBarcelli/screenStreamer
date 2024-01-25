#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QScreen>
#include <QImage>
#include <QRect>
#include <QDebug>
#include <QBuffer>
#include <QByteArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked,
            this,           &MainWindow::testClick);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::testClick() {
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
    qDebug()<<"compressed size :"<<byteArray.size();
}
