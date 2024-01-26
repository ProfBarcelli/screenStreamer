#include "screensampler.h"
#include <QScreen>
#include <QPixmap>
#include <QRect>
#include <QDebug>
#include <QGuiApplication>
#include <QThread>

ScreenSampler::ScreenSampler(int x, int y, int w, int h) : QThread(){
    setRectangle(x,y,w,h);
}

void ScreenSampler::stop() {
    isRunning=false;
}

void ScreenSampler::run(){
    //qDebug()<<"Sampler started";
    isRunning=true;
    QString result;
    while(isRunning) {
        updateScreenImage();
        emit sampledNewimage();
        QThread::msleep(1000/fps);
    }
    emit resultReady(result);
    //qDebug()<<"Sampler finished";
}

void ScreenSampler::setRectangle(int x, int y, int w, int h) {
    this->x=x;
    this->y=y;
    this->w=w;
    this->h=h;
}

void ScreenSampler::updateScreenImage() {
    QScreen* screen = QGuiApplication::primaryScreen();
    const QPixmap pixmap = screen->grabWindow(0);
    //qDebug()<<"x:"<<x<<", y:"<<y<<", w:"<<w<<", h:"<<h;
    const QRect rect(x,y,w,h);
    const QImage img = pixmap.toImage();
    mutex.lock();
    sampledImage = img.copy(rect);
    mutex.unlock();
}

QImage ScreenSampler::getSampledImage() {
    mutex.lock();
    QImage imgCopy = sampledImage.copy();
    mutex.unlock();
    return imgCopy;
}
