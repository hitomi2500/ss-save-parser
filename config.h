#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>

//enum LoadMode {LoadAuto,Load1M,Load512K};
//enum SaveMode {SaveAsLoad,Save1M,Save512K};
//enum LoadClusterSize {ClusterAuto,Cluster256,Cluster512};
enum ExtractMode {ExtractSSF,ExtractAR,ExtractFull,ExtractRaw,ExtractManual};
enum InsertMode {InsertSSF,InsertAR,InsertFull,InsertRaw,InsertManual};
enum DeleteMode {DeleteSingleSys,DeleteAllSys,DeleteFull};

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
};

#endif // CONFIG_H
