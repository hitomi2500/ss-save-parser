#include <QByteArray>
#include <QList>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextCodec>
#include <QPushButton>
#include <QXmlStreamReader>
#include "config.h"
#include "mainwindow.h"
#include "parselib.h"
#include "entersavedetailsdialog.h"

extern QList<SaveType> SavesList;
extern QByteArray HugeRAM;//full size buffer

void MainWindow::InsertSaves()
{
    QFile file_in;
    //we should ask for a filenames first, to find the format
    //there are two special formats, BUP and XML, they imply different sanity checks, so the checks come after file selection
    //for a "binary" (i.e. normal) saves, some sanity checks should be done once per insert and some once per file
    QStringList fileNames = QFileDialog::getOpenFileNames(this,tr("Load Savegame"), "", NULL);
    if (fileNames.isEmpty()) return; //return if user cancel
    //cycle through each save
    QString fileName;
    bool bGlobalBinaryChecksDone = false;

    if (fileNames.size() > 1)
    {
        //multiple insert
        for (int iListIndex=0; iListIndex<fileNames.size(); iListIndex++)
        {
            //trying to open next file
            fileName = fileNames.at(iListIndex);
            file_in.setFileName(fileName);
            if (!(file_in.open(QIODevice::ReadOnly)))
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Cannot open save file %s.").arg(fileName));
                msgBox.exec();
                return;
            }
            //file opened, move on

            //if the opened file is .BUP, expect the format to be known
            if(fileName.toUpper().endsWith(".BUP")) {
                InsertBUPSave(&file_in,false);
            }
            else if (fileName.toUpper().endsWith(".XML")) {
                InsertXMLSave(&file_in,false);
            }
            else {
                if (false == bGlobalBinaryChecksDone)
                {
                    if (false == GlobalBinaryInsertChecks())
                        return;
                    bGlobalBinaryChecksDone = true;
                }
                InsertBinarySave(&file_in,false);
            }
        }
    }
    else
    {
        //single insert
        fileName = fileNames.at(0);
        file_in.setFileName(fileName);
        if (!(file_in.open(QIODevice::ReadOnly)))
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Cannot open save file %s.").arg(fileName));
            msgBox.exec();
            return;
        }
        //file opened, move on

        //if the opened file is .BUP, expect the format to be known
        if(fileName.toUpper().endsWith(".BUP")) {
            InsertBUPSave(&file_in,true);
        }
        else if (fileName.toUpper().endsWith(".XML")) {
            InsertXMLSave(&file_in,true);
        }
        else {
            if (false == GlobalBinaryInsertChecks())
                    return;
            InsertBinarySave(&file_in,true);
        }
    }

    //ui->statusBar->showMessage(tr("Save from file %1 inserted").arg(fileName));
    TheConfig->LoadFromRegistry();//restoring config after all those temporary updates (if any)
}

bool MainWindow::GlobalBinaryInsertChecks()
{
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
        msgBox.setText(tr("The combination you set (no SAT, no size, all headers) is not reliably recoverable. There are possible workarounds, but i'm not into implementing them. Not inserting."));
        msgBox.exec();
        return false;
    }

    return true;
}



