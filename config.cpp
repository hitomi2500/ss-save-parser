#include "config.h"

Config::Config()
{
    TheSettings = new QSettings("Null Record Studio","Sega Saturn Backup RAM Parser");
}

Config::~Config()
{
    delete TheSettings;
}

void Config::UpdateFlags()
{
    switch (m_ExtractMode)
    {
    case ExtractSSF:
        m_bExtractDateTime=true;
        m_bExtractDescription=true;
        m_bExtractLanguage=false;
        m_bExtractName=false;
        m_bExtractSAT=false;
        m_bExtractSize=false;
        m_bExtractSys=false;
        m_bExtractSysAll=false;
        m_bExtractSysFillZero=false;
        break;
    case ExtractAR:
        m_bExtractDateTime=true;
        m_bExtractDescription=true;
        m_bExtractLanguage=true;
        m_bExtractName=true;
        m_bExtractSAT=true;
        m_bExtractSize=true;
        m_bExtractSys=true;
        m_bExtractSysAll=true;
        m_bExtractSysFillZero=true;
        break;
    case ExtractFull:
        m_bExtractDateTime=true;
        m_bExtractDescription=true;
        m_bExtractLanguage=true;
        m_bExtractName=true;
        m_bExtractSAT=true;
        m_bExtractSize=true;
        m_bExtractSys=true;
        m_bExtractSysAll=true;
        m_bExtractSysFillZero=false;
        break;
    case ExtractRaw:
        m_bExtractDateTime=false;
        m_bExtractDescription=false;
        m_bExtractLanguage=false;
        m_bExtractName=false;
        m_bExtractSAT=false;
        m_bExtractSize=false;
        m_bExtractSys=false;
        m_bExtractSysAll=false;
        m_bExtractSysFillZero=false;
        break;
    case ExtractDruidII:
        m_bExtractDateTime=true;
        m_bExtractDescription=true;
        m_bExtractLanguage=true;
        m_bExtractName=true;
        m_bExtractSAT=false;
        m_bExtractSize=true;
        m_bExtractSys=false;
        m_bExtractSysAll=false;
        m_bExtractSysFillZero=false;
        break;
    case ExtractManual:
        break;
    }
    switch (m_InsertMode)
    {
    case InsertSSF:
        m_bInsertDateTime=true;
        m_bInsertDescription=true;
        m_bInsertLanguage=false;
        m_bInsertName=false;
        m_bInsertSAT=false;
        m_bInsertSize=false;
        m_bInsertSys=false;
        m_bInsertSysAll=false;
        m_bInsertSysUseCounter=false;
        break;
    case InsertAR:
        m_bInsertDateTime=true;
        m_bInsertDescription=true;
        m_bInsertLanguage=true;
        m_bInsertName=true;
        m_bInsertSAT=true;
        m_bInsertSize=true;
        m_bInsertSys=true;
        m_bInsertSysAll=true;
        m_bInsertSysUseCounter=false;
        break;
    case InsertFull:
        m_bInsertDateTime=true;
        m_bInsertDescription=true;
        m_bInsertLanguage=true;
        m_bInsertName=true;
        m_bInsertSAT=true;
        m_bInsertSize=true;
        m_bInsertSys=true;
        m_bInsertSysAll=true;
        m_bInsertSysUseCounter=true;
        break;
    case InsertRaw:
        m_bInsertDateTime=false;
        m_bInsertDescription=false;
        m_bInsertLanguage=false;
        m_bInsertName=false;
        m_bInsertSAT=false;
        m_bInsertSize=false;
        m_bInsertSys=false;
        m_bInsertSysAll=false;
        m_bInsertSysUseCounter=false;
        break;
    case InsertDruidII:
        m_bInsertDateTime=true;
        m_bInsertDescription=true;
        m_bInsertLanguage=true;
        m_bInsertName=true;
        m_bInsertSAT=false;
        m_bInsertSize=true;
        m_bInsertSys=false;
        m_bInsertSysAll=false;
        m_bInsertSysUseCounter=false;
        break;
    case InsertManual:
        break;
    }
}

