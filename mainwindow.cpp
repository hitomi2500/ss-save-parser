#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QTextCodec>
#include <QTableWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "parselib.h"
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
    SetupWin = new SetupWindow(this);
    ui->tableWidget->setColumnCount(8);
    ui->tableWidget->setRowCount(0);
    sList.append("Name");
    sList.append("Comment");
    sList.append("Lng. code");
    sList.append("Date");
    sList.append("Bytes");
    sList.append("Blocks");
    sList.append("Clusters");
    sList.append("Counter");
    ui->tableWidget->setHorizontalHeaderLabels(sList);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->RepackButton->setEnabled(false);
    //ui->Repack_Button_2->setEnabled(false);
    ui->SaveButton->setEnabled(false);
    ui->DownloadButton->setEnabled(false);
    ui->UploadButton->setEnabled(false);
    ui->ExtractButton->setEnabled(false);
    ui->InsertButton->setEnabled(false);
    ui->EditButton->setEnabled(false);
    ui->DeleteButton->setEnabled(false);
    //ui->SetupButton->setEnabled(false);
    ui->ImageSizeLcdNumber->setDigitCount(8);
    ui->ImageSizeLcdNumber->setPalette(QPalette(Qt::black));
    ui->ImageSizeLcdNumber->setFrameStyle(QFrame::NoFrame);
    ui->ClusterSizeLcdNumber->setDigitCount(4);
    ui->ClusterSizeLcdNumber->setPalette(QPalette(Qt::black));
    ui->ClusterSizeLcdNumber->setFrameStyle(QFrame::NoFrame);
    this->setWindowIcon(QIcon(QPixmap(QString(":/images/masqurin_highwizard.xpm"),0,Qt::AutoColor)));
    this->setWindowTitle(this->windowTitle().append(" ").append(APP_VERSION));
    connect(ui->tableWidget->horizontalHeader(),
            SIGNAL(sectionClicked(int)),
            this,
            SLOT(on_Sort_Order_Changed(int)));
    connect(SetupWin,
            SIGNAL(accepted()),
            this,
            SLOT(on_Setup_Accepted()));
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
            ParseSAT(HugeRAM.mid(i*TheConfig->m_iClusterSize+34,8192),&tmpSave,TheConfig->m_iClusterSize);
            tmpSave.iStartCluster=i;
            SavesList.append(tmpSave);
        }
    }
    //update header lables according to sort order
    sList.clear();
    sList.append("Name");
    sList.append("Comment");
    sList.append("Lng. code");
    sList.append("Date");
    sList.append("Bytes");
    sList.append("Blocks");
    sList.append("Clusters");
    sList.append("Counter");

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
                    if (SavesList.at(i).Name < SavesList.at(j).Name ) SavesList.swap(i,j);
                    break;
                case 1:
                    if (SavesList.at(i).Comment < SavesList.at(j).Comment ) SavesList.swap(i,j);
                    break;
                case 2:
                    if (SavesList.at(i).iLanguageCode < SavesList.at(j).iLanguageCode ) SavesList.swap(i,j);
                    break;
                case 3:
                    if (SavesList.at(i).DateTime < SavesList.at(j).DateTime ) SavesList.swap(i,j);
                    break;
                case 4:
                    if (SavesList.at(i).iBytes < SavesList.at(j).iBytes ) SavesList.swap(i,j);
                    break;
                case 5:
                    if (SavesList.at(i).iBlocks < SavesList.at(j).iBlocks ) SavesList.swap(i,j);
                    break;
                case 6:
                    if (SavesList.at(i).iSATSize < SavesList.at(j).iSATSize ) SavesList.swap(i,j);
                    break;
                case 7:
                    if (SavesList.at(i).cCounter < SavesList.at(j).cCounter ) SavesList.swap(i,j);
                    break;
                }
            }
            else if (SortDir==SORT_ASCENDING)
            {//normal sort
                switch (iSortIndex)
                {
                case 0:
                    if (SavesList.at(i).Name > SavesList.at(j).Name ) SavesList.swap(i,j);
                    break;
                case 1:
                    if (SavesList.at(i).Comment > SavesList.at(j).Comment ) SavesList.swap(i,j);
                    break;
                case 2:
                    if (SavesList.at(i).iLanguageCode > SavesList.at(j).iLanguageCode ) SavesList.swap(i,j);
                    break;
                case 3:
                    if (SavesList.at(i).DateTime > SavesList.at(j).DateTime ) SavesList.swap(i,j);
                    break;
                case 4:
                    if (SavesList.at(i).iBytes > SavesList.at(j).iBytes ) SavesList.swap(i,j);
                    break;
                case 5:
                    if (SavesList.at(i).iBlocks > SavesList.at(j).iBlocks ) SavesList.swap(i,j);
                    break;
                case 6:
                    if (SavesList.at(i).iSATSize > SavesList.at(j).iSATSize ) SavesList.swap(i,j);
                    break;
                case 7:
                    if (SavesList.at(i).cCounter > SavesList.at(j).cCounter ) SavesList.swap(i,j);
                    break;
                }
            }

        }
    //calculate and display used size in %
    iUsedClusters=0;
    for (i=0; i<SavesList.size(); i++)
    {
        iUsedClusters+=SavesList.at(i).iSATSize;
    }
    ui->ImageFillProgressBar->setValue((100*iUsedClusters*TheConfig->m_iClusterSize)/TheConfig->m_iFileSize);
    //display image size and cluster size
    ui->ImageSizeLcdNumber->display(TheConfig->m_iFileSize);
    ui->ClusterSizeLcdNumber->display(TheConfig->m_iClusterSize);
    //display list
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setHorizontalHeaderLabels(sList);
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QTableWidgetItem *newItem;
    QString Items[8];
    for (i=0; i<SavesList.size(); i++)
    {
        tmpSave = SavesList.at(i);
        ui->tableWidget->insertRow(i);
        Items[0] = codec->toUnicode(tmpSave.Name.replace((char)0,(char)32));
        Items[1] = codec->toUnicode(tmpSave.Comment.replace((char)0,(char)32));
        Items[2] = QString("%1").arg(tmpSave.iLanguageCode);
        Items[3] = tmpSave.DateTime.toString("dd-MM-yyyy hh:mm");
        Items[4] = QString("%1").arg(tmpSave.iBytes);
        Items[5] = QString("%1").arg(tmpSave.iBlocks);
        if (tmpSave.iSATSize > 1)
            Items[6] = QString("%1(%2...%3)").arg(tmpSave.iSATSize).arg(tmpSave.iStartCluster).arg(tmpSave.SAT[tmpSave.iSATSize-2]);
        else
            Items[6] = QString("%1(%2)").arg(tmpSave.iSATSize).arg(tmpSave.iStartCluster);
        Items[7] = QString("%1").arg((int)tmpSave.cCounter);
        if (TheConfig->m_bShowHexValues)
        {
            //add hexes
            for (j=0;j<8;j++) Items[j].append(QChar::CarriageReturn);
            for (j=0;j<8;j++) Items[j].append(QChar::LineFeed);
            for (j=0;j<11;j++) Items[0].append(QString("%1 ").arg((unsigned char)tmpSave.Name[j],2,16,QChar('0')).toUpper());
            for (j=0;j<10;j++) Items[1].append(QString("%1 ").arg((unsigned char)tmpSave.Comment[j],2,16,QChar('0')).toUpper());
            Items[2].append(QString("%1 ").arg((unsigned char)tmpSave.iLanguageCode,2,16,QChar('0')).toUpper());
            for (j=0;j<4;j++) Items[3].append(QString("%1 ").arg((unsigned char)tmpSave.DateTimeRaw[j],2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)tmpSave.iBytes/0x1000000,2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)(tmpSave.iBytes/0x10000)%0x100,2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)(tmpSave.iBytes/0x100)%0x100,2,16,QChar('0')).toUpper());
            Items[4].append(QString("%1 ").arg((unsigned char)tmpSave.iBytes%0x100,2,16,QChar('0')).toUpper());
            Items[7].append(QString("%1 ").arg((unsigned char)tmpSave.cCounter,2,16,QChar('0')).toUpper());
        }
        for (j=0;j<8;j++)
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
        msgBox.setText(QString("Cannot open image file %s.").arg(fileName));
        msgBox.exec();
        return;
    }
    //okay, file is opened, now detect its mode
    //first we try to find "BackUpRam Format" 16-byte signature as is
    file_in.read(cbuf,32);
    if (QByteArray(cbuf,16).startsWith("BackUpRam Format"))
    {
        //raw mode detected
        IOSettings.IOMode = RAW_IO_MODE;
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
        else
        {
            QMessageBox msgBox;
            msgBox.setText(QString("Cannot detect signature in this file. Futher processing highly relies on that header, so i cannot proceed. If it's datel/AR/whatever image, i cannot proceed it. Yet."));
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
    HugeRAM.reserve(TheConfig->m_iFileSize);
    switch (j)
    {
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
    ui->statusBar->showMessage(QString("File loaded, size %1, cluster size is %2 bytes.").arg(TheConfig->m_iFileSize).arg(TheConfig->m_iClusterSize));
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
        msgBox.setText(QString("Cannot open image file %s.").arg(fileName));
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
}

void MainWindow::on_RepackButton_clicked()
{
    int iReadPointer;
    int iWritePointer;
    //int iWriteCluster;
    int iNewClusterSize;
    int iOldClusterSize;
    int iNewStartCluster;
    QByteArray HugeRAM2;
    HugeRAM2.clear();
    TheConfig->LoadFromRegistry();
    iOldClusterSize = TheConfig->m_iClusterSize;
    //getting whatever cluster size user wants
    //checking if user want to update io settings
    IOSettings.bIOModeChangeable = false; //we're here to change cluster size, not io mode
    IOSettings.bClusterSizeChangeable = true;
    FileIOSettingsDialog IOcheckDialog(&IOSettings);
    if (IOcheckDialog.exec() == QDialog::Rejected) return;

    //repacking image file
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
            msgBox.setText(QString("Save %1 doesn't fit and is removed").arg(QString(SavesList.at(iSave).Name)));
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
        for (int i=0;i<SavesList.at(iSave).iSATSize;i++)
        {
            if (0 == ((iReadPointer)%iOldClusterSize)) //sat reached end of cluster
            {
                //move onto next cluster in sat
                iReadPointer = SavesList.at(iSave).SAT[iSatIndex]*iOldClusterSize + 4;
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
                iReadPointer = SavesList.at(iSave).SAT[iSatIndex]*iOldClusterSize + 4;
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
    //update config
    TheConfig->m_iClusterSize = iNewClusterSize;
    TheConfig->SaveToRegistry();
    //reparse all the saves
    ParseHugeRAM();
    ui->statusBar->showMessage(QString("Repacked to cluster size %1 bytes").arg(TheConfig->m_iClusterSize));
}

void MainWindow::on_ExtractButton_clicked()
{
    //extract save from image
    char buf[256];
    SaveType tmpSave;
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
        msgBox.setText(QString("The combination you set (no SAT, no size, all headers) is extractable, but not reliably insertable. There are possible workarounds, but i'm not into implementing them. Not extracting."));
        msgBox.exec();
        return;
    }

    //get selected save
    tmpSave = SavesList.at(ui->tableWidget->currentRow());
    //choose file to save
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save Savegame"), "", NULL);
    if (fileName.isEmpty()) return; //return if user cancel
    QFile file_out(fileName);
    if (!(file_out.open(QIODevice::WriteOnly)))
    {
        QMessageBox msgBox;
        msgBox.setText(QString("Cannot open save file %s.").arg(fileName));
        msgBox.exec();
        return;
    }
    //file opened, move on

    //issue a warning if SAT is off and sys are on
    if ( (TheConfig->m_bExtractSAT==false) && ( (TheConfig->m_bExtractSys) || (TheConfig->m_bExtractSysAll) ) )
    {
        QMessageBox msgBox;
        msgBox.setText(QString("You're trying to do something strange saving headers and NOT saving SAT. This is possible setup all right, it will be extracted and inserted successfully, but data inside looks stupidly unrelated. I will continue, but you've been warned."));
        msgBox.exec();
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
        buf[0]=(char)tmpSave.iLanguageCode;
        file_out.write(buf,1);
    }
    if (TheConfig->m_bExtractDescription)
    {
        file_out.write(tmpSave.Comment,10);
    }
    if (TheConfig->m_bExtractDateTime)
    {
        file_out.write(tmpSave.DateTimeRaw,4);
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
    int iSATnDataSize = tmpSave.iSATSize*2 + tmpSave.iBytes;
    if ((iSATnDataSize + 34 ) < TheConfig->m_iClusterSize )
    {
        //writing single cluster
        if (TheConfig->m_bExtractSAT)
            file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34,tmpSave.iSATSize*2),tmpSave.iSATSize*2);
        file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34+tmpSave.iSATSize*2,tmpSave.iBytes),tmpSave.iBytes);
    }
    else
    {
        //writing first cluster
        if (TheConfig->m_bExtractSAT) //if saving SAT
            file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34,TheConfig->m_iClusterSize),TheConfig->m_iClusterSize-34);
        else //not saving SAT
            if (34+tmpSave.iSATSize*2 < TheConfig->m_iClusterSize) //if sat uses first cluster, but not fully
                file_out.write(HugeRAM.mid(tmpSave.iStartCluster*TheConfig->m_iClusterSize+34+tmpSave.iSATSize*2,TheConfig->m_iClusterSize),
                               TheConfig->m_iClusterSize-34-(tmpSave.iSATSize*2));

    }
    //now remaining clusters
    int iRemainingBytes = tmpSave.iSATSize*2+tmpSave.iBytes + 34 - TheConfig->m_iClusterSize;
    for (int i=0;i<tmpSave.iSATSize-1;i++)
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
                    file_out.write(HugeRAM.mid(tmpSave.SAT[i]*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
            }
            else if (iRemainingBytes <= tmpSave.iBytes)
            {
                //full data cluster
                file_out.write(HugeRAM.mid(tmpSave.SAT[i]*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
            }
            else
            {
                int iRemainingSAT = iRemainingBytes - tmpSave.iBytes;
                //SAT with data in the end
                if (TheConfig->m_bExtractSAT) //write as is
                    file_out.write(HugeRAM.mid(tmpSave.SAT[i]*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
                else  //only write data part
                    file_out.write(HugeRAM.mid(tmpSave.SAT[i]*TheConfig->m_iClusterSize+4+iRemainingSAT,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4-iRemainingSAT);
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
            file_out.write(HugeRAM.mid((tmpSave.SAT[i]*TheConfig->m_iClusterSize)+4,iRemainingBytes),iRemainingBytes);
            iRemainingBytes = 0;
        }
    }
    file_out.close();
    ui->statusBar->showMessage(QString("File %1 saved").arg(fileName));
}


void MainWindow::on_InsertButton_clicked()
{
    //insert save
    char buf[256];
    SaveType tmpSave;
    int iOldClusterSize;
    TheConfig->LoadFromRegistry();

    //first we must check if file is recoverable after all
    //since some configs are not
    //known examples :
    // 1) no SAT, no size, sys all. Since we cannot detect cluster size
    //      in this case, we don't know where to find headers.
    //      Theoretically, we can search for counter using both known
    //      cluster sizes, but that magic is unreliable, so no.
    if ( (TheConfig->m_bInsertSAT == false) &&
         (TheConfig->m_bInsertSize == false) &&
         TheConfig->m_bInsertSysAll )
    {
        QMessageBox msgBox;
        msgBox.setText(QString("The combination you set (no SAT, no size, all headers) is not reliably recoverable. There are possible workarounds, but i'm not into implementing them. Not inserting."));
        msgBox.exec();
        return;
    }

    //choose file to open
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load Savegame"), "", NULL);
    if (fileName.isEmpty()) return; //return if user cancel
    QFile file_in(fileName);
    if (!(file_in.open(QIODevice::ReadOnly)))
    {
        QMessageBox msgBox;
        msgBox.setText(QString("Cannot open save file %s.").arg(fileName));
        msgBox.exec();
        return;
    }
    //file opened, move on

    //check if we have enough space at the end of file
    //we don't insert at 0th or 1st cluster - it's signature and always zero
    //so starting with 2
    int iLastUsedCluster=2;
    for (int i=0; i<SavesList.size(); i++)
    {
        for (int j=0; j<SavesList.at(i).iSATSize;j++)
        {
            if (SavesList.at(i).SAT[j] > iLastUsedCluster)
                iLastUsedCluster = SavesList.at(i).SAT[j];
        }
    }
    //making a brutal check for inserting size:
    //each ClusterSize-4 requires additional 6 bytes (2 for SAT, 4 for header)
    //plus 36 additional (34 header, 2 zero-sat-entry)
    //if that won't fit into remaining clusters, boil out
    int iClustersRequired = (file_in.size())/(TheConfig->m_iClusterSize-4);
    int iBytesRequired = file_in.size()+iClustersRequired*6+36;
    iClustersRequired = iBytesRequired/(TheConfig->m_iClusterSize-4);
    if ((TheConfig->m_iFileSize/TheConfig->m_iClusterSize - iLastUsedCluster) <= iClustersRequired)
    {
        QMessageBox msgBox;
        msgBox.setText(QString("Not enough space in image to insert save file %1.").arg(fileName));
        msgBox.exec();
        return;
    }
    tmpSave.iStartCluster = iLastUsedCluster+1;

    //check done, go on

    //trying to get as much info from file as possible
    if (TheConfig->m_bInsertSys)
    {
        file_in.read(buf,4);
        if (TheConfig->m_bInsertSysUseCounter)
            tmpSave.cCounter = buf[3];
        else
            tmpSave.cCounter = 0;
    }
    else
    {
        //no counter was provided, making one up
        tmpSave.cCounter = 0;
    }
    if (TheConfig->m_bInsertName)
    {
        file_in.read(buf,11);
        tmpSave.Name = QByteArray(buf,11);
    }
    else
    {
        //no name was provided, using file name
        QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
        tmpSave.Name = codec->fromUnicode(fileName).left(11);
    }
    if (TheConfig->m_bInsertLanguage)
    {
        file_in.read(buf,1);
        tmpSave.iLanguageCode = buf[0];
    }
    else
    {
        //no language code was provided, making one up
        tmpSave.iLanguageCode = 0;
    }
    if (TheConfig->m_bInsertDescription)
    {
        file_in.read(buf,10);
        tmpSave.Comment = QByteArray(buf,10);
    }
    else
    {
        //no comment was provided, using holy one
        buf[0] = 0xBE; //se
        buf[1] = 0xB6; //g
        buf[2] = 0xDE; //a
        buf[3] = 0xBB; //sa
        buf[4] = 0xC0; //ta
        buf[5] = 0xB0; //-
        buf[6] = 0xDD; //n
        buf[7] = 0x2C; //,
        buf[8] = 0xBC; //shi
        buf[9] = 0xDB; //ro
        tmpSave.Comment = QByteArray(buf,10);
    }
    if (TheConfig->m_bInsertDateTime)
    {
        file_in.read(buf,4);
        tmpSave.DateTimeRaw = QByteArray(buf,4);
        tmpSave.DateTime = GetDateTimeFromRaw4Byte(tmpSave.DateTimeRaw);
    }
    else
    {
        //no data was provided, using current one
        tmpSave.DateTime = QDateTime::currentDateTime();
        //convert that one to raw
        tmpSave.DateTimeRaw = GetRaw4ByteFromDateTime(tmpSave.DateTime);
    }
    if (TheConfig->m_bInsertSize)
    {
        file_in.read(buf,4);
        tmpSave.iBytes = (unsigned char)buf[0]*0x1000000 +
                (unsigned char)buf[1]*0x10000 +
                (unsigned char)buf[2]*0x100 +
                (unsigned char)buf[3];
    }
    else
    {
        //no file size is provided, counting it
        //this value is not countable yet, we will count it later, after we'll read SAT
    }
    //Druid II specific - skip 2 zeroes after header
    if (TheConfig->m_ExtractMode == ExtractDruidII)
    {
        file_in.read(buf,2);
    }
    //sat
    if (TheConfig->m_bInsertSAT)
    {
        //old SAT is only acceptable if we use the same cluster value
        //since this could not be the fact, we're simply dumping old SAT
        //and recalculating a new one later.
        //but we need to know SAT size in order to find data start, and
        //to correctly count data size if it is not provided in header
        tmpSave.iSATSize=1;
        file_in.read(buf,2);
        while ( (buf[0]!=0) || (buf[1]!= 0) )
        {
             tmpSave.iSATSize++;
             if ((tmpSave.iSATSize*2+30)%(TheConfig->m_iClusterSize-4) == 0)
                file_in.read(buf,4);//if moving onto next SAT cluster, skip 4 bytes
             file_in.read(buf,2);
        }
        //okay, now that know old SAT size, we can calculate size, if it's not provided
        int iTmpSize = 0;
        //if (TheConfig->m_bInsertSize == false)
        {
            iTmpSize = file_in.size();
            if (TheConfig->m_bInsertDateTime) iTmpSize -= 4;
            if (TheConfig->m_bInsertDescription) iTmpSize -= 10;
            if (TheConfig->m_bInsertLanguage) iTmpSize -= 1;
            if (TheConfig->m_bInsertName) iTmpSize -= 11;
            if (TheConfig->m_bInsertSAT) iTmpSize -= tmpSave.iSATSize*2;
            if (TheConfig->m_bInsertSize) iTmpSize -= 4;
            if (TheConfig->m_bInsertSys) iTmpSize -= 4;
            if (TheConfig->m_bInsertSysAll) iTmpSize -= (tmpSave.iSATSize-1)*4;
        }
        //now size stuff
        if (TheConfig->m_bInsertSize)
        {
            //if the size was provided in file as well, do a sanity check
            if (tmpSave.iBytes != iTmpSize)
            {
                //sanity check failed, aborting
                QMessageBox msgBox;
                msgBox.setText(QString("It looks like this save is corrupted, saved with another features set, or is not a save at all. Continue?"));
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                if (msgBox.exec() == QMessageBox::Cancel)
                    return;
            }
            else
            {
                //sanity check succseeds, i'm happy!
            }
        }
        else
        {
            //no size was provided, we have no way but to use calculated
            tmpSave.iBytes = iTmpSize;
        }
        //calcuating old cluster size, this will be of need when insert_all_sys is on
        //for 1 cluster only SAT is sinlge 0, so it doesn't matter anyway
        //for more than 1 cluster : bytes/satsize < 252 for 256, > 254 for 512
        if (tmpSave.iBytes / tmpSave.iSATSize < 253 )
            iOldClusterSize = 256;
        else
            iOldClusterSize = 512;
        //okay, we know size, calculate new SAT size
        tmpSave.iSATSize = 1; //dumping old sat
        while ( (30 + tmpSave.iSATSize*2 + tmpSave.iBytes)/(TheConfig->m_iClusterSize-4) > tmpSave.iSATSize)
            tmpSave.iSATSize++;
        //fill new sat
        for (int i=0;i<(tmpSave.iSATSize-1);i++)
            tmpSave.SAT[i] = tmpSave.iStartCluster+i;
        tmpSave.SAT[tmpSave.iSATSize-1] = 0;
        //copy SAT to hugeram
        int iPointer = tmpSave.iStartCluster*TheConfig->m_iClusterSize + 34;
        for (int i=0;i<tmpSave.iSATSize;i++)
        {
            if (iPointer % TheConfig->m_iClusterSize == 0)
            {
                HugeRAM[iPointer] = 0;
                HugeRAM[iPointer+1] = 0;
                HugeRAM[iPointer+2] = 0;
                HugeRAM[iPointer+3] = (unsigned char)tmpSave.cCounter;
                iPointer+=4;
            }
            HugeRAM[iPointer] = (char) ( tmpSave.SAT[i] / 0x100 );
            HugeRAM[iPointer+1] = (char) ( tmpSave.SAT[i] % 0x100 );
            iPointer+=2;
        }
        //copy save itself to hugeram
        for (int i=0; i< tmpSave.iBytes; i++)
        {
            if (iPointer % TheConfig->m_iClusterSize == 0)
            {
                HugeRAM[iPointer] = 0;
                HugeRAM[iPointer+1] = 0;
                HugeRAM[iPointer+2] = 0;
                HugeRAM[iPointer+3] = (unsigned char)tmpSave.cCounter;
                iPointer+=4;
            }
            if ((TheConfig->m_bInsertSysAll) && (file_in.pos() % iOldClusterSize == 0) )
            {
                file_in.read(buf,4);
            }
            file_in.read(buf,1);
            HugeRAM[iPointer] = buf[0];
            iPointer++;
        }
        //we're done the case with integrated sat!
        //now only check & write header
    }
    else
    {
        //old SAT is not provided, that saves us some pain, we know data is right ahead
        //but if the data size was not provided as well, we must count data size manually
        //and we cannot calculate old clusters' size
        //now size stuff
        if (TheConfig->m_bInsertSize)
        {
            //size provided, that's good, we don't need to make any specific actions
            //fill new sat
            for (int i=0;i<(tmpSave.iSATSize-1);i++)
                tmpSave.SAT[i] = tmpSave.iStartCluster+i;
            tmpSave.SAT[tmpSave.iSATSize-1] = 0;
            //copy SAT to hugeram
            int iPointer = tmpSave.iStartCluster*TheConfig->m_iClusterSize + 34;
            for (int i=0;i<tmpSave.iSATSize;i++)
            {
                if (iPointer % TheConfig->m_iClusterSize == 0)
                {
                    HugeRAM[iPointer] = 0;
                    HugeRAM[iPointer+1] = 0;
                    HugeRAM[iPointer+2] = 0;
                    HugeRAM[iPointer+3] = (unsigned char)tmpSave.cCounter;
                    iPointer+=4;
                }
                HugeRAM[iPointer] = (char) ( tmpSave.SAT[i] / 0x100 );
                HugeRAM[iPointer+1] = (char) ( tmpSave.SAT[i] % 0x100 );
                iPointer+=2;
            }
            //copy save itself to hugeram
            for (int i=0; i< tmpSave.iBytes; i++)
            {
                if (iPointer % TheConfig->m_iClusterSize == 0)
                {
                    HugeRAM[iPointer] = 0;
                    HugeRAM[iPointer+1] = 0;
                    HugeRAM[iPointer+2] = 0;
                    HugeRAM[iPointer+3] = (unsigned char)tmpSave.cCounter;
                    iPointer+=4;
                }
                file_in.read(buf,1);
                HugeRAM[iPointer] = buf[0];
                iPointer++;
            }
            //we're done the case with no sat case!
            //now only check & write header
        }
    }

    //all possible data gathered, what is not gathered is guessed or generated
    //save and sat already is injected into hugeram before confirmtion
    // it is stupidly wrong, but it won't change anything unless header is written

    //open form to confirm data we're inserting
    EnterSaveDetailsDialog checkDialog(&tmpSave);
    if (checkDialog.exec() == QDialog::Rejected) return;

    //write header
    if (TheConfig->m_bInsertSysUseCounter)
    {
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0x80;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+1] = 0x00;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+2] = 0x00;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+3] = tmpSave.cCounter;
    }
    else
    {
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0x80;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+1] = 0x00;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+2] = 0x00;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+3] = 0x00;//what counter value should we use here?
    }
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+4,11,tmpSave.Name.left(11));
    buf[0] = tmpSave.iLanguageCode;
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+15,1,QByteArray(buf,1));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+16,10,tmpSave.Comment.left(10));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+26,4,tmpSave.DateTimeRaw.left(4));
    buf[0]= (unsigned char)(tmpSave.iBytes/0x1000000);
    buf[1] = (unsigned char)((tmpSave.iBytes%0x1000000)/0x10000);
    buf[2] = (unsigned char)((tmpSave.iBytes%0x10000)/0x100);
    buf[3] = (unsigned char)(tmpSave.iBytes % 0x100);
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+30,4,QByteArray(buf,4));

    ParseHugeRAM();

    ui->statusBar->showMessage(QString("Save from file %1 inserted").arg(fileName));
}

