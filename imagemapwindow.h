#ifndef IMAGEMAPWINDOW_H
#define IMAGEMAPWINDOW_H

#include <QDialog>
#include <QGraphicsScene>
#include "config.h"

namespace Ui {
class ImageMapWindow;
}

class ImageMapWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ImageMapWindow(QWidget *parent = 0, QList<SaveType> *pSavesList = 0, Config *pTheConfig = 0);
    ~ImageMapWindow();
    void UpdateData();
    void UpdateData(int iIndex);

private slots:
    void on_listWidget_currentRowChanged(int currentRow);

private:
    Ui::ImageMapWindow *ui;
    QList<SaveType> *SavesList;
    Config * TheConfig;
    QGraphicsScene scene;
    int iCurrentRow;
    int iMapWidth=1;
    int iMapHeight=1;
};

#endif // IMAGEMAPWINDOW_H
