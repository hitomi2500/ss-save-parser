#ifndef ENTERSAVEDETAILSDIALOG_H
#define ENTERSAVEDETAILSDIALOG_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class EnterSaveDetailsDialog;
}

class EnterSaveDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnterSaveDetailsDialog(SaveType *save,QWidget *parent = 0);
    ~EnterSaveDetailsDialog();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::EnterSaveDetailsDialog *ui;
    SaveType *wsave;
};

#endif // ENTERSAVEDETAILSDIALOG_H
