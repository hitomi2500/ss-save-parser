#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

#include <QDialog>
#include "config.h"

#define SETUPTYPE_FULL 0
#define SETUPTYPE_EXTRACT 1
#define SETUPTYPE_INSERT 2


namespace Ui {
class SetupWindow;
}

class SetupWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SetupWindow(QWidget *parent = 0, int SetupType = SETUPTYPE_FULL);
    ~SetupWindow();
    void UpdateFromConfig();
    Config *SetupConfig;

//signals:
    //SetupAccepted();

private slots:
    void on_SetupWindow_accepted();

    void on_comboBox_ExtractMode_currentIndexChanged(int index);

    void on_comboBox_InsertMode_currentIndexChanged(int index);

    void on_radioButton_pressed();

    void on_radioButton_2_pressed();

    void on_radioButton_3_pressed();

    void on_checkBox_ExtractSys_toggled(bool checked);

    void on_checkBox_FillSysZeros_toggled(bool checked);

    void on_checkBox_ExtractName_toggled(bool checked);

    void on_checkBox_ExtractComment_toggled(bool checked);

    void on_checkBox_ExtractDateTime_toggled(bool checked);

    void on_checkBox_ExtractSize_toggled(bool checked);

    void on_checkBox_ExtractSAT_toggled(bool checked);

    void on_checkBox_ExtractLanguage_toggled(bool checked);

    void on_checkBox_ExtractSysAll_toggled(bool checked);

    void on_checkBox_InsertSys_toggled(bool checked);

    void on_checkBox_InsertSysUseCounter_toggled(bool checked);

    void on_checkBox_InsertName_toggled(bool checked);

    void on_checkBox_InsertComment_toggled(bool checked);

    void on_checkBox_InsertDatetime_toggled(bool checked);

    void on_checkBox_InsertSize_toggled(bool checked);

    void on_checkBox_InsertSAT_toggled(bool checked);

    void on_checkBox_InsertLanguage_toggled(bool checked);

    void on_checkBox_ShowHex_toggled(bool checked);

    void on_checkBox_InsertSysAll_toggled(bool checked);

    void on_checkBox_Ask_Every_Extract_toggled(bool checked);

    void on_checkBox_Ask_Every_Insert_toggled(bool checked);

private:
    Ui::SetupWindow *ui;
    int iSetupType;
};

#endif // SETUPWINDOW_H
