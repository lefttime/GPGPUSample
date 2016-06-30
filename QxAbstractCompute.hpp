#ifndef QXABSTRACTCOMPUTE_H
#define QXABSTRACTCOMPUTE_H

#include <QxGLWidget.hpp>

#include <QList>
#include <QSize>

struct TextureParameters {
  GLenum texTarget;
  GLenum texInternalFormat;
  GLenum texFormat;

  TextureParameters() {
    texTarget         = GL_TEXTURE_RECTANGLE;
    texInternalFormat = GL_RGBA32F;
    texFormat         = GL_RGBA;
  }

  TextureParameters( GLenum target, GLenum internalFormat, GLenum format )
    : texTarget( target ), texInternalFormat( internalFormat ), texFormat( format ) {
  }
};

class QxAbstractCompute : public QObject
{
  Q_OBJECT;

public:

  QxAbstractCompute( const TextureParameters& config, const QSize& size );
  virtual ~QxAbstractCompute();

  qint64 compute();

  virtual bool prepare( const QList<float*>& fInput, float* fOutput );

protected:

  virtual const QString& source() const=0;
  virtual void computeRoutine()=0;

  GLuint program() const;
  virtual QRect targetRect() const;

  const QSize& size() const;
  void setupTexture( const TextureParameters& info, const GLuint texID );
  void transferToTexture( float* data, GLuint texID, const TextureParameters& par );

private:

  class QxAbstractComputePrivate;
  QScopedPointer<QxAbstractComputePrivate>         _pd;
};

#endif // QXABSTRACTCOMPUTE_H
