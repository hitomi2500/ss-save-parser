#include "setupwindow.h"
#include "ui_setupwindow.h"

SetupWindow::SetupWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupWindow)
{
    SetupConfig = new Config;
    ui->setupUi(this);
    ui->comboBox_9->addItem(QString("SSF-like"));
    ui->comboBox_9->addItem(QString("AR-like"));
    ui->comboBox_9->addItem(QString("Full"));
    ui->comboBox_9->addItem(QString("RAW"));
    ui->comboBox_9->addItem(QString("Manual"));
    ui->comboBox_10->addItem(QString("SSF-like"));
    ui->comboBox_10->addItem(QString("AR-like"));
    ui->comboBox_10->addItem(QString("Full"));
    ui->comboBox_10->addItem(QString("RAW"));
    ui->comboBox_10->addItem(QString("Manual"));
    SetupConfig->LoadFromRegistry();
    UpdateFromConfig();
}

SetupWindow::~SetupWindow()
{
    //SetupConfig->SaveToRegistry();
    delete ui;
}

void SetupWindow::on_SetupWindow_accepted()
{
    SetupConfig->SaveToRegistry();
    emit this->SetupAccepted();
}

void SetupWindow::UpdateFromConfig()
{
    switch (SetupConfig->m_ExtractMode)
    {
        case ExtractSSF:
            ui->comboBox_9->setCurrentIndex(0);
            ui->checkBox_17->setDisabled(true);
            ui->checkBox_17->setChecked(false);
            ui->checkBox_18->setDisabled(true);
            ui->checkBox_18->setChecked(false);
            ui->checkBox_19->setDisabled(true);
            ui->checkBox_19->setChecked(true);
            ui->checkBox_20->setDisabled(true);
            ui->checkBox_20->setChecked(true);
            ui->checkBox_21->setDisabled(true);
            ui->checkBox_21->setChecked(true);
            ui->checkBox_22->setDisabled(true);
            ui->checkBox_22->setChecked(true);
            ui->checkBox_23->setDisabled(true);
            ui->checkBox_23->setChecked(true);
            ui->checkBox_31->setDisabled(true);
            ui->checkBox_31->setChecked(true);
            ui->checkBox_33->setDisabled(true);
            ui->checkBox_33->setChecked(false);
            break;
        case ExtractAR:
            ui->comboBox_9->setCurrentIndex(1);
            ui->checkBox_17->setDisabled(true);
            ui->checkBox_17->setChecked(true);
            ui->checkBox_18->setDisabled(true);
            ui->checkBox_18->setChecked(true);
            ui->checkBox_19->setDisabled(true);
            ui->checkBox_19->setChecked(true);
            ui->checkBox_20->setDisabled(true);
            ui->checkBox_20->setChecked(true);
            ui->checkBox_21->setDisabled(true);
            ui->checkBox_21->setChecked(true);
            ui->checkBox_22->setDisabled(true);
            ui->checkBox_22->setChecked(true);
            ui->checkBox_23->setDisabled(true);
            ui->checkBox_23->setChecked(true);
            ui->checkBox_31->setDisabled(true);
            ui->checkBox_31->setChecked(true);
            ui->checkBox_33->setDisabled(true);
            ui->checkBox_33->setChecked(true);
            break;
        case ExtractFull:
            ui->comboBox_9->setCurrentIndex(2);
            ui->checkBox_17->setDisabled(true);
            ui->checkBox_17->setChecked(true);
            ui->checkBox_18->setDisabled(true);
            ui->checkBox_18->setChecked(false);
            ui->checkBox_19->setDisabled(true);
            ui->checkBox_19->setChecked(true);
            ui->checkBox_20->setDisabled(true);
            ui->checkBox_20->setChecked(true);
            ui->checkBox_21->setDisabled(true);
            ui->checkBox_21->setChecked(true);
            ui->checkBox_22->setDisabled(true);
            ui->checkBox_22->setChecked(true);
            ui->checkBox_23->setDisabled(true);
            ui->checkBox_23->setChecked(true);
            ui->checkBox_31->setDisabled(true);
            ui->checkBox_31->setChecked(true);
            ui->checkBox_33->setDisabled(true);
            ui->checkBox_33->setChecked(true);
            break;
        case ExtractRaw:
            ui->comboBox_9->setCurrentIndex(3);
            ui->checkBox_17->setDisabled(true);
            ui->checkBox_17->setChecked(false);
            ui->checkBox_18->setDisabled(true);
            ui->checkBox_18->setChecked(false);
            ui->checkBox_19->setDisabled(true);
            ui->checkBox_19->setChecked(false);
            ui->checkBox_20->setDisabled(true);
            ui->checkBox_20->setChecked(false);
            ui->checkBox_21->setDisabled(true);
            ui->checkBox_21->setChecked(false);
            ui->checkBox_22->setDisabled(true);
            ui->checkBox_22->setChecked(false);
            ui->checkBox_23->setDisabled(true);
            ui->checkBox_23->setChecked(false);
            ui->checkBox_31->setDisabled(true);
            ui->checkBox_31->setChecked(false);
            ui->checkBox_33->setDisabled(true);
            ui->checkBox_33->setChecked(false);
            break;
        case ExtractManual:
            ui->comboBox_9->setCurrentIndex(4);
            ui->checkBox_17->setDisabled(false);
            ui->checkBox_17->setChecked(SetupConfig->m_bExtractSys);
            ui->checkBox_18->setDisabled(false);
            ui->checkBox_18->setChecked(SetupConfig->m_bExtractSysFillZero);
            ui->checkBox_19->setDisabled(false);
            ui->checkBox_19->setChecked(SetupConfig->m_bExtractName);
            ui->checkBox_20->setDisabled(false);
            ui->checkBox_20->setChecked(SetupConfig->m_bExtractDescription);
            ui->checkBox_21->setDisabled(false);
            ui->checkBox_21->setChecked(SetupConfig->m_bExtractDateTime);
            ui->checkBox_22->setDisabled(false);
            ui->checkBox_22->setChecked(SetupConfig->m_bExtractSize);
            ui->checkBox_23->setDisabled(false);
            ui->checkBox_23->setChecked(SetupConfig->m_bExtractSAT);
            ui->checkBox_31->setDisabled(false);
            ui->checkBox_31->setChecked(SetupConfig->m_bExtractLanguage);
            ui->checkBox_33->setDisabled(false);
            ui->checkBox_33->setChecked(SetupConfig->m_bExtractSysAll);
            break;
    }

    switch (SetupConfig->m_InsertMode)
    {
        case InsertSSF:
            ui->comboBox_10->setCurrentIndex(0);
            ui->checkBox_24->setDisabled(true);
            ui->checkBox_24->setChecked(false);
            ui->checkBox_30->setDisabled(true);
            ui->checkBox_30->setChecked(false);
            ui->checkBox_25->setDisabled(true);
            ui->checkBox_25->setChecked(true);
            ui->checkBox_32->setDisabled(true);
            ui->checkBox_32->setChecked(true);
            ui->checkBox_26->setDisabled(true);
            ui->checkBox_26->setChecked(true);
            ui->checkBox_27->setDisabled(true);
            ui->checkBox_27->setChecked(true);
            ui->checkBox_28->setDisabled(true);
            ui->checkBox_28->setChecked(true);
            ui->checkBox_29->setDisabled(true);
            ui->checkBox_29->setChecked(true);
            ui->checkBox_34->setDisabled(true);
            ui->checkBox_34->setChecked(false);
            break;
        case InsertAR:
            ui->comboBox_10->setCurrentIndex(1);
            ui->checkBox_24->setDisabled(true);
            ui->checkBox_24->setChecked(true);
            ui->checkBox_30->setDisabled(true);
            ui->checkBox_30->setChecked(true);
            ui->checkBox_25->setDisabled(true);
            ui->checkBox_25->setChecked(true);
            ui->checkBox_32->setDisabled(true);
            ui->checkBox_32->setChecked(true);
            ui->checkBox_26->setDisabled(true);
            ui->checkBox_26->setChecked(true);
            ui->checkBox_27->setDisabled(true);
            ui->checkBox_27->setChecked(true);
            ui->checkBox_28->setDisabled(true);
            ui->checkBox_28->setChecked(true);
            ui->checkBox_29->setDisabled(true);
            ui->checkBox_29->setChecked(true);
            ui->checkBox_34->setDisabled(true);
            ui->checkBox_34->setChecked(true);
            break;
        case InsertFull:
            ui->comboBox_10->setCurrentIndex(2);
            ui->checkBox_24->setDisabled(true);
            ui->checkBox_24->setChecked(true);
            ui->checkBox_30->setDisabled(true);
            ui->checkBox_30->setChecked(false);
            ui->checkBox_25->setDisabled(true);
            ui->checkBox_25->setChecked(true);
            ui->checkBox_32->setDisabled(true);
            ui->checkBox_32->setChecked(true);
            ui->checkBox_26->setDisabled(true);
            ui->checkBox_26->setChecked(true);
            ui->checkBox_27->setDisabled(true);
            ui->checkBox_27->setChecked(true);
            ui->checkBox_28->setDisabled(true);
            ui->checkBox_28->setChecked(true);
            ui->checkBox_29->setDisabled(true);
            ui->checkBox_29->setChecked(true);
            ui->checkBox_34->setDisabled(true);
            ui->checkBox_34->setChecked(true);
            break;
        case InsertRaw:
            ui->comboBox_10->setCurrentIndex(3);
            ui->checkBox_24->setDisabled(true);
            ui->checkBox_24->setChecked(false);
            ui->checkBox_30->setDisabled(true);
            ui->checkBox_30->setChecked(false);
            ui->checkBox_25->setDisabled(true);
            ui->checkBox_25->setChecked(false);
            ui->checkBox_32->setDisabled(true);
            ui->checkBox_32->setChecked(false);
            ui->checkBox_26->setDisabled(true);
            ui->checkBox_26->setChecked(false);
            ui->checkBox_27->setDisabled(true);
            ui->checkBox_27->setChecked(false);
            ui->checkBox_28->setDisabled(true);
            ui->checkBox_28->setChecked(false);
            ui->checkBox_29->setDisabled(true);
            ui->checkBox_29->setChecked(false);
            ui->checkBox_34->setDisabled(true);
            ui->checkBox_34->setChecked(false);
            break;
        case InsertManual:
            ui->comboBox_10->setCurrentIndex(4);
            ui->checkBox_24->setDisabled(false);
            ui->checkBox_24->setChecked(SetupConfig->m_bInsertSys);
            ui->checkBox_30->setDisabled(false);
            ui->checkBox_30->setChecked(SetupConfig->m_bInsertSysUseCounter);
            ui->checkBox_25->setDisabled(false);
            ui->checkBox_25->setChecked(SetupConfig->m_bInsertName);
            ui->checkBox_32->setDisabled(false);
            ui->checkBox_32->setChecked(SetupConfig->m_bInsertLanguage);
            ui->checkBox_26->setDisabled(false);
            ui->checkBox_26->setChecked(SetupConfig->m_bInsertDescription);
            ui->checkBox_27->setDisabled(false);
            ui->checkBox_27->setChecked(SetupConfig->m_bInsertDateTime);
            ui->checkBox_28->setDisabled(false);
            ui->checkBox_28->setChecked(SetupConfig->m_bInsertSize);
            ui->checkBox_29->setDisabled(false);
            ui->checkBox_29->setChecked(SetupConfig->m_bInsertSAT);
            ui->checkBox_34->setDisabled(false);
            ui->checkBox_34->setChecked(SetupConfig->m_bInsertSysAll);
            break;
   }

    switch (SetupConfig->m_DeleteMode)
    {
        case DeleteSingleSys:
            ui->radioButton->setChecked(true);
            break;
        case DeleteAllSys:
            ui->radioButton_2->setChecked(true);
            break;
        case DeleteFull:
            ui->radioButton_3->setChecked(true);
            break;
    }

    ui->checkBox_ShowHex->setChecked(SetupConfig->m_bShowHexValues);

}

