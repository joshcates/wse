// NOTES:

// A better way to implement this may be to derive histogram and
// threshold bar from QGraphicsWidget to handle margins
// automatically. There are supposedly ways to do this using the new
// QGraphics View architecture which has analogous classes to QWidget,
// QLayout, etc, but I was not able to get the
// QGraphicsLayouts/Items/Views/Scene to function correctly, and so we
// have a more "flat" implementation. The helper class
// HistogramGraphicsWidget isn't really a widget, though the
// QGraphicsView-derived HistogramWidget does in fact behave like a
// widget, but it's job is only to house the scene and pass through
// event functions.

// The toggle histogram type button is in fact just a couple of
// pixmaps which are swapped out. It's amazing how difficult it is
// just to make a semi-transparent widget in Qt. One could imagine an
// interface such as widget->setOpacity(...) but no such thing
// exists. This page is a reference to using transparency to build a
// custom widget, but notice that it doesn't actually utilize any of
// the existing widget types:
// http://doc.trolltech.com/qq/qq16-fader.html. There are numerous
// requests for this type of thing, but none satisfactorily resolved.

#define LOAD_HISTOGRAM_TEST_DATA 1

// Qt includes
#include <QtGui>

// Core includes
//#include <Core/Utils/Log.h>
#define CORE_LOG_DEBUG(a)
#define CORE_LOG_ERROR(a)

#include <assert.h>
#include <sstream>
#include <iostream>
#include <algorithm>


//#include <wseCore/Math/MathFunctions.h>
namespace wseCore {
  template< class T >
  inline T Clamp( T value, T min, T max )
  {
    return value <= min ? min : value >= max ? max : value;
  }

  template< class T >
  inline T Abs( T value )
  {
    return ( value < T( 0 ) ) ? -value : value;
  }
}
//#include <wseCore/Math/MathFunctions.h>



// Interface includes
#include <histogramWidget.h>

namespace wse {

  //---------------------------------------------------------------------------
  // Static members
  //---------------------------------------------------------------------------
  const float HistogramScene::margin_ = 3.0f;
  const float HistogramScene::threshold_bar_height_pct_ = .10f;
  const float HistogramScene::threshold_bar_arrow_width_ = 20.0f;
  const float HistogramScene::threshold_bar_arrow_height_ = 10.0f;
  const QColor HistogramScene::inside_threshold_color_(0, 0, 255);
  const QColor HistogramScene::outside_threshold_color_(0, 255, 0);
  const float HistogramScene::mini_histogram_height_pct_ = .15f;
  const QColor HistogramScene::mini_window_color_(61, 245, 0);
  const QColor HistogramScene::mini_window_background_(61, 245, 0, 100);
  const float HistogramScene::mini_window_border_ = 2.6f;
  const float HistogramScene::mini_window_min_pct_ = 0.1f;
  const float HistogramScene::mini_window_scroll_rate_ = 0.0005f; // in percent/degree
  const float HistogramScene::mini_window_zoom_rate_ = 0.0010f;   // in percent/degree
  const float HistogramScene::overlay_fadein_time_ = .25f; // in seconds
  //  const int HistogramScene::display_value_hover_time_ = 550; // in milliseconds
  const int HistogramScene::display_value_hover_time_ = 50; // in milliseconds
  const QColor HistogramScene::threshold_arrow_color_(255, 255, 255);
  const QColor HistogramScene::threshold_arrow_offscreen_color_(150, 150, 255);

