#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QTextCodec>
#include <QTableWidget>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "parselib.h"
#include "import.h"
#include "entersavedetailsdialog.h"
#include "fileiosettingsdialog.h"
#include "newdialog.h"

QByteArray HugeRAM;//full size buffer

QList<SaveType> SavesList;
QStringList sList;
int iSortIndex = 0;
SortDiretion SortDir = SORT_NONE;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    TheConfig = new Config;
    SetupWin = new SetupWindow(this,SETUPTYPE_FULL);
    SetupWinExtract = new SetupWindow(this,SETUPTYPE_EXTRACT);
    SetupWinInsert = new SetupWindow(this,SETUPTYPE_INSERT);
    ImageMapWin = new ImageMapWindow(this,&SavesList,TheConfig);
    ui->tableWidget->setColumnCount(9);
    ui->tableWidget->setRowCount(0);
    sList.append(tr("Name"));
    sList.append(tr("Comment"));
    sList.append(tr("Lng. code"));
    sList.append(tr("Date"));
    sList.append(tr("Bytes"));
    sList.append(tr("Blocks"));
    sList.append(tr("1st custer"));
    sList.append(tr("Clusters"));
    sList.append(tr("Counter"));
    ui->NewButton->setIcon(QIcon(":/images/new.xpm"));
    ui->NewButton->setIconSize(QSize(64,64));
    ui->SaveButton->setIcon(QIcon(":/images/save.xpm"));
    ui->SaveButton->setIconSize(QSize(64,64));
    ui->LoadButton->setIcon(QIcon(":/images/open.xpm"));
    ui->LoadButton->setIconSize(QSize(64,64));
    ui->ExtractButton->setIcon(QIcon(":/images/cart_to_game.xpm"));
    ui->ExtractButton->setIconSize(QSize(73,23));
    ui->InsertButton->setIcon(QIcon(":/images/game_to_cart.xpm"));
    ui->InsertButton->setIconSize(QSize(73,23));
    ui->ImageMapButton->setIcon(QIcon(":/images/map.xpm"));
    ui->ImageMapButton->setIconSize(QSize(73,23));
    ui->tableWidget->setHorizontalHeaderLabels(sList);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);//  SingleSelection);
    ui->RepackButton->setEnabled(false);
    ui->SaveButton->setEnabled(false);
    ui->DownloadButton->setEnabled(false);
    ui->UploadButton->setEnabled(false);
    ui->ExtractButton->setEnabled(false);
    ui->InsertButton->setEnabled(false);
    ui->EditButton->setEnabled(false);
    ui->DeleteButton->setEnabled(false);
    ui->ImageSizeLcdNumber->setFont(QFont(QString("Liberation"),12,12,false));
    ui->ClusterSizeLcdNumber->setFont(QFont(QString("Liberation"),12,12,false));
    this->setWindowIcon(QIcon(QPixmap(QString(":/images/masqurin_highwizard.xpm"),0,Qt::AutoColor)));
    this->setWindowTitle(this->windowTitle().append(" ").append(APP_VERSION));
    connect(ui->tableWidget->horizontalHeader(),
            SIGNAL(sectionClicked(int)),
            this,
            SLOT(Sort_Order_Changed(int)));
    connect(SetupWin,
            SIGNAL(accepted()),
            this,
            SLOT(Setup_Accepted()));
    //allocating new saveram
    TheConfig->LoadFromRegistry();
    TheConfig->m_iClusterSize=512;//default
    TheConfig->m_iFileSize=512*1024;//default
    SavesList.clear();
    HugeRAM.clear();
    HugeRAM.fill((char)0,TheConfig->m_iFileSize);
    for (int i=0;i<TheConfig->m_iClusterSize/16;i++)
        HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
    ParseHugeRAM();//updating
}

MainWindow::~MainWindow()
{
    delete SetupWin;
    delete TheConfig;
    delete ui;
    HugeRAM.clear();
}

void MainWindow::on_SetupButton_clicked()
{
    SetupWin->show();
}


