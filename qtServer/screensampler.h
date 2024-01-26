#ifndef SCREENSAMPLER_H
#define SCREENSAMPLER_H

#include <QThread>
#include <QObject>
#include <QImage>
#include <QMutex>

class ScreenSampler : public QThread
{
    Q_OBJECT
    void run() override;

signals:
    void sampledNewimage();
    void resultReady(const QString &result);

public:
    ScreenSampler(int x, int y, int w, int h);
    int x,y,w,h;
    QImage getSampledImage();
    int fps = 50;
    void setRectangle(int x, int y, int w, int h);
    void stop();

private:
    QImage sampledImage;
    bool isRunning;
    void updateScreenImage();
    QMutex mutex;
};

#endif // SCREENSAMPLER_H
