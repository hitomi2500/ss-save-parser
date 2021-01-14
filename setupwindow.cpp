#include <QPicture>
#include <QPainter>
#include "setupwindow.h"
#include "ui_setupwindow.h"

SetupWindow::SetupWindow(QWidget *parent, int SetupType) :
    QDialog(parent),
    ui(new Ui::SetupWindow)
{
    int i,j;
    SetupConfig = new Config;
    iSetupType = SetupType;
    ui->setupUi(this);
    ui->comboBox_ExtractMode->addItem(tr("Mednafen"));
    ui->comboBox_ExtractMode->addItem(tr("Yabause"));
    ui->comboBox_ExtractMode->addItem(tr("SSF"));
    ui->comboBox_ExtractMode->addItem(tr("AR-like"));
    ui->comboBox_ExtractMode->addItem(tr("Full"));
    ui->comboBox_ExtractMode->addItem(tr("Raw"));
    ui->comboBox_ExtractMode->addItem(tr("Druid II"));
    ui->comboBox_ExtractMode->addItem(tr("Manual"));
    ui->comboBox_InsertMode->addItem(tr("Mednafen"));
    ui->comboBox_InsertMode->addItem(tr("Yabause"));
    ui->comboBox_InsertMode->addItem(tr("SSF"));
    ui->comboBox_InsertMode->addItem(tr("AR-like"));
    ui->comboBox_InsertMode->addItem(tr("Full"));
    ui->comboBox_InsertMode->addItem(tr("Raw"));
    ui->comboBox_InsertMode->addItem(tr("Druid II"));
    ui->comboBox_InsertMode->addItem(tr("Manual"));
    QImage img(QSize(10,10),QImage::Format_RGB32);
    img.fill(QColor(0,0,0));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0xFF0000);//sys header - red
    ui->label_Extract_L1->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L1->setPixmap(QPixmap::fromImage(img));
    ui->label_Extract_L2->setPixmap(QPixmap(":/images/sys_header_start.xpm"));
    ui->label_Insert_L2->setPixmap(QPixmap(":/images/sys_header_start.xpm"));
    ui->label_Extract_L3->setPixmap(QPixmap(":/images/sys_header_counter.xpm"));
    ui->label_Insert_L3->setPixmap(QPixmap(":/images/sys_header_counter.xpm"));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0xFF00FF);//name - magenta
    ui->label_Extract_L4->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L4->setPixmap(QPixmap::fromImage(img));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0x00FFFF);//language - cyan
    ui->label_Extract_L5->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L5->setPixmap(QPixmap::fromImage(img));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0xFFFF00);//comment - yellow
    ui->label_Extract_L6->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L6->setPixmap(QPixmap::fromImage(img));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0x009999);//datetime - darkwater
    ui->label_Extract_L7->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L7->setPixmap(QPixmap::fromImage(img));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0xFF9900);//size - orange
    ui->label_Extract_L8->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L8->setPixmap(QPixmap::fromImage(img));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0x0000FF);//SAT - blue
    ui->label_Extract_L9->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L9->setPixmap(QPixmap::fromImage(img));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0x00FF00);//data - green
    ui->label_Extract_L10->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L10->setPixmap(QPixmap::fromImage(img));
    for (i=1;i<9;i++) for (j=1;j<9;j++) img.setPixel(QPoint(i,j),0xFFFFFF);//empty - white
    ui->label_Extract_L11->setPixmap(QPixmap::fromImage(img));
    ui->label_Insert_L11->setPixmap(QPixmap::fromImage(img));
    this->setWindowTitle(this->windowTitle().append(" ").append(APP_VERSION));
    ui->label->setPixmap(QPixmap(":/images/blue_arrow.xpm"));
    ui->label_4->setPixmap(QPixmap(":/images/blue_arrow.xpm"));
    if (iSetupType == SETUPTYPE_EXTRACT)
    {
        //in extract only mode hide other tabs
        ui->tabWidget->removeTab(4);
        ui->tabWidget->removeTab(3);
        ui->tabWidget->removeTab(2);
        ui->tabWidget->removeTab(0);
        ui->checkBox_Ask_Every_Extract->hide();
    }
    if (iSetupType == SETUPTYPE_INSERT)
    {
        //in extract only mode hide other tabs
        ui->tabWidget->removeTab(4);
        ui->tabWidget->removeTab(3);
        ui->tabWidget->removeTab(1);
        ui->tabWidget->removeTab(0);
        ui->checkBox_Ask_Every_Insert->hide();
    }
    SetupConfig->LoadFromRegistry();
    UpdateFromConfig();
}

