#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>
#include "config.h"
#include "mainwindow.h"

namespace Ui {
class NewDialog;
}

class NewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewDialog(NewSettingsType *settings, QWidget *parent = 0);
    ~NewDialog();

private slots:
    void on_ClusterSizeComboBox_currentIndexChanged(int index);

    void on_buttonBox_accepted();


    void on_ImageSizeSpinBox_editingFinished();

private:
    Ui::NewDialog *ui;
    NewSettingsType * m_pSettings;
    NewSettingsType m_Settings;
};

#endif // NEWDIALOG_H