void MainWindow::InsertBinarySave(QFile *file_in, bool bSingle)
{
    //insert save
    char buf[256];
    QList<QByteArray> SysHeadersList;
    QByteArray SysHeader1st;
    SaveType tmpSave;
    int iOldClusterSize;
    TheConfig->LoadFromRegistry();
    SysHeadersList.clear();
    SysHeader1st.clear();
    int iOldSATSize,iNewSATSize;
    QString fileName = file_in->fileName();

    //copy insertwin's config from current one
    if (TheConfig->m_bAskFormatAtEveryInsert)
    {
        *(SetupWinInsert->SetupConfig) = *TheConfig;
    }

    //file opened, move on

    //change config per save if required
    if (TheConfig->m_bAskFormatAtEveryInsert)
    {
        SetupWinInsert->SetupConfig->m_bAskFormatAtEveryInsert = true;
        SetupWinInsert->SetupConfig->UpdateFlags();
        SetupWinInsert->UpdateFromConfig();
        SetupWinInsert->SetExtractInsertFilename(fileName);
        //opening format window as modal
        if (SetupWinInsert->exec() == QDialog::Rejected) return;
        //getting temporal config from it
        *TheConfig = *(SetupWinInsert->SetupConfig);
        //force m_bAskFormatAtEveryExtract flag in config
    }

    //check if we have enough space at the end of file
    //we don't insert at 0th or 1st cluster - they're signature cluster and always zero cluster
    //so minimal last used is 1 - when image is empty
    int iLastUsedCluster=1;
    for (int i=0; i<SavesList.size(); i++)
    {
        for (int j=0; j<SavesList.at(i).SAT.size();j++)
        {
            if (SavesList.at(i).SAT.at(j) > iLastUsedCluster)
                iLastUsedCluster = SavesList.at(i).SAT.at(j);
            if (SavesList.at(i).iStartCluster > iLastUsedCluster)
                iLastUsedCluster = SavesList.at(i).iStartCluster;
        }
    }
    //making a brutal check for inserting size:
    //each ClusterSize-4 requires additional 6 bytes (2 for SAT, 4 for header)
    //plus 36 additional (34 header, 2 zero-sat-entry)
    //if that won't fit into remaining clusters, boil out
    int iClustersRequired = (file_in->size())/(TheConfig->m_iClusterSize-4);
    int iBytesRequired = file_in->size()+iClustersRequired*6+36;
    iClustersRequired = iBytesRequired/(TheConfig->m_iClusterSize-4);
    if ((TheConfig->m_iFileSize/TheConfig->m_iClusterSize - iLastUsedCluster) <= iClustersRequired)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Not enough space in image to insert save file %1.").arg(fileName));
        msgBox.exec();
        return;
    }
    tmpSave.iStartCluster = iLastUsedCluster+1;

    //check done, go on

    //trying to get as much info from file as possible
    SysHeader1st.clear();
    if (TheConfig->m_bInsertSys)
    {
        file_in->read(buf,4);
        SysHeader1st.append(QByteArray(buf,4));
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
        file_in->read(buf,11);
        tmpSave.Name = QByteArray(buf,11);
    }
    else
    {
        //no name was provided, using file name
        QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
        //this is windows-specific, change something if porting
        tmpSave.Name = codec->fromUnicode(fileName.mid(fileName.lastIndexOf(QChar('/'))+1,11));
    }

    if (TheConfig->m_bInsertLanguage)
    {
        file_in->read(buf,1);
        tmpSave.cLanguageCode = (unsigned char) buf[0];
    }
    else
    {
        //no language code was provided, making one up
        tmpSave.cLanguageCode = 0;
    }

    if (TheConfig->m_bInsertDescription)
    {
        file_in->read(buf,10);
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

    //SSF specific - skip 1 zero and get language code after description
    if (TheConfig->m_InsertMode == InsertSSF)
    {
        file_in->read(buf,2);
        tmpSave.cLanguageCode = (unsigned char) buf[1];
    }

    if (TheConfig->m_bInsertDateTime)
    {
        file_in->read(buf,4);
        tmpSave.DateTimeRaw = QByteArray(buf,4);
        //SSF specific date/time
        if (TheConfig->m_InsertMode == InsertSSF)
        {
            tmpSave.DateTime = GetDateTimeFromSSF4Byte(tmpSave.DateTimeRaw);
            tmpSave.DateTimeRaw = GetRaw4ByteFromDateTime(tmpSave.DateTime);
        }
        else
        {
            tmpSave.DateTime = GetDateTimeFromRaw4Byte(tmpSave.DateTimeRaw);
        }
    }
    else
    {
        //no date was provided, using current one
        tmpSave.DateTime = QDateTime::currentDateTime();
        //convert that one to raw
        tmpSave.DateTimeRaw = GetRaw4ByteFromDateTime(tmpSave.DateTime);
    }

    if (TheConfig->m_bInsertSize)
    {
        file_in->read(buf,4);
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
    if (TheConfig->m_InsertMode == InsertDruidII)
    {
        file_in->read(buf,2);
    }

    //sat
    if (TheConfig->m_bInsertSAT)
    {
        //old SAT is only acceptable if we use the same cluster value
        //since this could not be the fact, we're simply dumping old SAT
        //and recalculating a new one later.
        //but we need to know SAT size in order to find data start, and
        //to correctly count data size if it is not provided in header
        iOldSATSize=1;
        file_in->read(buf,2);
        while ( (buf[0]!=0) || (buf[1]!= 0) )
        {
            iOldSATSize++;
            if ( (TheConfig->m_bInsertSysAll) && ((iOldSATSize*2+30)%(TheConfig->m_iClusterSize-4) == 0) )
            {
                file_in->read(buf,4);//if moving onto next SAT cluster, skip 4 bytes
                SysHeadersList.append(QByteArray(buf,4));
            }
            file_in->read(buf,2);
        }
        //okay, now that know old SAT size, we can calculate size, if it's not provided
        int iTmpSize = 0;
        //if (TheConfig->m_bInsertSize == false)
        {
            iTmpSize = file_in->size();
            if (TheConfig->m_bInsertDateTime) iTmpSize -= 4;
            if (TheConfig->m_bInsertDescription) iTmpSize -= 10;
            if (TheConfig->m_bInsertLanguage) iTmpSize -= 1;
            if (TheConfig->m_bInsertName) iTmpSize -= 11;
            if (TheConfig->m_bInsertSAT) iTmpSize -= iOldSATSize*2;
            if (TheConfig->m_bInsertSize) iTmpSize -= 4;
            if (TheConfig->m_bInsertSys) iTmpSize -= 4;
            if (TheConfig->m_bInsertSysAll) iTmpSize -= (iOldSATSize-1)*4;
        }
        //now size stuff
        if (TheConfig->m_bInsertSize)
        {
            //if the size was provided in file as well, do a sanity check
            if (tmpSave.iBytes != iTmpSize)
            {
                //sanity check failed, aborting
                QMessageBox msgBox;
                msgBox.setText(tr("It looks like this save is corrupted, saved with another features set, or is not a save at all. Continue?"));
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
        if (tmpSave.iBytes / iOldSATSize < 253 )
            iOldClusterSize = 256;
        else
            iOldClusterSize = 512;
        //okay, we know size, calculate new SAT size
        iNewSATSize = 1; //dumping old sat
        while ( (30 + iNewSATSize*2 + tmpSave.iBytes)/(TheConfig->m_iClusterSize-4) > iNewSATSize)
            iNewSATSize++;
        //fill new sat
        for (int i=0;i<iNewSATSize;i++)
            tmpSave.SAT.append(tmpSave.iStartCluster+i+1);
        tmpSave.SAT.append(0);
        //copy SAT to hugeram
        int iPointer = tmpSave.iStartCluster*TheConfig->m_iClusterSize + 34;
        for (int i=0;i<tmpSave.SAT.size();i++)
        {
            if (iPointer % TheConfig->m_iClusterSize == 0)
            {
                HugeRAM[iPointer] = 0;
                HugeRAM[iPointer+1] = 0;
                HugeRAM[iPointer+2] = 0;
                HugeRAM[iPointer+3] = tmpSave.cCounter;
                iPointer+=4;
            }
            HugeRAM[iPointer] = (char) ( tmpSave.SAT.at(i) / 0x100 );
            HugeRAM[iPointer+1] = (char) ( tmpSave.SAT.at(i) % 0x100 );
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
                HugeRAM[iPointer+3] = tmpSave.cCounter;
                iPointer+=4;
            }
            if ((TheConfig->m_bInsertSysAll) && (file_in->pos() % iOldClusterSize == 0) )
            {
                file_in->read(buf,4);
                SysHeadersList.append(QByteArray(buf,4));
            }
            file_in->read(buf,1);
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
        //and since we cannot calculate old clusters' size, we boiled out before in
        //the case sys headers were inserted
        //now size stuff
        if (!TheConfig->m_bInsertSize) //if no size was provided, calculate it
            tmpSave.iBytes = file_in->size()-file_in->pos();

        //calculate new SAT size
        iNewSATSize = 1;
        while ( (30 + iNewSATSize*2 + tmpSave.iBytes)/(TheConfig->m_iClusterSize-4) > iNewSATSize)
            iNewSATSize++;
        //fill new sat
        for (int i=0;i<iNewSATSize;i++)
            tmpSave.SAT.append(tmpSave.iStartCluster+i+1);
        tmpSave.SAT.append(0);
        //copy SAT to hugeram
        int iPointer = tmpSave.iStartCluster*TheConfig->m_iClusterSize + 34;
        for (int i=0;i<tmpSave.SAT.size();i++)
        {
            if (iPointer % TheConfig->m_iClusterSize == 0)
            {
                HugeRAM[iPointer] = 0;
                HugeRAM[iPointer+1] = 0;
                HugeRAM[iPointer+2] = 0;
                HugeRAM[iPointer+3] = tmpSave.cCounter;
                iPointer+=4;
            }
            HugeRAM[iPointer] = (char) ( tmpSave.SAT.at(i) / 0x100 );
            HugeRAM[iPointer+1] = (char) ( tmpSave.SAT.at(i) % 0x100 );
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
                HugeRAM[iPointer+3] = tmpSave.cCounter;
                iPointer+=4;
            }
            file_in->read(buf,1);
            HugeRAM[iPointer] = buf[0];
            iPointer++;
        }
    }

    //all possible data gathered, what is not gathered is guessed or generated
    //save and sat are already injected into hugeram before confirmation
    // it is stupidly wrong, but it won't change anything unless header is written

    //if it's single file mode, open form to confirm data we're inserting
    if (true == bSingle)
    {
        EnterSaveDetailsDialog checkDialog(&tmpSave);
        if (checkDialog.exec() == QDialog::Rejected)
        {
            return;
        }
    }

    //check for doubling saves with the same name
    bool bDuped = false;
    int iDupedWith = 0;
    for (int i=0; i < SavesList.size(); i++)
    {
        if (SavesList.at(i).Name == tmpSave.Name)
        {
            bDuped = true;
            iDupedWith = i;
        }
    }
    if (bDuped)
    {
        //dupe handling logic was faulty, so removing it
        //inserting all dupes, but issuing a warning for every dupe
        QMessageBox msgBox;
        msgBox.setText(tr("Save with a name %1 is already present in the image. This might raise issues in BIOS and games. Please remove duplicate saves manually.").arg(QString(tmpSave.Name)));
        msgBox.exec();
    }

    //first we must re-copy headers if they are to be inserted
    //since the previous cycle was async one
    if (TheConfig->m_bInsertSys)
    {
        //replacing first sys header
        HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize,4,SysHeader1st);
    }
    else
    {
        //creating dummy first sys header
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0x80;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+1] = 0x00;
        HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+2] = 0x00;
    }
    if (TheConfig->m_bInsertSysAll)
    {
        //replacing remaining sys headers
        //new SAT size might be different, so choosing smaller
        if (tmpSave.SAT.size() > SysHeadersList.size())
            for (int i=0;i<SysHeadersList.size();i++)
                HugeRAM.replace(tmpSave.SAT.at(i)*TheConfig->m_iClusterSize,4,SysHeadersList.at(i));
        else
            for (int i=0;i<(tmpSave.SAT.size()-1);i++)
                HugeRAM.replace(tmpSave.SAT.at(i)*TheConfig->m_iClusterSize,4,SysHeadersList.at(i));
    }
    //then we are to patch counter, regardless of what the config is,
    //since user had a chance to edit it already
    //patching first entity
    HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+3] = tmpSave.cCounter;
    //patching the rest
    for (int i=1; i<(tmpSave.SAT.size()); i++)
        HugeRAM[tmpSave.SAT.at(i-1)*TheConfig->m_iClusterSize+3] = tmpSave.cCounter;
    //write header
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+4,11,tmpSave.Name.left(11));
    buf[0] = (char) tmpSave.cLanguageCode;
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+15,1,QByteArray(buf,1));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+16,10,tmpSave.Comment.left(10));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+26,4,tmpSave.DateTimeRaw.left(4));
    buf[0]= (unsigned char)(tmpSave.iBytes/0x1000000);
    buf[1] = (unsigned char)((tmpSave.iBytes%0x1000000)/0x10000);
    buf[2] = (unsigned char)((tmpSave.iBytes%0x10000)/0x100);
    buf[3] = (unsigned char)(tmpSave.iBytes % 0x100);
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+30,4,QByteArray(buf,4));
    file_in->close();
    ParseHugeRAM();
    SysHeadersList.clear();
}


