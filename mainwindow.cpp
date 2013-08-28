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

// TODO: Convert UI "VM Name" lineEdit to auto-populated menu of some sort.
// TODO: Do STATUSBAR_TIMEOUT in a less kludgey way.
#define STATUSBAR_TIMEOUT 8000

/**
 * @brief constructor
 * @param parent  The window's parent (Qwidget*)
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    configureTrayIcon();
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

// =======
// Actions
// =======

/**
 * @brief Exit the app.
 */
void MainWindow::on_actionQuit_triggered()
{
    bool isQuit = true;
    if (isVMrunning())
    {
        // kludge to work around action initialed by notification icon:
        bool wasHidden = false;
        if (this->isHidden())
        {
            wasHidden = true;
            this->show();
        }

        // prompt for ACPI shutdown of VM:
        QMessageBox::StandardButton isSend = QMessageBox::question(
                    this,tr("VM is running"),
                    tr("Send ACPI shutdown signal to the Virtaul Machine?"),
                    QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                    QMessageBox::Yes);
        switch (isSend)
        {
        case QMessageBox::Yes:
            on_actionACPI_shutdown_triggered();
            break;
        case QMessageBox::No:
            break;
        default:
            isQuit = false;
            break;
        }

        // second part of kludge
        if (wasHidden)
            this->hide();
    }

    if (isQuit)
    {
        qApp->quit();
//    this->close();
    }
}

/**
 * @brief Display a dialog box that allows app preferences to be set.
 */
void MainWindow::on_actionPreferences_triggered()
{
    // Show preferences dialog, record changes in settings file
    PreferencesDialog preferencesDialog(this);
    preferencesDialog.setModal(true);
    preferencesDialog.exec();

    // Reset settings group(s) changed in dialog from file
    readSettingsExecutables();

    ui->statusBar->showMessage(tr("Preferences saved."), STATUSBAR_TIMEOUT);
}

/**
 * @brief Start the VM.
 */
void MainWindow::on_actionRun_emulator_triggered()
{
    if (isVMrunning())
    {
        ui->statusBar->showMessage(tr("VM is already running."), STATUSBAR_TIMEOUT);
        return;
    }

    QString program = "VBoxManage";
    QStringList arguments;
    arguments << "startvm" << getVMname();

    QProcess *theProcess = new QProcess(this);
    theProcess->start(program, arguments);

    theProcess->waitForFinished(-1);
    if (theProcess->exitCode() == 0)
        ui->statusBar->showMessage(tr("VM started."), STATUSBAR_TIMEOUT);
    else
        ui->statusBar->showMessage(tr("Problem starting VM. Is it already running?"));
}

/**
 * @brief Send the 'acpipowerbutton" signal to the VM.
 */
void MainWindow::on_actionACPI_shutdown_triggered()
{
    if (!isVMrunning())
    {
        ui->statusBar->showMessage(tr("VM is not running."), STATUSBAR_TIMEOUT);
        return;
    }

    QString program = "VBoxManage";
    QStringList arguments;
    arguments << "controlvm" << getVMname() << "acpipowerbutton";

    QProcess *theProcess = new QProcess(this);
    theProcess->start(program, arguments);

    theProcess->waitForFinished(-1);
    if (theProcess->exitCode() == 0)
        ui->statusBar->showMessage(tr("Shutdown signal sent to VM."), STATUSBAR_TIMEOUT);
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
    arguments << "guestproperty" << "enumerate" << getVMname();

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
 * @brief Start ADB server.
 */
void MainWindow::on_actionStart_server_triggered()
{
    // set up and execute a process to start adb server
    QString program = getADB();
    QStringList arguments;
    arguments << "start-server";

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);

    // capture standard out and show.
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();
    if (p_stdout.isEmpty())
        ui->statusBar->showMessage(tr("Server not started. Already running?"));
    else
        showLastLineinStatusBar(p_stdout);
}

/**
 * @brief Kill ADB server.
 */
void MainWindow::on_actionStop_server_triggered()
{
    // set up and execute a process to kill adb server
    QString program = getADB();
    QStringList arguments;
    arguments << "kill-server";

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);

    // capture standard out and show.
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();
    if (p_stdout.isEmpty())
        ui->statusBar->showMessage(tr("Message sent."));
    else
        showLastLineinStatusBar(p_stdout);
}

/**
 * @brief Connect the VM to the ADB.
 */
void MainWindow::on_actionConnect_triggered()
{
    if (!isVMrunning())
    {
        ui->statusBar->showMessage(tr("VM is not running."), STATUSBAR_TIMEOUT);
        return;
    }

    ui->statusBar->showMessage(tr("connecting..."));

    // set up and execute a process to start adb and connect
    QString program = getADB();
    QStringList arguments;
    arguments << "connect" << getIPAddr();

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);

    // capture standard out and show.
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();
    if (p_stdout.isEmpty())
        ui->statusBar->showMessage(tr("Problem running ADB. Is the location set?"));
    else
        showLastLineinStatusBar(p_stdout);
}

/**
 * @brief Disconnect the VM from the ADB.
 */
void MainWindow::on_actionDisconnect_triggered()
{
    if (!isVMrunning())
    {
        ui->statusBar->showMessage(tr("VM is not running."), STATUSBAR_TIMEOUT);
        return;
    }

    ui->statusBar->showMessage(tr("disconnecting..."));

    // set up and execute a process to start adb and disconnect
    QString program = getADB();

    QStringList arguments;
    arguments << "disconnect" << getIPAddr();

    QProcess *theProcess = new QProcess(this);
    theProcess->setProcessChannelMode(QProcess::MergedChannels);
    theProcess->start(program, arguments);

    // capture standard out and show.
    theProcess->waitForFinished(-1);
    QString p_stdout = theProcess->readAllStandardOutput();
    if (p_stdout.isEmpty())
        ui->statusBar->showMessage(tr("Problem running ADB. Is the location set?"));
    else if (p_stdout == "\n")
        ui->statusBar->showMessage(tr("disconnected"), STATUSBAR_TIMEOUT);
    else
        showLastLineinStatusBar(p_stdout);
}

