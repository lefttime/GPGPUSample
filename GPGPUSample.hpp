#ifndef GPGPUSample_HPP
#define GPGPUSample_HPP

#include <QMainWindow>

class GPGPUSample : public QMainWindow
{
  Q_OBJECT

public:

  explicit GPGPUSample( QWidget* parent=0 );
  ~GPGPUSample();

private Q_SLOTS:

  void on_gpuBtn_clicked();

private:

  class GPGPUSamplePrivate;
  QScopedPointer<GPGPUSamplePrivate>         _pd;
};

#endif // GPGPUSample_HPP
