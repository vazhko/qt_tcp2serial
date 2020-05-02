#include "bridge.h"
#include <QCoreApplication>


//const QString serialStr = "COM6";
//const quint32 tcpPortNum = 34952;


int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QString serialStr = QString(argv[1]);
    quint32 tcpPortNum = QString(argv[2]).toInt();

    if(argc < 3 ){
        qInfo() << "Usage: qt_tcp2serial.exe serport tcpport";
        qInfo() << "Example: qt_tcp2serial.exe COM6 34952";
        a.quit();
        return 0;
    }
    if((tcpPortNum <= 500) || (tcpPortNum > 0xffff)) {
        qInfo() << "tcpport number invalid 1000...65535";
        a.quit();
        return 0;
    }


    Bridge bridge;
    if (!bridge.start(serialStr, tcpPortNum)) {
        a.quit();
        return 0;
    }

    QObject::connect(&bridge, SIGNAL(finished()), &a, SLOT(quit()));

    return a.exec();
}