void SetupWindow::on_comboBox_9_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0 :
            SetupConfig->m_ExtractMode = ExtractSSF;
            break;
        case 1 :
            SetupConfig->m_ExtractMode = ExtractAR;
            break;
        case 2 :
            SetupConfig->m_ExtractMode = ExtractFull;
            break;
        case 3 :
            SetupConfig->m_ExtractMode = ExtractRaw;
            break;
        case 4 :
            SetupConfig->m_ExtractMode = ExtractManual;
            break;
    }
    UpdateFromConfig();
}

void SetupWindow::on_comboBox_10_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0 :
            SetupConfig->m_InsertMode = InsertSSF;
            break;
        case 1 :
            SetupConfig->m_InsertMode = InsertAR;
            break;
        case 2 :
            SetupConfig->m_InsertMode = InsertFull;
            break;
        case 3 :
            SetupConfig->m_InsertMode = InsertRaw;
            break;
        case 4 :
            SetupConfig->m_InsertMode = InsertManual;
            break;
    }
    UpdateFromConfig();
}

void SetupWindow::on_radioButton_pressed()
{
    SetupConfig->m_DeleteMode = DeleteSingleSys;
    //UpdateFromConfig();
}

void SetupWindow::on_radioButton_2_pressed()
{
    SetupConfig->m_DeleteMode = DeleteAllSys;
    //UpdateFromConfig();
}


