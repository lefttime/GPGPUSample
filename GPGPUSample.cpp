#include "GPGPUSample.hpp"
#include "ui_GPGPUSample.h"

#include <QxGeneralCompute.hpp>

class GPGPUSample::GPGPUSamplePrivate
{
public:

  GPGPUSamplePrivate( GPGPUSample* me ) : m_self( me ) {
    m_ui.setupUi( m_self );
  }

  void doGPGPU() {
    m_ui.gpuTime->setValue( theGeneralCompute->compute() );
  }

  GPGPUSample*         m_self;
  Ui::GPGPUSample      m_ui;
};

GPGPUSample::GPGPUSample( QWidget* parent ) : QMainWindow( parent ), _pd( new GPGPUSamplePrivate( this ) )
{
}

GPGPUSample::~GPGPUSample()
{
}

void GPGPUSample::on_gpuBtn_clicked()
{
  _pd->doGPGPU();
}
