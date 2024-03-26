#ifndef MULTICASTSTREAMER_H
#define MULTICASTSTREAMER_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QImage>
#include <QThread>
#include "limitedsender.h"
#include "queuedpacket.h"

#define N_PARTS_X 10
#define N_PARTS_Y 10
class LimitedSender;

class MulticastStreamer : public QThread
{
    Q_OBJECT
    void run() override;
public:
    MulticastStreamer();
    void updatePacket(QueuedPacket *qp);
    void togglePause();
    void sendPacket(QByteArray packet);
private:
    QUdpSocket *udpSocket4;
    QHostAddress *mCastGroupAddress4;
    quint16 mCastGroupPort = 5007;
    QImage *image;
    bool isRunning;
    bool isPaused;
    QueuedPacket *parts[N_PARTS_X][N_PARTS_Y];
    LimitedSender *limitedSender;

    void send(QueuedPacket *packet);
};

#endif // MULTICASTSTREAMER_H
