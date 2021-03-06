#include "serverlogic.h"
#include <QMessageBox>
#include <QObject>

ServerLogic::ServerLogic(QWidget *parent) : QWidget(parent) {

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &ServerLogic::timeUpdate);

    isColorGived = false;
    turnCount = 1;
    mines[0] = mines[1] = 3;
}

void ServerLogic::setChessboard(Chessboard* chessboard) {
    this->chessboard = chessboard;
}

void ServerLogic::setNowPlayer(Player* player) {
    nowPlayer = player;
}

void ServerLogic::setNextPlayer(Player* player) {
    nextPlayer = player;
}

void ServerLogic::timerStart(int time) {
    timeRemain = time;
    timer->start(1000);

    for (int i = 0; i < 2; i++)
        emit sendData("t " + QString::number(timeRemain), i);
}

void ServerLogic::timeUpdate() {
    timer->stop();
    timeRemain--;

    for (int i = 0; i < 2; i++)
        emit sendData("t " + QString::number(timeRemain), i);

    if (timeRemain <= 0) {
        timeover();
    } else {
        timer->start();
    }
}

void ServerLogic::changePlayer() {
    timer->stop();

    turnCount++;
    Player* p = nowPlayer;
    nowPlayer = nextPlayer;
    nextPlayer = p;

    for (int i = 0; i < 2; i++) {
        emit sendData("o", i);
        emit sendData("p " + QString::number(nowPlayer->getID()), i);
    }

    if (checkMovable()) {
        timerStart(20);
    } else {
        gameEnded(1, nowPlayer->getID());
    }
}

bool ServerLogic::checkMovable() {
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 12; j++) {
            Chess *chess = chessboard->get(i,j);
            if (chess == nullptr) continue;
            if (chess->getStatus() == 0) return true;
            if (chess->getColor() != nowPlayer->getColor()) continue;

            int type = chess->getType();
            if (type == 10 || type == 12) continue;

            int u = i + 5 * j;
            for (int i = 0; i < route[u].size(); i++) {
                int v = route[u][i].first;
                Chess* targetChess = chessboard->get(v % 5, v / 5);

                if (targetChess == nullptr) return true;
                if (targetChess->getStatus() == 1 && targetChess->getColor() != nowPlayer->getColor()) return true;
                int type2 = targetChess->getType();
                switch(type) {
                case 11:
                    switch(type2) {
                    case 12:
//                        if (nextPlayer->getLandminesRemains() == 0) return true;
                        if (mines[nextPlayer->getID()] == 0) return true;
                        break;
                    case 10:
                        return true;
                        break;
                    default:
                        return true;
                        break;
                    }
                    break;
                case 9:
                    switch(type2) {
                    case 12:
//                        if (nextPlayer->getLandminesRemains() == 0) return true;
                        if (mines[nextPlayer->getID()] == 0) return true;
                        break;
                    case 10:
                        return true;
                        break;
                    case 9:
                        return true;
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    switch(type2) {
                    case 12:
//                        if (nextPlayer->getLandminesRemains() == 0) return true;
                        if (mines[nextPlayer->getID()] == 0) return true;
                        break;
                    case 11: case 10:
                        break;
                    default:
                        if (type == type2) return true;
                        else if (type < type2) return true;
                        break;
                    }
                    break;
                }
            }
       }

    return false;
}

void ServerLogic::timeover() {

    emit sendData("w 0", nowPlayer->getID());

    for (int i = 0; i < 2; i++)
        emit sendData("6 " + QString::number(turnCount) + " " + QString::number(nowPlayer->getColor()), i);

    timer->stop();

    nowPlayer->setTimeoutCount();
    if (nowPlayer->getTimeoutCount() >= 3) gameEnded(1, nowPlayer->getID());
    else changePlayer();

}

void ServerLogic::gameEnded(int cmd, int id) {
    timer->stop();
    if (cmd == 0) {
        emit sendData("0 [Server] Game over! You Win !", id);
        emit sendData("g 0", id);
        emit sendData("0 [Server] Game over! You Lose!", 1 - id);
        emit sendData("g 1", 1 - id);
    } else {
        emit sendData("0 [Server] Game over! You Win !", 1 - id);
        emit sendData("g 0", 1 - id);
        emit sendData("0 [Server] Game over! You Lose!", id);
        emit sendData("g 1", id);
    }
    emit sendData("z", 0);
    emit sendData("z", 1);
}