void MainWindow::InsertBUPSave(QFile *file_in, bool bSingle)
{
    char buf[256];
    QList<QByteArray> SysHeadersList;
    QByteArray SysHeader1st;
    SaveType tmpSave;
    int iOldClusterSize;
    TheConfig->LoadFromRegistry();
    SysHeadersList.clear();
    SysHeader1st.clear();
    bool bOverwriteDupes = false;
    bool bFirstDupeFound = false;
    int iOldSATSize,iNewSATSize;
    QString fileName = file_in->fileName();

    //file opened, move on

    //do a simple signature check
    file_in->read(buf,4);
    buf[4] = 0;
    if (strcmp(buf,"Vmem") == 0)
    {
        //signature found, this is BUP file after all
        file_in->seek(16);
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Invalid BUP file %1 ").arg(fileName));
        msgBox.exec();
        return;
    }

    //check if we have enough space at the end of file
    //we don't insert at 0th or 1st cluster - they're signature cluster and always zero cluster
    //so minimal last used is 1 - when image is empty
    int iLastUsedCluster=1;
    for (int i=0; i<SavesList.size(); i++)
    {
        for (int j=0; j<SavesList.at(i).SAT.size();j++)
        {
            if (SavesList.at(i).SAT.at(j) > iLastUsedCluster)
                iLastUsedCluster = SavesList.at(i).SAT.at(j);
            if (SavesList.at(i).iStartCluster > iLastUsedCluster)
                iLastUsedCluster = SavesList.at(i).iStartCluster;
        }
    }
    //making a brutal check for inserting size:
    //each ClusterSize-4 requires additional 6 bytes (2 for SAT, 4 for header)
    //plus 36 additional (34 header, 2 zero-sat-entry)
    //if that won't fit into remaining clusters, boil out
    int iClustersRequired = (file_in->size())/(TheConfig->m_iClusterSize-4);
    int iBytesRequired = file_in->size()+iClustersRequired*6+36;
    iClustersRequired = iBytesRequired/(TheConfig->m_iClusterSize-4);
    if ((TheConfig->m_iFileSize/TheConfig->m_iClusterSize - iLastUsedCluster) <= iClustersRequired)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Not enough space in image to insert save file %1.").arg(fileName));
        msgBox.exec();
        return;
    }
    tmpSave.iStartCluster = iLastUsedCluster+1;

    //check done, go on

    //no counter was provided, making one up
    tmpSave.cCounter = 0;

    file_in->read(buf,11);
    tmpSave.Name = QByteArray(buf,11);

    file_in->read(buf,1);
    tmpSave.cLanguageCode = (unsigned char) buf[0];

    file_in->read(buf,10);
    tmpSave.Comment = QByteArray(buf,10);

    file_in->read(buf,2); //skipping 2 unknown bytes for BUP

    file_in->read(buf,4);
    tmpSave.DateTimeRaw = QByteArray(buf,4);
    tmpSave.DateTime = GetDateTimeFromRaw4Byte(tmpSave.DateTimeRaw);

    file_in->read(buf,4);
    tmpSave.iBytes = (unsigned char)buf[0]*0x1000000 +
            (unsigned char)buf[1]*0x10000 +
            (unsigned char)buf[2]*0x100 +
            (unsigned char)buf[3];

    //old SAT is not provided, that saves us some pain, we know data is right ahead
    //but if the data size was not provided as well, we must count data size manually
    //and since we cannot calculate old clusters' size, we boiled out before in
    //the case sys headers were inserted
    //now size stuff
    if (!TheConfig->m_bInsertSize) //if no size was provided, calculate it
        tmpSave.iBytes = file_in->size()-file_in->pos();

    //calculate new SAT size
    iNewSATSize = 1;
    while ( (30 + iNewSATSize*2 + tmpSave.iBytes)/(TheConfig->m_iClusterSize-4) > iNewSATSize)
        iNewSATSize++;
    //fill new sat
    for (int i=0;i<iNewSATSize;i++)
        tmpSave.SAT.append(tmpSave.iStartCluster+i+1);
    tmpSave.SAT.append(0);
    //copy SAT to hugeram
    int iPointer = tmpSave.iStartCluster*TheConfig->m_iClusterSize + 34;
    for (int i=0;i<tmpSave.SAT.size();i++)
    {
        if (iPointer % TheConfig->m_iClusterSize == 0)
        {
            HugeRAM[iPointer] = 0;
            HugeRAM[iPointer+1] = 0;
            HugeRAM[iPointer+2] = 0;
            HugeRAM[iPointer+3] = tmpSave.cCounter;
            iPointer+=4;
        }
        HugeRAM[iPointer] = (char) ( tmpSave.SAT.at(i) / 0x100 );
        HugeRAM[iPointer+1] = (char) ( tmpSave.SAT.at(i) % 0x100 );
        iPointer+=2;
    }

    file_in->seek(64);//skipping to data

    //copy save itself to hugeram
    for (int i=0; i< tmpSave.iBytes; i++)
    {
        if (iPointer % TheConfig->m_iClusterSize == 0)
        {
            HugeRAM[iPointer] = 0;
            HugeRAM[iPointer+1] = 0;
            HugeRAM[iPointer+2] = 0;
            HugeRAM[iPointer+3] = tmpSave.cCounter;
            iPointer+=4;
        }
        file_in->read(buf,1);
        HugeRAM[iPointer] = buf[0];
        iPointer++;
    }

    //all possible data gathered, what is not gathered is guessed or generated
    //save and sat are already injected into hugeram before confirmation
    // it is stupidly wrong, but it won't change anything unless header is written

    //if it's single file mode, open form to confirm data we're inserting
    if (true == bSingle)
    {
        EnterSaveDetailsDialog checkDialog(&tmpSave);
        if (checkDialog.exec() == QDialog::Rejected)
        {
            return;
        }
    }

    //check for doubling saves with the same name
    bool bDuped = false;
    int iDupedWith = 0;
    for (int i=0; i < SavesList.size(); i++)
    {
        if (SavesList.at(i).Name == tmpSave.Name)
        {
            bDuped = true;
            iDupedWith = i;
        }
    }
    if (bDuped)
    {
        //dupe handling logic was faulty, so removing it
        //inserting all dupes, but issuing a warning for every dupe
        QMessageBox msgBox;
        msgBox.setText(tr("Save with a name %1 is already present in the image. This might raise issues in BIOS and games. Please remove duplicate saves manually.").arg(QString(tmpSave.Name)));
        msgBox.exec();
    }


    //creating dummy first sys header
    HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize] = 0x80;
    HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+1] = 0x00;
    HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+2] = 0x00;

    //then we are to patch counter, regardless of what the config is,
    //since user had a chance to edit it already
    //patching first entity
    HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+3] = tmpSave.cCounter;
    //patching the rest
    for (int i=1; i<(tmpSave.SAT.size()); i++)
        HugeRAM[tmpSave.SAT.at(i-1)*TheConfig->m_iClusterSize+3] = tmpSave.cCounter;
    //write header
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+4,11,tmpSave.Name.left(11));
    buf[0] = (char) tmpSave.cLanguageCode;
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+15,1,QByteArray(buf,1));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+16,10,tmpSave.Comment.left(10));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+26,4,tmpSave.DateTimeRaw.left(4));
    buf[0]= (unsigned char)(tmpSave.iBytes/0x1000000);
    buf[1] = (unsigned char)((tmpSave.iBytes%0x1000000)/0x10000);
    buf[2] = (unsigned char)((tmpSave.iBytes%0x10000)/0x100);
    buf[3] = (unsigned char)(tmpSave.iBytes % 0x100);
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+30,4,QByteArray(buf,4));
    file_in->close();
    ParseHugeRAM();
    SysHeadersList.clear();

}


