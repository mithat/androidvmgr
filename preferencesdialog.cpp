#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QDebug>
#include <QSettings>
#include <QMessageBox>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    // Get settings from settings file
    QSettings settings;
    settings.beginGroup("executables");
    ui->lineEdit_path->setText(settings.value("adb_dir").toString());
    ui->cbo_adbOnPath->setChecked(settings.value("adb_on_path").toBool());
    settings.endGroup();

    // misc. GUI config
    setDirConfigState();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::on_buttonBox_accepted()
{
    // write new settings to config file
    QSettings settings;
    settings.beginGroup("executables");
    settings.setValue("adb_dir", ui->lineEdit_path->text());
    settings.setValue("adb_on_path", ui->cbo_adbOnPath->isChecked());
    settings.endGroup();
}

void PreferencesDialog::setDirConfigState()
{
    bool checkState = ui->cbo_adbOnPath->isChecked();
    ui->button_browse->setDisabled(checkState);
    ui->lineEdit_path->setDisabled(checkState);
}

void PreferencesDialog::on_button_browse_clicked()
{
    QMessageBox::warning(this, "Not implemented", "This function is not yet implemented.");
}
