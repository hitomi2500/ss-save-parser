#include <QPainter>
#include <QPicture>
#include <QDesktopWidget>
#include <QWidget>
#include <QTime>
#include "imagemapwindow.h"
#include "ui_imagemapwindow.h"
#include "math.h"

#define CELL_SIZE 10

ImageMapWindow::ImageMapWindow(QWidget *parent, QList<SaveType> *pSavesList, Config *pTheConfig) :
    QDialog(parent),
    ui(new Ui::ImageMapWindow)
{
    iCurrentRow=-1;
    SavesList = pSavesList;
    TheConfig = pTheConfig;
    ui->setupUi(this);
    QTimer tim();
}

ImageMapWindow::~ImageMapWindow()
{
    delete ui;
}

void ImageMapWindow::UpdateData()
{
    QListWidgetItem * item;
    ui->listWidget->clear();
    for (int i=0;i<SavesList->size();i++)
    {
        item = new QListWidgetItem(QString(SavesList->at(i).Name));
        ui->listWidget->addItem(item);
    }
    ui->listWidget->setCurrentRow(iCurrentRow);
    QDesktopWidget desky;
    QRect mainScreenSize = desky.screenGeometry(desky.primaryScreen());
    //we'll try to make our image look quad or 2:1 until we hit 2/3 of screen width limit
    //after that we just increase height
    //single block size is 5x5 pixel, with 1 pixel of black border, so it's 6x6
    int iHorizontalLimit = mainScreenSize.width() / ((CELL_SIZE*3)/2); // 1/3 screen reserve
    int iClustersNumber = TheConfig->m_iFileSize/TheConfig->m_iClusterSize;
    iMapWidth = sqrtf(iClustersNumber);
    if (iMapWidth > iHorizontalLimit) iMapWidth = iHorizontalLimit;
    iMapHeight = iClustersNumber/iMapWidth + 1;
    //clear
    scene.clear();
    //draw grid
    for (int i=0;i<=iMapWidth;i++)
        scene.addLine(QLine(i*CELL_SIZE,0,i*CELL_SIZE,iMapHeight*CELL_SIZE));
    for (int i=0;i<=iMapHeight;i++)
        scene.addLine(QLine(0,i*CELL_SIZE,iMapWidth*CELL_SIZE,i*CELL_SIZE));
    QColor color;
    //colour map
    for (int i=0;i<SavesList->size();i++)
    {
        //generating a new color
        color.setHsv(((800*i)/SavesList->size())%255,
                     255,
                     255-((800*i)/SavesList->size())/5);
        //colouring save content
        //first cluster
        int iX = SavesList->at(i).iStartCluster%iMapWidth;
        int iY = SavesList->at(i).iStartCluster/iMapWidth;
        scene.addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
        scene.addRect(QRect(iX*CELL_SIZE+3,iY*CELL_SIZE+3,CELL_SIZE-6,CELL_SIZE-6),QPen(Qt::white),QBrush(Qt::white,Qt::SolidPattern));
        //other clusters
        for (int j=0;j<SavesList->at(i).iSATSize;j++)
        {
            iX = SavesList->at(i).SAT[j]%iMapWidth;
            iY = SavesList->at(i).SAT[j]/iMapWidth;
            scene.addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
        }
        //list
        ui->listWidget->item(i)->setBackground(QBrush(color,Qt::SolidPattern));
        
    }
    //draw border for selection
    /*if (iCurrentRow >= 0)
    {
        int iX = SavesList->at(iCurrentRow).iStartCluster%iMapWidth;
        int iY = SavesList->at(iCurrentRow).iStartCluster/iMapWidth;
        scene.addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-2,iY*CELL_SIZE+CELL_SIZE-2),
                          QPen(Qt::black),QBrush(Qt::black,Qt::SolidPattern));
    }*/

    scene.setSceneRect(0,0,iMapWidth*CELL_SIZE,iMapHeight*CELL_SIZE);
    ui->graphicsView->setScene(&scene);
    //ui->graphicsView->setCacheMode(QGraphicsView::CacheNone);
    this->resize(mainScreenSize.width()-100,mainScreenSize.height()-100);//set size to full screen
    if ((iMapHeight * ((CELL_SIZE*3)/2) ) < mainScreenSize.height())
        this->resize(iMapWidth*CELL_SIZE + ui->listWidget->width() + 50,iMapHeight*CELL_SIZE + 50);
    else
        this->resize(iMapWidth*CELL_SIZE + ui->listWidget->width() + 50,mainScreenSize.height()-100);
    this->setGeometry((mainScreenSize.width()-this->width())/2,
                      (mainScreenSize.height()-this->height())/2,
                      this->width(),
                      this->height());
}

void ImageMapWindow::UpdateData(int iIndex)
{
    QColor color;
    //sanity check
    if (iIndex < 0 ) return;
    if (SavesList->size() < iIndex) return;
    //generating a new color
    color.setHsv(((800*iIndex)/SavesList->size())%255,
                   255,
                   255-((800*iIndex)/SavesList->size())/5);
    //colouring save content
    //first cluster
    int iX = SavesList->at(iIndex).iStartCluster%iMapWidth;
    int iY = SavesList->at(iIndex).iStartCluster/iMapWidth;
    scene.addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
    scene.addRect(QRect(iX*CELL_SIZE+3,iY*CELL_SIZE+3,CELL_SIZE-6,CELL_SIZE-6),QPen(Qt::white),QBrush(Qt::white,Qt::SolidPattern));
    //other clusters
    for (int j=0;j<SavesList->at(iIndex).iSATSize;j++)
    {
        iX = SavesList->at(iIndex).SAT[j]%iMapWidth;
        iY = SavesList->at(iIndex).SAT[j]/iMapWidth;
        scene.addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
    }
    //list
    ui->listWidget->item(iIndex)->setBackground(QBrush(color,Qt::SolidPattern));

    //draw border for selection
    if (iIndex == iCurrentRow)
    {
        iX = SavesList->at(iCurrentRow).iStartCluster%iMapWidth;
        iY = SavesList->at(iCurrentRow).iStartCluster/iMapWidth;
        scene.addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1),QPen(Qt::black));
        scene.addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
        scene.addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
        scene.addLine(QLine(iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
        for (int j=0;j<SavesList->at(iIndex).iSATSize;j++)
        {
            iX = SavesList->at(iIndex).SAT[j]%iMapWidth;
            iY = SavesList->at(iIndex).SAT[j]/iMapWidth;
            scene.addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1),QPen(Qt::black));
            scene.addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
            scene.addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
            scene.addLine(QLine(iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
        }
    }
    //ui->graphicsView->setScene(&scene);
}

void ImageMapWindow::on_listWidget_currentRowChanged(int currentRow)
{
    int iPrevRow = iCurrentRow;
    iCurrentRow = currentRow;
    UpdateData(iPrevRow);
    UpdateData(iCurrentRow);
}


