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
#include <QProcess>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("androvmgr");
    QCoreApplication::setApplicationName("androvmgr");
//    QCoreApplication::setOrganizationDomain("mithatkonar.com");

    ui->setupUi(this);

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

void MainWindow::on_action_Preferences_triggered()
{
    // TODO: Preferences dialog
    qDebug() << "Preferences not yet implemented. Hand edit the config file.";
    ui->statusBar->showMessage(tr("Preferences not yet implemented. :-("));
}

void MainWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_Run_emulator_triggered()
{
    QString program = "VBoxManage";
    QStringList arguments;
    arguments << "startvm" << ui->lineEdit_vmName->text();

    QProcess *theProcess = new QProcess(this);
    theProcess->start(program, arguments);

    // TODO: see if process started nicely and branch as appropriate.
    ui->statusBar->showMessage(tr("VM started"));
}

void MainWindow::on_action_Connect_triggered()
{
    ui->statusBar->showMessage(tr("connecting..."));

    QString program = adbPath + "/adb";
    QStringList arguments;
    arguments << "connect" << ui->lineEdit_ipAddr->text();

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);

    // capture standard out and show.
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();
    ui->statusBar->showMessage(p_stdout);
}

void MainWindow::on_actionDisconnect_triggered()
{
    ui->statusBar->showMessage(tr("disconnecting..."));

    QString program = adbPath + "/adb";
    QStringList arguments;
    arguments << "disconnect" << ui->lineEdit_ipAddr->text();

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);

    // capture standard out and show.
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();
    if (p_stdout == "\n")
        ui->statusBar->showMessage(tr("disconnected"));
    else
        ui->statusBar->showMessage(p_stdout);
}

void MainWindow::on_action_About_triggered()
{
    // TODO: About box strings need to go someplace better.
    // TODO: License info.
    QMessageBox::about(this,
                       tr("About Android VM Manager"),
                       tr("Android VM Manager\n\nCopyright (C) 2013 Mithat Konar"));
}

// Read all settings.
void MainWindow::readSettings()
{
    QSettings settings;

    settings.beginGroup("vm");
    ui->lineEdit_vmName->setText(settings.value("name").toString());
    ui->lineEdit_ipAddr->setText(settings.value("ip_addr").toString());
    settings.endGroup();

    settings.beginGroup("executables");
    adbPath = settings.value("adb_path").toString();
    settings.endGroup();
}

// Write all settings.
void MainWindow::writeSettings()
{
    writeSettingsVM();
    writeSettingsExecutables();
}

// Write settings related to Virtual Machine.
void MainWindow::writeSettingsVM()
{
    QSettings settings;
    settings.beginGroup("vm");
    settings.setValue("name", ui->lineEdit_vmName->text());
    settings.setValue("ip_addr", ui->lineEdit_ipAddr->text());
    settings.endGroup();
}

// Write settings related to executables.
void MainWindow::writeSettingsExecutables()
{
    QSettings settings;
    settings.beginGroup("executables");
    settings.setValue("adb_path", adbPath);
    settings.endGroup();
}

