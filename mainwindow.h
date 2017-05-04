#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <QTableWidget>
#include "setupwindow.h"
#include "imagemapwindow.h"
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
    void Sort_Order_Changed(int logicalIndex);

    void Setup_Accepted();

    void on_SetupButton_clicked();

    void on_RepackButton_clicked();

    void RepackImage(int iOldClusterSize, int iNewClusterSize);

    void on_InsertButton_clicked();

    void on_InsertXMLButton_clicked();

    void on_ExtractButton_clicked();

    void on_ExtractXMLButton_clicked();

    void on_DeleteButton_clicked();

    void on_LoadButton_clicked();

    void on_SaveButton_clicked();

    void on_NewButton_clicked();

    void on_ImageMapButton_clicked();

private:
    Ui::MainWindow *ui;
    SetupWindow *SetupWin;
    SetupWindow *SetupWinExtract;
    SetupWindow *SetupWinInsert;
    ImageMapWindow *ImageMapWin;
    Config *TheConfig;
    FileIOSettingsType IOSettings;
    NewSettingsType NewSettings;
    void ParseHugeRAM();

};

#endif // MAINWINDOW_H
