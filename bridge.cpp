#include "bridge.h"

#include <QTextCodec>

Bridge::Bridge(){    
}


bool Bridge::start(QString serName, quint32 portNum){
    if (!server.listen(QHostAddress::Any, portNum)){
        qInfo() << "Can't open TCP port " << portNum << " for listenning";
        return false;
    }

    qInfo() << "TCP port " << portNum << " is listened";

    connect(&server, SIGNAL(newConnection()), this, SLOT(onNewTcpConnectionSlot()));
    connect(&server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptErrorServerSlot(QAbstractSocket::SocketError)));


    serial.setPortName(serName);
    serial.setBaudRate(9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if(!serial.open(QIODevice::ReadWrite)){
        qInfo() << "Can't open serial port " << serName;
        return false;
    }

    qInfo() << "Serial port " << serName << "is opened";

    connect(&serial, SIGNAL(readyRead()), this, SLOT(readyReadSerialSlot()));
    connect(&serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleSerialErrorSlot(QSerialPort::SerialPortError)));
    return true;
}


QString Bridge::parseMessage(QByteArray mes){
    return mes.toHex(0);
}


void Bridge::acceptErrorServerSlot(QAbstractSocket::SocketError socketError){   
    qInfo() << "Server socket error: " << socketError;
    emit finished();

}

void Bridge::onNewTcpConnectionSlot(){
    socket = server.nextPendingConnection();
    //srvSocket = new QTcpSocket();
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyReadTcpSlot()));
    qInfo() << "TCP client " << socket->peerAddress().toString().mid(7) << "connected";
}

void Bridge::readyReadTcpSlot(){
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QByteArray msg = socket->readAll();
    if(serial.isOpen()) {
        serial.write(msg);
        qInfo() << "tcp: " << parseMessage(msg);
    } else {
        qInfo() << "Tcp port isn't opened";
        emit finished();
    }
}


void Bridge::handleSerialErrorSlot(QSerialPort::SerialPortError error){
    qInfo() << "Serial port error: " << error;
    emit finished();
}


void Bridge::readyReadSerialSlot(){
    QSerialPort *ser = qobject_cast<QSerialPort*>(sender());
    QByteArray msg = ser->readAll();
    if(socket->isOpen()) {
        socket->write(msg);
        qInfo() << "ser: " << parseMessage(msg);
    } else {
        qInfo() << "Serial port isn't opened";
        emit finished();
    }
}
