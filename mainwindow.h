/****************************************************************************
**
**
** Copyright 2016 Ludovico Depetris
**
**
** mainwindow.h is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** mainwindow.h is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mainwindow.h.  If not, see <http://www.gnu.org/licenses/>.
**
**
*****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMutex>
#include "knet.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:
    void handleResponse(const QString &msg);

    void handleError(const QString &msg);

    void on_lineCommand_returnPressed();

    void onDisconnected();

    void onConnected();

    void on_pushConnect_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    KNet *bot;
};

#endif // MAINWINDOW_H
