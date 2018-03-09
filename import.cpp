#include <QByteArray>
#include <QMessageBox>
#include <QFile>
#include <mainwindow.h>
#include <import.h>
#include <math.h>

bool ProbeMemCardPlus(QFile * file_in)
{
    //MemCardPlus  contains "MEMORY CARTRIDGE" id at 0x50
    file_in->seek(0);
    QByteArray cbuf = file_in->read(256);
    if (QByteArray(cbuf,96).right(16).startsWith("MEMORY CARTRIDGE"))
        return true;
    else
        return false;
}

bool ProbeEMS8Meg(QFile * file_in)
{
    //EMS 8 Meg card  contains "NEW 8 MEG RAM CARD+" id at 0x60 and is compatible with MemCardPlus
    file_in->seek(0);
    QByteArray cbuf = file_in->read(256);
    if (QByteArray(cbuf,115).right(19).startsWith("NEW 8 MEG RAM CARD+"))
        return true;
    else
        return false;
}

bool ProbeActionReplay(QFile * file_in)
{
    //ActionReplay contains "ACTION REPLAY +U" id at 0x50
    file_in->seek(0);
    QByteArray cbuf = file_in->read(256);
    if (QByteArray(cbuf,96).right(16).startsWith("ACTION REPLAY"))
        return true;
    else
        return false;
}

