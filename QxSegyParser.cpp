#include "QxSegyParser.hpp"

#include <QxSEGYExtractor.hpp>

#include <QFile>
#include <QElapsedTimer>

#include <omp.h>

class QxSegyParser::QxSegyParserPrivate
{
public:

  QxSegyParserPrivate( QxSegyParser* me ) : m_self( me ), m_file( 0 ), m_extractCount( 500 ) {
  }

  void init( const QString& filename ) {
    if( m_file && m_file->fileName()==filename ) {
      return;
    }
    delete m_file;

    m_file = new QFile( filename );
    if( !m_file->open( QFile::ReadOnly ) ) {
      delete m_file;
      m_file = 0;
      return;
    }

    qint16* buffer = (qint16*)m_file->map( 3200, 400 );
    m_sampleRate = swap_int16( buffer[8] );   // 3216;
    m_traceLength= swap_int16( buffer[10] );  // 3220;
    m_formatCode = swap_int16( buffer[12] );  // 3224;
    m_file->unmap( (uchar*)buffer );

    int bytes = 4;
    switch( m_formatCode ) {
    case 1:
    case 5: bytes = 4; break;

    case 3: bytes = 2; break;

    default: bytes = 4;
    }

    m_bytesPerTrace = 240 + m_traceLength*bytes;
    m_totalTraces = (m_file->size() - 3200 - 400) / m_bytesPerTrace;

    m_cpuBuffer.resize( m_extractCount*m_traceLength );
    m_gpuBuffer.resize( m_extractCount*m_traceLength );
    m_mpiBuffer.resize( m_extractCount*m_traceLength );
  }

  const QVector<float>& getDataByCPU( int startIdx, qint64& usedTime ) {
    QElapsedTimer timer;
    timer.start();

    qint64 offset = 3600 + qint64(startIdx) * m_bytesPerTrace;
    qint64 size   = m_bytesPerTrace*m_extractCount;
    unsigned char* buffer = m_file->map( offset, size );
    if( buffer ) {
      if( m_formatCode==3 ) {
        for( int idx = 0; idx < m_extractCount; ++idx ) {
          float* dataPtr = (float*)(buffer+m_bytesPerTrace*idx+240);
          for( int idy = 0; idy < m_traceLength; ++idy ) {
            m_cpuBuffer[idx*m_traceLength+idy] = swap_int16( *((qint16*)(dataPtr+idy)) );
          }
        }
      } else {
        for( int idx = 0; idx < m_extractCount; ++idx ) {
          float* dataPtr = (float*)(buffer+m_bytesPerTrace*idx+240);
          for( int idy = 0; idy < m_traceLength; ++idy ) {
            m_cpuBuffer[idx*m_traceLength+idy] = ibm2num( swap_int32( *((qint32*)(dataPtr+idy)) ) );
          }
        }
      }

      m_file->unmap( buffer );
    }

    usedTime = timer.elapsed();
    return m_cpuBuffer;
  }

  const QVector<float>& getDataByMPI( int startIdx, qint64& usedTime ) {
    QElapsedTimer timer;
    timer.start();

    qint64 offset = 3600 + qint64(startIdx) * m_bytesPerTrace;
    qint64 size   = m_bytesPerTrace*m_extractCount;
    unsigned char* buffer = m_file->map( offset, size );
    if( buffer ) {
      if( m_formatCode==3 ) {
#pragma omp parallel for
        for( int idx = 0; idx < m_extractCount; ++idx ) {
          float* dataPtr = (float*)(buffer+m_bytesPerTrace*idx+240);
          for( int idy = 0; idy < m_traceLength; ++idy ) {
            m_mpiBuffer[idx*m_traceLength+idy] = swap_int16( *((qint16*)(dataPtr+idy)) );
          }
        }
      } else {
#pragma omp parallel for
        for( int idx = 0; idx < m_extractCount; ++idx ) {
          float* dataPtr = (float*)(buffer+m_bytesPerTrace*idx+240);
          for( int idy = 0; idy < m_traceLength; ++idy ) {
            m_mpiBuffer[idx*m_traceLength+idy] = ibm2num( swap_int32( *((qint32*)(dataPtr+idy)) ) );
          }
        }
      }

      m_file->unmap( buffer );
    }

    usedTime = timer.elapsed();
    return m_mpiBuffer;
  }

