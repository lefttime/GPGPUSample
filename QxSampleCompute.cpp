#include "QxSampleCompute.hpp"

static char convolution[] = {
  "uniform sampler2DRect texture;"
  "uniform float fRadius;"
  "float nWidth = 1024.0;"
  "float nHeight= 1024.0;"
  "void main()"
  "{"
  "  vec2 sum = vec2(0.0, 0.0);"
  "  vec2 pos = gl_TexCoord[0].st;"
  "  vec4 fSum = vec4(0.0, 0.0, 0.0, 0.0);"
  "  vec4 fTotal = vec4(0.0, 0.0, 0.0, 0.0);"
  "  vec4 vec4Result = vec4(0.0, 0.0, 0.0, 0.0);"
  "  for( float ii=pos.x-fRadius; ii<=pos.x+fRadius+0.5; ii+=1.0 ) {"
  "    for( float jj=pos.y-fRadius; jj<=pos.y+fRadius+0.5; jj+=1.0 ) {"
  "      if( ii>=0.0 && jj>=0.0 && ii<nWidth && jj<nHeight ) {"
  "        fSum += texture2DRect( texture, vec2(ii, jj) );"
  "        fTotal += vec4(1.0, 1.0, 1.0, 1.0);"
  "      }"
  "      sum = vec2(ii, jj);"
  "    }"
  "  }"
  "  vec4Result = fSum / fTotal;"
  "  vec4Result = vec4(vec4Result.r);"
  "  gl_FragColor = vec4Result;"
  "}"
};

class QxSampleCompute::QxSampleComputePrivate
{
public:

  QxSampleComputePrivate( QxSampleCompute* me ) : m_self( me ) {
    m_source = QString( convolution );
  }

  void init() {
    m_config.texTarget         = GL_TEXTURE_RECTANGLE;
    m_config.texInternalFormat = GL_R32F;
    m_config.texFormat         = GL_RED;

    glGenTextures( 1, &m_texID );
    m_self->setupTexture( m_config, m_texID );
  }

  QxSampleCompute*         m_self;
  TextureParameters        m_config;

  QString                  m_source;

  GLuint                   m_texID;
};

QxSampleCompute::QxSampleCompute( const QSize& size )
  : QxAbstractCompute( TextureParameters(), size ), _pd( new QxSampleComputePrivate( this ) )
{
  _pd->init();
}

QxSampleCompute::~QxSampleCompute()
{
}

bool QxSampleCompute::prepare( const QList<float*>& fInput, float* fOutput )
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

const QString& QxSampleCompute::source() const
{
  return _pd->m_source;
}

void QxSampleCompute::computeRoutine()
{
  GLint radiusParam = glGetUniformLocation( program(), "fRadius" );
  glUniform1f( radiusParam, 2.0f );

  glActiveTexture( GL_TEXTURE0 );
}
