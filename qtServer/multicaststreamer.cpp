#include "multicaststreamer.h"
#include <QDateTime>
#include <QDebug>

MulticastStreamer::MulticastStreamer() : QThread() {
    isRunning = false;
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            parts[i][j]=NULL;
    udpSocket4 = new QUdpSocket(this);
    udpSocket4->bind(QHostAddress::AnyIPv4, 0);
    udpSocket4->setSocketOption(QAbstractSocket::MulticastTtlOption, 2);
    mCastGroupAddress4 = new QHostAddress(QStringLiteral("225.1.1.1"));
}

void MulticastStreamer::run() {
    isRunning = true;
    qDebug()<<"MulticastStreamer::run";
    while(isRunning) {
        qint64 cts = QDateTime::currentDateTime().toMSecsSinceEpoch();
        for(int i=0;i<4;i++)
            for(int j=0;j<4;j++) {
                if(parts[i][j]==NULL) continue;
                quint64 n = cts-parts[i][j]->timestamp;
                if(n > 1000) {
                    parts[i][j]->timestamp = cts;
                    qDebug()<<"sending x:"<<i<<", y:"<<j<<" size:"<<parts[i][j]->data.size();
                    send(parts[i][j]);
                } else {
                    //qDebug()<<cts<<" "<<n;
                }
            }
        msleep(20);
    }
}

void MulticastStreamer::send(QueuedPacket *packet) {
    if(packet==NULL) return;
    packet->mutex.lock();
    QByteArray data(packet->data);
    packet->mutex.unlock();
    udpSocket4->writeDatagram(data, *mCastGroupAddress4, mCastGroupPort);
}

void MulticastStreamer::updatePacket(int x, int y, QueuedPacket *qp) {
    if(parts[x][y]!=NULL && parts[x][y]->data.size() == qp->data.size())
        return;
    if(parts[x][y]!=NULL)
        delete parts[x][y];
    parts[x][y] = qp;
}