ImportResult ImportMemCardPlus(QByteArray *hugeram, QFile * file_in, int iClusterSize)
{
    char cbuf[256];
    char cbuf_prev[256];
    SaveType tmpSave;
    int iLastUsedCluster=2;
    //searching every 256 bytes
    file_in->seek(0);
    file_in->read(cbuf_prev,256);
    int iFileSize = file_in->size();
    while (iFileSize - file_in->pos()  > 256)
    {
        //read next 256 bytes
        file_in->read(cbuf,40);
        //check if save
        if ( (cbuf_prev[255]==-1) &&  (cbuf_prev[254]==-1) &&  (cbuf[0]!=-1) && (cbuf[22]==0) && (cbuf[23]==0))
        {
            //check passed, inserting
            //datel format is compression-key-based
            //key is a unique 16-bit value at offset 34 in save
            //it never appears in original save (datel searches this value from 0x0000 to 0xFFFF
            //                                  most of the time it's 0x000X or 0x001X)
            //RLE sequences are 48-bit:
            //0xKKKK 0xXXXX 0xYYYY - write YYYY copies of XXXX, KKKK is key
            tmpSave.iBytes = (unsigned char)cbuf[31]+0x100*(unsigned char)cbuf[30]+0x10000*(unsigned char)cbuf[29]+0x1000000*(unsigned char)cbuf[28];
            int iCompressionKey = (unsigned char)cbuf[35]+0x100*(unsigned char)cbuf[34];
            int iCompressedBytes = (unsigned char)cbuf[39]+0x100*(unsigned char)cbuf[38]+0x10000*(unsigned char)cbuf[37]+0x1000000*(unsigned char)cbuf[36];
            int iDecompressedSize = 0;
            //check if we have enough space at the end of file
            //making a brutal check for inserting size:
            //each ClusterSize-4 requires additional 6 bytes (2 for SAT, 4 for header)
            //plus 36 additional (34 header, 2 zero-sat-entry)
            //if that won't fit into remaining clusters, boil out
            int iClustersRequired = (tmpSave.iBytes)/(iClusterSize-4);
            int iBytesRequired = tmpSave.iBytes+iClustersRequired*6+36;
            iClustersRequired = iBytesRequired/(iClusterSize-4);
            if ((iFileSize/iClusterSize - iLastUsedCluster) <= iClustersRequired)
            {
                //no space left in image to decompress into
                QMessageBox msgBox;
                msgBox.setText(QObject::tr("Not enough space in image to insert ")+QString(tmpSave.Name)+QObject::tr(" and possibly some other saves. Please retry with a bigger image size."));
                msgBox.exec();
                return ImportOk;
            }
            tmpSave.iStartCluster = iLastUsedCluster+1;

            //check done, go on
            hugeram[0][tmpSave.iStartCluster*iClusterSize] = 0x80;
            hugeram[0][tmpSave.iStartCluster*iClusterSize+1] = 0x0;
            hugeram[0][tmpSave.iStartCluster*iClusterSize+2] = 0x0;
            hugeram[0][tmpSave.iStartCluster*iClusterSize+3] = 0x0;//making up counter
            hugeram->replace(tmpSave.iStartCluster*iClusterSize+4,11,QByteArray(&cbuf[0],11));//name
            hugeram[0][tmpSave.iStartCluster*iClusterSize+15] = cbuf[11];//lang code
            hugeram->replace(tmpSave.iStartCluster*iClusterSize+16,10,QByteArray(&cbuf[12],10));//comment
            hugeram->replace(tmpSave.iStartCluster*iClusterSize+26,4,QByteArray(&cbuf[24],4));//date
            hugeram->replace(tmpSave.iStartCluster*iClusterSize+30,4,QByteArray(&cbuf[28],4));//size
            //calculate sat
            int iSatSize = 1;
            tmpSave.Name = QByteArray(&cbuf[0],11);
            while ( (30 + iSatSize*2 + 2 + tmpSave.iBytes)/(iClusterSize-4) > iSatSize)
                iSatSize++;
            //fill new sat
            tmpSave.SAT.clear();
            for (int i=0;i<iSatSize;i++)
                tmpSave.SAT.append(tmpSave.iStartCluster+i+1);
            //add terminator
            tmpSave.SAT.append(0);
            //copy SAT to hugeram
            int iPointer = tmpSave.iStartCluster*iClusterSize + 34;
            for (int i=0;i<tmpSave.SAT.size();i++)
            {
                if (iPointer % iClusterSize == 0)
                {
                    hugeram[0][iPointer] = 0;
                    hugeram[0][iPointer+1] = 0;
                    hugeram[0][iPointer+2] = 0;
                    hugeram[0][iPointer+3] = (unsigned char)tmpSave.cCounter;
                    iPointer+=4;
                }
                hugeram[0][iPointer] = (char) ( tmpSave.SAT.at(i) / 0x100 );
                hugeram[0][iPointer+1] = (char) ( tmpSave.SAT.at(i) % 0x100 );
                iPointer+=2;
            }
            //copy data
            int ic=0;
            while (ic<iCompressedBytes)
            {
                if (iPointer % iClusterSize == 0)
                {
                    hugeram[0][iPointer] = 0;
                    hugeram[0][iPointer+1] = 0;
                    hugeram[0][iPointer+2] = 0;
                    hugeram[0][iPointer+3] = 0;//make up counter
                    iPointer+=4;
                }
                file_in->read(cbuf,2);
                ic+=2;
                if ((unsigned char)cbuf[1]+0x100*(unsigned char)cbuf[0] == iCompressionKey)
                {
                    //it's an RLE sequence!
                    file_in->read(cbuf,4);
                    ic+=4;
                    for (int j=0;j<((unsigned char)cbuf[3]+0x100*(unsigned char)cbuf[2]);j++)
                    {
                        hugeram[0][iPointer] = cbuf[0];
                        iPointer++;
                        hugeram[0][iPointer] = cbuf[1];
                        iPointer++;
                        iDecompressedSize+=2;
                        //if we catch cluster boundary while executing RLE sequence, insert sys header
                        if (iPointer % iClusterSize == 0)
                        {
                            hugeram[0][iPointer] = 0;
                            hugeram[0][iPointer+1] = 0;
                            hugeram[0][iPointer+2] = 0;
                            hugeram[0][iPointer+3] = 0;//make up counter
                            iPointer+=4;
                        }
                    }
                }
                else
                {
                    hugeram[0][iPointer] = cbuf[0];
                    iPointer++;
                    hugeram[0][iPointer] = cbuf[1]; //this might be wrong for odd-sized saves, but cluster end is unused anyway, so who cares?
                    iPointer++;
                    iDecompressedSize+=2;
                }
            }
            if (iDecompressedSize != tmpSave.iBytes)
            {
                //decompression result is different
                QMessageBox msgBox;
                msgBox.setText(QObject::tr("Decompression error for ")+QString(tmpSave.Name)+QObject::tr(". Size should be %2, but decompressed into %3").arg(tmpSave.iBytes).arg(iDecompressedSize));
                msgBox.exec();
            }
            int iPos = file_in->pos() % 256;
            for (int i=0;i<iPos;i++)
                cbuf_prev[i] = hugeram[0][iPointer-iPos+i];
            file_in->read(&cbuf_prev[iPos],256-iPos);
            iLastUsedCluster+=tmpSave.SAT.size();
        }
        else
        {
            //not a save, just some 256 bytes of whatever
            //we readed already 40 bytes, 216 to go
            file_in->read(&cbuf[40],216);
            for (int i=0;i<256;i++) cbuf_prev[i] = cbuf[i];
        }
    }
    return ImportOk;
}

ImportResult ImportEMS8Meg(QByteArray *hugeram, QFile * file_in, int iClusterSize)
{
    //same as MemCardPlus. probably.
    return ImportMemCardPlus(hugeram,file_in,iClusterSize);
}

