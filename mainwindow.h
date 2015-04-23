#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QTableWidget>
#include "setupwindow.h"
#include "config.h"

enum FileIOMode {SH2_IO_MODE,RAW_IO_MODE};
enum FileIOClusterSize {CLUSTER_512,CLUSTER_256,CLUSTER_64,CLUSTER_CUSTOM};
enum SortDiretion {SORT_ASCENDING,SORT_DESCENDING,SORT_NONE};

typedef struct MyIOSettings{
    FileIOMode IOMode;
    FileIOClusterSize IOClusterSize;
    int iIOCustomClusterSize;
    bool bIOModeChangeable;
    bool bClusterSizeChangeable;
} FileIOSettingsType;

typedef struct MyNewSettings{
    FileIOClusterSize IOClusterSize;
    int iIOCustomClusterSize;
    int iImageSize;
} NewSettingsType;

typedef struct MySave{
    QByteArray Name;
    QByteArray Comment;
    unsigned char cLanguageCode;
    QDateTime DateTime;
    QByteArray DateTimeRaw;
    int iBytes;
    int iBlocks;
    short SAT[2048];
    int iSATSize;
    int iStartCluster;
    unsigned char cCounter;
} SaveType;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_Sort_Order_Changed(int logicalIndex);

    void on_Setup_Accepted();

    void on_SetupButton_clicked();

    void on_RepackButton_clicked();

    void on_InsertButton_clicked();

    void on_ExtractButton_clicked();

    void on_DeleteButton_clicked();

    void on_LoadButton_clicked();

    void on_SaveButton_clicked();

    void on_NewButton_clicked();

private:
    Ui::MainWindow *ui;
    SetupWindow *SetupWin;
    Config *TheConfig;
    FileIOSettingsType IOSettings;
    NewSettingsType NewSettings;
    void ParseHugeRAM();

};

#endif // MAINWINDOW_H
