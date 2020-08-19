#ifndef IMPORT_H
#define IMPORT_H

#include "mainwindow.h"

enum ProbeResult {ProbeOk,ProbeFailed};
enum ImportResult {ImportOk,ImportFailed};

bool ProbeMemCardPlus(QFile * file_in);
bool ProbeEMS8Meg(QFile * file_in);
bool ProbeActionReplay(QFile * file_in);

ImportResult ImportMemCardPlus(QByteArray *hugeram, QFile * file_in, int iClusterSize);
ImportResult ImportEMS8Meg(QByteArray *hugeram, QFile * file_in, int iClusterSize);
ImportResult ImportActionReplay(QByteArray *hugeram, QFile * file_in);

#endif // IMPORT_H

