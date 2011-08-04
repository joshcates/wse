#ifndef WSE_INTERFACE_HISTOGRAMWIDGET_H
#define WSE_INTERFACE_HISTOGRAMWIDGET_H

// STL includes
#include <vector>

// Qt includes
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

class QGraphicsPathItem;
class QGraphicsRectItem;
class QGraphicsPolygonItem;
class QGraphicsListItem;

namespace wse {

  class HistogramScene;


  //----------------------------------------------------------------
  // HistogramGraphicsWidget
  //
  // Renders a portion of the histogram found in the given scene.
  //
  class HistogramGraphicsWidget : public QObject
  {
    Q_OBJECT

    friend class HistogramScene;

  public:
    HistogramGraphicsWidget(HistogramScene *scene);
    ~HistogramGraphicsWidget();

    // Return scene x coords to histogram pct.
    float to_histogram_pct(float v) const;

    void setRect(const QRectF &rect); // reposition and render item

    // Set how much data to render and where to start rendering the data.
    void setWindow(float start, float amt);

    void setShowMarkers(bool on) { show_markers_ = on; }

    // Bars or a curve.
    enum RenderMode { RM_CURVE_E, RM_BARS_E };
    void setRenderMode(RenderMode rm) { render_mode_ = rm; }

    // see if the given point is within the histogram
    bool hit(const QPointF &p) const;


  private:
    void update();
    void draw();
    void drawCurve(int startBin, int numToRender, float yScale, float shift);
    void drawBars(int startBin, int numToRender, float yScale, float shift);
    void cleanupMarkers();
    void cleanupBars();

    // state
    RenderMode render_mode_;
    bool show_markers_;

    // The physical bounds of this item in scene space
    QRectF bounds_;

    // Portion of the data to be rendered
    float start_;    // [0,1]
    float showpct_;  // [0,1]

    // Scene with data to be rendered (TODO: This will need to point to some Seg3D state data object).
    HistogramScene *scene_;

    // Renderable items
    QScopedPointer<QGraphicsRectItem> rect_;
    QScopedPointer<QGraphicsPathItem> path_;
    std::vector<QGraphicsRectItem *> bars_;
    std::vector<QGraphicsLineItem *> markers_;

    static const QColor marker_color_;
    static const QColor background_color_;
    static const QColor path_color_bottom_;
    static const QColor path_color_top_;
    static const QColor bar_color_bottom_;
    static const QColor bar_color_top_;
    static const QColor marker_bar_color_bottom_;
    static const QColor marker_bar_color_top_;
  };

  class HistogramScene : public QGraphicsScene
  {
    Q_OBJECT

    friend class HistogramGraphicsWidget;

  public:
    HistogramScene(QWidget *parent = 0);
    virtual ~HistogramScene();

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *e);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *e);
    virtual void keyPressEvent(QKeyEvent * e);

    void enterEvent();
    void leaveEvent();

    void setHistogram(const std::vector<int> &values,
                      const std::vector<float> &markers,
                      float min, float max);

    void updateScene(const QRectF &rect);

    // Returns current threshold value in range [0,1]
    float getLowerThreshold() const;
    void setLowerThreshold(float value);

    float getUpperThreshold() const;
    void setUpperThreshold(float value);

    float getBinMinValue(int i);
    float getBinMaxValue(int i);
    float getBinCenterValue(int i);
    float getBinValueWidth();

  public Q_SLOTS:
    void overlay_fadein_timer_event();
    void hover_timer_event();

  Q_SIGNALS:
    void lookupTableChanged(double lower, double upper);

  private:
    void draw_all();

    int getNumBins() {
      return values_.size();
    }

    // histogram data
    std::vector<int> values_;
    std::vector<float> markers_;
    float min_;
    float max_;
    float threshold_lower_;
    float threshold_upper_;

    // state
    bool dragging_lower_threshold_arrow_;
    bool dragging_upper_threshold_arrow_;
    bool resizing_window_left_;
    bool resizing_window_right_;
    bool moving_window_;
    float prev_pos_;
    QPoint tooltipPosition_;

    // overlay (bars/curve icon, histogram scale)
    bool show_overlay_;
    float overlay_alpha_;
    QScopedPointer<QGraphicsPixmapItem> show_bars_btn_;
    QScopedPointer<QGraphicsPixmapItem> show_curve_btn_;
    QTimer overlay_fadein_timer_;
    void show_overlay();
    void hide_overlay();
    static const float overlay_fadein_time_;
    void draw_overlay();

    // histogram
    QScopedPointer<HistogramGraphicsWidget> histogram_;
    void draw_histogram();
    QTimer hover_timer_;
    HistogramGraphicsWidget::RenderMode histogram_type_;
    static const int display_value_hover_time_;

    // mini histogram
    QScopedPointer<HistogramGraphicsWidget> mini_histogram_;
    QScopedPointer<QGraphicsRectItem> mini_window_;
    float mini_window_start_;    // [0,1]
    float mini_window_pct_;
    static const float mini_histogram_height_pct_;
    static const QColor mini_window_color_;
    static const QColor mini_window_background_;
    static const float mini_window_border_;
    static const float mini_window_min_pct_;
    static const float mini_window_scroll_rate_; // in percent/degree
    static const float mini_window_zoom_rate_; // in percent/degree
    void draw_mini_histogram();
    void draw_mini_window();

    // threshold bar
    QScopedPointer<QGraphicsRectItem> threshold_bar_left_;
    QScopedPointer<QGraphicsRectItem> threshold_bar_middle_;
    QScopedPointer<QGraphicsRectItem> threshold_bar_right_;
    QScopedPointer<QGraphicsPolygonItem> threshold_lower_arrow_;
    QScopedPointer<QGraphicsPolygonItem> threshold_upper_arrow_;
    QPointF threshold_bar_pos_;
    float threshold_bar_width_;
    static const float threshold_bar_height_pct_;
    static const float threshold_bar_arrow_width_;
    static const float threshold_bar_arrow_height_;
    static const QColor inside_threshold_color_;
    static const QColor outside_threshold_color_;
    static const QColor threshold_arrow_offscreen_color_;
    static const QColor threshold_arrow_color_;
    float to_threshold_bar_pct(float x) const;    // Scene space to local threshold bar coord.
    float get_threshold_lower_percent() const; // Actual (threshold_) to threshold bar coord.
    float get_threshold_upper_percent() const; // Actual (threshold_) to threshold bar coord.
    float threshold_bar_pct_to_threshold(float pct) const; // Threshold bar coord to actual threshold.
    void draw_threshold_bar();

    // margin
    static const float margin_;

    // a background QGraphicsItem to use for changing mouse cursor
    QScopedPointer<QGraphicsRectItem> canvas_;
  };

  class HistogramWidget : public QGraphicsView
  {
    Q_OBJECT

  public Q_SLOTS:
    void sceneThresholdChanged(double lower, double upper);

  Q_SIGNALS:
    void thresholdChanged(double lower, double upper);


  public:
    HistogramWidget(QWidget *parent = 0);
    virtual ~HistogramWidget();

    float getLowerThreshold();
    void setLowerThreshold(float value);

    float getUpperThreshold();
    void setUpperThreshold(float value);

    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

    void setHistogram(const std::vector<int> &values,
                      const std::vector<float> &markers,
                      float min, float max)
    {
      this->scene_->setHistogram(values, markers, min, max);
    }

    virtual void resizeEvent(QResizeEvent *event);

  private:
    QSharedPointer<HistogramScene> scene_;
  };

} // end namespace wse

#endif // WSE_INTERFACE_HISTOGRAMWIDGET_H