void MainWindow::ParseHugeRAM()
{
    SaveType tmpSave;
    int i,j;
    int iUsedClusters;
    TheConfig->LoadFromRegistry();
    //now parse file content and update the game list
    SavesList.clear();
    //scan each cluster for header and fill data
    for (i=0; i< ((TheConfig->m_iFileSize)/TheConfig->m_iClusterSize); i++)
    {
        if (ParseHeader(HugeRAM.mid(i*TheConfig->m_iClusterSize,34),&tmpSave) == ParseOk)
        {
            tmpSave.iStartCluster=i;
            ParseSAT(&HugeRAM,&tmpSave,TheConfig->m_iClusterSize);
            SavesList.append(tmpSave);
        }
    }
    //update header lables according to sort order
    sList.clear();
    sList.append(tr("Name"));
    sList.append(tr("Comment"));
    sList.append(tr("Lng. code"));
    sList.append(tr("Date"));
    sList.append(tr("Bytes"));
    sList.append(tr("Blocks"));
    sList.append(tr("1st cluster"));
    sList.append(tr("Clusters"));
    sList.append(tr("Counter"));

    if (SortDir == SORT_ASCENDING)
    {
        sList[iSortIndex].append(QChar::Space);
        sList[iSortIndex].append(QChar(0x2191));//arrow up
    }
    if (SortDir == SORT_DESCENDING)
    {
        sList[iSortIndex].append(QChar::Space);
        sList[iSortIndex].append(QChar(0x2193));//arrow down
    }
    ui->tableWidget->setHorizontalHeaderLabels(sList);

    //slow sort list by name
    //this sort is horrible , replace it by something more sane later!
    for (int i=0; i<SavesList.size(); i++)
        for (int j=i; j<SavesList.size(); j++)
        {
            if (SortDir == SORT_DESCENDING)
            {//inverted sort
                switch (iSortIndex)
                {
                case 0:
                    if (SavesList.at(i).Name < SavesList.at(j).Name ) SavesList.swapItemsAt(i,j);
                    break;
                case 1:
                    if (SavesList.at(i).Comment < SavesList.at(j).Comment ) SavesList.swapItemsAt(i,j);
                    break;
                case 2:
                    if (SavesList.at(i).cLanguageCode < SavesList.at(j).cLanguageCode ) SavesList.swapItemsAt(i,j);
                    break;
                case 3:
                    if (SavesList.at(i).DateTime < SavesList.at(j).DateTime ) SavesList.swapItemsAt(i,j);
                    break;
                case 4:
                    if (SavesList.at(i).iBytes < SavesList.at(j).iBytes ) SavesList.swapItemsAt(i,j);
                    break;
                case 5:
                    if (SavesList.at(i).iBlocks < SavesList.at(j).iBlocks ) SavesList.swapItemsAt(i,j);
                    break;
                case 6:
                    if (SavesList.at(i).iStartCluster < SavesList.at(j).iStartCluster ) SavesList.swapItemsAt(i,j);
                    break;
                case 7:
                    if (SavesList.at(i).SAT.size() < SavesList.at(j).SAT.size() ) SavesList.swapItemsAt(i,j);
                    break;
                case 8:
                    if (SavesList.at(i).cCounter < SavesList.at(j).cCounter ) SavesList.swapItemsAt(i,j);
                    break;
                }
            }
            else if (SortDir==SORT_ASCENDING)
            {//normal sort
                switch (iSortIndex)
                {
                case 0:
                    if (SavesList.at(i).Name > SavesList.at(j).Name ) SavesList.swapItemsAt(i,j);
                    break;
                case 1:
                    if (SavesList.at(i).Comment > SavesList.at(j).Comment ) SavesList.swapItemsAt(i,j);
                    break;
                case 2:
                    if (SavesList.at(i).cLanguageCode > SavesList.at(j).cLanguageCode ) SavesList.swapItemsAt(i,j);
                    break;
                case 3:
                    if (SavesList.at(i).DateTime > SavesList.at(j).DateTime ) SavesList.swapItemsAt(i,j);
                    break;
                case 4:
                    if (SavesList.at(i).iBytes > SavesList.at(j).iBytes ) SavesList.swapItemsAt(i,j);
                    break;
                case 5:
                    if (SavesList.at(i).iBlocks > SavesList.at(j).iBlocks ) SavesList.swapItemsAt(i,j);
                    break;
                case 6:
                    if (SavesList.at(i).iStartCluster > SavesList.at(j).iStartCluster ) SavesList.swapItemsAt(i,j);
                    break;
                case 7:
                    if (SavesList.at(i).SAT.size() > SavesList.at(j).SAT.size() ) SavesList.swapItemsAt(i,j);
                    break;
                case 8:
                    if (SavesList.at(i).cCounter > SavesList.at(j).cCounter ) SavesList.swapItemsAt(i,j);
                    break;
                }
            }

        }
    //calculate and display used size in %
    iUsedClusters=0;
    for (i=0; i<SavesList.size(); i++)
    {
        iUsedClusters+=SavesList.at(i).SAT.size();
    }
    ui->ImageFillProgressBar->setValue((100*iUsedClusters*TheConfig->m_iClusterSize)/TheConfig->m_iFileSize);
    //display image size and cluster size
    ui->ImageSizeLcdNumber->setText(QString("%1").arg(TheConfig->m_iFileSize));
    ui->ClusterSizeLcdNumber->setText(QString("%1").arg(TheConfig->m_iClusterSize));
    //display list
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setHorizontalHeaderLabels(sList);
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QTableWidgetItem *newItem;
    QString Items[9];
    for (i=0; i<SavesList.size(); i++)
    {
        tmpSave = SavesList.at(i);
        ui->tableWidget->insertRow(i);
        Items[0] = codec->toUnicode(tmpSave.Name.replace((char)0,(char)32));
        Items[1] = codec->toUnicode(tmpSave.Comment.replace((char)0,(char)32));
        Items[2] = QString("%1").arg(tmpSave.cLanguageCode);
        Items[3] = tmpSave.DateTime.toString("dd-MM-yyyy hh:mm");
        Items[4] = QString("%1").arg(tmpSave.iBytes);
        Items[5] = QString("%1").arg(tmpSave.iBlocks);
        Items[6] = QString("%1").arg(tmpSave.iStartCluster);
        if (tmpSave.SAT.size() > 1)
            Items[7] = QString("%1(%2...%3)").arg(tmpSave.SAT.size()).arg(tmpSave.iStartCluster).arg(tmpSave.SAT.at(tmpSave.SAT.size()-2));
        else
            Items[7] = QString("%1(%2)").arg(tmpSave.SAT.size()).arg(tmpSave.iStartCluster);
        Items[8] = QString("%1").arg((int)tmpSave.cCounter);
        if (TheConfig->m_bShowHexValues)
        {
            //add hexes
            for (j=0;j<8;j++) Items[j].append(QChar::CarriageReturn);
            for (j=0;j<8;j++) Items[j].append(QChar::LineFeed);
            for (j=0;j<11;j++) Items[0].append(QString("%1 ").arg((unsigned char)tmpSave.Name[j],2,16,QChar('0')).toUpper());
            for (j=0;j<10;j++) Items[1].append(QString("%1 ").arg((unsigned char)tmpSave.Comment[j],2,16,QChar('0')).toUpper());
            Items[2].append(QString("%1 ").arg(tmpSave.cLanguageCode,2,16,QChar('0')).toUpper());
            for (j=0;j<4;j++) Items[3].append(QString("%1 ").arg((unsigned char)tmpSave.DateTimeRaw[j],2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)tmpSave.iBytes/0x1000000,2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)(tmpSave.iBytes/0x10000)%0x100,2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)(tmpSave.iBytes/0x100)%0x100,2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)tmpSave.iBytes%0x100,2,16,QChar('0')).toUpper());
            Items[8].append(QString("%1 ").arg((unsigned char)tmpSave.cCounter,2,16,QChar('0')).toUpper());
        }
        for (j=0;j<9;j++)
        {
            newItem = new QTableWidgetItem(Items[j]);
            ui->tableWidget->setItem(i,j,newItem);
        }
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    if (SavesList.size()>0) //if the table is not empty
    {
        ui->tableWidget->selectRow(0);
        ui->SaveButton->setEnabled(true);//enable save
        ui->ExtractButton->setEnabled(true);//enable extract
        ui->InsertButton->setEnabled(true);//enable insert
        ui->RepackButton->setEnabled(true);//enable repack
        ui->DeleteButton->setEnabled(true);//enable delete
    }
    else
    {
        ui->SaveButton->setEnabled(true);//still enable save
        ui->ExtractButton->setEnabled(false);//disable extract
        ui->InsertButton->setEnabled(true);//enable insert
        ui->RepackButton->setEnabled(true);//enable repack
        ui->DeleteButton->setEnabled(false);//disable delete
    }
}

