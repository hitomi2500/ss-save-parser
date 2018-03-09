#include <QTextCodec>
#include "entersavedetailsdialog.h"
#include "ui_entersavedetailsdialog.h"
#include "parselib.h"

EnterSaveDetailsDialog::EnterSaveDetailsDialog(SaveType *save, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnterSaveDetailsDialog)
{
    QString s;
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    ui->setupUi(this);
    ui->lineEdit_2->setText(codec->toUnicode(save->Name));
    ui->lineEdit_3->setText(codec->toUnicode(save->Comment));
    ui->spinBox->setMaximum(255);
    ui->spinBox_2->setMaximum(255);
    ui->spinBox->setValue(QString(save->cCounter).toInt());
    ui->spinBox_2->setValue(save->cLanguageCode);
    ui->dateTimeEdit->setDateTime(save->DateTime);
    ui->label_7->setText(QString("%1").arg(save->iBytes));
    wsave=save;
    this->setWindowTitle(this->windowTitle().append(" ").append(APP_VERSION));
    ui->OptionalBox->setFlat(true);
}

EnterSaveDetailsDialog::~EnterSaveDetailsDialog()
{
    delete ui;
}

void EnterSaveDetailsDialog::on_buttonBox_accepted()
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    wsave->Name = codec->fromUnicode(ui->lineEdit_2->text());
    wsave->Comment = codec->fromUnicode(ui->lineEdit_3->text());
    wsave->cCounter = (unsigned char)ui->spinBox->value();
    wsave->cLanguageCode = ui->spinBox_2->value();
    wsave->DateTime = ui->dateTimeEdit->dateTime();
    wsave->DateTimeRaw = GetRaw4ByteFromDateTime(wsave->DateTime);
}
