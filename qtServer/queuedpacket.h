#ifndef QUEUEDPACKET_H
#define QUEUEDPACKET_H

#include <QByteArray>
#include <QMutex>

class QueuedPacket
{
private:
public:
    QueuedPacket(QByteArray data);
    QByteArray data;
    qint64 timestamp;
    QMutex mutex;
};

#endif // QUEUEDPACKET_H
