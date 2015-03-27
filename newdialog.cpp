#include "newdialog.h"
#include "ui_newdialog.h"

NewDialog::NewDialog(NewSettingsType *settings,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewDialog)
{
    m_pSettings = settings;
    ui->setupUi(this);
    ui->ClusterSizeComboBox->addItem(QString("512"));
    ui->ClusterSizeComboBox->addItem(QString("256"));
    ui->ClusterSizeComboBox->addItem(QString("64"));
    ui->ClusterSizeComboBox->addItem(QString("Custom"));
    m_Settings = *m_pSettings; //get detected values
    if (CLUSTER_512 == m_Settings.IOClusterSize)
    {
        ui->ClusterSizeComboBox->setCurrentIndex(0);
        ui->ImageSizeSpinBox->setValue(512);//just a prediction
    }
    else if (CLUSTER_256 == m_Settings.IOClusterSize)
    {
        ui->ClusterSizeComboBox->setCurrentIndex(1);
        ui->ImageSizeSpinBox->setValue(512);//just a prediction
    }
    else if (CLUSTER_64 == m_Settings.IOClusterSize)
    {
        ui->ClusterSizeComboBox->setCurrentIndex(2);
        ui->ImageSizeSpinBox->setValue(32);//just a prediction
    }
    else if (CLUSTER_CUSTOM == m_Settings.IOClusterSize)
    {
        ui->ClusterSizeComboBox->setCurrentIndex(3);\
        ui->ImageSizeSpinBox->setValue(512);//just a prediction
    }
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
    this->setWindowTitle(this->windowTitle().append(" ").append(APP_VERSION));
 }

NewDialog::~NewDialog()
{
    delete ui;
}

void NewDialog::on_ClusterSizeComboBox_currentIndexChanged(int index)
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

void NewDialog::on_buttonBox_accepted()
{
    m_Settings.iImageSize = ui->ImageSizeSpinBox->value();
    m_Settings.iIOCustomClusterSize = ui->CustomClusterSizeSpinBox->value();
    m_pSettings[0] = m_Settings;
}
