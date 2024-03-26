#include "multicaststreamer.h"
#include <QDateTime>
#include <QDebug>

MulticastStreamer::MulticastStreamer() : QThread() {
    isRunning = false;
    for(int i=0;i<N_PARTS_Y;i++)
        for(int j=0;j<N_PARTS_X;j++)
            parts[i][j]=NULL;
    udpSocket4 = new QUdpSocket(this);
    udpSocket4->bind(QHostAddress::AnyIPv4, 0);
    udpSocket4->setSocketOption(QAbstractSocket::MulticastTtlOption, 2);
    mCastGroupAddress4 = new QHostAddress(QStringLiteral("225.1.1.1"));
    isPaused=true;
    limitedSender = new LimitedSender(this);
    connect(limitedSender, &LimitedSender::finished, limitedSender, &QObject::deleteLater);
    limitedSender->start();
}

void MulticastStreamer::togglePause() {
    isPaused = !isPaused;
}

void MulticastStreamer::run() {
    isRunning = true;
    qDebug()<<"MulticastStreamer::run";
    while(isRunning) {
        if(!isPaused) {
            qint64 cts = QDateTime::currentDateTime().toMSecsSinceEpoch();
            for(int i=0;i<N_PARTS_X;i++)
                for(int j=0;j<N_PARTS_Y;j++) {
                    if(parts[i][j]==NULL) continue;
                    quint64 n = cts-parts[i][j]->timestamp;
                    if(n > 10000) {
                        qDebug()<<"updating x:"<<i<<", y:"<<j<<" size:"<<parts[i][j]->data.size();
                        parts[i][j]->mutex.lock();
                        parts[i][j]->timestamp = cts;
                        //send(parts[i][j]);
                        limitedSender->addPacket(parts[i][j]);
                        parts[i][j]->mutex.unlock();
                    } else {
                        //qDebug()<<cts<<" "<<n;
                    }
                }
        }
        msleep(20);
    }
}

/*
void MulticastStreamer::send(QueuedPacket *packet) {
    if(packet==NULL) return;
    packet->mutex.lock();
    QByteArray data(packet->data);
    packet->mutex.unlock();
    udpSocket4->writeDatagram(data, *mCastGroupAddress4, mCastGroupPort);
}*/

void MulticastStreamer::sendPacket(QByteArray data) {
    udpSocket4->writeDatagram(data, *mCastGroupAddress4, mCastGroupPort);
}


void MulticastStreamer::updatePacket(QueuedPacket *qp) {
    int x = qp->x, y = qp->y;
    if(parts[x][y]!=NULL && parts[x][y]->data.size() == qp->data.size()) {
        delete qp;
        return;
    }
    if(parts[x][y]!=NULL) {
        parts[x][y]->mutex.lock();
        parts[x][y]->mutex.unlock();
        delete parts[x][y];
    }
    parts[x][y] = qp;
}
