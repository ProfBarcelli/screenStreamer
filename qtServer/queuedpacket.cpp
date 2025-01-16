#include "queuedpacket.h"
#include <QDateTime>

QueuedPacket::QueuedPacket(QByteArray data) {
    setData(data);
}

void QueuedPacket::setData(QByteArray data)
{
    mutex.lock();
    this->data = data;
    timestamp = QDateTime().toMSecsSinceEpoch()-2000;
    mutex.unlock();
}

qint64 QueuedPacket::getTimestamp()
{
    return timestamp;
}

void QueuedPacket::setTimestamp(qint64 ts)
{
    mutex.lock();
    timestamp=ts;
    mutex.unlock();
}

void QueuedPacket::lock()
{
    mutex.lock();
}

void QueuedPacket::unlock()
{
    mutex.unlock();
}

QByteArray QueuedPacket::getData()
{
    mutex.lock();
    QByteArray d(data);
    mutex.unlock();
    return d;
}

int QueuedPacket::getDataSize()
{
    return data.size();
}
