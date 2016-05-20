#include <QApplication>
#include "GPGPUSample.hpp"

int main( int argc, char* argv[] )
{
  QApplication app( argc, argv );

  GPGPUSample w;
  w.show();

  return app.exec();
}
