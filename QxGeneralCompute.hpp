#ifndef QxGeneralCompute_HPP
#define QxGeneralCompute_HPP

#include <gl/glew.h>

#include <QGLWidget>

class QxGeneralCompute : public QGLWidget
{
public:

  static QxGeneralCompute* instance();
  ~QxGeneralCompute();

  qint64 compute() const;

private:

  QxGeneralCompute();

private:

  class QxGeneralComputePrivate;
  QScopedPointer<QxGeneralComputePrivate>         _pd;
};

#define theGeneralCompute QxGeneralCompute::instance()

#endif // QxGeneralCompute_HPP