/**
 * @brief Display a dialog that shows information about this app.
 */
void MainWindow::on_actionAbout_triggered()
{
    // TODO: About box strings should go someplace better.
    QMessageBox::about(this,
                       tr("About Android VM Manager"),
                       "<b>" + tr("Android VM Manager") + " " + VER + "</b><br />" +
                       "<br />" +
                       tr("Copyright &copy; 2013 Mithat Konar") + "<br />" +
                       tr("Licensed under the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GPLv3</a>") + "<br />" +
                       "<br />" +
                       "<a href=\"https://bitbucket.org/mithat/androidvmgr\">https://bitbucket.org/mithat/androidvmgr</a><br />"
                       );
}


// ===========
// Other slots
// ===========

/**
 * @brief Process mouse clicks on system tray icon.
 * @param reason The kind of mouse click (QSystemTrayIcon::ActivationReason)
 */
void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        // toggle visibility
        this->setVisible(!this->isVisible());
        break;
    default:
        ;
    }
}

// ===============
// Service members
// ===============

/**
 * @brief Show the last line contained in msg in the status bar.
 * @param msg A body of (optionally multiline) text. (QString)
 */
void MainWindow::showLastLineinStatusBar(QString msg)
{
    // The following will effectively show only the last non-blank line in the status bar
    // and send other lines to console.
    QStringList lines = msg.split("\n");
    Q_FOREACH (QString line, lines)
    {
        qDebug() << line;
        if (!line.isEmpty())
            ui->statusBar->showMessage(line, STATUSBAR_TIMEOUT);
    }
}

/**
 * @brief Return a string that is the name and path if required to the ADB executable.
 * @return ADB executable (QString)
 */
inline QString MainWindow::getADB()
{
    if (isAdbOnPath)
        return "adb"; // TODO: Conditionally compile for different platforms
    return adbExe;
}

/**
 * @brief Get the IP addres of the VM.
 * @return VM's IP address (QString)
 */
inline QString MainWindow::getIPAddr()
{
    return ui->lineEdit_ipAddr->text();
}

/**
 * @brief Get the name of the VM.
 * @return VM's name (QString)
 */
inline QString MainWindow::getVMname()
{
    return ui->lineEdit_vmName->text();
}

/**
 * @brief Is the VM running?
 * @return Running state of VM (boolean).
 */
bool MainWindow::isVMrunning()
{
    // get list of running VMS:
    QString program = "VBoxManage";
    QStringList arguments;
    arguments << "list" << "runningvms";

    QProcess *theProcess = new QProcess(this);
    theProcess->start(program, arguments);

    theProcess->waitForFinished(-1);

    // see if VM name is in results
    if (theProcess->exitCode() == 0)
    {
        QString p_stdout = theProcess->readAllStandardOutput();
        if (p_stdout.contains("\"" + getVMname() + "\"", Qt::CaseSensitive))
            return true;
    }

    return false;
}

// --------
// Settings
// --------

/**
 * @brief Write all settings.
 */
void MainWindow::writeSettings()
{
    writeSettingsVM();
    writeSettingsExecutables();
    writeWindowGeometry();
}

/**
 * @brief Write settings related to Virtual Machine.
 */
void MainWindow::writeSettingsVM()
{
    QSettings settings;
    settings.beginGroup("vm");
    settings.setValue("name", getVMname());
    settings.setValue("ip_addr", getIPAddr());
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

/**
 * @brief Write settings related to this window's geometry.
 */
void MainWindow::writeWindowGeometry()
{
    QSettings settings;
    settings.beginGroup("mainwindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
}

/**
 * @brief Read all settings.
 */
void MainWindow::readSettings()
{
    readSettingsVM();
    readSettingsExecutables();
    readSettingsGeometry();
}

/**
 * @brief Read settings related to Virtual Machine.
 */
void MainWindow::readSettingsVM()
{
    QSettings settings;

    settings.beginGroup("vm");
    ui->lineEdit_vmName->setText(settings.value("name").toString());
    ui->lineEdit_ipAddr->setText(settings.value("ip_addr").toString());
    settings.endGroup();
}

/**
 * @brief Read settings related to executables.
 */
void MainWindow::readSettingsExecutables()
{
    QSettings settings;

    settings.beginGroup("executables");
    adbExe = settings.value("adb_dir").toString();
    isAdbOnPath = settings.value("adb_on_path").toBool();
    settings.endGroup();
}

/**
 * @brief Read settings related to this window's geometry.
 */
void MainWindow::readSettingsGeometry()
{
    QSettings settings;

    settings.beginGroup("mainwindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    settings.endGroup();
}

/**
 * @brief Read settings related to this window's geometry.
 *
 * @return true if a tray icon was created, false otherwise (boolean)
 */
void MainWindow::configureTrayIcon()
{
    QMenu *trayIconMenu;
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(ui->actionRun_emulator);
    trayIconMenu->addAction(ui->actionACPI_shutdown);
    trayIconMenu->addAction(ui->actionConnect);
    trayIconMenu->addAction(ui->actionDisconnect);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionQuit);

    trayIcon = new QSystemTrayIcon(this->windowIcon(), this);
    trayIcon->setToolTip(this->windowTitle());
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    trayIcon->show();
}
