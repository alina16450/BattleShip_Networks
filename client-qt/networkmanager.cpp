#include "networkmanager.h"

networkmanager::networkmanager(QObject* parent)
    : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    tcpSocket = new QTcpSocket(this);

    udpSocket->bind(QHostAddress::AnyIPv4, 0);
    tcpSocket->connectToHost(QHostAddress("192.168.1.21"), 4040);

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &networkmanager::onUdpReadyRead);

    serverAddress = QHostAddress("192.168.1.21");
    serverPort = 8080;

    connect(tcpSocket, &QTcpSocket::readyRead,
            this, &networkmanager::onTcpReadyRead);

    connect(tcpSocket, &QTcpSocket::connected,
            this, &networkmanager::onTcpConnected);

    connect(tcpSocket, &QTcpSocket::disconnected,
            this, &networkmanager::onTcpDisconnected);
}

void networkmanager::sendUdpMessage(const QString& message)
{
    QByteArray data = message.toUtf8();
    udpSocket->writeDatagram(data, serverAddress, serverPort);
}

void networkmanager::onUdpReadyRead()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray buffer;
        buffer.resize(udpSocket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(buffer.data(), buffer.size(),
                                &sender, &senderPort);

        emit udpMessageReceived(QString::fromUtf8(buffer));
    }
}

void networkmanager::sendTcpMessage(const QString& msg)
{
    QByteArray data = msg.toUtf8();
    data.append('\n');
    tcpSocket->write(data);
}

void networkmanager::onTcpReadyRead()
{
    while (tcpSocket->bytesAvailable())
    {
        QString msg = QString::fromUtf8(tcpSocket->readLine()).trimmed();
        emit tcpMessageReceived(msg);
    }
}

void networkmanager::onTcpConnected()
{
    qDebug() << "TCP connected!";
}

void networkmanager::onTcpDisconnected()
{
    qDebug() << "TCP disconnected!";
}