void MainWindow::InsertXMLSave(QFile *file_in, bool bSingle)
{
    //insert save
    char buf[256];
    QList<QByteArray> SysHeadersList;
    QByteArray SysHeader1st;
    SaveType tmpSave;
    TheConfig->LoadFromRegistry();
    SysHeadersList.clear();
    SysHeader1st.clear();
    bool bOverwriteDupes = false;
    bool bFirstDupeFound = false;
    int iNewSATSize;
    QString fileName = file_in->fileName();

    //file opened, move on

    //check if we have enough space at the end of file
    //we don't insert at 0th or 1st cluster - they're signature cluster and always zero cluster
    //so minimal last used is 1 - when image is empty
    int iLastUsedCluster=1;
    for (int i=0; i<SavesList.size(); i++)
    {
        for (int j=0; j<SavesList.at(i).SAT.size();j++)
        {
            if (SavesList.at(i).SAT.at(j) > iLastUsedCluster)
                iLastUsedCluster = SavesList.at(i).SAT.at(j);
            if (SavesList.at(i).iStartCluster > iLastUsedCluster)
                iLastUsedCluster = SavesList.at(i).iStartCluster;
        }
    }

    //start xml reader
    QXmlStreamReader xml_read(file_in);
    //find size section in xml
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("size")));

    if (xml_read.hasError())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Broken XML file %1.").arg(fileName));
        msgBox.exec();
        return;
    }
    if (xml_read.atEnd())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The file %1 is definitely XML, but it misses required fields. Wrong XML ?").arg(fileName));
        msgBox.exec();
        return;
    }
    tmpSave.iBytes = QString(xml_read.readElementText()).toInt();

    //making a brutal check for inserting size:
    //each ClusterSize-4 requires additional 6 bytes (2 for SAT, 4 for header)
    //plus 36 additional (34 header, 2 zero-sat-entry)
    //if that won't fit into remaining clusters, boil out
    int iClustersRequired = (tmpSave.iBytes)/(TheConfig->m_iClusterSize-4);
    int iBytesRequired = file_in->size()+iClustersRequired*6+36;
    iClustersRequired = iBytesRequired/(TheConfig->m_iClusterSize-4);
    if ((TheConfig->m_iFileSize/TheConfig->m_iClusterSize - iLastUsedCluster) <= iClustersRequired)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Not enough space in image to insert save file %1.").arg(fileName));
        msgBox.exec();
        return;
    }
    tmpSave.iStartCluster = iLastUsedCluster+1;

    //check done, go on

    //xml should contain ALL mandatory fields, so reading'em all
    //counter
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("counter")));
    tmpSave.cCounter = xml_read.readElementText().toInt();

    //name
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("name_binary")));
    tmpSave.Name = QByteArray::fromBase64(xml_read.readElementText().toLatin1());

    //language code
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("language_code")));
    tmpSave.cLanguageCode = xml_read.readElementText().toInt();

    //description
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("comment_binary")));
    tmpSave.Comment = QByteArray::fromBase64(xml_read.readElementText().toLatin1());

    //size
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("size")));
    tmpSave.iBytes = xml_read.readElementText().toInt();

    //date and time
    int iYear,iMonth,iDay;
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("date")));
    for (int i =0;  i < xml_read.attributes().size();i++)
    {
        if (xml_read.attributes().at(i).name()==QString("year"))
            iYear =  xml_read.attributes().at(i).value().toInt();
        if (xml_read.attributes().at(i).name()==QString("month"))
            iMonth =  xml_read.attributes().at(i).value().toInt();
        if (xml_read.attributes().at(i).name()==QString("day"))
            iDay =  xml_read.attributes().at(i).value().toInt();
    }
    int iHour,iMinute,iSecond;
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("time")));
    for (int i =0;  i < xml_read.attributes().size();i++)
    {
        if (xml_read.attributes().at(i).name()==QString("hour"))
            iHour =  xml_read.attributes().at(i).value().toInt();
        if (xml_read.attributes().at(i).name()==QString("minute"))
            iMinute =  xml_read.attributes().at(i).value().toInt();
        if (xml_read.attributes().at(i).name()==QString("second"))
            iSecond =  xml_read.attributes().at(i).value().toInt();
    }
    tmpSave.DateTime.setDate(QDate(iYear,iMonth,iDay));
    tmpSave.DateTime.setTime(QTime(iHour,iMinute,iSecond,0));
    tmpSave.DateTimeRaw = GetRaw4ByteFromDateTime(tmpSave.DateTime);

    //calculate new SAT size
    iNewSATSize = 1;
    while ( (30 + iNewSATSize*2 + tmpSave.iBytes)/(TheConfig->m_iClusterSize-4) > iNewSATSize)
        iNewSATSize++;
    //fill new sat
    for (int i=0;i<iNewSATSize;i++)
        tmpSave.SAT.append(tmpSave.iStartCluster+i+1);
    tmpSave.SAT.append(0);
    //copy SAT to hugeram
    int iPointer = tmpSave.iStartCluster*TheConfig->m_iClusterSize + 34;
    for (int i=0;i<tmpSave.SAT.size();i++)
    {
        if (iPointer % TheConfig->m_iClusterSize == 0)
        {
            HugeRAM[iPointer] = 0;
            HugeRAM[iPointer+1] = 0;
            HugeRAM[iPointer+2] = 0;
            HugeRAM[iPointer+3] = tmpSave.cCounter;
            iPointer+=4;
        }
        HugeRAM[iPointer] = (char) ( tmpSave.SAT.at(i) / 0x100 );
        HugeRAM[iPointer+1] = (char) ( tmpSave.SAT.at(i) % 0x100 );
        iPointer+=2;
    }

    //copy save itself to hugeram
    file_in->seek(0);
    xml_read.setDevice(file_in);
    do xml_read.readNext(); while ((false==xml_read.atEnd())&&(xml_read.name()!=QString("data")));
    QByteArray rawdata =  QByteArray::fromBase64(xml_read.readElementText().toLatin1());
    int iRawPointer=0;
    for (int i=0; i< tmpSave.iBytes; i++)
    {
        if (iPointer % TheConfig->m_iClusterSize == 0)
        {
            HugeRAM[iPointer] = 0;
            HugeRAM[iPointer+1] = 0;
            HugeRAM[iPointer+2] = 0;
            HugeRAM[iPointer+3] = tmpSave.cCounter;
            iPointer+=4;
        }
        HugeRAM[iPointer] = rawdata.at(iRawPointer);
        iRawPointer++;
        iPointer++;
    }

    //TODO: output optional fields

    //all possible data gathered from xml
    //save and sat are already injected into hugeram before confirmation
    // it is stupidly wrong, but it won't change anything unless header is written

    //if it's single file mode, open form to confirm data we're inserting
    if (true == bSingle)
    {
        EnterSaveDetailsDialog checkDialog(&tmpSave);
        if (checkDialog.exec() == QDialog::Rejected) return;
    }

    //check for doubling saves with the same name
    bool bDuped = false;
    int iDupedWith = 0;
    for (int i=0; i < SavesList.size(); i++)
    {
        if (SavesList.at(i).Name == tmpSave.Name)
        {
            bDuped = true;
            iDupedWith = i;
        }
    }
    if (bDuped)
    {
        //dupe handling logic was faulty, so removing it
        //inserting all dupes, but issuing a warning for every dupe
        QMessageBox msgBox;
        msgBox.setText(tr("Save with a name %1 is already present in the image. This might raise issues in BIOS and games. Please remove duplicate saves manually.").arg(QString(tmpSave.Name)));
        msgBox.exec();
    }

    //first we must re-copy headers
    //replacing first sys header
    SysHeader1st[0] = 0x80;
    SysHeader1st[3] = tmpSave.cCounter;
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize,4,SysHeader1st);
    //then we are to patch counter, regardless of what the config is,
    //since user had a chance to edit it already
    //patching first entity
    HugeRAM[tmpSave.iStartCluster*TheConfig->m_iClusterSize+3] = tmpSave.cCounter;
    //patching the rest
    for (int i=1; i<(tmpSave.SAT.size()); i++)
        HugeRAM[tmpSave.SAT.at(i-1)*TheConfig->m_iClusterSize+3] = tmpSave.cCounter;
    //write header
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+4,11,tmpSave.Name.left(11));
    buf[0] = (char) tmpSave.cLanguageCode;
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+15,1,QByteArray(buf,1));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+16,10,tmpSave.Comment.left(10));
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+26,4,tmpSave.DateTimeRaw.left(4));
    buf[0]= (unsigned char)(tmpSave.iBytes/0x1000000);
    buf[1] = (unsigned char)((tmpSave.iBytes%0x1000000)/0x10000);
    buf[2] = (unsigned char)((tmpSave.iBytes%0x10000)/0x100);
    buf[3] = (unsigned char)(tmpSave.iBytes % 0x100);
    HugeRAM.replace(tmpSave.iStartCluster*TheConfig->m_iClusterSize+30,4,QByteArray(buf,4));
    file_in->close();
    ParseHugeRAM();
    SysHeadersList.clear();

}
