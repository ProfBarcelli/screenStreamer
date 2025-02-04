#include "multicaststreamer.h"
#include <QDateTime>
#include <QDebug>

MulticastStreamer::MulticastStreamer(int nh, int nw) : QThread() {
    isRunning = false;
    this->nw=nw;
    this->nh=nh;
    maxDelayUpdate = 10000;
    for(int i=0;i<nh;i++) {
        QVector<QueuedPacket*> row;
        for(int j=0;j<nw;j++)
            row.append(NULL);
        parts.append(row);
    }
/*    udpSocket4 = new QUdpSocket(this);
    udpSocket4->bind(QHostAddress::AnyIPv4, 0);
    udpSocket4->setSocketOption(QAbstractSocket::MulticastTtlOption, 2);
    mCastGroupAddress4 = new QHostAddress(QStringLiteral("225.1.1.1"));*/

    streamVisualizer = new StreamVisualizer();
    streamVisualizer->parts = &parts;
    streamVisualizer->streamQueue = &streamQueue;
    streamVisualizer->show();

    streamQueue.start();
}

void MulticastStreamer::run() {
    isRunning = true;
    qDebug()<<"MulticastStreamer::run";
    while(isRunning) {
        qint64 cts = QDateTime::currentDateTime().toMSecsSinceEpoch();
        for(int i=0;i<parts.size();i++)
            for(int j=0;j<parts[i].size();j++) {
                if(parts[i][j]==NULL) continue;
                quint64 n = cts-parts[i][j]->getTimestamp();
                if(n > maxDelayUpdate) {
                    parts[i][j]->setTimestamp(cts);
                    //qDebug()<<"sending x:"<<i<<", y:"<<j<<" size:"<<parts[i][j]->getDataSize();
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
    QByteArray data = packet->getData();
    streamQueue.add(data);
    //udpSocket4->writeDatagram(data, *mCastGroupAddress4, mCastGroupPort);
}

void MulticastStreamer::updatePacket(int x, int y, QueuedPacket *qp) {
    if(parts[x][y]!=NULL && parts[x][y]->getDataSize() == qp->getDataSize())
        return;
    if(parts[x][y]==NULL)
        parts[x][y] = qp;
    else
        parts[x][y]->setData(qp->getData());
}

void MulticastStreamer::setInterface(QNetworkInterface &interface)
{
    streamQueue.setInterface(interface);
}

void MulticastStreamer::setMcastIp(QString ip)
{
    streamQueue.setMcastIp(ip);
}

void MulticastStreamer::sendText(QString qstr)
{
    int w=-1;
    QByteArray data((char*)&w,sizeof(int));
    data.append(qstr.toUtf8());
    //qDebug()<<data.size();
    streamQueue.sendImmediatly(data);
}

void MulticastStreamer::sendTestPacket(char* data, int size)
{
    QByteArray d(data,size);
    streamQueue.sendImmediatly(d);
}

