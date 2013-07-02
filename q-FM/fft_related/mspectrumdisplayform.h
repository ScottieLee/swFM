#ifndef SPECTRUM_DISPLAY_FORM_H
#define SPECTRUM_DISPLAY_FORM_H

#include "ui_mspectrumdisplayform.h"

class SpectrumGUIClass;

#include "mSpectrumGUIClass.h"
#include "mFrequencyDisplayPlot.h"
#include "mTimeDomainDisplayPlot.h"
#include <QValidator>
#include <QTimer>
#include <vector>


//Controller of UI spectrumdisplayform. It use DisplayPlot instances to plot data.
class SpectrumDisplayForm : public QWidget, public Ui::SpectrumDisplayForm
{
  Q_OBJECT

  public:
  SpectrumDisplayForm(QWidget* parent = 0);
  ~SpectrumDisplayForm();
  
  void setSystem( SpectrumGUIClass * newSystem, const uint64_t numFFTDataPoints,
		  const uint64_t numTimeDomainDataPoints );

  int GetAverageCount();
  void SetAverageCount( const int newCount );
  void Reset();
  void AverageDataReset();
  void ResizeBuffers( const uint64_t numFFTDataPoints,
		      const uint64_t numTimeDomainDataPoints );
  
public slots:
  void resizeEvent( QResizeEvent * e );
  void customEvent( QEvent * e );
  void AvgLineEdit_valueChanged( int valueString );
  void MaxHoldCheckBox_toggled( bool newState );
  void MinHoldCheckBox_toggled( bool newState );
  void MinHoldResetBtn_clicked();
  void MaxHoldResetBtn_clicked();
  void TabChanged(int index);

  void SetFrequencyRange( const double newCenterFrequency,
			  const double newStartFrequency,
			  const double newStopFrequency );
  void closeEvent( QCloseEvent * e );
  void WindowTypeChanged( int newItem );
  void UseRFFrequenciesCB( bool useRFFlag );
  void FFTComboBoxSelectedCB(const QString&);

  void ToggleTabFrequency(const bool state);
  void ToggleTabTime(const bool state);


  void SetTimeDomainAxis(double min, double max);
  void SetFrequencyAxis(double min, double max);
  void SetUpdateTime(double t);

private slots:
  void newFrequencyData( const SpectrumUpdateEvent* );
  void UpdateGuiTimer();

protected:

private:
  void _AverageHistory( const double * newBuffer );

  int _historyEntryCount;
  int _historyEntry;
  std::vector<double*>* _historyVector;
  double* _averagedValues;
  uint64_t _numRealDataPoints;
  double* _realFFTDataPoints;
  QIntValidator* _intValidator;
  FrequencyDisplayPlot* _frequencyDisplayPlot;
  TimeDomainDisplayPlot* _timeDomainDisplayPlot;
  SpectrumGUIClass* _system;
  bool _systemSpecifiedFlag;
  double _centerFrequency;
  double _startFrequency;
  double _noiseFloorAmplitude;
  double _peakFrequency;
  double _peakAmplitude;
  double _stopFrequency;
  
  //SpectrumUpdateEvent _lastSpectrumEvent;
  
  // whether or not to use a particular display
  int d_plot_fft;
  int d_plot_time;


  QTimer *displayTimer;
  double d_update_time;
};

#endif /* SPECTRUM_DISPLAY_FORM_H */
