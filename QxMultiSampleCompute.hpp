#ifndef QxMultiSampleCompute_HPP
#define QxMultiSampleCompute_HPP

#include <QxAbstractCompute.hpp>

class QxMultiSampleCompute : public QxAbstractCompute
{
  Q_OBJECT;

public:

  QxMultiSampleCompute( const QSize& size );
  ~QxMultiSampleCompute();

  virtual bool prepare( const QList<float*>& fInput, float* fOutput );

protected:

  virtual const QString& source() const;
  virtual void computeRoutine();

private:

  class QxMultiSampleComputePrivate;
  QScopedPointer<QxMultiSampleComputePrivate>         _pd;
};

#endif // QxMultiSampleCompute_HPP