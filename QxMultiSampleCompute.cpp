#include "QxMultiSampleCompute.hpp"

static char addSample[] = {
  "#extension GL_EXT_gpu_shader4 : enable\n"
  "uniform sampler2DRect texture1;"
  "uniform sampler2DRect texture2;"
  "float valEx( float fval ) {"
  "  int val = int(fval);"
  "  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );"
  "  val = (val << 16) | ((val >> 16) & 0xFFFF);"
  "  uint dataUint32 = int(val);"
  "  float result ;"
  "  float sign = ( dataUint32 >> 31);"
  "  float exp = ((dataUint32 >> 24) & 0x7f ) - 64;"
  "  float frac = ( dataUint32 & 0x00ffffff );"
  "  frac = frac / pow(2.0, 24.0);"
  "  return (1 - 2*sign) * pow(16.0,exp) * frac;"
  "}"
  "void main()"
  "{"
  "  float rval = texture2DRect( texture1, gl_TexCoord[0].st ).r+ texture2DRect( texture2, gl_TexCoord[0].st ).r;"
  "  gl_FragColor = vec4( valEx(rval) );"
  "}"
};

int test()
{
  int val;
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );
  val = (val << 16) | ((val >> 16) & 0xFFFF);

  UINT dataUint32 = val;


  double result ;

  // gain sign from first bit
  double sign = (double)( dataUint32 >> 31);

  // gain exponent from first byte, last 7 bits
  double exp = (double)((dataUint32 >> 24) & 0x7f ) - 64;

  // gain mantissa from last 3 bytes
  double frac = (double)( dataUint32 & 0x00ffffff );
  frac = frac / pow(2.0, 24);
  result = (1 - 2*sign) * pow(16.0,exp) * frac;

  return result;
}

class QxMultiSampleCompute::QxMultiSampleComputePrivate
{
public:

  QxMultiSampleComputePrivate( QxMultiSampleCompute* me ) : m_self( me ) {
    m_source = QString( addSample );

    m_uniID[0] = GL_TEXTURE0;
    m_uniID[1] = GL_TEXTURE1;
  }

  ~QxMultiSampleComputePrivate() {
    glDeleteTextures( 2, m_texID );
  }

  void init() {
    m_config.texTarget         = GL_TEXTURE_RECTANGLE;
    m_config.texInternalFormat = GL_R32F;
    m_config.texFormat         = GL_RED;

    glGenTextures( 2, m_texID );
    for( int idx = 0; idx < 2; ++idx ) {
      glActiveTexture( m_uniID[idx] );
      m_self->setupTexture( m_config, m_texID[idx] );
    }
  }

  QxMultiSampleCompute*         m_self;
  TextureParameters             m_config;

  QString                       m_source;
  GLuint                        m_uniID[2];
  GLuint                        m_texID[2];
};

QxMultiSampleCompute::QxMultiSampleCompute( const QSize& size )
  : QxAbstractCompute( TextureParameters(), size ), _pd( new QxMultiSampleComputePrivate( this ) )
{
  _pd->init();
}

QxMultiSampleCompute::~QxMultiSampleCompute()
{
}

bool QxMultiSampleCompute::prepare( const QList<float*>& fInput, float* fOutput )
{
  bool result = QxAbstractCompute::prepare( fInput, fOutput );
  if( result ) {
    if( fInput.count() < 2 ) {
      result = false;
    } else {
      for( int idx = 0; idx < 2; ++idx ) {
        transferToTexture( fInput.at( idx ), _pd->m_texID[idx], _pd->m_config );
      }
    }
  }

  return result;
}

const QString& QxMultiSampleCompute::source() const
{
  return _pd->m_source;
}

void QxMultiSampleCompute::computeRoutine()
{
  GLint texParam1 = glGetUniformLocation( program(), "texture1" );
  GLint texParam2 = glGetUniformLocation( program(), "texture2" );
  glUniform1i( texParam1, 0 );
  glUniform1i( texParam2, 1 );
}