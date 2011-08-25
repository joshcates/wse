//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#include "wseWidgets.h"
#include <QtGui>
#include <QDebug>

namespace wse {
QStringList QImageList::mimeTypes() const
{
  QStringList strList;
  strList.append("text/uri-list");
  return strList;
}

Qt::DropActions QImageList::supportedDropActions()
{
  return Qt::CopyAction | Qt::MoveAction;
}

bool QImageList::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
  QList<QUrl> urlList = data->urls();
  for (int i = 0; i < urlList.size(); i++)
  {
    QString fname = urlList[i].toLocalFile();
    emit imageDropped(fname);
  }
  return true;
}

QPreviewScene::QPreviewScene(QWidget *parent) :
  QGraphicsScene(parent),
  mBackground(NULL)
{
  mBackground = new QGraphicsPixmapItem(0, this);
  mBackground->setOffset(0,0);
}

void QPreviewScene::setBackground(const QImage &bg) 
{
  mBackground->setPixmap(QPixmap::fromImage(bg));
  mBackground->show();
  //setBackgroundBrush(QColor(0,0,0));
  setSceneRect(QRectF(0,0,bg.width(),bg.height()));
}

void QPreviewScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
  //  const QMimeData *m = event->mimeData();
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();
}

void QPreviewScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
  // const QMimeData *m = event->mimeData();
  if (event->mimeData()->hasUrls())
    event->acceptProposedAction();
}

void QPreviewScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
  if (event->mimeData()->hasUrls())
  {
    QList<QUrl> urlList = event->mimeData()->urls();
    for (int i = 0; i < urlList.size(); i++)
    {
      QString fname = urlList[i].toLocalFile();
      emit imageDropped(fname);
    }
  }
  event->acceptProposedAction();
}

void QPreviewScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  QGraphicsScene::mousePressEvent(e);
  emit imagePicked(int(e->scenePos().x()), int(e->scenePos().y()));
}

QPreviewWidget::QPreviewWidget(QWidget *parent) :
  QGraphicsView(parent)
{
  mScene = new QPreviewScene(this);
  setScene(mScene);
  
  setRenderHint(QPainter::SmoothPixmapTransform, true);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  fitInView(0, 0, mScene->width(), mScene->height(), Qt::KeepAspectRatio);
  show();
}

void QPreviewWidget::resizeEvent(QResizeEvent *event)
{
  QGraphicsView::resizeEvent(event);
  fitInView(0,0,mScene->width(),mScene->height(), Qt::KeepAspectRatio);
}

void QPreviewWidget::setBackground(const QImage &bg) 
{
  mScene->setBackground(bg);
  fitInView(0,0,mScene->width(),mScene->height(), Qt::KeepAspectRatio);
}

} // end namespace wse
