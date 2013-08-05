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

//TODO: Rename actions to be consistent regarding underscores
//TODO: Add "Machine" or similar menu to take Start, Shutdown, Connect, etc.
//TODO: Add saving of window size to settings.

//TODO: Do STATUSBAR_TIMEOUT in a less kludgey way.
#define STATUSBAR_TIMEOUT 8000

/**
 * @brief constructor
 * @param parent  The window's parent (Qwidget*)
 */
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

/**
 * @brief deconstructor
 */
MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}

//==== Actions ====//

/**
 * @brief Exit the app.
 */
void MainWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

/**
 * @brief Display a dialog box that allows app preferences to be set.
 */
void MainWindow::on_action_Preferences_triggered()
{
    // Show preferences dialog, record changes in settings file
    PreferencesDialog preferencesDialog(this);
    preferencesDialog.setModal(true);
    preferencesDialog.exec();

    // Reset app settings from file
    readSettings();

    ui->statusBar->showMessage(tr("Preferences saved"), STATUSBAR_TIMEOUT);
}

/**
 * @brief Start the VM.
 */
void MainWindow::on_action_Run_emulator_triggered()
{
    QString program = "VBoxManage";
    QStringList arguments;
    arguments << "startvm" << ui->lineEdit_vmName->text();

    QProcess *theProcess = new QProcess(this);
    theProcess->start(program, arguments);

    theProcess->waitForFinished(-1);
    if (theProcess->exitCode() == 0)
        ui->statusBar->showMessage(tr("VM started"), STATUSBAR_TIMEOUT);
    else
        ui->statusBar->showMessage(tr("Problem starting VM. Is it already running?"));
}

/**
 * @brief Send the 'acpipowerbutton" signal to the VM.
 */
void MainWindow::on_actionACPI_shutdown_triggered()
{
    QString program = "VBoxManage";
    QStringList arguments;
    arguments << "controlvm" << ui->lineEdit_vmName->text() << "acpipowerbutton";

    QProcess *theProcess = new QProcess(this);
    theProcess->start(program, arguments);

    theProcess->waitForFinished(-1);
    if (theProcess->exitCode() == 0)
        ui->statusBar->showMessage(tr("Shutdown signal sent to VM"), STATUSBAR_TIMEOUT);
    else
        ui->statusBar->showMessage(tr("Problem sending shutdown signal. Is VM running?"));
}

/**
 * @brief Display a dialog that shows information about the VM.
 */
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

/**
 * @brief Connect the VM to the ADB.
 */
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
        ui->statusBar->showMessage(p_stdout, STATUSBAR_TIMEOUT);
}

/**
 * @brief Disconnect the VM from the ADB.
 */
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
        ui->statusBar->showMessage(tr("disconnected"), STATUSBAR_TIMEOUT);
    else
        ui->statusBar->showMessage(p_stdout);
}

/**
 * @brief Display a dialog that shows information about this app.
 */
void MainWindow::on_action_About_triggered()
{
    // TODO: About box strings should go someplace better.
    QMessageBox::about(this,
                       tr("About Android VM Manager"),
                       "<b>" + tr("Android VM Manager") + " " + VER + "</b><br />" +
                       "<br />" +
                       tr("Copyright (C) 2013 Mithat Konar") + "<br />" +
                       tr("Licensed under the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GPLv3</a>."));
}

//==== Settings ====//

/**
 * @brief Read all settings.
 */
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
}

/**
 * @brief Write all settings.
 */
void MainWindow::writeSettings()
{
    writeSettingsVM();
    writeSettingsExecutables();
}

/**
 * @brief Write settings related to Virtual Machine.
 */
void MainWindow::writeSettingsVM()
{
    QSettings settings;
    settings.beginGroup("vm");
    settings.setValue("name", ui->lineEdit_vmName->text());
    settings.setValue("ip_addr", ui->lineEdit_ipAddr->text());
    settings.endGroup();
}

/**
  * @brief Write settings related to executables.
  */
void MainWindow::writeSettingsExecutables()
{
    QSettings settings;
    settings.beginGroup("executables");
    settings.setValue("adb_dir", adbExe);
    settings.setValue("adb_on_path", isAdbOnPath);
    settings.endGroup();
}
