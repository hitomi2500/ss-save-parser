#ifndef PARSELIB_H
#define PARSELIB_H

#include "mainwindow.h"

enum ParseResult {ParseOk,NotAHeader,DataTooShort};

QDateTime GetDateTimeFromRaw4Byte(QByteArray buf);
QByteArray GetRaw4ByteFromDateTime(QDateTime dt);
QDateTime GetDateTimeFromSSF4Byte(QByteArray buf);
QByteArray GetSSF4ByteFromDateTime(QDateTime dt);

ParseResult ParseHeader(QByteArray buf, SaveType *save);
ParseResult ParseSAT(QByteArray buf, SaveType *save, int ClusterSize);

#endif // PARSELIB_H

