#ifndef QxSampleCompute_HPP
#define QxSampleCompute_HPP

#include <QxAbstractCompute.hpp>

class QxSampleCompute : public QxAbstractCompute
{
  Q_OBJECT;

public:

  QxSampleCompute( const QSize& size );
  ~QxSampleCompute();

  virtual bool prepare( const QList<float*>& fInput, float* fOutput );

protected:

  virtual const QString& source() const;
  virtual void computeRoutine();

private:

  class QxSampleComputePrivate;
  QScopedPointer<QxSampleComputePrivate>         _pd;
};

#endif // QxSampleCompute_HPP