void MainWindow::on_LoadButton_clicked()
{
    //loading a file into internal ram, eh? registry reading first
    char cbuf[256];
    int i,j;
    TheConfig->LoadFromRegistry();
    //now go on
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"), "", NULL);
    if (fileName.isEmpty()) return; //return if user cancel
    QFile file_in(fileName);
    if (!(file_in.open(QIODevice::ReadOnly)))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Cannot open image file %s.").arg(fileName));
        msgBox.exec();
        return;
    }
    //okay, file is opened, now detect its mode
    //first we try to find "BackUpRam Format" 16-byte signature as is
    file_in.read(cbuf,128);
    if (QByteArray(cbuf,16).startsWith("BackUpRam Format"))
    {
        //raw mode detected
        IOSettings.IOMode = RAW_IO_MODE;
    }
    else if (QByteArray(cbuf,16).startsWith("SEGA SEGASATURN"))
    {
        //some boot image detected
        //allocating new saveram
        TheConfig->LoadFromRegistry();
        NewDialog MyLittleNewDialog(&NewSettings);
        if (MyLittleNewDialog.exec() == QDialog::Rejected) return;
        SavesList.clear();
        HugeRAM.clear();
        HugeRAM.fill((char)0,NewSettings.iImageSize*1024);
        switch(NewSettings.IOClusterSize)
        {
        case CLUSTER_64:
            TheConfig->m_iClusterSize = 64;
            for (int i=0;i<4;i++)
                HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
            break;
        case CLUSTER_256:
            TheConfig->m_iClusterSize = 256;
            for (int i=0;i<16;i++)
                HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
            break;
        case CLUSTER_512:
            TheConfig->m_iClusterSize = 512;
            for (int i=0;i<32;i++)
                HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
            break;
        case CLUSTER_CUSTOM:
            TheConfig->m_iClusterSize = (NewSettings.iIOCustomClusterSize/16)*16;
            for (int i=0;i<NewSettings.iIOCustomClusterSize/16;i++)
                HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
            break;
        }
        TheConfig->m_iFileSize = NewSettings.iImageSize*1024;
        TheConfig->SaveToRegistry();

        //probing for different bootable cart types
        if (true == ProbeMemCardPlus(&file_in))
        {
            //supposing it's a Memory Cart Plus
            QMessageBox msgBox;
            msgBox.setText(tr("Mem Cart Plus image detected. It is supported as read-only, so saves will be copied to a new image. This feature is experimental. If the import fails, you can mail this image to wasca.team@gmail.com for analysis."));
            msgBox.setStandardButtons(QMessageBox::Ok );
            msgBox.exec();
            //parse resulting image
            ImportMemCardPlus(&HugeRAM,&file_in,TheConfig->m_iClusterSize);
        }
        else if (true == ProbeEMS8Meg(&file_in))
        {
            //supposing it's a Memory Cart Plus
            QMessageBox msgBox;
            msgBox.setText(tr("EMS 8 Meg image detected. It is supported as read-only, so saves will be copied to a new image. This feature is experimental. If the import fails, you can mail this image to wasca.team@gmail.com for analysis."));
            msgBox.setStandardButtons(QMessageBox::Ok );
            msgBox.exec();
            //parse resulting image
            ImportEMS8Meg(&HugeRAM,&file_in,TheConfig->m_iClusterSize);
        }
        else if (true == ProbeActionReplay(&file_in))
        {
            //supposing it's an Action Replay
            QMessageBox msgBox;
            msgBox.setText(tr("Action Replay image detected. It is supported as read-only, so saves will be copied to a new image. This feature is experimental. If the import fails, you can mail this image to wasca.team@gmail.com for analysis."));
            msgBox.exec();
            //if image size provided is smaller than the one we decoded, expand it
            if (TheConfig->m_iFileSize < 0x80000)
            {
                TheConfig->m_iFileSize = 0x80000;
                TheConfig->SaveToRegistry();
                HugeRAM.resize(0x80000);
                QMessageBox msgBox;
                msgBox.setText(tr("Action Replay import: expanding image size to 512KB"));
                msgBox.exec();
            }
            ImportActionReplay(&HugeRAM,&file_in);
            //okay, we inserted some existing image, now we try to convert it to user settings
            int iUserCluserSize = TheConfig->m_iClusterSize;
            //if cluster size is not 64, converting
            if (iUserCluserSize != 64)
            {
                TheConfig->m_iClusterSize = 64;
                TheConfig->SaveToRegistry();
                ParseHugeRAM();//required before repacking
                RepackImage(64,iUserCluserSize);
                TheConfig->m_iClusterSize = iUserCluserSize;
                TheConfig->SaveToRegistry();
            }
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Unknown bootable image detected. Trying as Action Relay. It is supported as read-only, so saves will be copied to a new image. This feature is experimental. If the import fails, you can mail this image to wasca.team@gmail.com for analysis."));
            msgBox.exec();
            //if image size provided is smaller than the one we decoded, expand it
            if (TheConfig->m_iFileSize < 0x80000)
            {
                TheConfig->m_iFileSize = 0x80000;
                TheConfig->SaveToRegistry();
                HugeRAM.resize(0x80000);
                QMessageBox msgBox;
                msgBox.setText(tr("Action Replay import: expanding image size to 512KB"));
                msgBox.exec();
            }
            ImportActionReplay(&HugeRAM,&file_in);
            //okay, we inserted some existing image, now we try to convert it to user settings
            int iUserCluserSize = TheConfig->m_iClusterSize;
            //if cluster size is not 64, converting
            if (iUserCluserSize != 64)
            {
                TheConfig->m_iClusterSize = 64;
                TheConfig->SaveToRegistry();
                ParseHugeRAM();//required before repacking
                RepackImage(64,iUserCluserSize);
                TheConfig->m_iClusterSize = iUserCluserSize;
                TheConfig->SaveToRegistry();
            }

        }

        //enable name sorting
        iSortIndex = 0;
        SortDir = SORT_ASCENDING;
        //parse
        ParseHugeRAM();
        file_in.close();
        return;
        //boot image processing done
    }
    else
    {
        //removing dummy data
        for (i=0;i<16;i++) cbuf[i] = cbuf[i*2+1];
        if (QByteArray(cbuf,16).startsWith("BackUpRam Format"))
        {
            //sh2 mode detected
            IOSettings.IOMode = SH2_IO_MODE;
        }
        else if (QByteArray(cbuf,16).startsWith("SEGA SEGASATURN"))
        {
            //some boot image detected
            QMessageBox msgBox;
            msgBox.setText(tr("I see what you did here. It is an image of some kind of a bootable cartridge, right? I can TRY to import (only import!) some saves off this image, but this is extremely experimental, so no promises. And you know what? I want that image. Contact me via github project ss-backup-parser. So, let us try then?"));
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            if (msgBox.exec() != QMessageBox::Cancel)
            {
                //import datel sh2 mode

                //enable name sorting
                iSortIndex = 0;
                SortDir = SORT_ASCENDING;
                //parse
                ParseHugeRAM();
            }
            file_in.close();
            return;
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Cannot detect signature in this file. Futher processing highly relies on that header, so i cannot proceed. If it's datel/AR/whatever image, i cannot proceed it. Yet."));
            msgBox.exec();
            file_in.close();
            return;
        }
    }

    //now detecting cluster and file size
    file_in.seek(0);
    if (RAW_IO_MODE == IOSettings.IOMode)
    {
        TheConfig->m_iFileSize = file_in.size();
        j=0;
        file_in.read(cbuf,16);
        while (QByteArray(cbuf,16).startsWith("BackUpRam Format"))
        {
            file_in.read(cbuf,16);
            j++;
        }
    }
    else
    {
        TheConfig->m_iFileSize = file_in.size()/2;
        j=0;
        file_in.read(cbuf,32);
        for (i=0;i<16;i++) cbuf[i] = cbuf[i*2+1];
        while (QByteArray(cbuf,16).startsWith("BackUpRam Format"))
        {
            file_in.read(cbuf,32);
            for (i=0;i<16;i++) cbuf[i] = cbuf[i*2+1];
            j++;
        }
    }
    HugeRAM.resize(TheConfig->m_iFileSize);
    switch (j)
    {
    //everything with a claster less than 64 is treated as 64
    case 1:
    case 2:
    case 3:
    case 4:
        IOSettings.IOClusterSize = CLUSTER_64;
        break;
    case 16:
        IOSettings.IOClusterSize = CLUSTER_256;
        break;
    case 32:
        IOSettings.IOClusterSize = CLUSTER_512;
        break;
    default:
        IOSettings.IOClusterSize = CLUSTER_CUSTOM;
        IOSettings.iIOCustomClusterSize = j * 16;
    }

    //okay, we 've detected everything
    //now asking user for override, just in case
    IOSettings.bIOModeChangeable = true;
    IOSettings.bClusterSizeChangeable = true;
    FileIOSettingsDialog IOcheckDialog(&IOSettings);
    if (IOcheckDialog.exec() == QDialog::Rejected) return;

    //update luster size in config
    switch(IOSettings.IOClusterSize)
    {
    case CLUSTER_512:
        TheConfig->m_iClusterSize = 512;
        break;
    case CLUSTER_256:
        TheConfig->m_iClusterSize = 256;
        break;
    case CLUSTER_64:
        TheConfig->m_iClusterSize = 64;
        break;
    case CLUSTER_CUSTOM:
        TheConfig->m_iClusterSize = IOSettings.iIOCustomClusterSize;
        break;
    }

    //io mode detected, loading whole file into ram
    file_in.seek(0);
    if (RAW_IO_MODE == IOSettings.IOMode)
    {
       HugeRAM = file_in.read(TheConfig->m_iFileSize);
    }
    else
    {
        for (i=0;i<TheConfig->m_iFileSize;i++)
        {
            file_in.read(cbuf,2);
            HugeRAM[i] = cbuf[1];
        }
    }
    ui->CurrentFileLabel->setText(tr("Current file : %1").arg(file_in.fileName()));
    ui->statusBar->showMessage(tr("File loaded, size %1, cluster size is %2 bytes.").arg(TheConfig->m_iFileSize).arg(TheConfig->m_iClusterSize));
    file_in.close();
    //enable name sorting
    iSortIndex = 0;
    SortDir = SORT_ASCENDING;
    //parse
    ParseHugeRAM();
}

