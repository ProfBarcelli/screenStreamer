#ifndef MULTICASTSTREAMER_H
#define MULTICASTSTREAMER_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QImage>
#include <QThread>
#include "queuedpacket.h"

class MulticastStreamer : public QThread
{
    Q_OBJECT
    void run() override;
public:
    MulticastStreamer();
    void updatePacket(int x, int y, QueuedPacket *qp);
private:
    QUdpSocket *udpSocket4;
    QHostAddress *mCastGroupAddress4;
    quint16 mCastGroupPort = 5007;
    QImage *image;
    bool isRunning;
    QueuedPacket *parts[4][4];

    void send(QueuedPacket *packet);
};

#endif // MULTICASTSTREAMER_H
