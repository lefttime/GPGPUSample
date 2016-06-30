#ifndef QxGLWidget_HPP
#define QxGLWidget_HPP

#include <gl/glew.h>
#include <QGLWidget>

class QxGLWidget : public QGLWidget
{
public:

  static QxGLWidget* instance();
  ~QxGLWidget();

  bool prepare( const QSize& size );

  void render();

private:

  QxGLWidget();

  class QxGLWidgetPrivate;
  QScopedPointer<QxGLWidgetPrivate>         _pd;
};

#define theGLWidget QxGLWidget::instance()

#endif // QxGLWidget_HPP
