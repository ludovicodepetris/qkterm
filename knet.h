/****************************************************************************
**
**
** Copyright 2016 Ludovico Depetris
**
**
** knet.h is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** knet.h is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with knet.h.  If not, see <http://www.gnu.org/licenses/>.
**
**
*****************************************************************************/


#ifndef KNET_H
#define KNET_H

#include <QThread>
#include <QMutex>
#include <QtNetwork>

class KNet : public QThread
{
    Q_OBJECT

public:
    KNet(QObject *parent = 0);
    virtual ~KNet();
    void connect(const QString &addr = "127.0.0.1", const quint16 &port = 9105, const int &timeout = 3000);
    void disconnect();
    void setWorkingDir(const QString &dir);
    void makeRequest(const QString &msg);
    void makeRequest(const QByteArray &bytes);
    void run();


protected:
    void _readFromServer(const int &msec = 20);
    void _writeToTheServer(const QByteArray &btw, const int &msec = 30000);
    QString _handleResponse(const QByteArray &br);

signals:
    void readyResponse(const QString &response);
    void error(const QString &error);

private:
    QString srv_addr;
    quint16 srv_port;
    int srv_timeout;
    QByteArray request;
    QString response;
    QMutex mutex;
    QTcpSocket *socket;
    bool quit;
    bool has_req, _saving, _loading, _loaded;
    QString working_dir;
    QFile *_using;
    QByteArray _bresponse;

};

#endif // KNET_H
