#include "containerdialog.h"
#include "ui_containerdialog.h"

ContainerDialog::ContainerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContainerDialog)
{
    ui->setupUi(this);
}

ContainerDialog::~ContainerDialog()
{
    delete ui;
}