ImportResult ImportActionReplay(QByteArray *hugeram, QFile * file_in)
{
    char cbuf[256];
    //AR actually holds the full cart image in RLE-compressed format
    //so we need to import it as-is, and then convert to user-provided
    //cluster size and image size
    //ok, off we go
    //int iFileSize = file_in->size();
    //parsing sequences
    int iHeadersEnd = 0x20000;
    file_in->seek(iHeadersEnd);
    //check if it's an uncompressed AR image
    file_in->read(cbuf,16);//get 16 bytes
    file_in->seek(iHeadersEnd);//return to start
    if (QByteArray(cbuf).startsWith("BackUpRam Format"))
    {
        //this is an uncompressed image, probably unpacked by Pseudo Saturn Kai, copy as-is
        QByteArray b = file_in->readAll();
        hugeram->replace(0,b.size(),b);
    }
    else
    {
        //compressed AR image
        bool bHeaderFound;
        int iPackedSize=-1;
        int iUnpackedSize=-1;
        unsigned char cActionReplayKey;
        unsigned char cActionReplayKey2;
        bool bKey2Used = false;
        do
        {
            bHeaderFound = false;
            file_in->read(cbuf,5);
            if (QByteArray(cbuf).startsWith("RLE01"))
            {
                //RLE01 header, parsing
                bHeaderFound = true;
                file_in->read(cbuf,1);
                cActionReplayKey = (unsigned char)cbuf[0];
                file_in->read(cbuf,4);
                iPackedSize = 0x1000000*(unsigned char)cbuf[0] + 0x10000*(unsigned char)cbuf[1] +
                        0x100*(unsigned char)cbuf[2] + (unsigned char)cbuf[3];
                iHeadersEnd+=10;//size of RLE01 header
            }
            else if (QByteArray(cbuf).startsWith("DEF02"))
            {
                //DEF02 header, parsing
                bHeaderFound = true;
                file_in->read(cbuf,1);
                cActionReplayKey2 = (unsigned char)cbuf[0];
                bKey2Used = true;
                file_in->read(cbuf,4);
                iUnpackedSize = 0x1000000*(unsigned char)cbuf[0] + 0x10000*(unsigned char)cbuf[1] +
                        0x100*(unsigned char)cbuf[2] + (unsigned char)cbuf[3];
                iHeadersEnd+=10;//size of RLE01 header
            }
        }while (bHeaderFound);

        file_in->seek(iHeadersEnd);

        //now brutally unpacking it as a full image
        int iPointer = 0;
        while (file_in->pos() < (0x20000+iPackedSize))
        {
            //AR format is compression-key-based
            //key is a 8-bit value stored at the card beginning
            //if it appears in uncompessed save, it is encoded as KK 00
            //RLE sequences are 24-bit:
            //0xKK 0xXX 0xYY - write XX copies of YY, KK is key
            file_in->read(cbuf,1);
            if ((unsigned char)cbuf[0] == cActionReplayKey)
            {
                //it's an RLE sequence! which one?
                file_in->read(&(cbuf[1]),1);
                if ((unsigned char)cbuf[1] == 0x00)
                {
                    //it's a KK 00 sequence, inserting single KK character
                    hugeram[0][iPointer] = cbuf[0];//inserting KK
                    iPointer++;
                }
                else
                {
                    //it's a KK XX YY sequence
                    file_in->read(&(cbuf[2]),1);
                    for (int j=0;j<((unsigned char)cbuf[1]);j++)
                    {
                        hugeram[0][iPointer] = cbuf[2];
                        iPointer++;
                    }
                }
            }
            else if (bKey2Used && ((unsigned char)cbuf[0] == cActionReplayKey2))
            {
                //it's a key2 RLE sequence
                file_in->read(&(cbuf[1]),1);
                if ((unsigned char)cbuf[1] == 0x00)
                {
                    //it's a KK 00 sequence, inserting single KK character
                    hugeram[0][iPointer] = cbuf[0];//inserting KK
                    iPointer++;
                }
                else
                {
                    //it's a KK XX YY DD sequence
                    //DD is an unknown field.
                    //DD could be a dummy data, redundacy data, checksum, or something else
                    //but the fact is DD is not needed for decompression
                    file_in->read(&(cbuf[2]),2);
                    for (int j=0;j<((unsigned char)cbuf[1]);j++)
                    {
                        hugeram[0][iPointer] = cbuf[2];
                        iPointer++;
                    }
                }
            }
            else
            {
                //it's not an RLE sequence, just a databyte
                hugeram[0][iPointer] = cbuf[0];//inserting databyte
                iPointer++;
            }
        }
    }

    //add header
    for (int i=0;i<4;i++)
        hugeram->replace(16*i,16,QByteArray("BackUpRam Format"));

    return ImportOk;

}