void MainWindow::on_DeleteButton_clicked()
{
    //delete save
    SaveType tmpSave;
    TheConfig->LoadFromRegistry();
    if (ui->tableWidget->rowCount() < 1)
    {
        QMessageBox msgBox;
        msgBox.setText(QString("Excuse me, but we have nothing to delete."));
        msgBox.exec();
        return;
    }
    tmpSave = SavesList.at(ui->tableWidget->currentRow());
    switch (TheConfig->m_DeleteMode)
    {
    case DeleteSingleSys:
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0;
        break;
    case DeleteAllSys:
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0;
        for (int i=0; i<tmpSave.iSATSize-1;i++)
            HugeRAM[tmpSave.SAT[i]*TheConfig->m_iClusterSize] = 0;
        break;
    case DeleteFull:
        QByteArray empty = QByteArray(TheConfig->m_iClusterSize,(char)0);
        HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize,TheConfig->m_iClusterSize,empty);
        for (int i=0; i<tmpSave.iSATSize-1;i++)
            HugeRAM.replace(tmpSave.SAT[i]*TheConfig->m_iClusterSize,TheConfig->m_iClusterSize,empty);
        break;
    }
    ParseHugeRAM();
    ui->statusBar->showMessage(QString("Save deleted"));
}

void MainWindow::on_Sort_Order_Changed(int logicalIndex)
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

void MainWindow::on_Setup_Accepted()
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
        msgBox.setText(QString("Do you want to clear existing data?"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        if (msgBox.exec() == QMessageBox::Cancel)
            return;
    }
    //NewSettings.iIOCustomClusterSize=TheConfig->m_iClusterSize;
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
    ParseHugeRAM();//updating
}
