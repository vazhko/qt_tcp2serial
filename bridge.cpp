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
    serial.setBaudRate(115200);
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

    if((mes[0] != '\x1') || (mes.length() < 17) || mes[mes.length() - 1] != '\x3') return mes.toHex(0);

    QByteArray cmd = mes.mid(3,1);
    QByteArray mes0 = mes.mid(4, mes.length() - 10);
    int pos = mes0.lastIndexOf(4);

    QByteArray err;
    if (pos > 0){
        mes0[pos] = ' ';
        err =  mes0.mid(pos + 1, 6);
        mes0.truncate(pos + 1);
    } else {
        //return mes.toHex(0);
    }

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QString ustring = codec->toUnicode(mes0);

    return cmd.toHex(0) + " " + ustring + err.toHex('|');

    //return mes.toHex(0);
}


void Bridge::acceptErrorServerSlot(QAbstractSocket::SocketError socketError){   
    qInfo() << "Server socket error: " << socketError;
    emit finished();

}

void Bridge::onNewTcpConnectionSlot(){
    socket = server.nextPendingConnection();    
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyReadTcpSlot()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnectedTcpSlot()));
    qInfo() << "TCP client " << socket->peerAddress().toString().mid(7) << "connected";
}

void Bridge::readyReadTcpSlot(){
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QByteArray msg = socket->readAll();
    if(serial.isOpen()) {
        serial.write(msg);
        qInfo() << "tcp: " << parseMessage(msg);
    } else {
        qInfo() << "Serial port isn't opened";
        emit finished();
    }
}

void Bridge::disconnectedTcpSlot(){
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    qInfo() << "TCP client " << socket->peerAddress().toString().mid(7) << "disconnected";

    if(socket->isOpen()) socket->close();
    else if(socket != nullptr) socket->deleteLater();
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
        qInfo() << "Tcp port isn't opened";
    }
}
