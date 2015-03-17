#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

#include <QDialog>
#include "config.h"

namespace Ui {
class SetupWindow;
}

class SetupWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SetupWindow(QWidget *parent = 0);
    ~SetupWindow();
    void UpdateFromConfig();

private slots:
    void on_SetupWindow_accepted();

    void on_comboBox_9_currentIndexChanged(int index);

    void on_comboBox_10_currentIndexChanged(int index);

    void on_radioButton_pressed();

    void on_radioButton_2_pressed();

    void on_radioButton_3_pressed();

    void on_checkBox_17_toggled(bool checked);

    void on_checkBox_18_toggled(bool checked);

    void on_checkBox_19_toggled(bool checked);

    void on_checkBox_20_toggled(bool checked);

    void on_checkBox_21_toggled(bool checked);

    void on_checkBox_22_toggled(bool checked);

    void on_checkBox_23_toggled(bool checked);

    void on_checkBox_31_toggled(bool checked);

    void on_checkBox_33_toggled(bool checked);

    void on_checkBox_24_toggled(bool checked);

    void on_checkBox_30_toggled(bool checked);

    void on_checkBox_25_toggled(bool checked);

    void on_checkBox_26_toggled(bool checked);

    void on_checkBox_27_toggled(bool checked);

    void on_checkBox_28_toggled(bool checked);

    void on_checkBox_29_toggled(bool checked);

    void on_checkBox_32_toggled(bool checked);

    void on_checkBox_34_toggled(bool checked);

private:
    Ui::SetupWindow *ui;
    Config *SetupConfig;
};

#endif // SETUPWINDOW_H
