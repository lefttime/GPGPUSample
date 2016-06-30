#ifndef QxSegyParser_HPP
#define QxSegyParser_HPP

#include <QObject>
#include <QVector>

class QxSegyParser : public QObject
{
  Q_OBJECT;

public:

  QxSegyParser( const QString& filename=QString(), QObject* parent=0 );
  ~QxSegyParser();

  void setFilename( const QString& filename );

  int totalTraces() const;
  void setExtractCount( int );

  const QVector<float>& getDataByCPU( int startIdx, qint64& usedTime );
  const QVector<float>& getDataByGPU( int startIdx, qint64& usedTime );
  const QVector<float>& getDataByMPI( int startIdx, qint64& usedTime );

  const QVector<float>& dataCPU();
  const QVector<float>& dataGPU();
  const QVector<float>& dataMPI();

private:

  class QxSegyParserPrivate;
  QScopedPointer<QxSegyParserPrivate>         _pd;
};

#endif // QxSegyParser_HPP
