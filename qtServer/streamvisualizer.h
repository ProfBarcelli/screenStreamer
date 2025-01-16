#ifndef STREAMVISUALIZER_H
#define STREAMVISUALIZER_H

#include <QWidget>
#include <QVector>
#include <QThread>
#include "queuedpacket.h"
#include "streamqueue.h"

namespace Ui {
class StreamVisualizer;
}

class StreamVisualizer : public QWidget
{
    Q_OBJECT

public:
    explicit StreamVisualizer(QWidget *parent = nullptr);
    ~StreamVisualizer();
    QVector<QVector<QueuedPacket*>> *parts;
    StreamQueue *streamQueue;

private:
    Ui::StreamVisualizer *ui;
    void update();
    QThread *workerThread;
};

#endif // STREAMVISUALIZER_H
