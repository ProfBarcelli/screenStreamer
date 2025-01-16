#ifndef LIMITEDSENDER_H
#define LIMITEDSENDER_H

#include "multicaststreamer.h"
#include "queuedpacket.h"
#include <QByteArray>
#include <QQueue>
#include <QThread>
#include <QUdpSocket>
#include <QMutex>

class MulticastStreamer;

class LimitedSender : public QThread
{
    Q_OBJECT
    void run() override;
public:
    LimitedSender(MulticastStreamer *mCastStreamer);
    void addPacket(QueuedPacket *packet);
private:
    MulticastStreamer *mCastStreamer;
    QQueue<QueuedPacket*> queue;
    QMutex mutex;
};

#endif // LIMITEDSENDER_H
