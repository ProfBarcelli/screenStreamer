#include "limitedsender.h"

LimitedSender::LimitedSender(MulticastStreamer *mCastStreamer) : QThread() {
    this->mCastStreamer = mCastStreamer;
}


void LimitedSender::addPacket(QueuedPacket *packet) {
    mutex.lock();
    bool found=false;
    for(auto it = queue.begin();it<queue.end();it++) {
        (*it)->mutex.lock();
        if( (*it)->x ==packet->x && (*it)->y==packet->y) {
            //(*it)->data = packet->data;
            found=true;
            (*it)->mutex.unlock();
            break;
        }
        (*it)->mutex.unlock();
    }
    if(!found && queue.size()<100) {
        queue.append(packet);
    }
    mutex.unlock();
}

void LimitedSender::run() {
    while(true) {
        mutex.lock();
        if(queue.size()>0) {
            QueuedPacket *packet = queue.first();
            queue.pop_front();
            //send
            mCastStreamer->sendPacket(packet->data);
        }
        mutex.unlock();
        usleep(1000);
    }
}