void MainWindow::on_SaveButton_clicked()
{
    char cbuf[256];
    //saving a file. registry reading first, since config could have been changed
    TheConfig->LoadFromRegistry();
    //checking if user want to update io settings
    IOSettings.bIOModeChangeable = true;
    IOSettings.bClusterSizeChangeable = false; //we can't change cluster size here.
                                               //actually we can, but ut will require repacking
                                               //and i'm not sure if user wants this at the moment
    FileIOSettingsDialog IOcheckDialog(&IOSettings);
    if (IOcheckDialog.exec() == QDialog::Rejected) return;
    //now go on
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save Image"), "", NULL);
    if (fileName.isEmpty()) return; //return if user cancel
    QFile file_out(fileName);
    if (!(file_out.open(QIODevice::WriteOnly)))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Cannot open image file %s.").arg(fileName));
        msgBox.exec();
        return;
    }
    //file opened, saving
    if (RAW_IO_MODE == IOSettings.IOMode)
    {
        file_out.write(HugeRAM,TheConfig->m_iFileSize);
    }
    else
    {
        cbuf[0]=(char)0xFF;
        for (int i=0;i<TheConfig->m_iFileSize;i++)
        {
            cbuf[1]=HugeRAM[i];
            file_out.write(cbuf,2);
        }
    }
    file_out.close();
    ui->CurrentFileLabel->setText(tr("Current file : %1").arg(file_out.fileName()));
}

void MainWindow::on_RepackButton_clicked()
{
    //int iWriteCluster;
    int iNewClusterSize;
    int iOldClusterSize;
    TheConfig->LoadFromRegistry();
    iOldClusterSize = TheConfig->m_iClusterSize;
    //getting whatever cluster size user wants
    //checking if user want to update io settings
    IOSettings.bIOModeChangeable = false; //we're here to change cluster size, not io mode
    IOSettings.bClusterSizeChangeable = true;
    FileIOSettingsDialog IOcheckDialog(&IOSettings);
    if (IOcheckDialog.exec() == QDialog::Rejected) return;

    switch (IOSettings.IOClusterSize)
    {
    case CLUSTER_512:
        iNewClusterSize = 512;
        break;
    case CLUSTER_256:
        iNewClusterSize = 256;
        break;
    case CLUSTER_64:
        iNewClusterSize = 64;
        break;
    case CLUSTER_CUSTOM:
        iNewClusterSize = IOSettings.iIOCustomClusterSize;
        break;
    default :
        iNewClusterSize = 512;
    }

    RepackImage(iOldClusterSize,iNewClusterSize);

    //update config
    TheConfig->m_iClusterSize = iNewClusterSize;
    TheConfig->SaveToRegistry();
    //reparse all the saves
    ParseHugeRAM();
    ui->statusBar->showMessage(tr("Repacked to cluster size %1 bytes").arg(TheConfig->m_iClusterSize));
}

