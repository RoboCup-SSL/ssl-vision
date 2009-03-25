#ifndef VIEWUPDATETHREAD_H
#define VIEWUPDATETHREAD_H

#include <QThread>
#include "GraphicsPrimitives.h"
#include "robocup_ssl_client.h"
#include "timer.h"

class ViewUpdateThread : public QThread
{
private:
    bool shutdown;
    RoboCupSSLClient client;
    SSL_WrapperPacket packet;
    SoccerView *soccer;
    int printRobotInfo(const SSL_DetectionRobot & robot, int teamID);

public:
    ViewUpdateThread(){}
    ViewUpdateThread(SoccerView *_soccer);
    void run();
    void Terminate();
};

#endif // VIEWUPDATETHREAD_H