void ServerLogic::onChangeChessNULL(QString msg, int x, int y) {
//    delete chessboard->get(x, y);
    chessboard->changeChess(nullptr, x, y);

    for (int i = 0; i < 2; i++)
        emit sendData(msg, i);
}

void ServerLogic::onChangeChess(QString msg, int x, int y, int status, int color, int type) {
//    delete chessboard->get(x, y);
    Chess *chess = new Chess(x, y, type, color, status);
    chessboard->changeChess(chess, x, y);

    for (int i = 0; i < 2; i++)
        emit sendData(msg, i);
}

void ServerLogic::flopChess(int x, int y) {
    Chess* chess = chessboard->get(x, y);

    chess->changeStatus();
    int color = chess->getColor();
    int type = chess->getType();
    int status = chess->getStatus();

    for (int i = 0; i < 2; i++)
        emit sendData("1 " + QString::number(turnCount) + " " + QString::number(color) + " " + QString::number(type) + " " + QString::number(x + 5 * y), i);

    if (!isColorGived) {
        if (nowPlayer->getLastColor() == color) {

            nowPlayer->changeColor(color);
            emit sendData("b " + QString::number(color) + " " + QString::number(nowPlayer->getID()), nowPlayer->getID());
            emit sendData("0 [Server] Your color is " + QString((color == 1)? "Red": "Blue") + " now!", nowPlayer->getID());
            if (color == 1) {
                emit sendData("m 0", nowPlayer->getID());
                emit sendData("m 1", nextPlayer->getID());
            } else {
                emit sendData("m 1", nowPlayer->getID());
                emit sendData("m 0", nextPlayer->getID());
            }
            nextPlayer->changeColor(3 - color);
            emit sendData("b " + QString::number(3 - color) + " " + QString::number(nextPlayer->getID()), nextPlayer->getID());
            emit sendData("0 [Server] Your color is " + QString((3 - color == 1)? "Red": "Blue") + " now!", nextPlayer->getID());

            isColorGived = true;

        }

        nowPlayer->changeLastColor(color);
    }

    QString msg = "c " + QString::number(x) + " " + QString::number(y) + " " + QString::number(status) + " " + QString::number(color) + " " + QString::number(type);
    for (int i = 0; i < 2; i++)
        emit sendData(msg, i);

    changePlayer();
}

void ServerLogic::getSurrender(int id) {
    if (turnCount >= 20) {
        gameEnded(1, id);
    } else {
        emit sendData("w 1", id);
    }
}

void ServerLogic::endGame(int id) {
    gameEnded(0, nowPlayer->getID());
}

void ServerLogic::initialize(int id) {
    //player info (cmd = 0)
    int firstPlayer = nowPlayer->getID();
    if (firstPlayer == 0) {
        emit sendData("q 0", id);
        emit sendData("q 0", 1 - id);
    } else if (firstPlayer == 1) {
        emit sendData("q 1", id);
        emit sendData("q 1", 1 - id);
    }

    //chessboard info (cmd = 1)
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 12; j++) {
            Chess* nowChess = chessboard->get(i, j);
            if (nowChess == nullptr) continue;
            QString pos = QString::number(i) + " " + QString::number(j);
            QString color = QString::number(nowChess->getColor());
            QString status = QString::number(nowChess->getStatus());
            QString type = QString::number(nowChess->getType());
            QString msg = "c " + pos + " " + status + " " + color + " " + type;
            emit sendData(msg, id);

        }
}

void ServerLogic::onMineBoomed(int id) {
    mines[id]--;
    int nowID = nowPlayer->getID();
    if (nowID == id) {
        emit sendData("0 [Server] Your team remains " + QString::number(mines[id]) + " landmines!", nowID);
        emit sendData("0 [Server] Enemy team remains " + QString::number(mines[id]) + " landmines!", 1 - nowID);
        //nowPlayer->lostLandmines();
    } else {
        emit sendData("0 [Server] Your team remains " + QString::number(mines[id])  + " landmines!", 1 -nowID);
        emit sendData("0 [Server] Enemy team remains " + QString::number(mines[id])  + " landmines!", nowID);
        //nextPlayer->lostLandmines();
    }

    for (int i = 0; i < 2; i++)
        emit sendData("l " + QString::number(id), i);
}
