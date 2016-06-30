#include "QxAbstractCompute.hpp"

#include <QxGLWidget.hpp>

#include <QtDebug>
#include <QElapsedTimer>

static char clean[] = {
  "void main()"
  "{"
  "  gl_FragColor=vec4(0.0, 0.0, 0.0, 0.0);"
  "}"
};

class QxAbstractCompute::QxAbstractComputePrivate
{
public:

  QxAbstractComputePrivate( QxAbstractCompute* me, const TextureParameters& config, const QSize& size )
    : m_self( me ), m_config( config ), m_size( size ), m_buffer( 0 ) {
  }

  ~QxAbstractComputePrivate() {
    glDeleteTextures( 1, &m_texID );
  }

  void init() {
    if( !theGLWidget->prepare( m_size ) ) {
      return;
    }

    glGenTextures( 1, &m_texID );
    m_self->setupTexture( m_config, m_texID );
  }

  bool prepare( float* fOutput ) {
    bool result = theGLWidget->prepare( m_size );
    if( result ) {
      m_buffer = fOutput;
    } else {
      m_buffer = 0;
    }

    return result;
  }

  void performComputation( char* shader_source ) {
    prepareGLSL( shader_source );

    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_config.texTarget, m_texID, 0 );

    glUseProgram( m_glslProgram );

    m_self->computeRoutine();

    theGLWidget->render();

    releaseGLSL();
  }

  bool prepareGLSL( char* shader_source ) {
    GLint status;

    m_glslProgram = glCreateProgram();
    m_fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

    const char* source = shader_source;
    glShaderSource( m_fragmentShader, 1, &source, NULL );

    glCompileShader( m_fragmentShader );
    glGetShaderiv( m_fragmentShader, GL_COMPILE_STATUS, &status );
    if( status!=GL_TRUE ) {
      GLint len;
      glGetShaderiv( m_fragmentShader, GL_INFO_LOG_LENGTH, &len );

      char* info = new char[len];
      glGetShaderInfoLog( m_fragmentShader, len, &len, info );
      qDebug() << info;
      delete info;

      glDeleteShader( m_fragmentShader );
      glDeleteProgram( m_glslProgram );

      return false;
    }

    glAttachShader( m_glslProgram, m_fragmentShader );

    glLinkProgram( m_glslProgram );
    glGetProgramiv( m_glslProgram, GL_LINK_STATUS, &status );
    if( status!=GL_TRUE ) {
      GLint len;
      glGetProgramiv( m_glslProgram, GL_INFO_LOG_LENGTH, &len );

      char* info = new char[len];
      glGetProgramInfoLog( m_glslProgram, len, &len, info );
      qDebug() << info;

      releaseGLSL();
      delete info;

      return false;
    }

    return true;
  }

  void releaseGLSL() {
    glDetachShader( m_glslProgram, m_fragmentShader );
    glDeleteShader( m_fragmentShader );
    glDeleteProgram( m_glslProgram );
  }

  void saveResult() {
    transferFromTextrue( m_buffer );
  }

  void transferFromTextrue( float* data ) {
    glReadBuffer( GL_COLOR_ATTACHMENT0 );

    QRect rc = m_self->targetRect();
    glReadPixels( rc.left(), rc.top(), rc.width(), rc.height(), m_config.texFormat, GL_FLOAT, data );
  }

  QxAbstractCompute*         m_self;
  TextureParameters          m_config;

  QSize                      m_size;
  GLuint                     m_texID;

  GLuint                     m_glslProgram;
  GLuint                     m_fragmentShader;

  float*                     m_buffer;
};

QxAbstractCompute::QxAbstractCompute( const TextureParameters& config, const QSize& size )
  : _pd( new QxAbstractComputePrivate( this, config, size ) )
{
  _pd->init();
}

QxAbstractCompute::~QxAbstractCompute()
{
}

qint64 QxAbstractCompute::compute()
{
  QElapsedTimer timer;
  timer.start();

  _pd->performComputation( clean );
  _pd->performComputation( source().toLocal8Bit().data() );
  _pd->saveResult();

  return timer.elapsed();
}

bool QxAbstractCompute::prepare( const QList<float*>& fInput, float* fOutput )
{
  Q_UNUSED( fInput );
  return _pd->prepare( fOutput );
}

GLuint QxAbstractCompute::program() const
{
  return _pd->m_glslProgram;
}

QRect QxAbstractCompute::targetRect() const
{
  return QRect(0, 0, size().width(), size().height());
}

const QSize& QxAbstractCompute::size() const
{
  return _pd->m_size;
}

void QxAbstractCompute::setupTexture( const TextureParameters& info, const GLuint texID )
{
  glBindTexture(   info.texTarget, texID );
  glTexParameteri( info.texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( info.texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( info.texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( info.texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP );

  glTexImage2D( info.texTarget, 0, info.texInternalFormat, size().width(), size().height(), 0, info.texFormat, GL_FLOAT, 0 );
}

void QxAbstractCompute::transferToTexture( float* data, GLuint texID, const TextureParameters& par )
{
  glBindTexture( par.texTarget, texID );
  glTexSubImage2D( par.texTarget, 0, 0, 0, _pd->m_size.width(), _pd->m_size.height(), par.texFormat, GL_FLOAT, data );
}