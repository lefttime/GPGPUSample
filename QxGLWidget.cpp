#include "QxGLWidget.hpp"

class QxGLWidget::QxGLWidgetPrivate
{
public:

  QxGLWidgetPrivate( QxGLWidget* me ) : m_self( me ), m_glewInitialized( false ) {
  }

  ~QxGLWidgetPrivate() {
    if( m_glewInitialized ) {
      glDeleteFramebuffers( 1, &m_fb );
    }
  }

  void init() {
    m_self->makeCurrent();
    if( !m_glewInitialized ) {
      m_glewInitialized = (glewInit()==GLEW_OK);

      glGenFramebuffers( 1, &m_fb );
      glBindFramebuffer( GL_FRAMEBUFFER, m_fb );
    }
  }

  bool prepare( const QSize& size ) {
    if( !m_glewInitialized ) {
      return false;
    }

    m_size = size;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, m_size.width(), 0.0, m_size.height() );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glViewport( 0, 0, m_size.width(), m_size.height() );

    return true;
  }

  void doRender() {
    glFinish();

    glDrawBuffer( GL_COLOR_ATTACHMENT0 );

    glPolygonMode( GL_FRONT, GL_FILL );
    glBegin( GL_QUADS );
    {
      glTexCoord2f( 0.0f,           0.0f            ); glVertex2f( 0.0f,           0.0f            );
      glTexCoord2f( m_size.width(), 0.0f            ); glVertex2f( m_size.width(), 0.0f            );
      glTexCoord2f( m_size.width(), m_size.height() ); glVertex2f( m_size.width(), m_size.height() );
      glTexCoord2f( 0.0f,           m_size.height() ); glVertex2f( 0.0f,           m_size.height() );
    }
    glEnd();

    glFinish();
  }

  QxGLWidget*         m_self;
  bool                m_glewInitialized;

  GLuint              m_fb;
  QSize               m_size;
};

QxGLWidget* QxGLWidget::instance()
{
  static QxGLWidget* selfObject = 0;
  if( !selfObject ) {
    selfObject = new QxGLWidget;
  }

  return selfObject;
}

QxGLWidget::~QxGLWidget()
{
}

bool QxGLWidget::prepare( const QSize& size )
{
  return _pd->prepare( size );
}

void QxGLWidget::render()
{
  _pd->doRender();
}

QxGLWidget::QxGLWidget() : _pd( new QxGLWidgetPrivate( this ) )
{
  _pd->init();
}
