#include "queuedpacket.h"
#include <QDateTime>

QueuedPacket::QueuedPacket(int x, int y, QByteArray data) {
    this->data = data;
    this->x=x;
    this->y=y;
    timestamp = QDateTime().toMSecsSinceEpoch()-2000;
}
