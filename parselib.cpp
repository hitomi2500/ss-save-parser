#include <QByteArray>
#include <mainwindow.h>
#include <parselib.h>
#include <math.h>

QDateTime GetDateTimeFromRaw4Byte(QByteArray buf)
{
    QDateTime d;
    qint64 tim = 0;
    if (buf.size() < 4) return QDateTime::fromMSecsSinceEpoch(tim,Qt::UTC); ;
    d.setDate(QDate(1980,1,1));
    d.setTime(QTime(0,0,0,0));
    tim = d.toMSecsSinceEpoch();
    tim += (qint64)60000*((unsigned char)(buf.at(0))*0x1000000
                  + (unsigned char)(buf.at(1))*0x10000
                  + (unsigned char)(buf.at(2))*0x100
                  + (unsigned char)(buf.at(3)));
    return QDateTime::fromMSecsSinceEpoch(tim,Qt::UTC);
}

QByteArray GetRaw4ByteFromDateTime(QDateTime dt)
{
    char buf[4];
    qint64 tim = 0;
    QDateTime d;
    d.setDate(QDate(1980,1,1));
    d.setTime(QTime(0,0,0,0));
    tim = (dt.toMSecsSinceEpoch() - d.toMSecsSinceEpoch() )/60000  + dt.offsetFromUtc()/60 ;
    buf[0] = tim/0x1000000;
    buf[1] = (tim%0x1000000)/0x10000;
    buf[2] = (tim%0x10000)/0x100;
    buf[3] = tim % 0x100;
    return QByteArray(buf,4);
}

QDateTime GetDateTimeFromSSF4Byte(QByteArray buf)
{
    QDateTime d;
    qint64 tim = 0;
    if (buf.size() < 4) return QDateTime::fromMSecsSinceEpoch(tim,Qt::UTC); ;
    d.setDate(QDate(1980,1,1));
    d.setTime(QTime(0,0,0,0));
    d=d.addYears(((unsigned char)buf.at(3))*2);
    d=d.addYears((((unsigned char)buf.at(2))&0x80)/0x80);
    d=d.addMonths(((((unsigned char)buf.at(2))&0x78)/0x8)-1);
    d=d.addDays((((unsigned char)buf.at(2))&0x7)*4);
    d=d.addDays(((((unsigned char)buf.at(1))&0xC0)/0x40)-1);
    d=d.addSecs(((((unsigned char)buf.at(1))&0x3E)/0x2)*60*60);//hours
    d=d.addSecs(((((unsigned char)buf.at(1))&0x01)*32)*60);//minutes
    d=d.addSecs(((((unsigned char)buf.at(0))&0xF8)/0x08)*60);//minutes
    return d;
}

QByteArray GetSSF4ByteFromDateTime(QDateTime dt)
{
    char buf[4];
    //qint64 tim = 0;
    QDateTime d;
    d.setDate(QDate(1980,1,1));
    d.setTime(QTime(0,0,0,0));
    //tim = (dt.toMSecsSinceEpoch() - d.toMSecsSinceEpoch() )/60000  + dt.offsetFromUtc()/60 ;
    buf[3] = (dt.date().year()-1980)/2;
    buf[2] = 0x80*((dt.date().year()-1980)%2);
    buf[2] |= 0x08*(dt.date().month()+1);
    buf[2] |= ((dt.date().day()+1)/4);
    buf[1] |= 0x40*((dt.date().day()+1)%4);
    buf[1] |= 0x02*(dt.time().hour());
    buf[1] |= (dt.time().minute()/32);
    buf[0] |= 0x08*(dt.time().minute()%32);
    return QByteArray(buf,4);
}


ParseResult ParseHeader(QByteArray buf, SaveType *save)
{
if (buf.size() < 34 ) return DataTooShort;
if (buf.at(0)!=(char)0x80) return NotAHeader;
save->cCounter = (unsigned char)buf.at(3);
save->Name = buf.mid(4,11);
save->cLanguageCode = (unsigned char)buf.at(15);
save->Comment = buf.mid(16,10);
save->DateTimeRaw = buf.mid(26,4);
/*save->DateTime.setDate(QDate(1980,1,1));
save->DateTime.setTime(QTime(0,0,0,0));
qint64 tim = save->DateTime.toMSecsSinceEpoch();
tim += (qint64)60000*((unsigned char)(buf.at(26))*0x1000000
              + (unsigned char)(buf.at(27))*0x10000
              + (unsigned char)(buf.at(28))*0x100
              + (unsigned char)(buf.at(29)));
save->DateTime = QDateTime::fromMSecsSinceEpoch(tim,Qt::UTC);
*/
save->DateTime = GetDateTimeFromRaw4Byte(save->DateTimeRaw);

save->iBytes = (unsigned char)(buf.at(30))*0x1000000
            + (unsigned char)(buf.at(31))*0x10000
            + (unsigned char)(buf.at(32))*0x100
            + (unsigned char)(buf.at(33));
//exact formula for blocks is yet to be found
//but since blocks are in-bios unit only, maybe it doesn't matter
//save->iBlocks = save->iBytes/64+1;
//if (save->iBytes%64>32) save->iBlocks++;
//using formula from Sega Saturn Technical Bulletin #9 for now
save->iBlocks = ceil((32.0+save->iBytes)/64.0);

//some sanity checks
if (save->iBytes > 1024*1024) return NotAHeader; //is there a save bigger than 1M?
if (save->iBytes <= 0) return NotAHeader; //now that's too negative
if (save->DateTime.date().year() > 2100) return NotAHeader; //someone playing Saturn in 22th century? I love you!
if (save->DateTime.date().year() < 1990) return NotAHeader; //now that's too negative
return ParseOk;
}

ParseResult ParseSAT(QByteArray * hugeram, SaveType *save, int iClusterSize)
{
    //okay, now SAT stuff
    //calculating SAT size
    int iCurrentCluster = save->iStartCluster;
    int iCurrentClusterIndex = 0;
    int iCurrentPointer = 34;
    short sCurrentEntry = 1; //any nonzero will do
    save->SAT.clear();
    while (sCurrentEntry)
    {
        if (iCurrentPointer >= iClusterSize)//we hit end of cluster
        {
            iCurrentPointer = 4;//skipping header of the next cluster
            iCurrentClusterIndex++;
            iCurrentCluster = save->SAT.at(iCurrentClusterIndex-1);
        }
        sCurrentEntry =  (unsigned char)(hugeram->at(iCurrentCluster*iClusterSize+iCurrentPointer))*0x100
                       + (unsigned char)(hugeram->at(iCurrentCluster*iClusterSize+iCurrentPointer+1));
        save->SAT.append(sCurrentEntry);
        iCurrentPointer+=2;
    }
    return ParseOk;
}
