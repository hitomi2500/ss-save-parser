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


void MainWindow::ExtractSaves(int iStart,int iEnd)
{
    //we should ask for a filenames first, to find the format
    //there are two special formats, BUP and XML, they imply different sanity checks, so the checks come after file selection
    //for a "binary" (i.e. normal) saves, some sanity checks should be done once per insert and some once per file
    //QStringList fileNames = QFileDialog::getOpenFileNames(this,tr("Load Savegame"), "", NULL);
    //if (fileNames.isEmpty()) return; //return if user cancel
    //cycle through each save
    QString fileName;
    SaveType tmpSave;
    QFile file_out;
    QString folderName;
    TheConfig->LoadFromRegistry();

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

        //extract single save
        if(fileName.toUpper().endsWith(".BUP")) {
            ExtractBUPSave(&file_out,tmpSave,true);
        }
        else if (fileName.toUpper().endsWith(".XML")) {
            ExtractXMLSave(&file_out,tmpSave,true);
        }
        else {
            if (false == GlobalBinaryExtractChecks())
                    return;
            ExtractBinarySave(&file_out,tmpSave,true);
        }
        file_out.close();
    }
    else
    {
        //when saving multiple files, ask for folder, not for file
        QFileDialog dialog(this);
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec())
            folderName = dialog.selectedFiles().at(0);
        else return; //return if user cancel

        //do a multiple extract - BUP and XML are not supported yet, so binary only
        for (int i=iStart; i<iEnd; i++)
        {
            //if multiple saves load every file now
            tmpSave = SavesList.at(i);
            QString fileName = folderName+QString("/")+QString(tmpSave.Name);
            file_out.setFileName(fileName);
            if (!(file_out.open(QIODevice::WriteOnly)))
            {
                QMessageBox msgBox;
                msgBox.setText(tr("Cannot open save file %s.").arg(fileName));
                msgBox.exec();
                return;
            }
            ExtractBinarySave(&file_out,tmpSave,false);
            file_out.close();
        }
    }

    //ui->statusBar->showMessage(tr("Save from file %1 inserted").arg(fileName));
    TheConfig->LoadFromRegistry();//restoring config after all those temporary updates (if any)

}

bool MainWindow::GlobalBinaryExtractChecks()
{
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
        return false;
    }

    //issue a warning if SAT is off and sys are on
    if ( (TheConfig->m_bExtractSAT==false) && ( (TheConfig->m_bExtractSys) || (TheConfig->m_bExtractSysAll) ) )
    {
        QMessageBox msgBox;
        msgBox.setText(tr("You're trying to do something strange saving headers and NOT saving SAT. This is possible setup all right, it will be extracted and inserted successfully, but data inside looks stupidly unrelated. I will continue, but you've been warned."));
        msgBox.exec();
    }

    return true;
}

