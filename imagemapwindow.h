#ifndef IMAGEMAPWINDOW_H
#define IMAGEMAPWINDOW_H

#include <QDialog>
#include <QGraphicsScene>
#include "config.h"

class ImageMapWindow;

class MyLittleScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MyLittleScene(QWidget *parent = 0, ImageMapWindow * mappy = 0);

protected :
    void mousePressEvent(QGraphicsSceneMouseEvent * event);

private:
    ImageMapWindow * pMap;
};


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
    int iMapWidth;
    int iCurrentRow;
    QList<SaveType> *SavesList;
    Ui::ImageMapWindow *ui;

private slots:
    void on_listWidget_currentRowChanged(int currentRow);
    void blink();

private:
    Config * TheConfig;
    MyLittleScene * scene;
    int iMapHeight;
    int iBlink;
};

#endif // IMAGEMAPWINDOW_H
