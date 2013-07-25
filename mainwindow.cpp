/*
 *
 * Copyright 2013 Mithat Konar
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->toolBtn_run->setDefaultAction(ui->action_Run_emulator);
    ui->toolBtn_connect->setDefaultAction(ui->action_Connect);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionPreferences_triggered()
{
    // TODO: launch preferences dialog
    qDebug() << "on_actionPreferences_triggered()";
}

void MainWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_Run_emulator_triggered()
{
    // TODO: issue QProcess: VBoxManage startvm <name-of-vm>
    // https://qt-project.org/doc/qt-4.8/qprocess.html
    qDebug() << "on_action_Run_emulator_triggered()";
}

void MainWindow::on_action_Connect_triggered()
{
    // TODO: issue system command: <path-to>/adm connect <ip-address>
    qDebug() << "on_action_Connect_triggered()";
}

void MainWindow::on_action_About_triggered()
{
    // TODO: About box strings need to go someplace better.
    QMessageBox::about(this, tr("About AndroVM Manager"), tr("Copyright (C) 2013 Mithat Konar"));
}

void MainWindow::on_lineEdit_vmName_editingFinished()
{
    // TODO: Stick updated string into preferences
    qDebug() << ui->label_vmName->text() << ": " << ui->lineEdit_vmName->text();
}

void MainWindow::on_lineEdit_ipAddr_editingFinished()
{
    // TODO: Stick updated string into preferences
    qDebug() << ui->label_ipAddr->text() << ": "<< ui->lineEdit_ipAddr->text();
}