void SetupWindow::on_radioButton_3_pressed()
{
    SetupConfig->m_DeleteMode = DeleteFull;
    //UpdateFromConfig();

}

void SetupWindow::on_checkBox_17_toggled(bool checked)
{
    SetupConfig->m_bExtractSys = checked;
}

void SetupWindow::on_checkBox_18_toggled(bool checked)
{
    SetupConfig->m_bExtractSysFillZero = checked;
}

void SetupWindow::on_checkBox_19_toggled(bool checked)
{
    SetupConfig->m_bExtractName = checked;
}

void SetupWindow::on_checkBox_20_toggled(bool checked)
{
    SetupConfig->m_bExtractDescription = checked;
}

void SetupWindow::on_checkBox_21_toggled(bool checked)
{
    SetupConfig->m_bExtractDateTime = checked;
}

void SetupWindow::on_checkBox_22_toggled(bool checked)
{
    SetupConfig->m_bExtractSize = checked;
}

void SetupWindow::on_checkBox_23_toggled(bool checked)
{
    SetupConfig->m_bExtractSAT = checked;
}

void SetupWindow::on_checkBox_31_toggled(bool checked)
{
    SetupConfig->m_bExtractLanguage = checked;
}

void SetupWindow::on_checkBox_33_toggled(bool checked)
{
    SetupConfig->m_bExtractSysAll = checked;
}