void Config::SaveToRegistry()
{
    UpdateFlags();
    TheSettings->setValue("Extract Mode",(int)m_ExtractMode);
    TheSettings->setValue("Extract Sys",m_bExtractSys);
    TheSettings->setValue("Extract Sys All",m_bExtractSysAll);
    TheSettings->setValue("Extract Sys Fill Zero",m_bExtractSysFillZero);
    TheSettings->setValue("Extract Name",m_bExtractName);
    TheSettings->setValue("Extract Description",m_bExtractDescription);
    TheSettings->setValue("Extract DateTime",m_bExtractDateTime);
    TheSettings->setValue("Extract Size",m_bExtractSize);
    TheSettings->setValue("Extract SAT",m_bExtractSAT);
    TheSettings->setValue("Extract Language",m_bExtractLanguage);
    TheSettings->setValue("Insert Mode",(int)m_InsertMode);
    TheSettings->setValue("Insert Sys",m_bInsertSys);
    TheSettings->setValue("Insert Sys All",m_bInsertSysAll);
    TheSettings->setValue("Insert Sys Use Counter",m_bInsertSysUseCounter);
    TheSettings->setValue("Insert Name",m_bInsertName);
    TheSettings->setValue("Insert Description",m_bInsertDescription);
    TheSettings->setValue("Insert DateTime",m_bInsertDateTime);
    TheSettings->setValue("Insert Size",m_bInsertSize);
    TheSettings->setValue("Insert SAT",m_bInsertSAT);
    TheSettings->setValue("Insert Language",m_bInsertLanguage);
    TheSettings->setValue("Delete Mode",(int)m_DeleteMode);
    TheSettings->setValue("Show Hex Values",m_bShowHexValues);
    TheSettings->setValue("Ask For Format At Every Extract",m_bAskFormatAtEveryExtract);
    TheSettings->setValue("Ask For Format At Every Insert",m_bAskFormatAtEveryInsert);
    TheSettings->sync();
}

void Config::LoadFromRegistry()
{
    m_ExtractMode = (ExtractMode)TheSettings->value("Extract Mode").toInt();
    m_bExtractSys = TheSettings->value("Extract Sys").toBool();
    m_bExtractSysAll = TheSettings->value("Extract Sys All").toBool();
    m_bExtractSysFillZero = TheSettings->value("Extract Sys Fill Zero").toBool();
    m_bExtractName = TheSettings->value("Extract Name").toBool();
    m_bExtractDescription = TheSettings->value("Extract Description").toBool();
    m_bExtractDateTime = TheSettings->value("Extract DateTime").toBool();
    m_bExtractSize = TheSettings->value("Extract Size").toBool();
    m_bExtractSAT = TheSettings->value("Extract SAT").toBool();
    m_bExtractLanguage = TheSettings->value("Extract Language").toBool();
    m_InsertMode = (InsertMode)TheSettings->value("Insert Mode").toInt();
    m_bInsertSys = (bool)TheSettings->value("Insert Sys").toBool();
    m_bInsertSysAll = (bool)TheSettings->value("Insert Sys All").toBool();
    m_bInsertSysUseCounter = (bool)TheSettings->value("Insert Sys Use Counter").toBool();
    m_bInsertName = (bool)TheSettings->value("Insert Name").toBool();
    m_bInsertDescription =  (bool)TheSettings->value("Insert Description").toBool();
    m_bInsertDateTime = (bool)TheSettings->value("Insert DateTime").toBool();
    m_bInsertSize = (bool)TheSettings->value("Insert Size").toBool();
    m_bInsertSAT = (bool)TheSettings->value("Insert SAT").toBool();
    m_bInsertLanguage = (bool)TheSettings->value("Insert Language").toBool();
    m_DeleteMode = (DeleteMode)TheSettings->value("Delete Mode").toInt();
    m_bShowHexValues = (bool)TheSettings->value("Show Hex Values").toBool();
    m_bAskFormatAtEveryExtract = (bool)TheSettings->value("Ask For Format At Every Extract").toBool();
    m_bAskFormatAtEveryInsert = (bool)TheSettings->value("Ask For Format At Every Insert").toBool();
    UpdateFlags();
}