void MainWindow::RepackImage(int iOldClusterSize, int iNewClusterSize)
{
    int iReadPointer;
    int iWritePointer;
    int iNewStartCluster;
    QByteArray HugeRAM2;
    HugeRAM2.clear();

    //repacking image file

    //copy signature
    for (int i=0;i<iNewClusterSize/16;i++)
        HugeRAM2.append(HugeRAM.left(16));
    //now add one empty sector. looks like it is ignored or treated specifically in bios
    HugeRAM2.append(QByteArray(iNewClusterSize,(char)0));
    iWritePointer = 2*iNewClusterSize;

    //start repacking the games
    for (int iSave=0; iSave<SavesList.size(); iSave++)
    {
        iReadPointer = SavesList.at(iSave).iStartCluster*iOldClusterSize;
        //new save - copy header as is
        HugeRAM2.append(HugeRAM.mid(iReadPointer,34));
        iReadPointer+=34;
        iWritePointer+=34;
        //SAT should be recalculated. the problem here is
        //that until we know SAT size we can't be sure of how much clusters
        //we need to add to SAT. So we iterate until enough
        int iNewSATSize = ( 34 + SavesList.at(iSave).iBytes )/iNewClusterSize + 1;
        while ( ( (34 + SavesList.at(iSave).iBytes + iNewSATSize*6)/iNewClusterSize + 1) > iNewSATSize )
            iNewSATSize++;
        //check if save still fit into new pack mode
        if (HugeRAM2.size()+34+iNewSATSize*6+SavesList.at(iSave).iBytes > TheConfig->m_iFileSize)
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Save %1 doesn't fit and is removed").arg(QString(SavesList.at(iSave).Name)));
            msgBox.exec();
        }
        //okay, simply using next available sectors for SAT
        iNewStartCluster = iWritePointer/iNewClusterSize;
        for (int i=1;i<iNewSATSize;i++)
        {
            if (0 == ((iWritePointer)%iNewClusterSize)) //sat reached end of cluster
            {
                HugeRAM2.append((char)0);
                HugeRAM2.append((char)0);
                HugeRAM2.append((char)0);
                HugeRAM2.append(SavesList.at(iSave).cCounter);
                iWritePointer+=4;
                HugeRAM2.append((iNewStartCluster+i)/0x100);
                HugeRAM2.append((iNewStartCluster+i)%0x100);
                iWritePointer+=2;
            }
            else
            {
                HugeRAM2.append((iNewStartCluster+i)/0x100);
                HugeRAM2.append((iNewStartCluster+i)%0x100);
                iWritePointer+=2;
            }
        }
        //SAT end-of-table
        HugeRAM2.append((char)0);
        HugeRAM2.append((char)0);
        iWritePointer+=2;
        //move read pointer to the end of sat
        int iSatIndex = 0;
        for (int i=0;i<SavesList.at(iSave).SAT.size();i++)
        {
            if (0 == ((iReadPointer)%iOldClusterSize)) //sat reached end of cluster
            {
                //move onto next cluster in sat
                iReadPointer = SavesList.at(iSave).SAT.at(iSatIndex)*iOldClusterSize + 4;
                iSatIndex++;
            }
            iReadPointer+=2;
        }
        //now copy data
        for (int i=0; i<SavesList.at(iSave).iBytes; i++)
        {
            //check current read position
            if (0 == ((iReadPointer) % iOldClusterSize)) //hit end of old cluster
            {
                //move onto next cluster in sat
                iReadPointer = SavesList.at(iSave).SAT.at(iSatIndex)*iOldClusterSize + 4;
                iSatIndex++;
            }
            //check current write position
            if (0 == ((iWritePointer) % iNewClusterSize)) //hit end of new cluster
            {
                HugeRAM2.append((char)0);
                HugeRAM2.append((char)0);
                HugeRAM2.append((char)0);
                HugeRAM2.append(SavesList.at(iSave).cCounter);
                iWritePointer+=4;
            }
            //copy itself
            HugeRAM2[iWritePointer] = HugeRAM[iReadPointer];
            //increment
            iReadPointer++;
            iWritePointer++;
        }
        //terminate last cluster
        iWritePointer += iNewClusterSize-(HugeRAM2.size()%iNewClusterSize);
        HugeRAM2.append(QByteArray(iNewClusterSize-(HugeRAM2.size()%iNewClusterSize),(char)0));
    }
    //terminate image
    HugeRAM2.append(QByteArray(TheConfig->m_iFileSize-(HugeRAM2.size()),(char)0));
    //copy back
    HugeRAM.clear();
    HugeRAM.append(HugeRAM2);
}