void SetupWindow::on_checkBox_24_toggled(bool checked)
{
    SetupConfig->m_bInsertSys = checked;
}

void SetupWindow::on_checkBox_30_toggled(bool checked)
{
    SetupConfig->m_bInsertSysUseCounter = checked;
}

void SetupWindow::on_checkBox_25_toggled(bool checked)
{
    SetupConfig->m_bInsertName = checked;
}

void SetupWindow::on_checkBox_26_toggled(bool checked)
{
    SetupConfig->m_bInsertDescription = checked;
}

void SetupWindow::on_checkBox_27_toggled(bool checked)
{
    SetupConfig->m_bInsertDateTime = checked;
}

void SetupWindow::on_checkBox_28_toggled(bool checked)
{
    SetupConfig->m_bInsertSize = checked;
}

void SetupWindow::on_checkBox_29_toggled(bool checked)
{
    SetupConfig->m_bInsertSAT = checked;
}

void SetupWindow::on_checkBox_32_toggled(bool checked)
{
    SetupConfig->m_bInsertLanguage = checked;
}

void SetupWindow::on_checkBox_34_toggled(bool checked)
{
    SetupConfig->m_bInsertSysAll = checked;
}

void SetupWindow::on_checkBox_ShowHex_toggled(bool checked)
{
    SetupConfig->m_bShowHexValues = checked;
}
