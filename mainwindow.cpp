/****************************************************************************
**
**
** Copyright 2016 Ludovico Depetris
**
**
** mainwindow.cpp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** mainwindow.cpp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mainwindow.cpp.  If not, see <http://www.gnu.org/licenses/>.
**
**
*****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    bot = new KNet();

    connect(bot,SIGNAL(readyResponse(QString)), this, SLOT(handleResponse(QString)));

    connect(bot,SIGNAL(error(QString)), this, SLOT(handleError(QString)));
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleResponse(const QString &msg)
{

    ui->textResult->textCursor().movePosition(QTextCursor::End);
    ui->textResult->textCursor().insertText(msg);
    QScrollBar *sb = ui->textResult->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::handleError(const QString &msg)
{
    ui->textResult->textCursor().movePosition(QTextCursor::End);
    ui->textResult->textCursor().insertText(msg);
    QScrollBar *sb = ui->textResult->verticalScrollBar();
    sb->setValue(sb->maximum());
    ui->pushConnect->setChecked(false);

}

void MainWindow::on_lineCommand_returnPressed()
{
    bot->makeRequest(ui->lineCommand->text());
    ui->lineCommand->clear();
}


void MainWindow::onDisconnected()
{
    ui->pushConnect->setText("Connetti");
    ui->lineAddress->setEnabled(true);
    ui->spinPort->setEnabled(true);
    ui->lineCommand->setEnabled(false);
}

void MainWindow::onConnected()
{
    ui->pushConnect->setText("Disconnetti");
    ui->lineAddress->setEnabled(false);
    ui->spinPort->setEnabled(false);
    ui->lineCommand->setEnabled(true);
}

void MainWindow::on_pushConnect_toggled(bool checked)
{
    if(checked){
        bot->connect(ui->lineAddress->text(), ui->spinPort->value());
        onConnected();
    }else{
        bot->disconnect();
        onDisconnected();
    }
}
