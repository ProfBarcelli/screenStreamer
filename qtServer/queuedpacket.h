#ifndef QUEUEDPACKET_H
#define QUEUEDPACKET_H

#include <QByteArray>
#include <QMutex>

class QueuedPacket
{
private:
public:
    QueuedPacket(int x, int y,QByteArray data);
    QByteArray data;
    int x,y;
    qint64 timestamp;
    QMutex mutex;
};

#endif // QUEUEDPACKET_H
