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
#include <QCloseEvent>

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

// ===============
// Action handlers
// ===============

/**
 * @brief Exit the app.
 */
void MainWindow::on_actionQuit_triggered()
{
    this->appQuit();
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
    // notification defaults
    QString theTitle = tr("VM startup");
    QString theMessage;
    QSystemTrayIcon::MessageIcon theIcon = QSystemTrayIcon::Information;
    int timeout = STATUSBAR_TIMEOUT;

    if (isVMrunning()) {
        theMessage = tr("VM is already running.");
        theIcon = QSystemTrayIcon::Warning;

    }
    else {
        QString program = "VBoxManage";
        QStringList arguments;
        arguments << "startvm" << getVMname();

        QProcess *theProcess = new QProcess(this);
        theProcess->start(program, arguments);

        theProcess->waitForFinished(-1);
        if (theProcess->exitCode() == 0) {
            theMessage = tr("VM started.");
        }
        else {
            theMessage = tr("Problem starting VM. Does it exist?");
            theIcon = QSystemTrayIcon::Critical;
            timeout = 0;
        }
    }

    updateNotification(theTitle, theMessage, theIcon, timeout);
}

/**
 * @brief Send the 'acpipowerbutton" signal to the VM.
 */
void MainWindow::on_actionACPI_shutdown_triggered()
{
    // notification defaults
    QString theTitle = tr("ACPI shutdown");
    QString theMessage;
    QSystemTrayIcon::MessageIcon theIcon = QSystemTrayIcon::Information;
    int timeout = STATUSBAR_TIMEOUT;

    if (!isVMrunning()) {
        theMessage = tr("VM is not running.");
        theIcon = QSystemTrayIcon::Warning;
    }
    else {
        QString program = "VBoxManage";
        QStringList arguments;
        arguments << "controlvm" << getVMname() << "acpipowerbutton";

        QProcess *theProcess = new QProcess(this);

        // double-click powers off for 4.4
        for (int i = 0; i < 2; i++) {
            theProcess->start(program, arguments);
            theProcess->waitForFinished(-1);
            if (theProcess->exitCode() != 0) {
                theMessage = tr("Problem sending shutdown signal.");
                theIcon = QSystemTrayIcon::Critical;
                timeout = 0;
                return;
            }
            theProcess->waitForFinished(-1);
        }
        theMessage = tr("Shutdown signal sent to VM.");
    }

    updateNotification(theTitle, theMessage, theIcon, timeout);
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
        ui->statusBar->showMessage(tr("Server not started. Already running? Path set?"));
    else
    {
        ui->statusBar->showMessage(lastLine(p_stdout));
        qDebug() << p_stdout;
    }
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
        ui->statusBar->showMessage(lastLine(p_stdout));

}

/**
 * @brief Connect the VM to the ADB.
 */
void MainWindow::on_actionConnect_triggered()
{
    // notification defaults
    QString theTitle = tr("VM to ADB connection");
    QString theMessage;
    QSystemTrayIcon::MessageIcon theIcon = QSystemTrayIcon::Information;
    int timeout = STATUSBAR_TIMEOUT;

    if (!isVMrunning()) {
        theMessage = tr("VM is not running.");
        theIcon = QSystemTrayIcon::Warning;
    }
    else {
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
        theMessage = theProcess->readAllStandardOutput();
        if (theMessage.isEmpty()) {
            theMessage=tr("Problem running ADB. Is the location set?");
            theIcon = QSystemTrayIcon::Critical;
            timeout = 0;
        }
    }

    updateNotification(theTitle, theMessage, theIcon, timeout);
}

/**
 * @brief Disconnect the VM from the ADB.
 */
void MainWindow::on_actionDisconnect_triggered()
{
    // notification defaults
    QString theTitle = tr("VM to ADB connection");
    QString theMessage;
    QSystemTrayIcon::MessageIcon theIcon = QSystemTrayIcon::Information;
    int timeout = STATUSBAR_TIMEOUT;

    if (!isVMrunning()) {
        theMessage = tr("VM is not running.");
        theIcon = QSystemTrayIcon::Warning;
    }
    else {
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
        theMessage = theProcess->readAllStandardOutput();
        if (theMessage.isEmpty()) {
            theMessage = tr("Problem running ADB. Is the location set?");
            theIcon = QSystemTrayIcon::Critical;
            timeout = 0;
        }
        else if (theMessage == "\n")
            theMessage = tr("disconnected");
    }

    updateNotification(theTitle, theMessage, theIcon, timeout);
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
 * @brief Safely quit the app
 */
void MainWindow::appQuit()
{
    bool isQuit = true;
    if (isVMrunning()) {
        // first part of kludge to work around action initialed by notification icon:
        bool wasHidden = this->isHidden();
        if (wasHidden)
            this->show();

        // prompt for ACPI shutdown of VM:
        QMessageBox::StandardButton isSend = QMessageBox::question(
                    this,tr("VM is running"),
                    tr("Send ACPI shutdown signal to the Virtual Machine?"),
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

        // second part of kludge:
        if (wasHidden)
            this->hide();
    }

    if (isQuit) {
        qApp->quit();
//    this->close();
    }
}

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
// Service methods
// ===============

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

/**
 * @brief Show statusbar or systray ballon notification as appropriate.
 * @param theTitle   the title to be used in balloon notifications (QString)
 * @param theMessage the message to be shown; can be multi-line; only last line shown in statusbar (QString)
 * @param theIcon    the icon to be used balloon notifications (QString)
 * @param timeout    timeout for both statusbar and balloon notification (int)
 */
void MainWindow::updateNotification(const QString &theTitle,
                         const QString &theMessage,
                         const QSystemTrayIcon::MessageIcon theIcon,
                         const int timeout)
{
    ui->statusBar->showMessage(lastLine(theMessage), timeout);
    if (this->isHidden() && trayIcon->isVisible())
        trayIcon->showMessage(theTitle,
                              theMessage + " (" + this->getVMname()+")",
                              theIcon, timeout);
}

/**
 * @brief Return the last non-blank line contained in msg.
 * @param msg A body of (optionally multiline) text. (QString)
 * @return The last line in msg (QString)
 */
QString MainWindow::lastLine(const QString &msg)
{
    // The following will effectively nab the last non-blank line
    // and send other lines to console.
    QString lastLine = "";
    QStringList lines = msg.split("\n");
//    Q_FOREACH (QString line, lines) {
//        if (!line.isEmpty())
//            lastLine = line;
//    }
    for (int i=0; i<lines.length(); i++)
    {
        qDebug() << lines[i];
        if (!lines[i].isEmpty())
            lastLine = lines[i];
    }
    return lastLine;
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
    if (theProcess->exitCode() == 0) {
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
 * @brief Configure the system tray icon.
 */
void MainWindow::configureTrayIcon()
{
    //TODO: Popup status bar action messages (i.e., status bar messages) when main UI is hidden
    QMenu *trayIconMenu = new QMenu(this);
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
