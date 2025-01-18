#ifndef MULTICASTSTREAMER_H
#define MULTICASTSTREAMER_H

#include <QUdpSocket>
#include <QHostAddress>
#include <QImage>
#include <QThread>
#include <QVector>
#include "queuedpacket.h"
#include "streamvisualizer.h"
#include "streamqueue.h"

#define N_PARTS_X 10
#define N_PARTS_Y 10
class LimitedSender;

class MulticastStreamer : public QThread
{
    Q_OBJECT
    void run() override;
public:
    MulticastStreamer(int nh, int nw);
    void updatePacket(int x, int y, QueuedPacket *qp);
    void setInterface(QNetworkInterface &interface);
    void setMcastIp(QString ip);
private:
    /*QUdpSocket *udpSocket4;
    QHostAddress *mCastGroupAddress4;
    quint16 mCastGroupPort = 5007;*/
    int nw, nh;
    int maxDelayUpdate;

    StreamVisualizer *streamVisualizer;

    QImage *image;
    bool isRunning;
    QVector<QVector<QueuedPacket*>> parts;

    StreamQueue streamQueue;
    void send(QueuedPacket *packet);
};

#endif // MULTICASTSTREAMER_H
