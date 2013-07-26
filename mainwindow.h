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
    void on_action_Preferences_triggered();

    void on_action_Quit_triggered();

    void on_action_Connect_triggered();

    void on_action_About_triggered();

    void on_action_Run_emulator_triggered();

    void on_actionDisconnect_triggered();

    void on_actionVM_info_triggered();

private:
    Ui::MainWindow *ui;

    // State variables:
    QString adbExe;     // adb executable including directory where the executable is.
    bool isAdbOnPath;   // true iff adb is on PATH (supercedes adbExe)
    /* vmName and vmIPaddr stored in UI for now */

    // Service member functions:
    void writeSettings();

    void writeSettingsExecutables();

    void writeSettingsVM();

    void readSettings();
};

#endif // MAINWINDOW_H
