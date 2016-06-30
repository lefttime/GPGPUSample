#include "GPGPUSample.hpp"
#include "ui_GPGPUSample.h"

#include <QxSegyParser.hpp>
#include <QxSampleCompute.hpp>
#include <QxMultiSampleCompute.hpp>

#include <QFileDialog>
#include <QElapsedTimer>

class GPGPUSample::GPGPUSamplePrivate
{
public:

  GPGPUSamplePrivate( GPGPUSample* me ) : m_self( me ) {
    m_ui.setupUi( m_self );

    m_parser.setExtractCount( m_ui.extractCount->value() );
  }

  void doGPGPU() {
    //sampleConvolution();
    //multiSample();
  }

  void sampleConvolution() {
    GLuint unWidth = 1024;
    GLuint unHeight= 1024;
    GLuint unNoData = unWidth * unHeight;

    float* pfInput = new float[unNoData];
    float* pfOutput = new float[4*unNoData];
    for( unsigned idx = 0; idx < unNoData; ++idx ) {
      pfInput[idx] = idx;
    }

    QxSampleCompute compute( QSize(unWidth, unHeight) );
    if( compute.prepare( QList<float*>()<<pfInput, pfOutput ) ) {
      m_ui.gpuTime->setValue( compute.compute() );
    }

    delete pfOutput;
    delete pfInput;
  }

  void multiSample() {
    GLuint unWidth = 1024;
    GLuint unHeight= 1024;
    GLuint unNoData = unWidth * unHeight;

    float* pfInput = new float[unNoData];
    float* pfOutput = new float[4*unNoData];
    for( unsigned idx = 0; idx < unNoData; ++idx ) {
      pfInput[idx] = idx;
    }

    QxMultiSampleCompute compute( QSize(unWidth, unHeight) );
    if( compute.prepare( QList<float*>()<<pfInput<<pfInput, pfOutput ) ) {
      m_ui.gpuTime->setValue( compute.compute() );
    }

    delete pfOutput;
    delete pfInput;
  }

  GPGPUSample*         m_self;
  Ui::GPGPUSample      m_ui;

  QxSegyParser         m_parser;
};

GPGPUSample::GPGPUSample( QWidget* parent ) : QMainWindow( parent ), _pd( new GPGPUSamplePrivate( this ) )
{
}

GPGPUSample::~GPGPUSample()
{
}

void GPGPUSample::on_loadBtn_clicked()
{
  QString filename = QFileDialog::getOpenFileName( this,
                                                   tr( "Open File" ),
                                                   "",
                                                   tr( "SEGY Files (*.sgy *.segy)" ) );
  if( !filename.isEmpty() ) {
    _pd->m_ui.filename->setText( filename );
  }
}

void GPGPUSample::on_extractCount_valueChanged( int extractCount )
{
  _pd->m_parser.setExtractCount( extractCount );
}

void GPGPUSample::on_slices_valueChanged( int slice )
{
  qint64 cpuTime = 0;
  qint64 gpuTime = 0;
  qint64 mpiTime = 0;

  if( _pd->m_ui.cbCPU->isChecked() ) {
    _pd->m_parser.getDataByCPU( slice, cpuTime );
    _pd->m_ui.cpuCount->setValue( slice );
  }
  if( _pd->m_ui.cbGPU->isChecked() ) {
    _pd->m_parser.getDataByGPU( slice, gpuTime );
    _pd->m_ui.gpuCount->setValue( slice );
  }
  if( _pd->m_ui.cbMPI->isChecked() ) {
    _pd->m_parser.getDataByMPI( slice, mpiTime );
    _pd->m_ui.mpiCount->setValue( slice );
  }

  const QVector<float>& cpuData = _pd->m_parser.dataCPU();
  const QVector<float>& gpuData = _pd->m_parser.dataGPU();
  const QVector<float>& mpiData = _pd->m_parser.dataMPI();

  cpuData;
  gpuData;
  mpiData;

  _pd->m_ui.cpuTime->setValue( cpuTime );
  _pd->m_ui.gpuTime->setValue( gpuTime );
  _pd->m_ui.mpiTime->setValue( mpiTime );
}

void GPGPUSample::on_filename_textChanged( const QString& filename )
{
  _pd->m_parser.setFilename( filename );
  _pd->m_ui.slices->setRange( 0, _pd->m_parser.totalTraces() );
  _pd->m_ui.extractCount->setRange( 0, _pd->m_parser.totalTraces() );
}
