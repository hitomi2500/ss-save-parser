#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QDateTime>

enum ExtractMode {ExtractYabaSanshiroJson,ExtractSSF,ExtractAR,ExtractFull,ExtractRaw,ExtractDruidII,ExtractManual};
enum InsertMode {InsertYabaSanshiroJson,InsertSSF,InsertAR,InsertFull,InsertRaw,InsertDruidII,InsertManual};
enum DeleteMode {DeleteSingleSys,DeleteAllSys,DeleteFull};

typedef struct MySave{
    QByteArray Name;
    QByteArray Comment;
    unsigned char cLanguageCode;
    QDateTime DateTime;
    QByteArray DateTimeRaw;
    int iBytes;
    int iBlocks;
    QList<short> SAT;
    //int iSATSize;
    int iStartCluster;
    unsigned char cCounter;
} SaveType;

class Config
{
public:
    Config();
    ~Config();
    QSettings *TheSettings;
    void SaveToRegistry();
    void LoadFromRegistry();
    void UpdateFlags();
//    LoadMode m_LoadMode;
//    SaveMode m_SaveMode;
//    LoadClusterSize m_LoadClusterSize;
    int m_iClusterSize;
    int m_iFileSize;
    ExtractMode m_ExtractMode;
    bool m_bExtractSys;
    bool m_bExtractSysAll;
    bool m_bExtractSysFillZero;
    bool m_bExtractName;
    bool m_bExtractLanguage;
    bool m_bExtractDescription;
    bool m_bExtractDateTime;
    bool m_bExtractSize;
    bool m_bExtractSAT;
    InsertMode m_InsertMode;
    bool m_bInsertSys;
    bool m_bInsertSysAll;
    bool m_bInsertSysUseCounter;
    bool m_bInsertName;
    bool m_bInsertLanguage;
    bool m_bInsertDescription;
    bool m_bInsertDateTime;
    bool m_bInsertSize;
    bool m_bInsertSAT;
    DeleteMode m_DeleteMode;
    bool m_bShowHexValues;
    bool m_bAskFormatAtEveryExtract;
    bool m_bAskFormatAtEveryInsert;
};

#endif // CONFIG_H
