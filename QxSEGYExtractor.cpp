#include "QxSEGYExtractor.hpp"

static char dataFormat[] = {
  "#extension GL_EXT_gpu_shader4 : enable\n"
  "uniform usampler2DRect texture;"
  "uint swap_int32( uint val ) {"
  "  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );"
  "  return (val << 16) | ((val >> 16) & 0xFFFF);"
  "}"
  "float ibm2num( uint dataUint32 ) {"
  "  float sign = ( dataUint32 >> 31);"
  "  float exp = ((dataUint32 >> 24) & 0x7f ) - 64;"
  "  float frac = ( dataUint32 & 0x00ffffff );"
  "  frac = frac / pow(2.0, 24.0);"
  "  return (1 - 2*sign) * pow(16.0,exp) * frac;"
  "}"
  "void main()"
  "{"
  "  gl_FragColor = vec4( ibm2num( swap_int32( texture2DRect( texture, gl_TexCoord[0].st ).r ) ) );"
  "}"
};

class QxSEGYExtractor::QxSEGYExtractorPrivate
{
public:

  QxSEGYExtractorPrivate( QxSEGYExtractor* me ) : m_self( me ) {
    m_source = QString( dataFormat );
  }

  void init() {
    m_config.texTarget         = GL_TEXTURE_RECTANGLE;
    m_config.texInternalFormat = GL_R32UI;
    m_config.texFormat         = GL_RED_INTEGER;

    glGenTextures( 1, &m_texID );
    glActiveTexture( GL_TEXTURE0 );
    setupTexture( m_config, m_texID );
  }

  void setupTexture( const TextureParameters& info, const GLuint texID ) {
    glBindTexture(   info.texTarget, texID );
    glTexParameteri( info.texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( info.texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( info.texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( info.texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP );

    int width = m_self->size().width();
    int height = m_self->size().height();
    glTexImage2D( info.texTarget, 0, info.texInternalFormat, width, height, 0, info.texFormat, GL_UNSIGNED_INT, 0 );
  }

  void transferToTexture( unsigned int* data, GLuint texID, const TextureParameters& par ) {
    glBindTexture( par.texTarget, texID );
    int width = m_self->size().width();
    int height = m_self->size().height();
    glTexSubImage2D( par.texTarget, 0, 0, 0, width, height, par.texFormat, GL_UNSIGNED_INT, data );
  }

  QxSEGYExtractor*         m_self;
  TextureParameters        m_config;

  QString                  m_source;

  GLuint                   m_texID;
};

QxSEGYExtractor::QxSEGYExtractor( const QSize& size )
  : QxAbstractCompute( TextureParameters(GL_TEXTURE_RECTANGLE, GL_R32F, GL_RED), size ), _pd( new QxSEGYExtractorPrivate( this ) )
{
  _pd->init();
}

QxSEGYExtractor::~QxSEGYExtractor()
{
}

bool QxSEGYExtractor::prepare( const QList<unsigned int*>& fInput, float* fOutput )
{
  bool result = QxAbstractCompute::prepare( QList<float*>(), fOutput );
  if( result ) {
    if( fInput.isEmpty() ) {
      result = false;
    } else {
      _pd->transferToTexture( fInput.first(), _pd->m_texID, _pd->m_config );
      glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    }
  }

  return result;
}

bool QxSEGYExtractor::prepare( const QList<float*>& fInput, float* fOutput )
{
  bool result = QxAbstractCompute::prepare( fInput, fOutput );
  if( result ) {
    if( fInput.isEmpty() ) {
      result = false;
    } else {
      transferToTexture( fInput.first(), _pd->m_texID, _pd->m_config );
      glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    }
  }

  return result;
}

QRect QxSEGYExtractor::targetRect() const
{
  int xOffset = 240/sizeof(float);
  return QRect(xOffset, 0, size().width()-xOffset, size().height());
}

const QString& QxSEGYExtractor::source() const
{
  return _pd->m_source;
}

void QxSEGYExtractor::computeRoutine()
{
  //GLint tcParam = glGetUniformLocation( program(), "traceCount" );
  //GLint tlParam = glGetUniformLocation( program(), "traceLength" );
  //glUniform1f( tcParam, size().height() );
  //glUniform1f( tlParam, size().width() );
}
