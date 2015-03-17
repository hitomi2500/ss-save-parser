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

ParseResult ParseHeader(QByteArray buf, SaveType *save)
{
if (buf.size() < 34 ) return DataTooShort;
if (buf.at(0)!=(char)0x80) return NotAHeader;
save->cCounter = buf.at(3);
save->Name = buf.mid(4,11);
save->iLanguageCode = buf.at(15);
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

return ParseOk;
}

ParseResult ParseSAT(QByteArray buf, SaveType *save, int ClusterSize)
{
    //okay, now SAT stuff
    //calculating SAT size
    int i=0;
    int i2=0;
    short s = 1;
    while (s)
    {
        if (0 == ((i*2+34)%ClusterSize)) i+=2;//jumping over cluster header
        s =  (unsigned char)(buf.at(i*2))*0x100 + (unsigned char)(buf.at(1+i*2));
        save->SAT[i2]=s;
        i++;
        i2++;
    }
    save->iSATSize = i2;
    return ParseOk;
}
