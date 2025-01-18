#include "streamqueue.h"
#include <qvariant.h>
#include <QtNetwork/QNetworkInterface>

QNetworkInterface getEthernetInterface()
{
    // Iterate through all network interfaces
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        // Check if the interface is up and is an Ethernet type (wired connection)
        if (interface.flags() & QNetworkInterface::IsUp && interface.name().contains("eth", Qt::CaseInsensitive)) {
            qDebug()<<"Found it"<<interface;
            return interface;
        }
    }
    return QNetworkInterface();  // Return an empty QNetworkInterface if no Ethernet is found
}


StreamQueue::StreamQueue() : QThread() {
    interface = getEthernetInterface();
    initMcast();
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
            //qDebug()<<packetsToSend.size();
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

void StreamQueue::initMcast()
{
    udpSocket4 = new QUdpSocket(this);
    udpSocket4->bind(QHostAddress::AnyIPv4, 0);
    QVariant ttl=255;
    udpSocket4->setSocketOption(QAbstractSocket::MulticastTtlOption, ttl);
    mCastGroupAddress4 = new QHostAddress(mCastIp);
    udpSocket4->joinMulticastGroup(*mCastGroupAddress4);
    int loopFlag = 0;
    udpSocket4->setSocketOption(QAbstractSocket::MulticastLoopbackOption,loopFlag);
    udpSocket4->setMulticastInterface(interface);
    qDebug()<<"Multicasting on "<<interface.humanReadableName()<<" with IP "<<mCastIp;
}

void StreamQueue::setInterface(QNetworkInterface &interface)
{
    this->interface=interface;
    initMcast();
}

void StreamQueue::setMcastIp(QString ip)
{
    mCastIp=ip;
    initMcast();
}

