#ifndef BRIDGE_H
#define BRIDGE_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/qtcpserver.h>


class Bridge : public QObject{
    Q_OBJECT
public:
    Bridge();
    bool start(QString serName, quint32 portNum);

private:
    QSerialPort serial;
    QTcpServer server;
    QTcpSocket *socket;
    QString parseMessage(QByteArray);

public slots:
    void onNewTcpConnectionSlot();
    void acceptErrorServerSlot(QAbstractSocket::SocketError socketError);
    void readyReadTcpSlot();
    void disconnectedTcpSlot();

    void handleSerialErrorSlot(QSerialPort::SerialPortError error);
    void readyReadSerialSlot();


signals:
    void finished();
};

#endif // BRIDGE_H