SetupWindow::~SetupWindow()
{
    delete ui;
}

void SetupWindow::on_SetupWindow_accepted()
{
    if (iSetupType == SETUPTYPE_FULL)
        SetupConfig->SaveToRegistry(); //using registry setup only for a full one
}

void SetupWindow::UpdateFromConfig()
{
    switch (SetupConfig->m_ExtractMode)
    {
        case ExtractSSF:
            ui->comboBox_ExtractMode->setCurrentIndex(0);
            ui->checkBox_ExtractSys->setDisabled(true);
            ui->checkBox_ExtractSys->setChecked(false);
            ui->checkBox_FillSysZeros->setDisabled(true);
            ui->checkBox_FillSysZeros->setChecked(false);
            ui->checkBox_ExtractName->setDisabled(true);
            ui->checkBox_ExtractName->setChecked(false);
            ui->checkBox_ExtractComment->setDisabled(true);
            ui->checkBox_ExtractComment->setChecked(true);
            ui->checkBox_ExtractDateTime->setDisabled(true);
            ui->checkBox_ExtractDateTime->setChecked(true);
            ui->checkBox_ExtractSize->setDisabled(true);
            ui->checkBox_ExtractSize->setChecked(false);
            ui->checkBox_ExtractSAT->setDisabled(true);
            ui->checkBox_ExtractSAT->setChecked(false);
            ui->checkBox_ExtractLanguage->setDisabled(true);
            ui->checkBox_ExtractLanguage->setChecked(false);
            ui->checkBox_ExtractSysAll->setDisabled(true);
            ui->checkBox_ExtractSysAll->setChecked(false);
            break;
        case ExtractAR:
            ui->comboBox_ExtractMode->setCurrentIndex(1);
            ui->checkBox_ExtractSys->setDisabled(true);
            ui->checkBox_ExtractSys->setChecked(true);
            ui->checkBox_FillSysZeros->setDisabled(true);
            ui->checkBox_FillSysZeros->setChecked(true);
            ui->checkBox_ExtractName->setDisabled(true);
            ui->checkBox_ExtractName->setChecked(true);
            ui->checkBox_ExtractComment->setDisabled(true);
            ui->checkBox_ExtractComment->setChecked(true);
            ui->checkBox_ExtractDateTime->setDisabled(true);
            ui->checkBox_ExtractDateTime->setChecked(true);
            ui->checkBox_ExtractSize->setDisabled(true);
            ui->checkBox_ExtractSize->setChecked(true);
            ui->checkBox_ExtractSAT->setDisabled(true);
            ui->checkBox_ExtractSAT->setChecked(true);
            ui->checkBox_ExtractLanguage->setDisabled(true);
            ui->checkBox_ExtractLanguage->setChecked(true);
            ui->checkBox_ExtractSysAll->setDisabled(true);
            ui->checkBox_ExtractSysAll->setChecked(true);
            break;
    case ExtractFull:
            ui->comboBox_ExtractMode->setCurrentIndex(2);
            ui->checkBox_ExtractSys->setDisabled(true);
            ui->checkBox_ExtractSys->setChecked(true);
            ui->checkBox_FillSysZeros->setDisabled(true);
            ui->checkBox_FillSysZeros->setChecked(false);
            ui->checkBox_ExtractName->setDisabled(true);
            ui->checkBox_ExtractName->setChecked(true);
            ui->checkBox_ExtractComment->setDisabled(true);
            ui->checkBox_ExtractComment->setChecked(true);
            ui->checkBox_ExtractDateTime->setDisabled(true);
            ui->checkBox_ExtractDateTime->setChecked(true);
            ui->checkBox_ExtractSize->setDisabled(true);
            ui->checkBox_ExtractSize->setChecked(true);
            ui->checkBox_ExtractSAT->setDisabled(true);
            ui->checkBox_ExtractSAT->setChecked(true);
            ui->checkBox_ExtractLanguage->setDisabled(true);
            ui->checkBox_ExtractLanguage->setChecked(true);
            ui->checkBox_ExtractSysAll->setDisabled(true);
            ui->checkBox_ExtractSysAll->setChecked(true);
            break;
        case ExtractRaw:
            ui->comboBox_ExtractMode->setCurrentIndex(3);
            ui->checkBox_ExtractSys->setDisabled(true);
            ui->checkBox_ExtractSys->setChecked(false);
            ui->checkBox_FillSysZeros->setDisabled(true);
            ui->checkBox_FillSysZeros->setChecked(false);
            ui->checkBox_ExtractName->setDisabled(true);
            ui->checkBox_ExtractName->setChecked(false);
            ui->checkBox_ExtractComment->setDisabled(true);
            ui->checkBox_ExtractComment->setChecked(false);
            ui->checkBox_ExtractDateTime->setDisabled(true);
            ui->checkBox_ExtractDateTime->setChecked(false);
            ui->checkBox_ExtractSize->setDisabled(true);
            ui->checkBox_ExtractSize->setChecked(false);
            ui->checkBox_ExtractSAT->setDisabled(true);
            ui->checkBox_ExtractSAT->setChecked(false);
            ui->checkBox_ExtractLanguage->setDisabled(true);
            ui->checkBox_ExtractLanguage->setChecked(false);
            ui->checkBox_ExtractSysAll->setDisabled(true);
            ui->checkBox_ExtractSysAll->setChecked(false);
            break;
        case ExtractDruidII:
            ui->comboBox_ExtractMode->setCurrentIndex(4);
            ui->checkBox_ExtractSys->setDisabled(true);
            ui->checkBox_ExtractSys->setChecked(false);
            ui->checkBox_FillSysZeros->setDisabled(true);
            ui->checkBox_FillSysZeros->setChecked(false);
            ui->checkBox_ExtractName->setDisabled(true);
            ui->checkBox_ExtractName->setChecked(true);
            ui->checkBox_ExtractComment->setDisabled(true);
            ui->checkBox_ExtractComment->setChecked(true);
            ui->checkBox_ExtractDateTime->setDisabled(true);
            ui->checkBox_ExtractDateTime->setChecked(true);
            ui->checkBox_ExtractSize->setDisabled(true);
            ui->checkBox_ExtractSize->setChecked(true);
            ui->checkBox_ExtractSAT->setDisabled(true);
            ui->checkBox_ExtractSAT->setChecked(false);
            ui->checkBox_ExtractLanguage->setDisabled(true);
            ui->checkBox_ExtractLanguage->setChecked(true);
            ui->checkBox_ExtractSysAll->setDisabled(true);
            ui->checkBox_ExtractSysAll->setChecked(false);
            break;
        case ExtractManual:
            ui->comboBox_ExtractMode->setCurrentIndex(5);
            ui->checkBox_ExtractSys->setDisabled(false);
            ui->checkBox_ExtractSys->setChecked(SetupConfig->m_bExtractSys);
            ui->checkBox_FillSysZeros->setDisabled(false);
            ui->checkBox_FillSysZeros->setChecked(SetupConfig->m_bExtractSysFillZero);
            ui->checkBox_ExtractName->setDisabled(false);
            ui->checkBox_ExtractName->setChecked(SetupConfig->m_bExtractName);
            ui->checkBox_ExtractComment->setDisabled(false);
            ui->checkBox_ExtractComment->setChecked(SetupConfig->m_bExtractDescription);
            ui->checkBox_ExtractDateTime->setDisabled(false);
            ui->checkBox_ExtractDateTime->setChecked(SetupConfig->m_bExtractDateTime);
            ui->checkBox_ExtractSize->setDisabled(false);
            ui->checkBox_ExtractSize->setChecked(SetupConfig->m_bExtractSize);
            ui->checkBox_ExtractSAT->setDisabled(false);
            ui->checkBox_ExtractSAT->setChecked(SetupConfig->m_bExtractSAT);
            ui->checkBox_ExtractLanguage->setDisabled(false);
            ui->checkBox_ExtractLanguage->setChecked(SetupConfig->m_bExtractLanguage);
            ui->checkBox_ExtractSysAll->setDisabled(false);
            ui->checkBox_ExtractSysAll->setChecked(SetupConfig->m_bExtractSysAll);
            break;
    }

    switch (SetupConfig->m_InsertMode)
    {
        case InsertSSF:
            ui->comboBox_InsertMode->setCurrentIndex(0);
            ui->checkBox_InsertSys->setDisabled(true);
            ui->checkBox_InsertSys->setChecked(false);
            ui->checkBox_InsertSysAll->setDisabled(true);
            ui->checkBox_InsertSysAll->setChecked(false);
            ui->checkBox_InsertSysUseCounter->setDisabled(true);
            ui->checkBox_InsertSysUseCounter->setChecked(false);
            ui->checkBox_InsertName->setDisabled(true);
            ui->checkBox_InsertName->setChecked(false);
            ui->checkBox_InsertLanguage->setDisabled(true);
            ui->checkBox_InsertLanguage->setChecked(false);
            ui->checkBox_InsertComment->setDisabled(true);
            ui->checkBox_InsertComment->setChecked(true);
            ui->checkBox_InsertDatetime->setDisabled(true);
            ui->checkBox_InsertDatetime->setChecked(true);
            ui->checkBox_InsertSize->setDisabled(true);
            ui->checkBox_InsertSize->setChecked(false);
            ui->checkBox_InsertSAT->setDisabled(true);
            ui->checkBox_InsertSAT->setChecked(false);
            break;
        case InsertAR:
            ui->comboBox_InsertMode->setCurrentIndex(1);
            ui->checkBox_InsertSys->setDisabled(true);
            ui->checkBox_InsertSys->setChecked(true);
            ui->checkBox_InsertSysAll->setDisabled(true);
            ui->checkBox_InsertSysAll->setChecked(true);
            ui->checkBox_InsertSysUseCounter->setDisabled(true);
            ui->checkBox_InsertSysUseCounter->setChecked(false);
            ui->checkBox_InsertName->setDisabled(true);
            ui->checkBox_InsertName->setChecked(true);
            ui->checkBox_InsertLanguage->setDisabled(true);
            ui->checkBox_InsertLanguage->setChecked(true);
            ui->checkBox_InsertComment->setDisabled(true);
            ui->checkBox_InsertComment->setChecked(true);
            ui->checkBox_InsertDatetime->setDisabled(true);
            ui->checkBox_InsertDatetime->setChecked(true);
            ui->checkBox_InsertSize->setDisabled(true);
            ui->checkBox_InsertSize->setChecked(true);
            ui->checkBox_InsertSAT->setDisabled(true);
            ui->checkBox_InsertSAT->setChecked(true);
            break;
        case InsertFull:
            ui->comboBox_InsertMode->setCurrentIndex(2);
            ui->checkBox_InsertSys->setDisabled(true);
            ui->checkBox_InsertSys->setChecked(true);
            ui->checkBox_InsertSysAll->setDisabled(true);
            ui->checkBox_InsertSysAll->setChecked(true);
            ui->checkBox_InsertSysUseCounter->setDisabled(true);
            ui->checkBox_InsertSysUseCounter->setChecked(true);
            ui->checkBox_InsertName->setDisabled(true);
            ui->checkBox_InsertName->setChecked(true);
            ui->checkBox_InsertLanguage->setDisabled(true);
            ui->checkBox_InsertLanguage->setChecked(true);
            ui->checkBox_InsertComment->setDisabled(true);
            ui->checkBox_InsertComment->setChecked(true);
            ui->checkBox_InsertDatetime->setDisabled(true);
            ui->checkBox_InsertDatetime->setChecked(true);
            ui->checkBox_InsertSize->setDisabled(true);
            ui->checkBox_InsertSize->setChecked(true);
            ui->checkBox_InsertSAT->setDisabled(true);
            ui->checkBox_InsertSAT->setChecked(true);
            break;
        case InsertRaw:
            ui->comboBox_InsertMode->setCurrentIndex(3);
            ui->checkBox_InsertSys->setDisabled(true);
            ui->checkBox_InsertSys->setChecked(false);
            ui->checkBox_InsertSysAll->setDisabled(true);
            ui->checkBox_InsertSysAll->setChecked(false);
            ui->checkBox_InsertSysUseCounter->setDisabled(true);
            ui->checkBox_InsertSysUseCounter->setChecked(false);
            ui->checkBox_InsertName->setDisabled(true);
            ui->checkBox_InsertName->setChecked(false);
            ui->checkBox_InsertLanguage->setDisabled(true);
            ui->checkBox_InsertLanguage->setChecked(false);
            ui->checkBox_InsertComment->setDisabled(true);
            ui->checkBox_InsertComment->setChecked(false);
            ui->checkBox_InsertDatetime->setDisabled(true);
            ui->checkBox_InsertDatetime->setChecked(false);
            ui->checkBox_InsertSize->setDisabled(true);
            ui->checkBox_InsertSize->setChecked(false);
            ui->checkBox_InsertSAT->setDisabled(true);
            ui->checkBox_InsertSAT->setChecked(false);
            break;
        case InsertDruidII:
            ui->comboBox_InsertMode->setCurrentIndex(4);
            ui->checkBox_InsertSys->setDisabled(true);
            ui->checkBox_InsertSys->setChecked(false);
            ui->checkBox_InsertSysAll->setDisabled(true);
            ui->checkBox_InsertSysAll->setChecked(false);
            ui->checkBox_InsertSysUseCounter->setDisabled(true);
            ui->checkBox_InsertSysUseCounter->setChecked(false);
            ui->checkBox_InsertName->setDisabled(true);
            ui->checkBox_InsertName->setChecked(true);
            ui->checkBox_InsertLanguage->setDisabled(true);
            ui->checkBox_InsertLanguage->setChecked(true);
            ui->checkBox_InsertComment->setDisabled(true);
            ui->checkBox_InsertComment->setChecked(true);
            ui->checkBox_InsertDatetime->setDisabled(true);
            ui->checkBox_InsertDatetime->setChecked(true);
            ui->checkBox_InsertSize->setDisabled(true);
            ui->checkBox_InsertSize->setChecked(true);
            ui->checkBox_InsertSAT->setDisabled(true);
            ui->checkBox_InsertSAT->setChecked(false);
            break;
        case InsertManual:
            ui->comboBox_InsertMode->setCurrentIndex(5);
            ui->checkBox_InsertSys->setDisabled(false);
            ui->checkBox_InsertSys->setChecked(SetupConfig->m_bInsertSys);
            ui->checkBox_InsertSysAll->setDisabled(false);
            ui->checkBox_InsertSysAll->setChecked(SetupConfig->m_bInsertSysAll);
            ui->checkBox_InsertSysUseCounter->setDisabled(false);
            ui->checkBox_InsertSysUseCounter->setChecked(SetupConfig->m_bInsertSysUseCounter);
            ui->checkBox_InsertName->setDisabled(false);
            ui->checkBox_InsertName->setChecked(SetupConfig->m_bInsertName);
            ui->checkBox_InsertLanguage->setDisabled(false);
            ui->checkBox_InsertLanguage->setChecked(SetupConfig->m_bInsertLanguage);
            ui->checkBox_InsertComment->setDisabled(false);
            ui->checkBox_InsertComment->setChecked(SetupConfig->m_bInsertDescription);
            ui->checkBox_InsertDatetime->setDisabled(false);
            ui->checkBox_InsertDatetime->setChecked(SetupConfig->m_bInsertDateTime);
            ui->checkBox_InsertSize->setDisabled(false);
            ui->checkBox_InsertSize->setChecked(SetupConfig->m_bInsertSize);
            ui->checkBox_InsertSAT->setDisabled(false);
            ui->checkBox_InsertSAT->setChecked(SetupConfig->m_bInsertSAT);
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
    ui->checkBox_Ask_Every_Extract->setChecked(SetupConfig->m_bAskFormatAtEveryExtract);
    ui->checkBox_Ask_Every_Insert->setChecked(SetupConfig->m_bAskFormatAtEveryInsert);

    QPicture DemoImagePic;
    QPicture DemoExtractFilePic;
    QPicture DemoInsertFilePic;
    QPainter MyLittlePainter;
    int iCurrentPos;
    int iCurrentPosAtImage;
    MyLittlePainter.begin(&DemoImagePic);
    iCurrentPos = 0;
    MyLittlePainter.setPen(Qt::gray);
    //draw grid
    for (int i=0;i<=16;i++) MyLittlePainter.drawLine(QPoint(i*11,0),QPoint(i*11,275));
    for (int i=0;i<=25;i++) MyLittlePainter.drawLine(QPoint(0,i*11),QPoint(177,i*11));
    //draw header
    for (int i=0;i<4;i++)
    {
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
        iCurrentPos++;
    }
    //header start
    MyLittlePainter.fillRect(3,3,6,6,QBrush(Qt::green,Qt::SolidPattern));
    //header counter
    MyLittlePainter.fillRect(36,3,6,6,QBrush(Qt::yellow,Qt::SolidPattern));
    //draw name
    for (int i=0;i<11;i++)
    {
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::magenta,Qt::SolidPattern));
        iCurrentPos++;
    }
    //draw language code
    {
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::cyan,Qt::SolidPattern));
        iCurrentPos++;
    }
    //draw comment
    for (int i=0;i<10;i++)
    {
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::yellow,Qt::SolidPattern));
        iCurrentPos++;
    }
    //draw datetime
    for (int i=0;i<4;i++)
    {
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(QColor(0x00,0x99,0x99),Qt::SolidPattern));
        iCurrentPos++;
    }
    //draw size
    for (int i=0;i<4;i++)
    {
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(QColor(0xFF,0x99,0x00),Qt::SolidPattern));
        iCurrentPos++;
    }
    //draw sat, making it overwriting cluster for clarity
    for (int i=0;i<84;i++)
    {
        if (iCurrentPos%64 == 0)
        {
            //adding header
            for (int j=0;j<4;j++)
            {
                MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                iCurrentPos++;
            }
            //header counter
            MyLittlePainter.fillRect(36,3+11*(iCurrentPos/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
        }
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::blue,Qt::SolidPattern));
        iCurrentPos++;
    }
    //draw data, making it overwriting cluster for clarity
    for (int i=0;i<220;i++)
    {
        if (iCurrentPos%64 == 0)
        {
            //adding header
            for (int j=0;j<4;j++)
            {
                MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                iCurrentPos++;
            }
            //header counter
            MyLittlePainter.fillRect(3+11*((iCurrentPos-1)%16),3+11*((iCurrentPos-1)/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
        }
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::green,Qt::SolidPattern));
        iCurrentPos++;
    }
    MyLittlePainter.end();

    //now some dynamic stuff
    //first extracting
    MyLittlePainter.begin(&DemoExtractFilePic);
    iCurrentPos = 0;
    MyLittlePainter.setPen(Qt::gray);
    //draw grid
    for (int i=0;i<=16;i++) MyLittlePainter.drawLine(QPoint(i*11,0),QPoint(i*11,275));
    for (int i=0;i<=25;i++) MyLittlePainter.drawLine(QPoint(0,i*11),QPoint(177,i*11));
    //draw header
    if (SetupConfig->m_bExtractSys)
    {
        if (SetupConfig->m_bExtractSysFillZero)
        {
            for (int i=0;i<4;i++)
            {
                MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
                iCurrentPos++;
            }
        }
        else
        {
            //draw header
            for (int i=0;i<4;i++)
            {
                MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                iCurrentPos++;
            }
            //header start
            MyLittlePainter.fillRect(3+11*((iCurrentPos-4)%16),3+11*((iCurrentPos-4)/16),6,6,QBrush(Qt::green,Qt::SolidPattern));

            //header counter
            MyLittlePainter.fillRect(3+11*((iCurrentPos-1)%16),3+11*((iCurrentPos-1)/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
        }
    }
    if (SetupConfig->m_bExtractName)
    {
        //draw name
        for (int i=0;i<11;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::magenta,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_bExtractLanguage)
    {
        //draw language code
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::cyan,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_bExtractDescription)
    {
        //draw comment
        for (int i=0;i<10;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::yellow,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_ExtractMode == ExtractSSF)
    {
        //SSF specific - add 1 zero and language code after description
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
        iCurrentPos++;
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::cyan,Qt::SolidPattern));
        iCurrentPos++;
    }
    if (SetupConfig->m_bExtractDateTime)
    {
        //draw datetime
        for (int i=0;i<4;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(QColor(0x00,0x99,0x99),Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_bExtractSize)
    {
        //draw size
        for (int i=0;i<4;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(QColor(0xFF,0x99,0x00),Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_ExtractMode == ExtractDruidII)
    {
        //Druid II specific - add 2 zeroes after header
        for (int i=0;i<2;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    iCurrentPosAtImage = 34;
    //draw sat, making it overwriting cluster for clarity
    for (int i=0;i<84;i++)
    {
        if (iCurrentPosAtImage%64 == 0)
        {
            iCurrentPosAtImage+=4;
            if (SetupConfig->m_bExtractSysAll)
            {
                //adding header
                for (int j=0;j<3;j++)
                {
                    if (SetupConfig->m_bExtractSysFillZero)
                        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
                    else
                        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                    iCurrentPos++;
                }
                //header counter
                if (SetupConfig->m_bExtractSysFillZero)
                {
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
                }
                else
                {
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                    MyLittlePainter.fillRect(3+11*(iCurrentPos%16),3+11*(iCurrentPos/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
                }
                iCurrentPos++;
            }
        }
        if (SetupConfig->m_bExtractSAT)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::blue,Qt::SolidPattern));
            iCurrentPos++;
        }
        iCurrentPosAtImage++;
    }
    //draw data, making it overwriting cluster for clarity
    for (int i=0;i<220;i++)
    {
        if (iCurrentPosAtImage%64 == 0)
        {
            iCurrentPosAtImage+=4;
            if (SetupConfig->m_bExtractSysAll)
            {
                //adding header
                for (int j=0;j<3;j++)
                {
                    if (SetupConfig->m_bExtractSysFillZero)
                        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
                    else
                        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                    iCurrentPos++;
                }
                //header counter
                if (SetupConfig->m_bExtractSysFillZero)
                {
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
                }
                else
                {
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                    MyLittlePainter.fillRect(3+11*(iCurrentPos%16),3+11*(iCurrentPos/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
                }
                iCurrentPos++;
            }
        }
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::green,Qt::SolidPattern));
        iCurrentPos++;
        iCurrentPosAtImage++;
    }
    MyLittlePainter.end();

    //now inserting
    MyLittlePainter.begin(&DemoInsertFilePic);
    iCurrentPos = 0;
    MyLittlePainter.setPen(Qt::gray);
    //draw grid
    for (int i=0;i<=16;i++) MyLittlePainter.drawLine(QPoint(i*11,0),QPoint(i*11,275));
    for (int i=0;i<=25;i++) MyLittlePainter.drawLine(QPoint(0,i*11),QPoint(177,i*11));
    //draw header
    if (SetupConfig->m_bInsertSys)
    {
        //draw header
        for (int i=0;i<4;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
            iCurrentPos++;
        }
        //header start
        MyLittlePainter.fillRect(3+11*((iCurrentPos-4)%16),3+11*((iCurrentPos-4)/16),6,6,QBrush(Qt::green,Qt::SolidPattern));

        if (SetupConfig->m_bInsertSysUseCounter)
        {
            //header counter
            MyLittlePainter.fillRect(3+11*((iCurrentPos-1)%16),3+11*((iCurrentPos-1)/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
        }
        else
        {
            iCurrentPos--;
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_bInsertName)
    {
        //draw name
        for (int i=0;i<11;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::magenta,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_bInsertLanguage)
    {
        //draw language code
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::cyan,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_bInsertDescription)
    {
        //draw comment
        for (int i=0;i<10;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::yellow,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_InsertMode == InsertSSF)
    {
        //SSF specific - add zero and language after description
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
        iCurrentPos++;
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::cyan,Qt::SolidPattern));
        iCurrentPos++;
    }
    if (SetupConfig->m_bInsertDateTime)
    {
        //draw datetime
        for (int i=0;i<4;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(QColor(0x00,0x99,0x99),Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_bInsertSize)
    {
        //draw size
        for (int i=0;i<4;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(QColor(0xFF,0x99,0x00),Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    if (SetupConfig->m_InsertMode == InsertDruidII)
    {
        //Druid II specific - add 2 zeroes after header
        for (int i=0;i<2;i++)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
            iCurrentPos++;
        }
    }
    iCurrentPosAtImage = 34;
    //draw sat, making it overwriting cluster for clarity
    for (int i=0;i<84;i++)
    {
        if (iCurrentPosAtImage%64 == 0)
        {
            iCurrentPosAtImage+=4;
            if (SetupConfig->m_bInsertSysAll)
            {
                //adding header
                for (int j=0;j<4;j++)
                {
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                    iCurrentPos++;
                }
                //header counter
                if (SetupConfig->m_bInsertSysUseCounter)
                {
                    iCurrentPos--;
                    MyLittlePainter.fillRect(3+11*(iCurrentPos%16),3+11*(iCurrentPos/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
                    iCurrentPos++;
                }
                else
                {
                    iCurrentPos--;
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
                    iCurrentPos++;
                }
            }
        }
        if (SetupConfig->m_bInsertSAT)
        {
            MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::blue,Qt::SolidPattern));
            iCurrentPos++;
        }
        iCurrentPosAtImage++;
    }
    //draw data, making it overwriting cluster for clarity
    for (int i=0;i<220;i++)
    {
        if (iCurrentPosAtImage%64 == 0)
        {
            iCurrentPosAtImage+=4;
            if (SetupConfig->m_bInsertSysAll)
            {
                //adding header
                for (int j=0;j<4;j++)
                {
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::red,Qt::SolidPattern));
                    iCurrentPos++;
                }
                //header counter
                if (SetupConfig->m_bInsertSysUseCounter)
                {
                    iCurrentPos--;
                    MyLittlePainter.fillRect(3+11*(iCurrentPos%16),3+11*(iCurrentPos/16),6,6,QBrush(Qt::yellow,Qt::SolidPattern));
                    iCurrentPos++;
                }
                else
                {
                    iCurrentPos--;
                    MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::white,Qt::SolidPattern));
                    iCurrentPos++;
                }
            }
        }
        MyLittlePainter.fillRect(1+11*(iCurrentPos%16),1+11*(iCurrentPos/16),10,10,QBrush(Qt::green,Qt::SolidPattern));
        iCurrentPos++;
        iCurrentPosAtImage++;
    }
    MyLittlePainter.end();


    ui->label_DemoExtractImage->setPicture(DemoImagePic);
    ui->label_DemoInsertImage->setPicture(DemoImagePic);
    ui->label_DemoExtractFile->setPicture(DemoExtractFilePic);
    ui->label_DemoInsertFile->setPicture(DemoInsertFilePic);

}

void SetupWindow::on_comboBox_ExtractMode_currentIndexChanged(int index)
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
            SetupConfig->m_ExtractMode = ExtractDruidII;
            break;
        case 5 :
            SetupConfig->m_ExtractMode = ExtractManual;
            break;
    }
    SetupConfig->UpdateFlags();
    UpdateFromConfig();
}

void SetupWindow::on_comboBox_InsertMode_currentIndexChanged(int index)
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
            SetupConfig->m_InsertMode = InsertDruidII;
            break;
        case 5 :
            SetupConfig->m_InsertMode = InsertManual;
            break;
    }
    SetupConfig->UpdateFlags();
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

void SetupWindow::on_checkBox_ExtractSys_toggled(bool checked)
{
    SetupConfig->m_bExtractSys = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ExtractSysAll_toggled(bool checked)
{
    SetupConfig->m_bExtractSysAll = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_FillSysZeros_toggled(bool checked)
{
    SetupConfig->m_bExtractSysFillZero = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ExtractName_toggled(bool checked)
{
    SetupConfig->m_bExtractName = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ExtractComment_toggled(bool checked)
{
    SetupConfig->m_bExtractDescription = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ExtractDateTime_toggled(bool checked)
{
    SetupConfig->m_bExtractDateTime = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ExtractSize_toggled(bool checked)
{
    SetupConfig->m_bExtractSize = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ExtractSAT_toggled(bool checked)
{
    SetupConfig->m_bExtractSAT = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ExtractLanguage_toggled(bool checked)
{
    SetupConfig->m_bExtractLanguage = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertSys_toggled(bool checked)
{
    SetupConfig->m_bInsertSys = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertSysAll_toggled(bool checked)
{
    SetupConfig->m_bInsertSysAll = checked;
    UpdateFromConfig();

}

void SetupWindow::on_checkBox_InsertSysUseCounter_toggled(bool checked)
{
    SetupConfig->m_bInsertSysUseCounter = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertName_toggled(bool checked)
{
    SetupConfig->m_bInsertName = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertComment_toggled(bool checked)
{
    SetupConfig->m_bInsertDescription = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertDatetime_toggled(bool checked)
{
    SetupConfig->m_bInsertDateTime = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertSize_toggled(bool checked)
{
    SetupConfig->m_bInsertSize = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertSAT_toggled(bool checked)
{
    SetupConfig->m_bInsertSAT = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_InsertLanguage_toggled(bool checked)
{
    SetupConfig->m_bInsertLanguage = checked;
    UpdateFromConfig();
}

void SetupWindow::on_checkBox_ShowHex_toggled(bool checked)
{
    SetupConfig->m_bShowHexValues = checked;
}

void SetupWindow::on_checkBox_Ask_Every_Extract_toggled(bool checked)
{
    SetupConfig->m_bAskFormatAtEveryExtract = checked;
}

void SetupWindow::on_checkBox_Ask_Every_Insert_toggled(bool checked)
{
    SetupConfig->m_bAskFormatAtEveryInsert = checked;

}

void SetupWindow::SetExtractInsertFilename(QString filename)
{
    if (iSetupType == SETUPTYPE_EXTRACT)
    {
        setWindowTitle(QString("Extracting mode for file %1").arg(filename));
    }
    if (iSetupType == SETUPTYPE_INSERT)
    {
        setWindowTitle(QString("Inserting mode for file %1").arg(filename));
    }
}
