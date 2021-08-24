#include "client.h"
#include <QDebug>
#include <QMessageBox>

Client::Client(QDialog *parent): QDialog(parent) {
    clientSocket = new QTcpSocket();

    connect(clientSocket, &QTcpSocket::connected, this, &Client::onConnection);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Client::onDisconnection);
    connect(clientSocket, &QTcpSocket::readyRead, this, &Client::onReadyRead);

    createUI();
    isConnected = false;
}

void Client::showUI() {
    show();
}

Client::~Client() {

}

void Client::createUI() {
    grid = new QGridLayout();

    label = new QLabel(tr("Enter IP:"));
    textBox = new QLineEdit();
    textBox->setInputMask("000.000.000.000; ");

    buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    buttons->setCenterButtons(true);


    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        onConnection();
        done(QDialog::Accepted);
    });

    connect(buttons, &QDialogButtonBox::rejected, this, [this]() {
        onDisconnection();
        done(QDialog::Rejected);
    });

    grid->addWidget(label, 0, 0, 1, 1);
    grid->addWidget(textBox, 0, 1, 1, 1);
    grid->addWidget(buttons, 1, 0, 1, 2);
    setLayout(grid);

    this->setFixedSize(225, 100);

}

void Client::onConnection() {
    serverIP = textBox->text();
    if (!isConnected) {
        clientSocket->connectToHost(QHostAddress(serverIP), 8888);
        if (clientSocket->waitForConnected(1000)) {
            QMessageBox::information(this, "", "Connection succeeded!");
            isConnected = true;
        } else {
            clientSocket->disconnect();
            QMessageBox::warning(this, "", "Connection failed!");
        }

    }
}

void Client::onDisconnection() {
    clientSocket->disconnectFromHost();
    clientSocket = nullptr;
    QMessageBox::information(this, "", "Disconnection succeeded!");
}

void Client::onReadyRead() {

    if (clientSocket == nullptr) return;

    while(clientSocket->canReadLine()) {
        QString msg = clientSocket->readLine();
        QTextStream in(&msg);



    }
}

void Client::sendDataSlot(QString msg) {
    QByteArray str = msg.toUtf8();
    str.append('\n');
    if (clientSocket != nullptr) {
        clientSocket->write(str);
        qDebug() << str;
    }
}