  const QColor HistogramGraphicsWidget::marker_color_(184, 46, 0);
  const QColor HistogramGraphicsWidget::background_color_(0, 61, 246, 50);
  const QColor HistogramGraphicsWidget::path_color_bottom_(50, 50, 50);
  const QColor HistogramGraphicsWidget::path_color_top_(150, 150, 150);
  const QColor HistogramGraphicsWidget::bar_color_bottom_(50, 50, 50);
  const QColor HistogramGraphicsWidget::bar_color_top_(150, 150, 150);
  const QColor HistogramGraphicsWidget::marker_bar_color_bottom_(150, 50, 50);
  const QColor HistogramGraphicsWidget::marker_bar_color_top_(250, 150, 150);

  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  HistogramGraphicsWidget::HistogramGraphicsWidget(HistogramScene *scene) :
      render_mode_(RM_CURVE_E),
      show_markers_(true),
      start_(0),
      showpct_(1),
      scene_(scene),
      rect_(new QGraphicsRectItem(0, scene)),
      path_(new QGraphicsPathItem(0, scene))
  {
    QLinearGradient grad(QPointF(0.0,0.0), QPointF(0.0,1.0));
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad.setColorAt(0, path_color_bottom_);
    grad.setColorAt(1, path_color_top_);

    path_->setBrush(QBrush(grad));
    path_->setPen(QPen(QColor(0,0,0), 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    path_->setZValue(0.0);

    rect_->setPen(QPen(QColor(0,0,0), 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    rect_->setZValue(0.0);
    QBrush brush(background_color_);
    rect_->setBrush(brush);


  }

  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  HistogramGraphicsWidget::~HistogramGraphicsWidget()
  {
    cleanupMarkers();
    cleanupBars();
  }

  void HistogramGraphicsWidget::setWindow(float start, float amt)
  {
    start_ = start;
    showpct_ = amt;
    //update();       // This may be "ideal" correct, but in practice always followed by setRect, which also updates.
  }

  void HistogramGraphicsWidget::update()
  {
    draw();
    rect_->update();
  }

  void HistogramGraphicsWidget::setRect(const QRectF &rect)
  {
    bounds_ = rect;
    path_->setPos(rect.x(), rect.y());
    rect_->setRect(rect);
    update();
  }

  void HistogramGraphicsWidget::draw()
  {
    if (!scene_) return;
    if (scene_->values_.size() < 1) return;

    int numBins = scene_->values_.size();
    int startBin = start_ * (float)numBins;

    float binStartPct = (float)startBin / (float)numBins;
    float shift = (start_ - binStartPct) * numBins;

    int endBin = wseCore::Clamp<float>(start_ + showpct_, 0.0f, 1.0f) * (float)(numBins);
    if (endBin >= numBins) {
      endBin = numBins-1;
    }
    float binEndPct = (float)endBin / (float)numBins;
    float rShift = (start_ + showpct_ - binEndPct) * numBins;
    if (rShift == 0) {
      endBin--;
    }

    int numToRender = endBin - startBin + 1;

    assert (numToRender <= scene_->values_.size());

    //assert((render_mode_ == RM_CURVE_E && numToRender > 1) || (render_mode_ == RM_BARS_E && numToRender > 0));

    // yScale (NOTE: uses entire histogram to keep the histogram scale the same for all window sizes).
    float yScale = bounds_.height() / (float)(*std::max_element(scene_->values_.begin(), scene_->values_.end()));


    std::stringstream ss;
    ss << "rendering histogram from bin " << startBin << " (" << start_ << ") to "
        << startBin + numToRender - 1 << " (" << showpct_ << ")";
    CORE_LOG_DEBUG(ss.str());

    // Clear old geometry.
    cleanupMarkers();
    cleanupBars();
    path_->hide();

    if (render_mode_ == RM_CURVE_E)
    {
      drawCurve(startBin, numToRender, yScale, shift);
    }
    else if (render_mode_ == RM_BARS_E)
    {
      drawBars(startBin, numToRender, yScale, shift);
    }
    else
    {
      assert(false);
    }


    // Draw markers
    if (show_markers_)
    {
      int margin = 1;
      int clipLeft = bounds_.x() + margin;
      int clipRight = bounds_.x()+bounds_.width() - margin;
      int clipWidth = clipRight - clipLeft;


      for (unsigned int i = 0; i < scene_->markers_.size(); i++)
      {
        float x_pct = (scene_->markers_[i] - scene_->min_) / (scene_->max_ - scene_->min_);

        float screenRatio = (x_pct - start_) / (showpct_);

        //float x = clipLeft - (start_ * clipWidth) + (x_pct * clipWidth * showpct_);

        float x = clipLeft + (screenRatio * clipWidth);

        if (x > clipLeft && x < clipRight) {

          QGraphicsLineItem *line = new QGraphicsLineItem(0, scene_);
          line->setLine(x, bounds_.y(),
                        x, bounds_.y() + bounds_.height());
          line->setPen(QPen(marker_color_, 1.5, Qt::DashLine, Qt::SquareCap, Qt::BevelJoin));
          line->setZValue(1.0);
          markers_.push_back(line);
        }
      }
    }

    // tmp: draw the center lines of each bin
//    if (show_markers_)
//    {
//      int margin = 1;
//      int clipLeft = bounds_.x() + margin;
//      int clipRight = bounds_.x()+bounds_.width() - margin;
//      int clipWidth = clipRight - clipLeft;


//      for (unsigned int i = 0; i < scene_->getNumBins(); i++)
//      {
//        float x_pct = scene_->getBinCenterValue(i) / (scene_->max_ - scene_->min_);

//        float screenRatio = (x_pct - start_) / (showpct_);

//        //float x = clipLeft - (start_ * clipWidth) + (x_pct * clipWidth * showpct_);

//        float x = clipLeft + (screenRatio * clipWidth);

//        QGraphicsLineItem *line = new QGraphicsLineItem(0, scene_);
//        line->setLine(x, bounds_.y(),
//                      x, bounds_.y() + bounds_.height());

//        static QColor qcolor (0, 0, 255);

//        line->setPen(QPen(qcolor, 1.5, Qt::DashLine, Qt::SquareCap, Qt::BevelJoin));
//        line->setZValue(1.0);
//        markers_.push_back(line);
//      }
//    }

  }

  void HistogramGraphicsWidget::drawCurve(int startBin, int numToRender, float yScale, float shift)
  {

    int margin = 1;
    int clipLeft = bounds_.x() + margin;
    int clipRight = bounds_.x()+bounds_.width() - margin;
    int clipWidth = clipRight - clipLeft;

    //    int numBins = scene_->values_.size();
    //    float binWidth = (clipWidth) / (float)(numBins) / showpct_;


    int endBin = startBin + numToRender;
    startBin--;
    endBin++;
    startBin = wseCore::Clamp<int>(startBin, 0, scene_->getNumBins()-1);
    endBin = wseCore::Clamp<int>(endBin, 0, scene_->getNumBins()-1);


    // Draw path
    QPainterPath path;
    //path.setFillRule(Qt::OddEvenFill);
    path.moveTo(0, bounds_.height());

    float x = 0;
    float y = 0;
    for (int i = startBin; i <= endBin; i++)
    {
      float x_pct = scene_->getBinCenterValue(i) / (scene_->max_ - scene_->min_);
      float screenRatio = (x_pct - start_) / (showpct_);
      x = clipLeft + (screenRatio * clipWidth);
      y = bounds_.height() - (float(scene_->values_[i])*yScale);
      path.lineTo(x, y);
//      std::cerr << "bin[" << i << "]: lineTo(" << x << ", " << y << "), value = " << scene_->values_[i] << "\n";
    }
    path.lineTo(bounds_.width(), y);
    path.lineTo(bounds_.width(), bounds_.height());

    // clip the path to the bounding rectangle.  I don't know why I had to shift by 2 here.
    QPainterPath clipPath;
    clipPath.moveTo(bounds_.x()-2,0);
    clipPath.lineTo(bounds_.x()-2,bounds_.height());
    clipPath.lineTo(clipRight-2,bounds_.height());
    clipPath.lineTo(clipRight-2,0);

    path = path.intersected(clipPath);

    path_->setPath(path);
    path_->show();
  }

  void HistogramGraphicsWidget::drawBars(int startBin, int numToRender, float yScale, float shift)
  {
    //    std::cerr << "drawBars(startBin = " << startBin << ", numToRender = " << numToRender << ", yScale = " << yScale << ")\n";

    int margin = 1;
    int clipLeft = bounds_.x() + margin;
    int clipRight = bounds_.x()+bounds_.width() - margin;
    int clipWidth = clipRight - clipLeft;

    int numBins = scene_->values_.size();
    float binWidth = (clipWidth) / (float)(numBins) / showpct_;


    //float binWidth = bounds_.width() / float(numToRender);

    QLinearGradient grad(QPointF(0.0,0.0), QPointF(0.0,1.0));
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad.setColorAt(0, bar_color_bottom_);
    grad.setColorAt(1, bar_color_top_);

    for (unsigned int i = startBin; i < (unsigned)(startBin + numToRender); i++)
    {
      assert (i < scene_->values_.size());
      QGraphicsRectItem *bar = new QGraphicsRectItem(0, scene_);
      qreal x = clipLeft + (i - startBin) * binWidth - (shift*binWidth);
      if (x < clipLeft) {
        x = clipLeft;
      }

      qreal width = binWidth;
      if (x + width > clipRight) {
        width = clipRight - x;
      }

      //      std::cerr << "bar " << i << ", x = " << x << ", width = " << width << "\n";

      bar->setRect(x,
                   bounds_.y() + bounds_.height() - (float)(scene_->values_[i]) * yScale,
                   width, (float)(scene_->values_[i]) * yScale);
      bar->setBrush(QBrush(grad));
      bar->setZValue(1.0);
      bars_.push_back(bar);
    }
  }

  void HistogramGraphicsWidget::cleanupMarkers()
  {
    for (unsigned int i = 0; i < markers_.size(); i++)
    {
      scene_->removeItem(markers_[i]);
      delete markers_[i];
    }
    markers_.clear();
  }

  void HistogramGraphicsWidget::cleanupBars()
  {
    for (unsigned int i = 0; i < bars_.size(); i++)
    {
      scene_->removeItem(bars_[i]);
      delete bars_[i];
    }
    bars_.clear();
  }

  float HistogramGraphicsWidget::to_histogram_pct(float v) const
  {
    float pct = wseCore::Clamp<float>((v - bounds_.x()) / bounds_.width(), 0.0f, 1.0f);
    //    std::cerr << "pct = " << pct << ", start_ = " << start_ << ", showpct_ = " << showpct_ << "\n";
    pct = start_ + pct * showpct_;
    //    std::cerr << "to_histogram_pct returning " << pct << "\n";
    return pct;
  }

  bool HistogramGraphicsWidget::hit(const QPointF &p) const
  {
    // adjust the rectangle a bit to make it easier to click inside
    QRectF rect = rect_->rect();
    rect.adjust(-3,0,3,0);
    return rect.contains(p);
  }




  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  HistogramScene::HistogramScene(QWidget *parent) :
      QGraphicsScene(parent),
      threshold_lower_(0.25f),
      threshold_upper_(1.0f),
      dragging_lower_threshold_arrow_(false),
      dragging_upper_threshold_arrow_(false),
      resizing_window_left_(false),
      resizing_window_right_(false),
      moving_window_(false),
      prev_pos_(0),
      show_overlay_(false),
      overlay_alpha_(0.0f),
      show_bars_btn_(new QGraphicsPixmapItem(QPixmap(QString::fromUtf8(":/CORVIZ/Resources/bars.png")))),
      show_curve_btn_(new QGraphicsPixmapItem(QPixmap(QString::fromUtf8(":/CORVIZ/Resources/lines.png")))),
      histogram_(new HistogramGraphicsWidget(this)),
      //            histogram_type_(HistogramGraphicsWidget::RM_CURVE_E),
      histogram_type_(HistogramGraphicsWidget::RM_BARS_E),
      mini_histogram_(new HistogramGraphicsWidget(this)),
      mini_window_(new QGraphicsRectItem(0, this)),
      //mini_window_start_(0.25f),
      //mini_window_pct_(0.5f),
      mini_window_start_(0.0f),
      mini_window_pct_(1.0f),
      threshold_bar_left_(new QGraphicsRectItem(0, this)),
      threshold_bar_middle_(new QGraphicsRectItem(0, this)),
      threshold_bar_right_(new QGraphicsRectItem(0, this)),
      threshold_lower_arrow_(new QGraphicsPolygonItem(0, this)),
      threshold_upper_arrow_(new QGraphicsPolygonItem(0, this)),
      threshold_bar_width_(100),
      canvas_(new QGraphicsRectItem(0, this))
  {
    canvas_->setPen(QPen(QColor(255, 255, 255, 0)));

    mini_histogram_->setShowMarkers(false);

    mini_window_->setPen(QPen(mini_window_color_, mini_window_border_, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    mini_window_->setZValue(2.0);
    QBrush brush(mini_window_background_);   // Set semi-transparent brush
    mini_window_->setBrush(brush);

    threshold_bar_left_->setZValue(0.0);
    threshold_bar_middle_->setZValue(0.0);
    threshold_bar_right_->setZValue(0.0);

    threshold_lower_arrow_->setPen(QPen(QColor(0,0,0), 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    threshold_lower_arrow_->setZValue(1.0);

    threshold_upper_arrow_->setPen(QPen(QColor(0,0,0), 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    threshold_upper_arrow_->setZValue(1.0);

    // Overlay
    show_bars_btn_->setScale(0.5);
    show_curve_btn_->setScale(0.5);
    show_bars_btn_->setZValue(1.1);
    show_curve_btn_->setZValue(1.1);
    this->addItem(show_bars_btn_.data());
    this->addItem(show_curve_btn_.data());
    show_bars_btn_->hide();
    show_curve_btn_->hide();

    // Timers
    hover_timer_.setSingleShot(true);
    this->connect(&overlay_fadein_timer_, SIGNAL(timeout()), this, SLOT(overlay_fadein_timer_event()));
    this->connect(&hover_timer_, SIGNAL(timeout()), this, SLOT(hover_timer_event()));

#ifdef LOAD_HISTOGRAM_TEST_DATA
    //    std::vector<int> points;
    //    for (int i = 0; i < 100; i++)
    //    {
    //      float x = float(i)/20.0 - 2.5;
    //      float e = exp(-(x*x)/2.0);
    //      float norm = e/sqrt(6.283);
    //      points.push_back(100.0*norm);
    //    }
    //    std::vector<int> markers;
    //    markers.push_back(70);
    //    markers.push_back(90);
    //    setHistogram(points, markers);

    std::vector<int> points;
    
    for (int i = 0; i < 25; i++) {
      points.push_back((i/25.0) * 86);
    }
    
    for (int i = 0; i < 100; i++)
    {
      float x = float(i-25)/20.0 - 2.5;
      float e = exp(-(x*x)/2.0);
      float norm = e/sqrt(6.283);
      
      float value = floor(1000.0*norm);
      
      //value = 0;
      
      float x2 = float(i+15)/20.0 - 2.5;
      float e2 = exp(-(x2*x2)/2.0);
      float norm2 = e2/sqrt(6.283);
      
      //value = value + (100.0*norm2);
      
      value = floor(1000*norm2);
      //std::cerr << value << std::endl;
      
      //float value = 100.0*norm2;
      
      
//      points.push_back(1000.0*norm);
//      points.push_back(1000.0*norm);
      
      float val = abs((1000.0*norm2) - (500.0*norm));
      points.push_back(val);
      
      
      
    
    }

//    for (int i = 0; i < 10; i++)
//    {
//      float x = float(i)/20.0 - 2.5;
//      float e = exp(-(x*x)/2.0);
//      float norm = e/sqrt(6.283);
//      points.push_back(i);
//    }
    std::vector<float> markers;
    markers.push_back(50);
    markers.push_back(30);
    markers.push_back(70);
    setHistogram(points, markers, 0, 100);
#endif
  }

  HistogramScene::~HistogramScene()
  {
  }

  void HistogramScene::wheelEvent(QGraphicsSceneWheelEvent *e)
  {
    QGraphicsScene::wheelEvent(e);

    // If we're in the main histogram window, scroll l/r or zoom in/out.
    if (histogram_->hit(e->scenePos()))
    {
      float degrees = e->delta();
      if (e->orientation() == Qt::Horizontal)
      {
        mini_window_start_ -= degrees * mini_window_scroll_rate_; // in horizontal, right is negative. *shrug*
        mini_window_start_ = wseCore::Clamp<float>(mini_window_start_, 0.0f, 1.0f - mini_window_pct_);
      }
      else
      {
        float orig_pct = mini_window_pct_;
        mini_window_pct_ += degrees * mini_window_scroll_rate_;
        mini_window_pct_ = wseCore::Clamp<float>(mini_window_pct_, mini_window_min_pct_, 1.0f);
        mini_window_start_ = wseCore::Clamp<float>(mini_window_start_ + 0.5f * (orig_pct - mini_window_pct_),
                                                0.0f, 1.0f - mini_window_pct_);
      }
      draw_all();
    }
  }

  void HistogramScene::updateScene(const QRectF &rect)
  {
    this->setSceneRect(rect);
    canvas_->setRect(rect);

    draw_all();
  }

  void HistogramScene::draw_all()
  {
    draw_threshold_bar();
    draw_mini_histogram();
    draw_histogram();
    draw_overlay();
    update();
  }

  void HistogramScene::draw_overlay()
  {
    show_bars_btn_->hide();
    show_curve_btn_->hide();

    if (show_overlay_)
    {
      if (histogram_type_ == HistogramGraphicsWidget::RM_CURVE_E)
      {
        show_bars_btn_->show();
        show_bars_btn_->setOpacity(overlay_alpha_);
        show_bars_btn_->setPos(margin_ * 4, mini_histogram_height_pct_ * height() + 3 * margin_);
      }
      else if (histogram_type_ == HistogramGraphicsWidget::RM_BARS_E)
      {
        show_curve_btn_->show();
        show_curve_btn_->setOpacity(overlay_alpha_);
        show_curve_btn_->setPos(margin_ * 4, mini_histogram_height_pct_ * height() + 3 * margin_);
      }
      else
      {
        assert(false);
      }
    }

    update();
  }

  void HistogramScene::draw_mini_window()
  {
    float mwheight = mini_histogram_height_pct_ * height() - 2 * margin_ + 2 * mini_window_border_;
    float w = width() - 2 * margin_;
    float mwwidth = w * mini_window_pct_ + mini_window_border_;
    float mwstart = margin_ + w * mini_window_start_ - (mini_window_border_ / 2);
    mini_window_->setRect(mwstart, margin_ - mini_window_border_, mwwidth, mwheight);
    mini_window_->update();
  }

  //---------------------------------------------------------------------------
  // Convert from scene coords to a percentage along the threshold bar.
  //---------------------------------------------------------------------------
  float HistogramScene::to_threshold_bar_pct(float x) const
  {
    float pct = wseCore::Clamp<float>((x - threshold_bar_pos_.x()) / threshold_bar_width_, 0.0f, 1.0f);
    return pct;
  }


  void HistogramScene::keyPressEvent(QKeyEvent * e) {
    const float fineIncrement = 0.0025f;
    const float largeIncrement = 0.05f;
	  
    switch( e->key() )
    {
    case Qt::Key_Left:
      setLowerThreshold(threshold_lower_ - fineIncrement);
      break;

    case Qt::Key_Right:
      setLowerThreshold(threshold_lower_ + fineIncrement);
      break;

    case Qt::Key_Up:
      setLowerThreshold(threshold_lower_ + largeIncrement);
      break;

    case Qt::Key_Down:
      setLowerThreshold(threshold_lower_ - largeIncrement);
      break;

    }
  }

  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  void HistogramScene::mousePressEvent(QGraphicsSceneMouseEvent *e)
  {


    QGraphicsScene::mousePressEvent(e);

    // See if we're in the mini-histogram
    if (mini_histogram_->hit(e->scenePos()))
    {
      // If we're on the edge of the mini window, start a resize.
      if (wseCore::Abs(e->scenePos().x() - mini_window_->rect().x()) < 3)
      {
        resizing_window_left_ = true;
        prev_pos_ = e->scenePos().x() / width();
      }
      else if (wseCore::Abs(e->scenePos().x() - mini_window_->rect().x() - mini_window_->rect().width()) < 3)
      {
        resizing_window_right_ = true;
        prev_pos_ = e->scenePos().x() / width();
      }
      // else if we're inside the mini window, start a move.
      else if (mini_window_->contains(e->pos()))
      {
        moving_window_ = true;
        prev_pos_ = mini_histogram_->to_histogram_pct(e->scenePos().x());
      }
      // If we're outside the mini window, reposition it and start a move.
      else
      {
        float pct = mini_histogram_->to_histogram_pct(e->scenePos().x());
        mini_window_start_ = wseCore::Clamp<float>(pct - 0.5f * mini_window_pct_, 0.0f, 1.0f - mini_window_pct_);
        moving_window_ = true;
        prev_pos_ = mini_histogram_->to_histogram_pct(e->scenePos().x());
        draw_all();
      }
    }
    else
    {
      // iterate over the list of graphics items to see if we have a hit
      QList<QGraphicsItem *> iList = items(e->scenePos());
      for (int i = 0; i < iList.size(); i++)
      {
        QGraphicsItem *it = iList[i];

        // Toggle histogram floating button
        if (it == show_bars_btn_.data())
        {
          histogram_type_ = HistogramGraphicsWidget::RM_BARS_E;
          draw_all();
          return;
        }
        else if (it == show_curve_btn_.data())
        {
          histogram_type_ = HistogramGraphicsWidget::RM_CURVE_E;
          draw_all();
          return;
        }
        else
        {
          // see if we picked the colorbar arrow
          QGraphicsPolygonItem *pit = qgraphicsitem_cast<QGraphicsPolygonItem *>(it);
          if (pit)
          {
            if (pit == threshold_lower_arrow_.data()) {
              dragging_lower_threshold_arrow_ = true;
              return;
            } else if (pit == threshold_upper_arrow_.data()) {
              dragging_upper_threshold_arrow_ = true;
              return;
            }
          }
          else
          {
            if (e->buttons() == Qt::RightButton) {
              // set the threshold to the clicked value
              setUpperThreshold(threshold_bar_pct_to_threshold(to_threshold_bar_pct(e->pos().x())));
            } else {
              // set the threshold to the clicked value
              setLowerThreshold(threshold_bar_pct_to_threshold(to_threshold_bar_pct(e->pos().x())));
            }
          }
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  // Returns current threshold value in range [0,1]
  //---------------------------------------------------------------------------
  float HistogramScene::getLowerThreshold() const
  {
    return threshold_lower_;
  }

  void HistogramScene::setLowerThreshold(float value) {
    threshold_lower_ = value;
    threshold_upper_ = wseCore::Clamp<float>(threshold_upper_, threshold_lower_, 1.0f);
    Q_EMIT lookupTableChanged(threshold_lower_, threshold_upper_);
    draw_threshold_bar();
    //draw_all();
  }

  float HistogramScene::getUpperThreshold() const
  {
    return threshold_upper_;
  }

  void HistogramScene::setUpperThreshold(float value) {
    threshold_upper_ = value;
    threshold_lower_ = wseCore::Clamp<float>(threshold_lower_, 0, threshold_upper_);
    Q_EMIT lookupTableChanged(threshold_lower_, threshold_upper_);
    draw_threshold_bar();
    //draw_all();
  }

  float HistogramScene::threshold_bar_pct_to_threshold(float threshold_bar_pct) const
  {
    float threshold = mini_window_start_ + threshold_bar_pct * mini_window_pct_;
    return threshold;
  }

  float HistogramScene::get_threshold_lower_percent() const
  {
    // Clamp to mini_window_ range, scale to threshold_bar_ width.
    float pct = wseCore::Clamp<float>(threshold_lower_ - mini_window_start_, 0.0f, mini_window_pct_);
    pct /= mini_window_pct_;
    return pct;
  }

  float HistogramScene::get_threshold_upper_percent() const
  {
    // Clamp to mini_window_ range, scale to threshold_bar_ width.
    float pct = wseCore::Clamp<float>(threshold_upper_ - mini_window_start_, 0.0f, mini_window_pct_);
    pct /= mini_window_pct_;
    return pct;
  }

  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  void HistogramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
  {
    QGraphicsScene::mouseMoveEvent(e);

    // Reset hover timer and tooltip
    hover_timer_.stop();

    QToolTip::hideText();
    //canvas_->setToolTip(QString());

    if (moving_window_)
    {
      float curr_pos = mini_histogram_->to_histogram_pct(e->scenePos().x());
      mini_window_start_ = wseCore::Clamp<float>(mini_window_start_ + (curr_pos - prev_pos_), 0.0f, 1.0f - mini_window_pct_);
      prev_pos_ = curr_pos;
      draw_all();
    }
    else if (resizing_window_left_)
    {
      float curr_pos = e->scenePos().x() / width();
      float prev_start = mini_window_start_;

      mini_window_start_ = wseCore::Clamp<float>(mini_window_start_ + (curr_pos - prev_pos_), 0.0f,
                                              mini_window_start_ + mini_window_pct_ - mini_window_min_pct_);

      mini_window_pct_ = wseCore::Clamp<float>(mini_window_pct_ + (prev_start - mini_window_start_), 
                                            mini_window_min_pct_, 1.0f);
      prev_pos_ = wseCore::Clamp<float>(curr_pos, 0.0f, mini_window_start_ + mini_window_pct_ - mini_window_min_pct_);
      draw_all();
    }
    else if (resizing_window_right_)
    {
      float curr_pos = e->scenePos().x() / width();
      mini_window_pct_ = wseCore::Clamp<float>(mini_window_pct_ + (curr_pos - prev_pos_), mini_window_min_pct_,
                                            1.0f - mini_window_start_);
      prev_pos_ = wseCore::Clamp<float>(curr_pos, mini_window_start_ + mini_window_pct_, 1.0f);
      draw_all();
    }
    else if (dragging_lower_threshold_arrow_)
    {
      float before = get_threshold_lower_percent();
      float threshold_bar_pct = to_threshold_bar_pct(e->scenePos().x());
      if (threshold_bar_pct != before)
      {
        setLowerThreshold(threshold_bar_pct_to_threshold(threshold_bar_pct));
      }
    }
    else if (dragging_upper_threshold_arrow_)
    {
      float before = get_threshold_upper_percent();
      float threshold_bar_pct = to_threshold_bar_pct(e->scenePos().x());
      if (threshold_bar_pct != before)
      {
        setUpperThreshold(threshold_bar_pct_to_threshold(threshold_bar_pct));
      }
    }
    else
    {
      // If we're on the edge of the mini window, change the cursor to a double arrow.
      if (mini_histogram_->hit(e->scenePos()) &&
          (wseCore::Abs(e->scenePos().x() - mini_window_->rect().x()) < 3 ||
           wseCore::Abs(e->scenePos().x() - mini_window_->rect().x() - mini_window_->rect().width()) < 3))
      {
        canvas_->setCursor(Qt::SizeHorCursor);
      }
      // else if we're in the main histogram, change to a crosshair
      else if (histogram_->hit(e->scenePos()))
      {
        canvas_->setCursor(Qt::CrossCursor);
        tooltipPosition_ = e->screenPos();

        //hover_timer_event();
        hover_timer_.start(display_value_hover_time_);
        prev_pos_ = histogram_->to_histogram_pct(e->scenePos().x());
      }
      // else change the cursor back to normal
      else
      {
        canvas_->unsetCursor();
      }
    }
  }

  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  void HistogramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
  {
    QGraphicsScene::mouseReleaseEvent(e);
    dragging_lower_threshold_arrow_ = false;
    dragging_upper_threshold_arrow_ = false;
    moving_window_ = false;
    resizing_window_left_ = false;
    resizing_window_right_ = false;
  }


  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  void HistogramScene::setHistogram(const std::vector<int> &values,
                                    const std::vector<float> &markers,
                                    float min, float max)
  {
    values_ = values;
    markers_ = markers;
    min_ = min;
    max_ = max;
    draw_all();
  }

  void HistogramScene::draw_mini_histogram()
  {
    if (values_.size() < 1) return;

    mini_histogram_->setRenderMode(histogram_type_);
    mini_histogram_->setRect(QRect(margin_, margin_, width() - 2 * margin_,
                                   mini_histogram_height_pct_ * height() - 2 * margin_));

    draw_mini_window();
  }

  void HistogramScene::draw_histogram()
  {
    if (values_.size() < 1) return;

    histogram_->setRenderMode(histogram_type_);
    histogram_->setWindow(mini_window_start_, mini_window_pct_);
    histogram_->setRect(QRect(margin_, margin_ + mini_histogram_height_pct_ * height(), // position
                              width() - 2 * margin_,                                    // width
                              height() - mini_histogram_height_pct_ * height() -        // height
                              threshold_bar_height_pct_ * height() - margin_));
  }

  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  void HistogramScene::draw_threshold_bar()
  {
    // Set the threshold bars
    float threshold_bar_height_ = height() * threshold_bar_height_pct_ - 2 * margin_;
    threshold_bar_pos_ = QPointF(margin_, height() - threshold_bar_height_ - margin_);
    threshold_bar_width_ = width() - 2 * margin_;
    float threshold_lower_bar_pct = get_threshold_lower_percent();
    float threshold_upper_bar_pct = get_threshold_upper_percent();
    float tbarmidleft = threshold_lower_bar_pct * threshold_bar_width_;
    float tbarmidright = threshold_upper_bar_pct * threshold_bar_width_;


    float startx = threshold_bar_pos_.x();
    float starty = threshold_bar_pos_.y();

    //    float leftBarWidth = tbarmidleft;
    float middleBarWidth = tbarmidright - tbarmidleft;
    float rightBarWidth = threshold_bar_width_ - tbarmidright;

    threshold_bar_left_->setRect(startx, starty, tbarmidleft, threshold_bar_height_);

    threshold_bar_middle_->setRect(startx + tbarmidleft, starty,
      middleBarWidth, threshold_bar_height_);

    threshold_bar_right_->setRect(startx + tbarmidright, starty,
      rightBarWidth, threshold_bar_height_);

    threshold_bar_left_->setBrush(QBrush(inside_threshold_color_));
    threshold_bar_middle_->setBrush(QBrush(outside_threshold_color_));
    threshold_bar_right_->setBrush(QBrush(inside_threshold_color_));

    // qreal theHeight = height();

    float leftx = -0.5 * threshold_bar_arrow_width_;
    float midx = 0;
    float rightx = 0.5 * threshold_bar_arrow_width_;

    // Make threshold bar arrow.
    QPolygonF lowerPolygon;
    lowerPolygon 
      << QPointF(leftx, height())
      << QPointF(midx, height() - 10)
      << QPointF(rightx, height());
    lowerPolygon.translate(threshold_bar_pos_.x() + tbarmidleft, 0);
    threshold_lower_arrow_->setPolygon(lowerPolygon);


    QPolygonF upperPolygon;
    upperPolygon 
      << QPointF(rightx, height() - threshold_bar_height_ - 2 * margin_)
      << QPointF(midx, height() - threshold_bar_height_ - 2 * margin_ + 10)
      << QPointF(leftx, height() - threshold_bar_height_ - 2 * margin_);
    upperPolygon.translate(threshold_bar_pos_.x() + tbarmidright, 0);
    threshold_upper_arrow_->setPolygon(upperPolygon);
		
	
    if (threshold_lower_ < mini_window_start_ || threshold_lower_ > mini_window_start_ + mini_window_pct_)
    {
      threshold_lower_arrow_->setBrush(threshold_arrow_offscreen_color_);
    }
    else
    {
      threshold_lower_arrow_->setBrush(threshold_arrow_color_);
    }


    if (threshold_upper_ < mini_window_start_ || threshold_upper_ > mini_window_start_ + mini_window_pct_)
    {
      threshold_upper_arrow_->setBrush(threshold_arrow_offscreen_color_);
    }
    else
    {
      threshold_upper_arrow_->setBrush(threshold_arrow_color_);
    }

  }

  void HistogramScene::enterEvent()
  {
    show_overlay();
  }

  void HistogramScene::leaveEvent()
  {
    hide_overlay();
  }

  void HistogramScene::show_overlay()
  {
    show_overlay_ = true;
    overlay_alpha_ = 0.0f;
    overlay_fadein_timer_.start(16); // ~60Hz
  }

  void HistogramScene::hide_overlay()
  {
    show_overlay_ = false;
    overlay_alpha_ = 0.0f;
    overlay_fadein_timer_.stop();

    draw_overlay();
  }

  void HistogramScene::overlay_fadein_timer_event()
  {
    overlay_alpha_ += 1.0f / 60.0f / overlay_fadein_time_; // assumes QTimer is accurate, which it's not.
    if (overlay_alpha_ >= 1.0f)
    {
      overlay_alpha_ = 1.0f;
      overlay_fadein_timer_.stop();
    }

    draw_overlay();
  }

  void HistogramScene::hover_timer_event()
  {
    // Show the value at the current location in the histogram.
    int bin = floor(prev_pos_ * values_.size());
    bin = wseCore::Clamp<int>(bin, 0, values_.size()-1);
    char v[1024];
    sprintf(v, "bin %d\nvalue range: [%G - %G]\n%d pixels", bin, getBinMinValue(bin), getBinMaxValue(bin), values_[bin]);
    QToolTip::showText(tooltipPosition_, v);
    canvas_->setToolTip(v);
  }


  float HistogramScene::getBinMinValue(int bin) {
    float binValueMin = bin * getBinValueWidth() + min_;
    return binValueMin;
  }

  float HistogramScene::getBinCenterValue(int bin) {
    return getBinMaxValue(bin) - (getBinValueWidth()/2) + min_;
  }

  float HistogramScene::getBinValueWidth() {
    float range = (max_ - min_);
    float binValueWidth = range / getNumBins();
    return binValueWidth;
  }

  float HistogramScene::getBinMaxValue(int bin) {
    float range = (max_ - min_);
    float binValueWidth = range / getNumBins();
    float binValueMax = (bin+1) * binValueWidth + min_;
    return binValueMax;
  }


  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  HistogramWidget::HistogramWidget(QWidget *parent) :
      QGraphicsView(parent),
      scene_(new HistogramScene(this))
  {
    this->setMouseTracking(true);

    setScene(scene_.data());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing);
    setAlignment(Qt::AlignCenter);

    connect (scene_.data(), SIGNAL(lookupTableChanged(double, double)), this, SLOT(sceneThresholdChanged(double, double)));

    resetTransform();
    show();
  }

  HistogramWidget::~HistogramWidget()
  {
  }


  void HistogramWidget::sceneThresholdChanged(double lower, double upper) {
    Q_EMIT thresholdChanged(lower, upper);
  }


  float HistogramWidget::getLowerThreshold() {
    return scene_.data()->getLowerThreshold();
  }

  void HistogramWidget::setLowerThreshold(float value) {
    scene_.data()->setLowerThreshold(value);
  }

  float HistogramWidget::getUpperThreshold() {
    return scene_.data()->getUpperThreshold();
  }

  void HistogramWidget::setUpperThreshold(float value) {
    scene_.data()->setUpperThreshold(value);
  }



  //---------------------------------------------------------------------------
  //
  //---------------------------------------------------------------------------
  void HistogramWidget::resizeEvent(QResizeEvent *event)
  {
    QGraphicsView::resizeEvent(event);
    scene_->updateScene(QRectF(0, 0, event->size().width(), event->size().height()));

    if (isTransformed())
    {
      CORE_LOG_ERROR("Histogram View should not transform contents."); // the Scene takes its own resolution into account
    }
    update();
  }

  void HistogramWidget::enterEvent(QEvent *e)
  {
    scene_->enterEvent();
  }

  void HistogramWidget::leaveEvent(QEvent *e)
  {
    scene_->leaveEvent();
  }





} // end namespace wse
