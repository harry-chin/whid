#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include "logging.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QSettings settings;

    ui->dbPathEdit->setText(settings.value("new-dbpath", settings.value("dbpath")).toString());
    ui->onQuitStrategyGroup->setId(ui->onQuitJustQuitOtion, 0);
    ui->onQuitStrategyGroup->setId(ui->onQuitkAskOption, 1);
    ui->onQuitStrategyGroup->setId(ui->onQuitCommitOPtion, 2);

    const auto quit_strategy = settings.value("quit-strategy", "commit").toString();

    if (quit_strategy == "quit") {
        ui->onQuitJustQuitOtion->setChecked(true);
    } else if (quit_strategy == "ask") {
        ui->onQuitkAskOption->setChecked(true);
    } else {
        ui->onQuitCommitOPtion->setChecked(true);
    }

    connect(ui->dbSelectPathBtn, SIGNAL(clicked()), this, SLOT(selectDbFile()));

    ui->saveWindowState->setCheckState(
                settings.value("restore-window-state", true).toBool()
                ? Qt::Checked : Qt::Unchecked);

    ui->enableLoggingCheck->setCheckState(
                settings.value("log-enabled", false).toBool()
                ? Qt::Checked : Qt::Unchecked);
    ui->logAppendCheck->setCheckState(
                settings.value("log-append", false).toBool()
                ? Qt::Checked : Qt::Unchecked);
    ui->logPerformanceCheck->setCheckState(
                settings.value("log-performance", false).toBool()
                ? Qt::Checked : Qt::Unchecked);
    ui->logPathEdit->setText(settings.value("log-path", "whid.log").toString());

    ui->tabWidget->setCurrentIndex(0);

    connect(this, SIGNAL(logSettingsChanged()), Logging::instance(), SLOT(changed()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    bool must_restart = false;

    QSettings settings;
    const auto db_path = ui->dbPathEdit->text();
    if (settings.value("dbpath").toString() != db_path) {
        qInfo() << "Preparing to change the database-path on next restart";
        settings.setValue("new-dbpath", db_path);
        settings.setValue("new-dbpath-copy", ui->moveDatabase->isChecked());
        must_restart = true;
    }

    QString quit_strategy = "commit";

    switch(ui->onQuitStrategyGroup->checkedId()) {
        case 0:
            quit_strategy = "quit";
            break;
        case 1:
            quit_strategy = "ask";
            break;
        default:
            ;
    }

    settings.setValue("quit-strategy", quit_strategy);
    settings.setValue("restore-window-state",
                      ui->saveWindowState->checkState() == Qt::Checked);

    if (must_restart) {
        QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("Some of the settings will take affect after you restart the application.");
            msgBox.setInformativeText("Press OK to continue.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
    }

    settings.setValue("restore-window-state",
                      ui->saveWindowState->checkState() == Qt::Checked);

    bool log_changed = false;

    if (settings.value("log-enabled",false)
            != (ui->enableLoggingCheck->checkState() == Qt::Checked)) {
        log_changed = true;
    }

    settings.setValue("log-enabled",
                      ui->enableLoggingCheck->checkState() == Qt::Checked);

    settings.setValue("log-append",
                      ui->logAppendCheck->checkState() == Qt::Checked);

    settings.setValue("log-performance",
                      ui->logPerformanceCheck->checkState() == Qt::Checked);

    if (settings.value("log-path") != ui->logPathEdit->text()) {
        log_changed = true;
    }
    settings.setValue("log-path", ui->logPathEdit->text());

    if (log_changed) {
        emit logSettingsChanged();
    }

    QDialog::accept();
}

void SettingsDialog::selectDbFile()
{
    auto path = QFileDialog::getSaveFileName(this,
                                             "Select Datatabase",
                                             ui->dbPathEdit->text(),
                                             "SQLite Files (*.db)",
                                             Q_NULLPTR,
                                             QFileDialog::DontConfirmOverwrite);

    if (!path.isNull() && !path.isEmpty()) {
        ui->dbPathEdit->setText(path);
    }
}

void SettingsDialog::on_actionSelect_Log_File_triggered()
{
    auto path = QFileDialog::getSaveFileName(this,
                                             "Select log file",
                                             ui->logPathEdit->text(),
                                             "Log files (*.log)",
                                             Q_NULLPTR,
                                             QFileDialog::DontConfirmOverwrite);

    if (!path.isNull() && !path.isEmpty()) {
        ui->logPathEdit->setText(path);
    }
}
