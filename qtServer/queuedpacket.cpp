#include "queuedpacket.h"
#include <QDateTime>

QueuedPacket::QueuedPacket(QByteArray data) {
    this->data = data;
    timestamp = QDateTime().toMSecsSinceEpoch()-2000;
}
