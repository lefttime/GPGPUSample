#ifndef QxSEGYExtractor_HPP
#define QxSEGYExtractor_HPP

#include <QxAbstractCompute.hpp>

class QxSEGYExtractor : public QxAbstractCompute
{
  Q_OBJECT;

public:

  QxSEGYExtractor( const QSize& size );
  ~QxSEGYExtractor();

  bool prepare( const QList<unsigned int*>& fInput, float* fOutput );
  virtual bool prepare( const QList<float*>& fInput, float* fOutput );

protected:

  virtual QRect targetRect() const;

  virtual const QString& source() const;
  virtual void computeRoutine();

private:

  class QxSEGYExtractorPrivate;
  QScopedPointer<QxSEGYExtractorPrivate>         _pd;
};

#endif // QxSEGYExtractor_HPP
