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
#include "preferencesdialog.h"

#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("androidvmgr");
    QCoreApplication::setApplicationName("androidvmgr");
//    QCoreApplication::setOrganizationDomain("mithatkonar.com");

    ui->setupUi(this);

    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

//==== Actions ====//

void MainWindow::on_action_Preferences_triggered()
{
    // Show preferences dialog, record changes in settings file
    PreferencesDialog preferencesDialog(this);
    preferencesDialog.setModal(true);
    preferencesDialog.exec();

    // Reset app settings from file
    readSettings();

    ui->statusBar->showMessage(tr("Preferences saved"));
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

    theProcess->waitForFinished(-1);
    if (theProcess->exitCode() == 0)
        ui->statusBar->showMessage(tr("VM started"));
    else
        ui->statusBar->showMessage(tr("Problem starting VM. Is it already running?"));
}

void MainWindow::on_actionVM_info_triggered()
{
    // Ask VBoxManage for a available VM properties.
    QString program = "VBoxManage";
    QStringList arguments;
    arguments << "guestproperty" << "enumerate" << ui->lineEdit_vmName->text();

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();

    // Report results
    // TODO: Custom dialog that makes VM info less ugly.
    QMessageBox::information(this, tr("VM info"), p_stdout);
}

void MainWindow::on_action_Connect_triggered()
{
    ui->statusBar->showMessage(tr("connecting..."));

    // set up and execute a process to start adb and connect
    QString program;
    if (isAdbOnPath)
        program = "adb";    // TODO: this should be abstracted out and conditionally compiled for different platforms
    else
        program = adbExe;

    QStringList arguments;
    arguments << "connect" << ui->lineEdit_ipAddr->text();

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);

    // capture standard out and show.
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();
    if (p_stdout.isEmpty())
        ui->statusBar->showMessage(tr("Problem running adb. Is the location set?"));
    else
        ui->statusBar->showMessage(p_stdout);
}

void MainWindow::on_actionDisconnect_triggered()
{
    ui->statusBar->showMessage(tr("disconnecting..."));

    // set up and execute a process to start adb and disconnect
    QString program;
    if (isAdbOnPath)
        program = "adb";    // TODO: this should be abstracted out and conditionally compiled for different platforms
    else
        program = adbExe;

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
    // TODO: License info in about box.
    QMessageBox::about(this,
                       tr("About Android VM Manager"),
                       tr("Android VM Manager\n\nCopyright (C) 2013 Mithat Konar"));
}

//==== Settings ====//

// Read all settings.
void MainWindow::readSettings()
{
    QSettings settings;

    settings.beginGroup("vm");
    ui->lineEdit_vmName->setText(settings.value("name").toString());
    ui->lineEdit_ipAddr->setText(settings.value("ip_addr").toString());
    settings.endGroup();

    settings.beginGroup("executables");
    adbExe = settings.value("adb_dir").toString();
    isAdbOnPath = settings.value("adb_on_path").toBool();
    settings.endGroup();

    qDebug() << "adbDir:\t" << adbExe;
    qDebug() << "adbOnPath:\t" << isAdbOnPath;
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
    settings.setValue("adb_dir", adbExe);
    settings.setValue("adb_on_path", isAdbOnPath);
    settings.endGroup();
}
