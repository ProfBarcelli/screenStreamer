#include "streamvisualizer.h"
#include "ui_streamvisualizer.h"
#include <QDateTime>

StreamVisualizer::StreamVisualizer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StreamVisualizer)
{
    ui->setupUi(this);
    workerThread = new QThread(this);
    QObject::connect(workerThread, &QThread::started, [this]() {
        while(workerThread->isRunning()) {
            update();
            QThread::msleep(100);
        }
    });
    workerThread->start();
}

void StreamVisualizer::update() {
    if(parts==NULL)
        return;
    qint64 cts = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QString s="";
    for(int i=0;i<parts->size();i++) {
        for(int j=0;j<(*parts)[i].size();j++) {
            if((*parts)[i][j]!=NULL)
                s.append(QString::number((cts-(*parts)[i][j]->getTimestamp())/1000));
            else
                s.append("\t");
            s.append("\t");
        }
        s+="\n";
    }
    if(streamQueue!=NULL) {
        s+="\nAvg stream speed "+ QString::number(streamQueue->getAvgSpeedKBs())+" KB/s";
    }
    ui->label->setText(s);
}

StreamVisualizer::~StreamVisualizer()
{
    delete ui;
}
