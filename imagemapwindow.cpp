#include <QPainter>
#include <QPicture>
#include <QDesktopWidget>
#include <QWidget>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include "imagemapwindow.h"
#include "ui_imagemapwindow.h"
#include "math.h"

#define CELL_SIZE 10

MyLittleScene::MyLittleScene(QWidget *parent, ImageMapWindow * mappy) :
    QGraphicsScene(parent)
{
    pMap = mappy;
}

void MyLittleScene::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    int iX = event->scenePos().x();
    int iY = event->scenePos().y();
    //getting cluster number out if ix,iy
    int iClusterNumber = (iY/CELL_SIZE)*pMap->iMapWidth + (iX/CELL_SIZE);
    //search for hits in the current list
    int iFoundRow = -1;
    for (int i=0; i<pMap->SavesList->size(); i++)
    {
        if (pMap->SavesList->at(i).iStartCluster == iClusterNumber)
            iFoundRow = i;
        for (int j=0;j<pMap->SavesList->at(i).SAT.size()-1;j++)
        {
            if (pMap->SavesList->at(i).SAT.at(j)==iClusterNumber)
                iFoundRow = i;
        }
    }

    int iPrevRow = pMap->iCurrentRow;
    pMap->iCurrentRow = iFoundRow;
    pMap->ui->listWidget->setCurrentRow(iFoundRow);
    pMap->UpdateData(iPrevRow);
    pMap->UpdateData(pMap->iCurrentRow);
    return;
}

ImageMapWindow::ImageMapWindow(QWidget *parent, QList<SaveType> *pSavesList, Config *pTheConfig) :
    QDialog(parent),
    ui(new Ui::ImageMapWindow)
{
    iCurrentRow=-1;
    SavesList = pSavesList;
    TheConfig = pTheConfig;
    scene = new MyLittleScene(this,this);
    ui->setupUi(this);
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(blink()));
    timer->start(120);
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
    scene->clear();
    //draw grid
    for (int i=0;i<=iMapWidth;i++)
        scene->addLine(QLine(i*CELL_SIZE,0,i*CELL_SIZE,iMapHeight*CELL_SIZE));
    for (int i=0;i<=iMapHeight;i++)
        scene->addLine(QLine(0,i*CELL_SIZE,iMapWidth*CELL_SIZE,i*CELL_SIZE));
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
        scene->addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
        scene->addRect(QRect(iX*CELL_SIZE+3,iY*CELL_SIZE+3,CELL_SIZE-6,CELL_SIZE-6),QPen(Qt::white),QBrush(Qt::white,Qt::SolidPattern));
        //other clusters
        for (int j=0;j<SavesList->at(i).SAT.size()-1;j++)
        {
            iX = SavesList->at(i).SAT.at(j)%iMapWidth;
            iY = SavesList->at(i).SAT.at(j)/iMapWidth;
            scene->addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
        }
        //list
        //ui->listWidget->item(i)->setBackground(QBrush(color,Qt::SolidPattern));
        QImage img(QSize(10,10),QImage::Format_RGB32);
        img.fill(color);
        ui->listWidget->item(i)->setIcon(QIcon(QPixmap::fromImage(img)));
    }

    scene->setSceneRect(0,0,iMapWidth*CELL_SIZE,iMapHeight*CELL_SIZE);
    ui->graphicsView->setScene(scene);
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
    scene->addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
    scene->addRect(QRect(iX*CELL_SIZE+3,iY*CELL_SIZE+3,CELL_SIZE-6,CELL_SIZE-6),QPen(Qt::white),QBrush(Qt::white,Qt::SolidPattern));
    //other clusters
    for (int j=0;j<SavesList->at(iIndex).SAT.size()-1;j++)
    {
        iX = SavesList->at(iIndex).SAT.at(j)%iMapWidth;
        iY = SavesList->at(iIndex).SAT.at(j)/iMapWidth;
        scene->addRect(QRect(iX*CELL_SIZE+1,iY*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2),QPen(color),QBrush(color,Qt::SolidPattern));
    }
    //list
    //ui->listWidget->item(iIndex)->setBackground(QBrush(color,Qt::SolidPattern));
    QImage img(QSize(10,10),QImage::Format_RGB32);
    img.fill(color);
    ui->listWidget->item(iIndex)->setIcon(QIcon(QPixmap::fromImage(img)));
}

void ImageMapWindow::on_listWidget_currentRowChanged(int currentRow)
{
    int iPrevRow = iCurrentRow;
    iCurrentRow = currentRow;
    UpdateData(iPrevRow);
    UpdateData(iCurrentRow);
}

void ImageMapWindow::blink()
{
    int iX,iY;
    if (iBlink==0)
    {
        iBlink = 1;
        if (iCurrentRow>=0)
        {
            iX = SavesList->at(iCurrentRow).iStartCluster%iMapWidth;
            iY = SavesList->at(iCurrentRow).iStartCluster/iMapWidth;
            scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1),QPen(Qt::white));
            scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::white));
            scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::white));
            scene->addLine(QLine(iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::white));
            for (int j=0;j<SavesList->at(iCurrentRow).SAT.size()-1;j++)
            {
                iX = SavesList->at(iCurrentRow).SAT.at(j)%iMapWidth;
                iY = SavesList->at(iCurrentRow).SAT.at(j)/iMapWidth;
                scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1),QPen(Qt::white));
                scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::white));
                scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::white));
                scene->addLine(QLine(iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::white));
            }
        }
    }
    else
    {
        iBlink = 0;
        if (iCurrentRow>=0)
        {
            iX = SavesList->at(iCurrentRow).iStartCluster%iMapWidth;
            iY = SavesList->at(iCurrentRow).iStartCluster/iMapWidth;
            scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1),QPen(Qt::black));
            scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
            scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
            scene->addLine(QLine(iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
            for (int j=0;j<SavesList->at(iCurrentRow).SAT.size()-1;j++)
            {
                iX = SavesList->at(iCurrentRow).SAT.at(j)%iMapWidth;
                iY = SavesList->at(iCurrentRow).SAT.at(j)/iMapWidth;
                scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1),QPen(Qt::black));
                scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
                scene->addLine(QLine(iX*CELL_SIZE+1,iY*CELL_SIZE+1,iX*CELL_SIZE+1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
                scene->addLine(QLine(iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+1,iX*CELL_SIZE+CELL_SIZE-1,iY*CELL_SIZE+CELL_SIZE-1),QPen(Qt::black));
            }
        }
    }
}
