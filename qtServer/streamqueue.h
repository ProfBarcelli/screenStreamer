#ifndef STREAMQUEUE_H
#define STREAMQUEUE_H

#include <QQueue>
#include <QThread>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMutex>
#include <QNetworkInterface>

class StreamQueue : public QThread
{
    Q_OBJECT
    void run() override;
private:
    QQueue<QByteArray> packetsToSend;
    QUdpSocket *udpSocket4;
    QHostAddress *mCastGroupAddress4;
    QString mCastIp = "225.1.1.1";
    quint16 mCastGroupPort = 5007;
    int maxSize = 100;
    float avgSize = 0;
    float alpha=0.9;
    QMutex mutex;

    void initMcast();
    QNetworkInterface interface;
public:
    StreamQueue();
    void add(QByteArray packet);
    float getAvgSpeedKBs();
    void setInterface(QNetworkInterface &interface);
    void setMcastIp(QString ip);
};

#endif // STREAMQUEUE_H
