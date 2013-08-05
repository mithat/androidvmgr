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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

/**
 * @brief constructor
 * @param parent  The window's parent (Qwidget*)
 */
PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    // Get settings from file and set in GUI
    QSettings settings;
    settings.beginGroup("executables");
    ui->lineEdit_path->setText(settings.value("adb_dir").toString());
    ui->cbo_adbOnPath->setChecked(settings.value("adb_on_path").toBool());
    settings.endGroup();

    // misc. GUI config
    setDirConfigState();
}

/**
 * @brief deconstructor
 */
PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

/**
 * @brief Write new settings to config file.
 */
void PreferencesDialog::on_buttonBox_accepted()
{
    QSettings settings;
    settings.beginGroup("executables");
    settings.setValue("adb_dir", ui->lineEdit_path->text());
    settings.setValue("adb_on_path", ui->cbo_adbOnPath->isChecked());
    settings.endGroup();
}

/**
 * @brief Enable/disable the dialog's adb path browsing to
 *        reflect the state of the "on PATH" checkbox.
 */
void PreferencesDialog::setDirConfigState()
{
    bool checkState = ui->cbo_adbOnPath->isChecked();
    ui->button_browse->setDisabled(checkState);
    ui->lineEdit_path->setDisabled(checkState);
}

/**
 * @brief Open a file browser to set the adb path.
 */
void PreferencesDialog::on_button_browse_clicked()
{
    // TODO: Should the filter be conditionally compiled for different platforms?
    QString adb = QFileDialog::getOpenFileName(this,
                                               tr("Open ADB executable"),
                                               ui->lineEdit_path->text(),
                                               tr("ADB executable (adb adb.exe)"), 0,
                                               QFileDialog::DontResolveSymlinks);
    if (!adb.isNull())
        ui->lineEdit_path->setText(adb);
}