void MainWindow::on_ExtractButton_clicked()
{
    //extract save from image
    char buf[256];
    SaveType tmpSave;
    QFile file_out;
    QString fileName;
    QString folderName;
    TheConfig->LoadFromRegistry();

    //first we must check if file is recoverable after all
    //since some configs are not
    //known examples :
    // 1) no SAT, no size, sys all. Since we cannot detect cluster size
    //      in this case, we don't know where to find headers.
    //      Theoretically, we can search for counter using both known
    //      cluster sizes, but that magic is unreliable, so no.
    if ( (TheConfig->m_bExtractSAT == false) &&
         (TheConfig->m_bExtractSize == false) &&
         TheConfig->m_bExtractSysAll )
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The combination you set (no SAT, no size, all headers) is extractable, but not reliably insertable. There are possible workarounds, but i'm not into implementing them. Not extracting."));
        msgBox.exec();
        return;
    }

    //issue a warning if SAT is off and sys are on
    if ( (TheConfig->m_bExtractSAT==false) && ( (TheConfig->m_bExtractSys) || (TheConfig->m_bExtractSysAll) ) )
    {
        QMessageBox msgBox;
        msgBox.setText(tr("You're trying to do something strange saving headers and NOT saving SAT. This is possible setup all right, it will be extracted and inserted successfully, but data inside looks stupidly unrelated. I will continue, but you've been warned."));
        msgBox.exec();
    }

    //get selected save or save range
    int iStart = ui->tableWidget->selectedRanges().at(0).topRow();
    int iEnd = ui->tableWidget->selectedRanges().at(0).bottomRow();
    if (iStart==iEnd)
    {
        //choose single file to save
        tmpSave = SavesList.at(iStart);//ui->tableWidget->currentRow());
        fileName = QFileDialog::getSaveFileName(this,tr("Save Savegame"), QString(tmpSave.Name), NULL);
        if (fileName.isEmpty()) return; //return if user cancel
        file_out.setFileName(fileName);
        if (!(file_out.open(QIODevice::WriteOnly)))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Cannot open save file %s.").arg(fileName));
            msgBox.exec();
            return;
        }
    }
    else
    {
        //when saving multiple files, ask for folder, not for file
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec())
            folderName = dialog.selectedFiles().at(0);
        else return; //return if user cancel
    }
    //file/folder opened, move on

    //copy extractwin's config from current one
    if (TheConfig->m_bAskFormatAtEveryExtract)
    {
        *(SetupWinExtract->SetupConfig) = *TheConfig;
    }

    //saves cycle
    for (int iSaveIndex = iStart; iSaveIndex <= iEnd; iSaveIndex++)
    {
        if (iStart != iEnd)
        {
            //if multiple saves load every file now
            tmpSave = SavesList.at(iSaveIndex);//ui->tableWidget->currentRow());
            QString fileName = folderName+QString("/")+QString(tmpSave.Name);
            file_out.setFileName(fileName);
            if (!(file_out.open(QIODevice::WriteOnly)))
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Cannot open save file %s.").arg(fileName));
                msgBox.exec();
                return;
            }
        }
        if (TheConfig->m_bAskFormatAtEveryExtract)
        {
            SetupWinExtract->SetupConfig->m_bAskFormatAtEveryExtract = true;
            SetupWinExtract->SetupConfig->UpdateFlags();
            SetupWinExtract->UpdateFromConfig();
            SetupWinExtract->SetExtractInsertFilename(QString(tmpSave.Name));
            //opening format window as modal
            if (SetupWinExtract->exec() == QDialog::Rejected) return;
            //getting temporal config from it
            *TheConfig = *(SetupWinExtract->SetupConfig);
        }
        //1st cluster
        if (TheConfig->m_bExtractSys)
        {
            if (TheConfig->m_bExtractSysFillZero)
            {
                buf[0]=(char)0;
                buf[1]=(char)0;
                buf[2]=(char)0;
                buf[3]=(char)0;
            }
            else
            {
                buf[0] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize];
                buf[1] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+1];
                buf[2] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+2];
                buf[3] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+3];
            }
            file_out.write(buf,4);
        }
        if (TheConfig->m_bExtractName)
        {
            file_out.write(tmpSave.Name,11);
        }
        if (TheConfig->m_bExtractLanguage)
        {
            buf[0]=(char)tmpSave.cLanguageCode;
            file_out.write(buf,1);
        }
        if (TheConfig->m_bExtractDescription)
        {
            file_out.write(tmpSave.Comment,10);
        }
        //SSF specific - add zero and language code after description
        if (TheConfig->m_ExtractMode == ExtractSSF)
        {
            buf[0]=(char)0;
            buf[1]=(char)tmpSave.cLanguageCode;
            file_out.write(buf,2);
        }
        if (TheConfig->m_bExtractDateTime)
        {
            //SSF specific date/time
            if (TheConfig->m_ExtractMode == ExtractSSF)
            {
                tmpSave.DateTimeRaw = GetSSF4ByteFromDateTime(tmpSave.DateTime);
                file_out.write(tmpSave.DateTimeRaw,4);
            }
            else
            {
                file_out.write(tmpSave.DateTimeRaw,4);
            }
        }
        if (TheConfig->m_bExtractSize)
        {
            buf[0]=(unsigned char)(tmpSave.iBytes/0x1000000);
            buf[1]=(unsigned char)(tmpSave.iBytes/0x10000);
            buf[2]=(unsigned char)(tmpSave.iBytes/0x100);
            buf[3]=(unsigned char)(tmpSave.iBytes);
            file_out.write(buf,4);
        }
        //Druid II specific - add 2 zeroes after header
        if (TheConfig->m_ExtractMode == ExtractDruidII)
        {
            buf[0]=(unsigned char)0;
            buf[1]=(unsigned char)0;
            file_out.write(buf,2);
        }
        //write 1st cluster
        int iSATnDataSize = tmpSave.SAT.size()*2 + tmpSave.iBytes;
        if ((iSATnDataSize + 34 ) < TheConfig->m_iClusterSize )
        {
            //writing single cluster
            if (TheConfig->m_bExtractSAT)
                file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34,tmpSave.SAT.size()*2),tmpSave.SAT.size()*2);
            file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34+tmpSave.SAT.size()*2,tmpSave.iBytes),tmpSave.iBytes);
        }
        else
        {
            //writing first cluster
            if (TheConfig->m_bExtractSAT) //if saving SAT
                file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34,TheConfig->m_iClusterSize),TheConfig->m_iClusterSize-34);
            else //not saving SAT
                if (34+tmpSave.SAT.size()*2 < TheConfig->m_iClusterSize) //if sat uses first cluster, but not fully
                    file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34+tmpSave.SAT.size()*2,TheConfig->m_iClusterSize),
                                   TheConfig->m_iClusterSize-34-(tmpSave.SAT.size()*2));

        }
        //now remaining clusters
        int iRemainingBytes = tmpSave.SAT.size()*2+tmpSave.iBytes + 34 - TheConfig->m_iClusterSize;
        for (int i=0;i<tmpSave.SAT.size()-1;i++)
        {
            if ( iRemainingBytes > (TheConfig->m_iClusterSize-4)) //not counting headers
            {
                //full middle block
                if (TheConfig->m_bExtractSysAll)
                {
                    //extract header
                    if (TheConfig->m_bExtractSysFillZero)
                    {
                        buf[0]=(char)0;
                        buf[1]=(char)0;
                        buf[2]=(char)0;
                        buf[3]=(char)0;
                    }
                    else
                    {
                        buf[0]=(char)0;
                        buf[1]=(char)0;
                        buf[2]=(char)0;
                        buf[3]=tmpSave.cCounter;
                    }
                    file_out.write(buf,4);
                }
                //three cases here: full sat cluster, ful data cluster, sat with data in the end
                //detecting
                if ((iRemainingBytes - tmpSave.iBytes) >= (TheConfig->m_iClusterSize-4) )
                {
                    //full SAT cluster
                    if (TheConfig->m_bExtractSAT)
                        file_out.write(HugeRAM.mid(tmpSave.SAT.at(i)*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
                }
                else if (iRemainingBytes <= tmpSave.iBytes)
                {
                    //full data cluster
                    file_out.write(HugeRAM.mid(tmpSave.SAT.at(i)*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
                }
                else
                {
                    int iRemainingSAT = iRemainingBytes - tmpSave.iBytes;
                    //SAT with data in the end
                    if (TheConfig->m_bExtractSAT) //write as is
                        file_out.write(HugeRAM.mid(tmpSave.SAT.at(i)*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
                    else  //only write data part
                        file_out.write(HugeRAM.mid(tmpSave.SAT.at(i)*TheConfig->m_iClusterSize+4+iRemainingSAT,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4-iRemainingSAT);
                }
                iRemainingBytes -= TheConfig->m_iClusterSize;
                iRemainingBytes +=4;
            }
            else
            {
                //write last cluster
                //last cluster here is definitely NOT SAT, so no SAT checks here
                // (this last cluster case is not used when save is only 1 cluster,
                //  only when save is 2 or more clusters, so don't worry)
                if (TheConfig->m_bExtractSysAll)
                {
                    //extract header
                    if (TheConfig->m_bExtractSysFillZero)
                    {
                        buf[0]=(char)0;
                        buf[1]=(char)0;
                        buf[2]=(char)0;
                        buf[3]=(char)0;
                    }
                    else
                    {
                        buf[0]=(char)0;
                        buf[1]=(char)0;
                        buf[2]=(char)0;
                        buf[3]=tmpSave.cCounter;
                    }
                    file_out.write(buf,4);
                }
                file_out.write(HugeRAM.mid((tmpSave.SAT.at(i)*TheConfig->m_iClusterSize)+4,iRemainingBytes),iRemainingBytes);
                iRemainingBytes = 0;
            }
        }
        file_out.close();
    }
    TheConfig->LoadFromRegistry();//restoring config after all those temporal updates (if any)
    ui->statusBar->showMessage(tr("File %1 saved").arg(fileName));
}

/*void MainWindow::on_ExtractXMLButton_clicked()
{
    //extract save from image
    SaveType tmpSave;
    QFile file_out;
    QString fileName;
    QString folderName;
    TheConfig->LoadFromRegistry();\

    //XML extraction is similar to binary with some exceptions:
    // 1) all extract flags are ignored, everything is extracted except SAT and 2..n headers
    // 2) some additional data fields are added, like player's name, comment, source etc.

    //get selected save or save range
    int iStart = ui->tableWidget->selectedRanges().at(0).topRow();
    int iEnd = ui->tableWidget->selectedRanges().at(0).bottomRow();
    if (iStart==iEnd)
    {
        //choose single file to save
        tmpSave = SavesList.at(iStart);//ui->tableWidget->currentRow());
        fileName = QFileDialog::getSaveFileName(this,tr("Save Savegame"), QString(tmpSave.Name), NULL);
        if (fileName.isEmpty()) return; //return if user cancel
        file_out.setFileName(fileName);
        if (!(file_out.open(QIODevice::WriteOnly)))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Cannot open save file %s.").arg(fileName));
            msgBox.exec();
            return;
        }
    }
    else
    {
        //when saving multiple files, ask for folder, not for file
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec())
            folderName = dialog.selectedFiles().at(0);
        else return; //return if user cancel
    }
    //file/folder opened, move on

    //saves cycle
    for (int iSaveIndex = iStart; iSaveIndex <= iEnd; iSaveIndex++)
    {
        if (iStart != iEnd)
        {
            //if multiple saves load every file now
            tmpSave = SavesList.at(iSaveIndex);
            QString fileName = folderName+QString("/")+QString(tmpSave.Name);
            file_out.setFileName(fileName);
            if (!(file_out.open(QIODevice::WriteOnly)))
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Cannot open save file %s.").arg(fileName));
                msgBox.exec();
                return;
            }
        }
        //TODO: ask user for additional fields
        //collect all binary save data
        QByteArray tmpdata;
        tmpdata.clear();
        int iCurrentCluster = -1;
        int iCurrentPos = 34;
        //skipping SAT first
        for (int i=0;i<tmpSave.SAT.size()*2;i++)
        {
            if (iCurrentPos == TheConfig->m_iClusterSize-1)
            {
                iCurrentCluster++;
                iCurrentPos = 4;
            }
            else
                iCurrentPos++;
        }
        //now copying data
        for (int i=0;i<tmpSave.iBytes;i++)
        {
            if (-1 == iCurrentCluster)
                tmpdata.append(HugeRAM.at(tmpSave.iStartCluster*TheConfig->m_iClusterSize+iCurrentPos));
            else
                tmpdata.append(HugeRAM.at(tmpSave.SAT.at(iCurrentCluster)*TheConfig->m_iClusterSize+iCurrentPos));

            if (iCurrentPos == TheConfig->m_iClusterSize-1)
            {
                iCurrentCluster++;
                iCurrentPos = 4;
            }
            else
                iCurrentPos++;
        }

        //start xml writer
        QXmlStreamWriter xml_write(&file_out);
        xml_write.setAutoFormatting(true);
        xml_write.writeStartDocument(QString("1.0"));
        xml_write.writeStartElement(QString("Sega_Saturn_Save"));
        xml_write.writeStartElement(QString("mandatory"));
        xml_write.writeStartElement("counter");
        xml_write.writeCharacters(QString("%1").arg(tmpSave.cCounter));
        xml_write.writeEndElement();//counter
        xml_write.writeStartElement("name");
        xml_write.writeCharacters(QString(tmpSave.Name));
        xml_write.writeEndElement();//name
        xml_write.writeStartElement("name_binary");
        xml_write.writeCharacters(QString(tmpSave.Name.toBase64()));
        xml_write.writeEndElement();//name_binary
        xml_write.writeStartElement("comment");
        xml_write.writeCharacters(QString(tmpSave.Comment));
        xml_write.writeEndElement();//name_binary
        xml_write.writeStartElement("comment_binary");
        xml_write.writeCharacters(QString(tmpSave.Comment.toBase64()));
        xml_write.writeEndElement();//comment
        xml_write.writeStartElement("language_code");
        xml_write.writeCharacters(QString("%1").arg(tmpSave.cLanguageCode));
        xml_write.writeEndElement();//language_code
        xml_write.writeStartElement("size");
        xml_write.writeCharacters(QString("%1").arg(tmpSave.iBytes));
        xml_write.writeEndElement();//size
        xml_write.writeStartElement(QString("date"));
        xml_write.writeAttribute("year",QString("%1").arg(tmpSave.DateTime.date().year()));
        xml_write.writeAttribute("month",QString("%1").arg(tmpSave.DateTime.date().month()))        ;
        xml_write.writeAttribute("day",QString("%1").arg(tmpSave.DateTime.date().day()));
        xml_write.writeEndElement();//date
        xml_write.writeStartElement(QString("time"));
        xml_write.writeAttribute("hour",QString("%1").arg(tmpSave.DateTime.time().hour()));
        xml_write.writeAttribute("minute",QString("%1").arg(tmpSave.DateTime.time().minute()))        ;
        xml_write.writeAttribute("second",QString("%1").arg(tmpSave.DateTime.time().second()));
        xml_write.writeEndElement();//time
        xml_write.writeStartElement("data");
        xml_write.writeCharacters(QString(tmpdata.toBase64()));
        xml_write.writeEndElement();//data
        xml_write.writeEndElement();//mandatory
        xml_write.writeStartElement(QString("optional"));
        xml_write.writeEndElement();//optional
        xml_write.writeEndElement();//Sega Saturn Save
        xml_write.writeEndDocument();
        //1st cluster header
        /*buf[0] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize];
        buf[1] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+1];
        buf[2] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+2];
        buf[3] = HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+3];
        //file_out.write(buf,4);
        //file_out.write(tmpSave.Name,11);
        buf[0]=(char)tmpSave.cLanguageCode;
        //file_out.write(buf,1);
        //file_out.write(tmpSave.Comment,10);
        //file_out.write(tmpSave.DateTimeRaw,4);
        buf[0]=(unsigned char)(tmpSave.iBytes/0x1000000);
        buf[1]=(unsigned char)(tmpSave.iBytes/0x10000);
        buf[2]=(unsigned char)(tmpSave.iBytes/0x100);
        buf[3]=(unsigned char)(tmpSave.iBytes);*/
      /*  //file_out.write(buf,4);
        //write data
        file_out.close();
        ui->statusBar->showMessage(tr("File %1 saved").arg(fileName));
    }
}*/


void MainWindow::on_InsertButton_clicked()
{
    InsertSaves();
}

void MainWindow::on_DeleteButton_clicked()
{
    //delete save
    SaveType tmpSave;
    TheConfig->LoadFromRegistry();
    if (ui->tableWidget->rowCount() < 1)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Excuse me, but we have nothing to delete."));
        msgBox.exec();
        return;
    }
    //get selected save or save range
    int iStart = ui->tableWidget->selectedRanges().at(0).topRow();
    int iEnd = ui->tableWidget->selectedRanges().at(0).bottomRow();
    for (int iSaveIndex = iStart; iSaveIndex<=iEnd; iSaveIndex++)
    {
        tmpSave = SavesList.at(iSaveIndex);
        switch (TheConfig->m_DeleteMode)
        {
        case DeleteSingleSys:
            HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0;
            break;
        case DeleteAllSys:
            HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0;
            for (int i=0; i<tmpSave.SAT.size()-1;i++)
                HugeRAM[tmpSave.SAT.at(i)*TheConfig->m_iClusterSize] = 0;
            break;
        case DeleteFull:
            QByteArray empty = QByteArray(TheConfig->m_iClusterSize,(char)0);
            HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize,TheConfig->m_iClusterSize,empty);
            for (int i=0; i<tmpSave.SAT.size()-1;i++)
                HugeRAM.replace(tmpSave.SAT.at(i)*TheConfig->m_iClusterSize,TheConfig->m_iClusterSize,empty);
            break;
        }
    }
    ParseHugeRAM();
    ui->statusBar->showMessage(tr("Save deleted"));
}

void MainWindow::Sort_Order_Changed(int logicalIndex)
{
    if (iSortIndex == logicalIndex)
    {
        switch (SortDir)
        {
        case SORT_ASCENDING:
            SortDir = SORT_DESCENDING;
            break;
        case SORT_DESCENDING:
            SortDir = SORT_NONE;
            break;
        case SORT_NONE:
            SortDir = SORT_ASCENDING;
            break;
        }
    }
    else SortDir = SORT_ASCENDING;
    iSortIndex = logicalIndex;
    ParseHugeRAM();
}

void MainWindow::Setup_Accepted()
{
    ParseHugeRAM();
}

void MainWindow::on_NewButton_clicked()
{
    //allocating new saveram
    TheConfig->LoadFromRegistry();
    if (SavesList.size()>0)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Do you want to clear existing data?"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if (msgBox.exec() == QMessageBox::Cancel)
            return;
    }
    //NewSettings.iIOCustomClusterSize=TheConfig->m_iClusterSize;
    NewDialog MyLittleNewDialog(&NewSettings);
    if (MyLittleNewDialog.exec() == QDialog::Rejected) return;
    if (NewSettings.iImageSize*1024 < TheConfig->m_iClusterSize*3)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Image size is too small. Try a bigger one please. "));
        msgBox.exec();
        return;
    }
    SavesList.clear();
    HugeRAM.clear();
    HugeRAM.fill((char)0,NewSettings.iImageSize*1024);
    switch(NewSettings.IOClusterSize)
    {
    case CLUSTER_64:
        TheConfig->m_iClusterSize = 64;
        for (int i=0;i<4;i++)
            HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
    break;
    case CLUSTER_256:
        TheConfig->m_iClusterSize = 256;
        for (int i=0;i<16;i++)
            HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
    break;
    case CLUSTER_512:
        TheConfig->m_iClusterSize = 512;
        for (int i=0;i<32;i++)
            HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
    break;
    case CLUSTER_CUSTOM:
        TheConfig->m_iClusterSize = (NewSettings.iIOCustomClusterSize/16)*16;
        for (int i=0;i<NewSettings.iIOCustomClusterSize/16;i++)
            HugeRAM.replace(16*i,16,QByteArray("BackUpRam Format"));
    break;
    }
    TheConfig->m_iFileSize = NewSettings.iImageSize*1024;
    TheConfig->SaveToRegistry();
    ui->CurrentFileLabel->setText(tr("Current file : None"));
    ParseHugeRAM();//updating
}

void MainWindow::on_ImageMapButton_clicked()
{
    ImageMapWin->show();
    ImageMapWin->UpdateData();
}
