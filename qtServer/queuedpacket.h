#ifndef QUEUEDPACKET_H
#define QUEUEDPACKET_H

#include <QByteArray>
#include <QMutex>

class QueuedPacket
{
private:
    QByteArray data;
    qint64 timestamp;
    QMutex mutex;
public:
    QueuedPacket(QByteArray data);
    void setData(QByteArray data);
    qint64 getTimestamp();
    void setTimestamp(qint64 ts);
    void lock();
    void unlock();
    QByteArray getData();
    int getDataSize();
};

#endif // QUEUEDPACKET_H
