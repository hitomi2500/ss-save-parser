#ifndef FILEIOSETTINGSDIALOG_H
#define FILEIOSETTINGSDIALOG_H

#include <QDialog>
#include "config.h"
#include "mainwindow.h"

namespace Ui {
class FileIOSettingsDialog;
}

class FileIOSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileIOSettingsDialog(FileIOSettingsType *settings, QWidget *parent = 0);
    ~FileIOSettingsDialog();

private slots:
    void on_FileIOModeComboBox_currentIndexChanged(int index);

    void on_ClusterSizeComboBox_currentIndexChanged(int index);

    void on_CustomClusterSizeSpinBox_valueChanged(int arg1);

    void on_buttonBox_accepted();

private:
    Ui::FileIOSettingsDialog *ui;
    FileIOSettingsType * m_pSettings;
    FileIOSettingsType m_Settings;
};

#endif // FILEIOSETTINGSDIALOG_H
