#ifndef STREAMQUEUE_H
#define STREAMQUEUE_H

#include <QQueue>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMutex>

class StreamQueue : public QThread
{
    Q_OBJECT
    void run() override;
private:
    QQueue<QByteArray> packetsToSend;
    QUdpSocket *udpSocket4;
    QHostAddress *mCastGroupAddress4;
    quint16 mCastGroupPort = 5007;
    int maxSize = 100;
    float avgSize = 0;
    float alpha=0.9;
    QMutex mutex;

public:
    StreamQueue();
    void add(QByteArray packet);
    float getAvgSpeedKBs();
};

#endif // STREAMQUEUE_H