  const QVector<float>& getDataByGPU( int startIdx, qint64& usedTime ) {
    QElapsedTimer timer;
    timer.start();

    qint64 offset = 3600 + qint64(startIdx) * m_bytesPerTrace;
    qint64 size   = m_bytesPerTrace*m_extractCount;
    unsigned char* buffer = m_file->map( offset, size );
    if( buffer ) {
      QxSEGYExtractor extractor( QSize(m_traceLength+240/sizeof(float), m_extractCount) );
      extractor.prepare( QList<unsigned int*>()<<(unsigned int*)buffer, m_gpuBuffer.data() );
      extractor.compute();
      m_file->unmap( buffer );
    }

    usedTime = timer.elapsed();
    return m_gpuBuffer;
  }

  qint16 swap_int16( qint16 val ) {
    return (val << 8) | ((val >> 8) & 0xFF);
  }

  qint32 swap_int32( qint32 val ) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | ((val >> 16) & 0xFFFF);
  }

  qreal ibm2num(quint32 dataUint32 ) {
    qreal result ;

    // gain sign from first bit
    float sign = (float)( dataUint32 >> 31);

    // gain exponent from first byte, last 7 bits
    float exp = (float)((dataUint32 >> 24) & 0x7f ) - 64;

    // gain mantissa from last 3 bytes
    float frac = (float)( dataUint32 & 0x00ffffff );
    frac = frac / pow(2.0, 24);
    result = (1 - 2*sign) * pow(16.0f,exp) * frac;

    return result;
  }

  QxSegyParser*         m_self;

  QFile*                m_file;

  qint16                m_sampleRate;
  qint16                m_traceLength;
  qint16                m_formatCode;
  qint16                m_bytesPerTrace;
  qint32                m_totalTraces;

  int                   m_extractCount;

  QVector<float>        m_cpuBuffer;
  QVector<float>        m_gpuBuffer;
  QVector<float>        m_mpiBuffer;
};

QxSegyParser::QxSegyParser( const QString& filename, QObject* parent )
  : QObject( parent ), _pd( new QxSegyParserPrivate( this ) )
{
  _pd->init( filename );
}

QxSegyParser::~QxSegyParser()
{
}

void QxSegyParser::setFilename( const QString& filename )
{
  _pd->init( filename );
}

int QxSegyParser::totalTraces() const
{
  return _pd->m_totalTraces;
}

void QxSegyParser::setExtractCount( int extractCount )
{
  if( _pd->m_extractCount!=extractCount ) {
    _pd->m_extractCount = extractCount;
    _pd->m_cpuBuffer.resize( extractCount*_pd->m_traceLength );
    _pd->m_gpuBuffer.resize( extractCount*_pd->m_traceLength );
    _pd->m_mpiBuffer.resize( extractCount*_pd->m_traceLength );
  }
}

const QVector<float>& QxSegyParser::getDataByCPU( int startIdx, qint64& usedTime )
{
  return _pd->getDataByCPU( startIdx, usedTime );
}

const QVector<float>& QxSegyParser::getDataByGPU( int startIdx, qint64& usedTime )
{
  return _pd->getDataByGPU( startIdx, usedTime );
}

const QVector<float>& QxSegyParser::getDataByMPI( int startIdx, qint64& usedTime )
{
  return _pd->getDataByMPI( startIdx, usedTime );
}

const QVector<float>& QxSegyParser::dataCPU()
{
  return _pd->m_cpuBuffer;
}

const QVector<float>& QxSegyParser::dataGPU()
{
  return _pd->m_gpuBuffer;
}

const QVector<float>& QxSegyParser::dataMPI()
{
  return _pd->m_mpiBuffer;
}
