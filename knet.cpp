/****************************************************************************
**
**
** Copyright 2016 Ludovico Depetris
**
**
** knet.cpp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** knet.cpp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with knet.cpp.  If not, see <http://www.gnu.org/licenses/>.
**
**
*****************************************************************************/


#include "knet.h"
#include <QTextStream>

KNet::KNet(QObject *parent) : QThread(parent)
{

    working_dir = QDir::homePath();
    _using = new QFile();
    quit = _loading = _loaded = _saving = false;
}

KNet::~KNet()
{
    disconnect();
}

void KNet::connect(const QString &addr, const quint16 &port, const int &timeout)
{
    mutex.lock();
    srv_addr = addr;
    srv_port = port;
    srv_timeout = timeout;
    quit = false;
    mutex.unlock();
    makeRequest(QString("as"));
}

void KNet::disconnect()
{
    mutex.lock();
    quit = true;
    mutex.unlock();
    wait();
}

void KNet::setWorkingDir(const QString &dir)
{
    working_dir = dir;
}

void KNet::makeRequest(const QString &msg)
{
    mutex.lock();
    request = QByteArray(QString(msg + "\n").toUtf8());
    has_req = true;
    mutex.unlock();

    if (!isRunning())
        start();

}

void KNet::makeRequest(const QByteArray &bytes)
{
    mutex.lock();
    request = bytes;
    has_req = true;
    mutex.unlock();

    if (!isRunning())
        start();
}

void KNet::run()
{
    mutex.lock();
    QString _addr = srv_addr;
    quint16 _port = srv_port;
    int _timeout = srv_timeout;
    mutex.unlock();

    socket = new QTcpSocket();
    bool _quit, _has_req;
    _quit = _has_req = false;

    if(socket->state() != QAbstractSocket::ConnectedState)
    {        
        socket->connectToHost(_addr, _port, QTcpSocket::ReadWrite);

        if (!socket->waitForConnected(_timeout))
        {

            emit error(socket->errorString());

            return;
        }
    }

    do
    {

        // Check connection state
        if(socket->state() != QAbstractSocket::ConnectedState)
        {
            emit error("Connection closed.");
            return;
        }

        // Handle shared variables
        mutex.lock();
        QByteArray _req(request);
        if(this->has_req && _has_req)
        {
            has_req = _has_req = false;
        }
        else if(this->has_req)
        {
            _has_req = true;
        }
        _quit = quit;
        mutex.unlock();

        _readFromServer();

        if(_has_req){
            _writeToTheServer(_req);
        }

    }
    while(!_quit);

    socket->disconnectFromHost();
}

void KNet::_readFromServer(const int &msec)
{
    // Try to read and 15msec for bytes available
    if(socket->waitForReadyRead(msec))
    {
        // Read bytes available
        while(socket->bytesAvailable() > 0){
            _bresponse += socket->readAll();
        }
    }
    // If reading is finished
    else if(!_bresponse.isEmpty())
    {
        emit readyResponse(_handleResponse(_bresponse));
        _bresponse.clear();

    }
}

void KNet::_writeToTheServer(const QByteArray &btw, const int &msec)
{
    // Writing bytes on the socket
    socket->write(btw);
    // Wait to write and check for errors

    if (!socket->waitForBytesWritten(msec)) {
        emit error(socket->errorString());
        return;
    }
}



QString KNet::_handleResponse(const QByteArray &br)
{
    QString _resp(br);
    _resp.remove(QRegExp("(\u0017)|(\u0018)"));

    int _p = _resp.indexOf(QRegExp("(\u0005\u0002[ABCDE])"));

    if( _p >= 0)
    {
        QChar _sub = _resp.at(_p+2);

        if(_sub == 'B')
        {
            QDir::setCurrent(working_dir);

            _using->setFileName(_resp.mid(_p+3));
            _using->open(QIODevice::WriteOnly);

            char packet_bytes[] = {
                0x02, 0x42, 0x20, 0x20, 0x20, 0x20, 0x30, 0x17,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a
            };

            QByteArray ar(packet_bytes);

            _writeToTheServer(ar);
            _saving = true;
            _resp = _resp.left(_p);
        }


        if(_sub == 'D' && _saving)
        {

            if(_using->isOpen())
            {
                QString _txt = _resp.mid(_p).remove(QRegExp("(Program [^\n\r]+\r\n)", Qt::CaseSensitive));
                _txt.replace(QRegExp("(\r\nReal\r\n)"), "\r\n");
                _txt.replace(QRegExp("(\r\nTransformation\r\n)"), "\r\n");
                _txt.replace(QRegExp("(\r\nString\r\n)"), "\r\n");
                _txt = _txt.remove(QRegExp("(\u0017{0,1}\u0005\u0002[DE]{0,1})"));
                QTextStream out(_using);
                out << _txt;
            }

            if(_resp.endsWith("E"))
            {

                if(_using->isOpen()) _using->close();

                char packet_bytes[] = {
                    0x02, 0x45, 0x20, 0x20, 0x20, 0x20, 0x30, 0x17,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a
                };

                QByteArray ar(packet_bytes);
                _writeToTheServer(ar);
                _saving = false;
            }
            _resp = _resp.left(_p);
        }

        if(_sub == 'A')
        {

            QDir::setCurrent(working_dir);

            _using->setFileName(_resp.mid(_p+3));
            _using->open(QIODevice::ReadOnly);

            char start_load[] = {
                0x02, 0x41, 0x20, 0x20, 0x20, 0x20, 0x30, 0x17
            };

            _writeToTheServer(QByteArray(start_load));
            _loading = true;
            _resp = _resp.left(_p);
        }

        if(_sub == 'C' && _loading)
        {
            if(_using->isOpen() && !_loaded && _resp.contains("Loading") && !_resp.endsWith("\u0005\u0002E"))
            {

                QByteArray _bl;

                char header[] = {
                    0x02, 0x43, 0x20, 0x20, 0x20, 0x20, 0x30, 0x2e
                };

                _bl.append(header);
                _bl.resize(7);
                _bl.append(_using->readAll());
                _bl.append(0x17);
                _writeToTheServer(_bl);
                _using->close();
                _loaded = true;

                _resp = _resp.mid(_p+3);

            }
            else if(_loaded)
            {

                char load_ok[] = {
                    0x02, 0x43, 0x20, 0x20, 0x20, 0x20, 0x30, 0x1a,
                    0x17
                };
                _writeToTheServer(QByteArray(load_ok));
                _loaded = false;
                _resp = _resp.mid(_p+3);
            }
        }

        if(_sub == 'E' && _loading)
        {
            char end_load[] = {
                0x02, 0x45, 0x20, 0x20, 0x20, 0x20, 0x30, 0x17,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a
            };
            _writeToTheServer(QByteArray(end_load));
            _loading = false;
            _resp = _resp.left(_p);
        }
    }


    return _resp;

}
