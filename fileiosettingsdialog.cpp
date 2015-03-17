#include "fileiosettingsdialog.h"
#include "ui_fileiosettingsdialog.h"

FileIOSettingsDialog::FileIOSettingsDialog(FileIOSettingsType *settings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileIOSettingsDialog)
{
    m_pSettings = settings;
    ui->setupUi(this);
    ui->FileIOModeComboBox->addItem(QString("SH2 mode (dummy byte before each byte)"));
    ui->FileIOModeComboBox->addItem(QString("Raw mode"));
    ui->ClusterSizeComboBox->addItem(QString("512"));
    ui->ClusterSizeComboBox->addItem(QString("256"));
    ui->ClusterSizeComboBox->addItem(QString("64"));
    ui->ClusterSizeComboBox->addItem(QString("Custom"));
    m_Settings = *m_pSettings; //get detected values
    if (RAW_IO_MODE == m_Settings.IOMode)
        ui->FileIOModeComboBox->setCurrentIndex(1);
    else
        ui->FileIOModeComboBox->setCurrentIndex(0);
    if (CLUSTER_512 == m_Settings.IOClusterSize)
        ui->ClusterSizeComboBox->setCurrentIndex(0);
    else if (CLUSTER_256 == m_Settings.IOClusterSize)
        ui->ClusterSizeComboBox->setCurrentIndex(1);
    else if (CLUSTER_64 == m_Settings.IOClusterSize)
        ui->ClusterSizeComboBox->setCurrentIndex(2);
    else if (CLUSTER_CUSTOM == m_Settings.IOClusterSize)
        ui->ClusterSizeComboBox->setCurrentIndex(3);
    if (m_Settings.bIOModeChangeable)
    {
        ui->label->setDisabled(false);
        ui->FileIOModeComboBox->setDisabled(false);
    }
    else
    {
        ui->label->setDisabled(true);
        ui->FileIOModeComboBox->setDisabled(true);
    }
    if (m_Settings.bClusterSizeChangeable)
    {
        ui->label_2->setDisabled(false);
        ui->ClusterSizeComboBox->setDisabled(false);
        if (CLUSTER_CUSTOM == m_Settings.IOClusterSize)
        {
            ui->label_3->setDisabled(false);
            ui->CustomClusterSizeSpinBox->setDisabled(false);
        }
        else
        {
            ui->label_3->setDisabled(true);
            ui->CustomClusterSizeSpinBox->setDisabled(true);
        }
    }
    else
    {
        ui->label_2->setDisabled(true);
        ui->ClusterSizeComboBox->setDisabled(true);
        ui->label_3->setDisabled(true);
        ui->CustomClusterSizeSpinBox->setDisabled(true);
    }
}

FileIOSettingsDialog::~FileIOSettingsDialog()
{
    delete ui;
}

void FileIOSettingsDialog::on_FileIOModeComboBox_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        m_Settings.IOMode = SH2_IO_MODE;
        break;
    case 1:
        m_Settings.IOMode = RAW_IO_MODE;
        break;
    }
}

void FileIOSettingsDialog::on_ClusterSizeComboBox_currentIndexChanged(int index)
{
    ui->label_3->setDisabled(true);
    ui->CustomClusterSizeSpinBox->setDisabled(true);
    switch (index)
    {
    case 0:
        m_Settings.IOClusterSize = CLUSTER_512;
        break;
    case 1:
        m_Settings.IOClusterSize = CLUSTER_256;
        break;
    case 2:
        m_Settings.IOClusterSize = CLUSTER_64;
        break;
    case 3:
        m_Settings.IOClusterSize = CLUSTER_CUSTOM;
        ui->label_3->setDisabled(false);
        ui->CustomClusterSizeSpinBox->setDisabled(false);
        break;
    }
}

void FileIOSettingsDialog::on_CustomClusterSizeSpinBox_valueChanged(int arg1)
{
    m_Settings.iIOCustomClusterSize = arg1;
}

void FileIOSettingsDialog::on_buttonBox_accepted()
{
    m_pSettings[0] = m_Settings;
}
