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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

/**
 * @brief The app's main window.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent (QCloseEvent * event);

private slots:
    void on_actionQuit_triggered();

    void on_actionRun_emulator_triggered();

    void on_actionACPI_shutdown_triggered();

    void on_actionConnect_triggered();

    void on_actionDisconnect_triggered();

    void on_actionVM_info_triggered();

    void on_actionPreferences_triggered();

    void on_actionAbout_triggered();

    void on_actionStart_server_triggered();

    void on_actionStop_server_triggered();

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void appQuit();

private:
    Ui::MainWindow *ui;

    // UI elements
    QSystemTrayIcon *trayIcon;

    // State variables:
    QString adbExe;     // adb executable including directory where the executable is.
    bool isAdbOnPath;   // true iff adb is on PATH (supercedes adbExe)

    // Service member functions:
    // TODO: some of these are probably good candidates to become slots.
    void writeSettings();
    void writeSettingsExecutables();
    void writeSettingsVM();
    void writeWindowGeometry();
    void readSettings();
    void readSettingsVM();
    void readSettingsExecutables();
    void readSettingsGeometry();
    void updateNotification(const QString &theTitle,
                             const QString &theMessage,
                             const QSystemTrayIcon::MessageIcon theIcon,
                             const int timeout);
    QString lastLine(const QString &);
    void configureTrayIcon();

    // psuedo-accessors:
    /* vmName and vmIPaddr stored in UI for now */
    QString getVMname();
    QString getIPAddr();
    QString getADB();
    bool isVMrunning();
};

#endif // MAINWINDOW_H
