#include "QxGeneralCompute.hpp"

#include <QtDebug>
#include <QElapsedTimer>

#define MASK_RADIUS 2

char clean[] = {
  "void main()"
  "{"
  "  gl_FragColor=vec4(0.0, 0.0, 0.0, 0.0);"
  "}"
};

char convolution[] = {
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
  "  gl_FragColor = vec4Result;"
  "}"
};

typedef struct {
  GLenum texTarget;
  GLenum texInternalFormat;
  GLenum texFormat;
  char* shader_source;
} TextureParameters;

static bool glewInitialized = false;

class QxGeneralCompute::QxGeneralComputePrivate
{
public:

  QxGeneralComputePrivate( QxGeneralCompute* me ) : m_self( me ) {
  }

  void init() {
    m_self->makeCurrent();
    if( !glewInitialized ) {
      glewInitialized = (glewInit()==GLEW_OK);
    }

    if( glewInitialized ) {
      m_textureParameters.texTarget         = GL_TEXTURE_RECTANGLE;
      m_textureParameters.texInternalFormat = GL_RGBA32F;
      m_textureParameters.texFormat         = GL_RGBA;
    }
  }

  void initGLSL() {
    GLint status;

    m_glslProgram = glCreateProgram();
    m_fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

    const char* source = m_textureParameters.shader_source;
    glShaderSource( m_fragmentShader, 1, &source, NULL );

    glCompileShader( m_fragmentShader );
    glGetShaderiv( m_fragmentShader, GL_COMPILE_STATUS, &status );
    if( !status ) {
      char* log;
      GLint length;
      glGetShaderiv( m_fragmentShader, GL_INFO_LOG_LENGTH, &length );
      log = new char[length];
      glGetShaderInfoLog( m_fragmentShader, length, &length, log );
      qDebug() << log;
      delete log;
    }

    glAttachShader( m_glslProgram, m_fragmentShader );

    glLinkProgram( m_glslProgram );

    m_radiusParam = glGetUniformLocation( m_glslProgram, "fRadius" );
  }

  void initFBO() {
    glGenFramebuffers( 1, &m_fb );
    glBindFramebuffer( GL_FRAMEBUFFER, m_fb );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, m_width, 0.0, m_height );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glViewport( 0, 0, m_width, m_height );
  }

  void setupTexture( const GLuint texID ) {
    glBindTexture(   m_textureParameters.texTarget, texID );
    glTexParameteri( m_textureParameters.texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( m_textureParameters.texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( m_textureParameters.texTarget, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( m_textureParameters.texTarget, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexImage2D( m_textureParameters.texTarget, 0, m_textureParameters.texInternalFormat, m_width, m_height, 0, m_textureParameters.texFormat, GL_FLOAT, 0 );
  }

  void transferFromTextrue( float* data ) {
    glReadBuffer( GL_COLOR_ATTACHMENT0 );
    glReadPixels( 0, 0, m_width, m_height, m_textureParameters.texFormat, GL_FLOAT, data );
  }

  void transferToTexture( float* data, GLuint texID ) {
    glBindTexture( m_textureParameters.texTarget, texID );
    glTexSubImage2D( m_textureParameters.texTarget, 0, 0, 0, m_width, m_height, m_textureParameters.texFormat, GL_FLOAT, data );
  }

  void createTextures() {
    glGenTextures( 1, &m_yTexID );
    glGenTextures( 1, &m_xTexID );

    setupTexture( m_yTexID );
    setupTexture( m_xTexID );
    transferToTexture( m_pfInput, m_xTexID );

    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
  }

  void performComputation( char* shader_source ) {
    m_textureParameters.shader_source = shader_source;
    initGLSL();

    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_textureParameters.texTarget, m_yTexID, 0 );

    glUseProgram( m_glslProgram );

    glActiveTexture( GL_TEXTURE0 );

    glUniform1f( m_radiusParam, (float)MASK_RADIUS );

    glFinish();

    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    glPolygonMode( GL_FRONT, GL_FILL );
    glBegin( GL_QUADS );
    {
      glTexCoord2f( 0.0f,    0.0f     ); glVertex2f( 0.0f,    0.0f     );
      glTexCoord2f( m_width, 0.0f     ); glVertex2f( m_width, 0.0f     );
      glTexCoord2f( m_width, m_height ); glVertex2f( m_width, m_height );
      glTexCoord2f( 0.0f,    m_height ); glVertex2f( 0.0f,    m_height );
    }
    glEnd();

    glFinish();
  }

  void prepare( GLuint width, GLuint height, float* pfInput, float* pfOutput ) {
    m_width = width;
    m_height = height;
    m_pfInput = pfInput;
    m_pfOutput = pfOutput;

    initFBO();
    createTextures();
  }

  void release() {
    glDetachShader( m_glslProgram, m_fragmentShader );
    glDeleteShader( m_fragmentShader );
    glDeleteProgram( m_glslProgram );
    glDeleteFramebuffers( 1, &m_fb );
    glDeleteTextures( 1, &m_yTexID );
    glDeleteTextures( 1, &m_xTexID );
  }

  QxGeneralCompute*         m_self;
  TextureParameters         m_textureParameters;

  GLuint                    m_width;
  GLuint                    m_height;
  float*                    m_pfInput;
  float*                    m_pfOutput;

  GLuint                    m_fb;
  GLuint                    m_yTexID;
  GLuint                    m_xTexID;

  GLint                     m_radiusParam;
  GLuint                    m_glslProgram;
  GLuint                    m_fragmentShader;
};

QxGeneralCompute* QxGeneralCompute::instance()
{
  static QxGeneralCompute* selfObject = 0;
  if( !selfObject ) {
    selfObject = new QxGeneralCompute();
  }
  return selfObject;
}

QxGeneralCompute::~QxGeneralCompute()
{
}

qint64 QxGeneralCompute::compute() const
{
  if( !glewInitialized ) {
	return -1;
  }
	
  QElapsedTimer timer;
  timer.start();

  GLuint unWidth = 1024;
  GLuint unHeight= 1024;
  GLuint unNoData = 4 * unWidth * unHeight;

  float* pfInput = new float[unNoData];
  float* pfOutput = new float[unNoData];
  for( unsigned idx = 0; idx < unNoData; ++idx ) {
    pfInput[idx] = idx;
  }

  _pd->prepare( unWidth, unHeight, pfInput, pfOutput );

  _pd->performComputation( clean );
  _pd->performComputation( convolution );

  _pd->transferFromTextrue( pfOutput );

  _pd->release();

  delete pfOutput;
  delete pfInput;

  return timer.elapsed();
}

QxGeneralCompute::QxGeneralCompute() : _pd( new QxGeneralComputePrivate( this ) )
{
  _pd->init();
}
