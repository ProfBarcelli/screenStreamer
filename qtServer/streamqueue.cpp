#include "streamqueue.h"
#include <qvariant.h>

StreamQueue::StreamQueue() : QThread() {
    udpSocket4 = new QUdpSocket(this);
    udpSocket4->bind(QHostAddress::AnyIPv4, 0);
    QVariant ttl=2;
    udpSocket4->setSocketOption(QAbstractSocket::MulticastTtlOption, ttl);
    mCastGroupAddress4 = new QHostAddress(QStringLiteral("225.1.1.1"));
}


void StreamQueue::run()
{
    while(isRunning()) {
        mutex.lock();
        if(!packetsToSend.isEmpty()) {
            QByteArray data= packetsToSend.dequeue();
            mutex.unlock();
            udpSocket4->writeDatagram(data, *mCastGroupAddress4, mCastGroupPort);
            //qDebug()<<data.size();
            avgSize = avgSize*alpha + (1-alpha)*data.size();
            QThread::msleep(1);
        }
        else
            mutex.unlock();
    }
}

void StreamQueue::add(QByteArray packet)
{
    mutex.lock();
    if(packetsToSend.size()<maxSize)
        packetsToSend.enqueue(packet);
    else
        qDebug()<<"Queue full "<<avgSize<<"KB/s";
    mutex.unlock();
}

float StreamQueue::getAvgSpeedKBs()
{
    return avgSize;
}