void MainWindow::ExtractBinarySave(QFile *file_out, SaveType Save, bool bSingle)
{
    //extract save from image
    char buf[256];
    QString fileName;
    QString folderName;
    TheConfig->LoadFromRegistry();

    //file/folder opened, move on

    //copy extractwin's config from current one
    if (TheConfig->m_bAskFormatAtEveryExtract)
    {
        *(SetupWinExtract->SetupConfig) = *TheConfig;
    }

    if (TheConfig->m_bAskFormatAtEveryExtract)
    {
        SetupWinExtract->SetupConfig->m_bAskFormatAtEveryExtract = true;
        SetupWinExtract->SetupConfig->UpdateFlags();
        SetupWinExtract->UpdateFromConfig();
        SetupWinExtract->SetExtractInsertFilename(QString(Save.Name));
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
            buf[0] = HugeRAM[Save.iStartCluster*TheConfig->m_iClusterSize];
            buf[1] = HugeRAM[Save.iStartCluster*TheConfig->m_iClusterSize+1];
            buf[2] = HugeRAM[Save.iStartCluster*TheConfig->m_iClusterSize+2];
            buf[3] = HugeRAM[Save.iStartCluster*TheConfig->m_iClusterSize+3];
        }
        file_out->write(buf,4);
    }
    if (TheConfig->m_bExtractName)
    {
        file_out->write(Save.Name,11);
    }
    if (TheConfig->m_bExtractLanguage)
    {
        buf[0]=(char)Save.cLanguageCode;
        file_out->write(buf,1);
    }
    if (TheConfig->m_bExtractDescription)
    {
        file_out->write(Save.Comment,10);
    }
    //SSF specific - add zero and language code after description
    if (TheConfig->m_ExtractMode == ExtractSSF)
    {
        buf[0]=(char)0;
        buf[1]=(char)Save.cLanguageCode;
        file_out->write(buf,2);
    }
    if (TheConfig->m_bExtractDateTime)
    {
        //SSF specific date/time
        if (TheConfig->m_ExtractMode == ExtractSSF)
        {
            Save.DateTimeRaw = GetSSF4ByteFromDateTime(Save.DateTime);
            file_out->write(Save.DateTimeRaw,4);
        }
        else
        {
            file_out->write(Save.DateTimeRaw,4);
        }
    }
    if (TheConfig->m_bExtractSize)
    {
        buf[0]=(unsigned char)(Save.iBytes/0x1000000);
        buf[1]=(unsigned char)(Save.iBytes/0x10000);
        buf[2]=(unsigned char)(Save.iBytes/0x100);
        buf[3]=(unsigned char)(Save.iBytes);
        file_out->write(buf,4);
    }
    //Druid II specific - add 2 zeroes after header
    if (TheConfig->m_ExtractMode == ExtractDruidII)
    {
        buf[0]=(unsigned char)0;
        buf[1]=(unsigned char)0;
        file_out->write(buf,2);
    }
    //write 1st cluster
    int iSATnDataSize = Save.SAT.size()*2 + Save.iBytes;
    if ((iSATnDataSize + 34 ) < TheConfig->m_iClusterSize )
    {
        //writing single cluster
        if (TheConfig->m_bExtractSAT)
            file_out->write(HugeRAM.mid(Save.iStartCluster*TheConfig->m_iClusterSize+34,Save.SAT.size()*2),Save.SAT.size()*2);
        file_out->write(HugeRAM.mid(Save.iStartCluster*TheConfig->m_iClusterSize+34+Save.SAT.size()*2,Save.iBytes),Save.iBytes);
    }
    else
    {
        //writing first cluster
        if (TheConfig->m_bExtractSAT) //if saving SAT
            file_out->write(HugeRAM.mid(Save.iStartCluster*TheConfig->m_iClusterSize+34,TheConfig->m_iClusterSize),TheConfig->m_iClusterSize-34);
        else //not saving SAT
            if (34+Save.SAT.size()*2 < TheConfig->m_iClusterSize) //if sat uses first cluster, but not fully
                file_out->write(HugeRAM.mid(Save.iStartCluster*TheConfig->m_iClusterSize+34+Save.SAT.size()*2,TheConfig->m_iClusterSize),
                               TheConfig->m_iClusterSize-34-(Save.SAT.size()*2));

    }
    //now remaining clusters
    int iRemainingBytes = Save.SAT.size()*2+Save.iBytes + 34 - TheConfig->m_iClusterSize;
    for (int i=0;i<Save.SAT.size()-1;i++)
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
                    buf[3]=Save.cCounter;
                }
                file_out->write(buf,4);
            }
            //three cases here: full sat cluster, ful data cluster, sat with data in the end
            //detecting
            if ((iRemainingBytes - Save.iBytes) >= (TheConfig->m_iClusterSize-4) )
            {
                //full SAT cluster
                if (TheConfig->m_bExtractSAT)
                    file_out->write(HugeRAM.mid(Save.SAT.at(i)*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
            }
            else if (iRemainingBytes <= Save.iBytes)
            {
                //full data cluster
                file_out->write(HugeRAM.mid(Save.SAT.at(i)*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
            }
            else
            {
                int iRemainingSAT = iRemainingBytes - Save.iBytes;
                //SAT with data in the end
                if (TheConfig->m_bExtractSAT) //write as is
                    file_out->write(HugeRAM.mid(Save.SAT.at(i)*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
                else  //only write data part
                    file_out->write(HugeRAM.mid(Save.SAT.at(i)*TheConfig->m_iClusterSize+4+iRemainingSAT,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4-iRemainingSAT);
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
                    buf[3]=Save.cCounter;
                }
                file_out->write(buf,4);
            }
            file_out->write(HugeRAM.mid((Save.SAT.at(i)*TheConfig->m_iClusterSize)+4,iRemainingBytes),iRemainingBytes);
            iRemainingBytes = 0;
        }
    }
}

void MainWindow::ExtractBUPSave(QFile *file_out, SaveType Save, bool bSingle)
{
    //extract save from image
    char buf[256];
    QString fileName;
    QString folderName;
    TheConfig->LoadFromRegistry();
    int iWritten = 0;

    //file/folder opened, move on

    //add signature
    file_out->write("Vmem");
    //unknown up to 16
    for (int i=0;i<12;i++)
        file_out->write("\0",1);
    iWritten+=16;

    file_out->write(Save.Name,11);
    iWritten+=11;

    buf[0]=(char)Save.cLanguageCode;
    file_out->write(buf,1);
    iWritten+=1;

    file_out->write(Save.Comment,10);
    iWritten+=10;

    //skipping 2 unknown bytes for BUP
    for (int i=0;i<2;i++)
        file_out->write("\0",1);
    iWritten+=2;

    file_out->write(Save.DateTimeRaw,4);
    iWritten+=4;

    buf[0]=(unsigned char)(Save.iBytes/0x1000000);
    buf[1]=(unsigned char)(Save.iBytes/0x10000);
    buf[2]=(unsigned char)(Save.iBytes/0x100);
    buf[3]=(unsigned char)(Save.iBytes);
    file_out->write(buf,4);
    iWritten+=4;

    while(iWritten<64)
    {
        file_out->write("\0",1);//skipping to data
        iWritten++;
    }

    //write 1st cluster
    int iSATnDataSize = Save.SAT.size()*2 + Save.iBytes;
    if ((iSATnDataSize + 34 ) < TheConfig->m_iClusterSize )
    {
        //writing single cluster
        file_out->write(HugeRAM.mid(Save.iStartCluster*TheConfig->m_iClusterSize+34+Save.SAT.size()*2,Save.iBytes),Save.iBytes);
    }
    else
    {
        //writing first cluster
        if (34+Save.SAT.size()*2 < TheConfig->m_iClusterSize) //if sat uses first cluster, but not fully
            file_out->write(HugeRAM.mid(Save.iStartCluster*TheConfig->m_iClusterSize+34+Save.SAT.size()*2,TheConfig->m_iClusterSize),
                           TheConfig->m_iClusterSize-34-(Save.SAT.size()*2));

    }
    //now remaining clusters
    int iRemainingBytes = Save.SAT.size()*2+Save.iBytes + 34 - TheConfig->m_iClusterSize;
    for (int i=0;i<Save.SAT.size()-1;i++)
    {
        if ( iRemainingBytes > (TheConfig->m_iClusterSize-4)) //not counting headers
        {
            //three cases here: full sat cluster, ful data cluster, sat with data in the end
            //detecting
            if ((iRemainingBytes - Save.iBytes) >= (TheConfig->m_iClusterSize-4) )
            {
                //full SAT cluster
            }
            else if (iRemainingBytes <= Save.iBytes)
            {
                //full data cluster
                file_out->write(HugeRAM.mid(Save.SAT.at(i)*TheConfig->m_iClusterSize+4,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4);
            }
            else
            {
                int iRemainingSAT = iRemainingBytes - Save.iBytes;
                //only write data part
                file_out->write(HugeRAM.mid(Save.SAT.at(i)*TheConfig->m_iClusterSize+4+iRemainingSAT,TheConfig->m_iClusterSize-4),TheConfig->m_iClusterSize-4-iRemainingSAT);
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
            file_out->write(HugeRAM.mid((Save.SAT.at(i)*TheConfig->m_iClusterSize)+4,iRemainingBytes),iRemainingBytes);
            iRemainingBytes = 0;
        }
    }
}

void MainWindow::ExtractXMLSave(QFile *file_out, SaveType Save, bool bSingle)
{
    //XML extraction is similar to binary with some exceptions:
    // 1) all extract flags are ignored, everything is extracted except SAT and 2..n headers
    // 2) some additional data fields are added, like player's name, comment, source etc.

    //file/folder opened, move on

    //TODO: ask user for additional fields

    //collect all binary save data
    QByteArray tmpdata;
    tmpdata.clear();
    int iCurrentCluster = -1;
    int iCurrentPos = 34;
    //skipping SAT first
    for (int i=0;i<Save.SAT.size()*2;i++)
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
    for (int i=0;i<Save.iBytes;i++)
    {
        if (-1 == iCurrentCluster)
            tmpdata.append(HugeRAM.at(Save.iStartCluster*TheConfig->m_iClusterSize+iCurrentPos));
        else
            tmpdata.append(HugeRAM.at(Save.SAT.at(iCurrentCluster)*TheConfig->m_iClusterSize+iCurrentPos));

        if (iCurrentPos == TheConfig->m_iClusterSize-1)
        {
            iCurrentCluster++;
            iCurrentPos = 4;
        }
        else
            iCurrentPos++;
    }

    //start xml writer
    QXmlStreamWriter xml_write(file_out);
    xml_write.setAutoFormatting(true);
    xml_write.writeStartDocument(QString("1.0"));
    xml_write.writeStartElement(QString("Sega_Saturn_Save"));
    xml_write.writeStartElement(QString("mandatory"));
    xml_write.writeStartElement("counter");
    xml_write.writeCharacters(QString("%1").arg(Save.cCounter));
    xml_write.writeEndElement();//counter
    xml_write.writeStartElement("name");
    xml_write.writeCharacters(QString(Save.Name));
    xml_write.writeEndElement();//name
    xml_write.writeStartElement("name_binary");
    xml_write.writeCharacters(QString(Save.Name.toBase64()));
    xml_write.writeEndElement();//name_binary
    xml_write.writeStartElement("comment");
    xml_write.writeCharacters(QString(Save.Comment));
    xml_write.writeEndElement();//name_binary
    xml_write.writeStartElement("comment_binary");
    xml_write.writeCharacters(QString(Save.Comment.toBase64()));
    xml_write.writeEndElement();//comment
    xml_write.writeStartElement("language_code");
    xml_write.writeCharacters(QString("%1").arg(Save.cLanguageCode));
    xml_write.writeEndElement();//language_code
    xml_write.writeStartElement("size");
    xml_write.writeCharacters(QString("%1").arg(Save.iBytes));
    xml_write.writeEndElement();//size
    xml_write.writeStartElement(QString("date"));
    xml_write.writeAttribute("year",QString("%1").arg(Save.DateTime.date().year()));
    xml_write.writeAttribute("month",QString("%1").arg(Save.DateTime.date().month()));
    xml_write.writeAttribute("day",QString("%1").arg(Save.DateTime.date().day()));
    xml_write.writeEndElement();//date
    xml_write.writeStartElement(QString("time"));
    xml_write.writeAttribute("hour",QString("%1").arg(Save.DateTime.time().hour()));
    xml_write.writeAttribute("minute",QString("%1").arg(Save.DateTime.time().minute()));
    xml_write.writeAttribute("second",QString("%1").arg(Save.DateTime.time().second()));
    xml_write.writeEndElement();//time
    xml_write.writeStartElement("data");
    xml_write.writeCharacters(QString(tmpdata.toBase64()));
    xml_write.writeEndElement();//data
    xml_write.writeEndElement();//mandatory
    xml_write.writeStartElement(QString("optional"));
    xml_write.writeEndElement();//optional
    xml_write.writeEndElement();//Sega Saturn Save
    xml_write.writeEndDocument();

}
