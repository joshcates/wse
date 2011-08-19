//---------------------------------------------------------------------------
//
// Copyright 2010 University of Utah.  All rights reserved
//
//---------------------------------------------------------------------------
#ifndef WIDGETS_H
#define WIDGETS_H

#include <QLabel>
#include <QListWidget>
#include <QGraphicsScene>
#include <QGraphicsView>

/** Extends the QListWidget to support drag-and-drop of filenames. */
class QImageList : public QListWidget
{
  Q_OBJECT
public:
  QImageList(QWidget *parent = NULL) : QListWidget(parent) {}
  ~QImageList() {}

protected:
  QStringList mimeTypes() const;
  Qt::DropActions supportedDropActions();
  bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
  
signals:
  void imageDropped(QString);
};


/** */
class QPreviewScene : public QGraphicsScene
{
  Q_OBJECT
public:
  QPreviewScene(QWidget *parent = 0);
  ~QPreviewScene() {}

  void setBackground(const QImage &bg);

protected:
  void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
  void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
  void dropEvent(QGraphicsSceneDragDropEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *e);

signals:
  void imageDropped(QString);
  void imagePicked(int,int);

private:
  QGraphicsPixmapItem *mBackground;
  QGraphicsSimpleTextItem *mText;

};

class QPreviewWidget : public QGraphicsView
{
  Q_OBJECT
public:
  QPreviewWidget(QWidget *parent = 0);
  ~QPreviewWidget() {}
  
  void resizeEvent(QResizeEvent *event);
  
  void setBackground(const QImage &bg);

private:
  QPreviewScene *mScene;
};

#endif
