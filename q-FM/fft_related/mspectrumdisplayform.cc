#include <cmath>
#include <QColorDialog>
#include <QMessageBox>
#include "mspectrumdisplayform.h"


SpectrumDisplayForm::SpectrumDisplayForm(QWidget* parent)
  : QWidget(parent)
{
  setupUi(this);

  _systemSpecifiedFlag = false;
  _intValidator = new QIntValidator(this);//The QIntValidator class provides a validator that ensures a string contains a valid integer within a specified range
  _intValidator->setBottom(0);
  _frequencyDisplayPlot = new FrequencyDisplayPlot(FrequencyPlotDisplayFrame);
  _timeDomainDisplayPlot = new TimeDomainDisplayPlot(TimeDomainDisplayFrame);
  _numRealDataPoints = 1024;
  _realFFTDataPoints = new double[_numRealDataPoints];
  _averagedValues = new double[_numRealDataPoints];
  _historyVector = new std::vector<double*>;
  
  AvgLineEdit->setRange(0, 500);                 // Set range of Average box value from 0 to 500
  MinHoldCheckBox_toggled( false );
  MaxHoldCheckBox_toggled( false );
  

  _peakFrequency = 0;
  _peakAmplitude = -HUGE_VAL;
  
  _noiseFloorAmplitude = -HUGE_VAL;

  
  _frequencyDisplayPlot->SetLowerIntensityLevel(-200);   //Initial Lower Marker value is -200
  _frequencyDisplayPlot->SetUpperIntensityLevel(-200);

  // Load up the acceptable FFT sizes...
  FFTSizeComboBox->clear();
  for(long fftSize = SpectrumGUIClass::MIN_FFT_SIZE; fftSize <= SpectrumGUIClass::MAX_FFT_SIZE; fftSize *= 2){
    FFTSizeComboBox->insertItem(FFTSizeComboBox->count(), QString("%1").arg(fftSize));
  }
  Reset();  //Average Data reset

  ToggleTabFrequency(false);
  ToggleTabTime(false);

  // Create a timer to update plots at the specified rate
  displayTimer = new QTimer(this);
  connect(displayTimer, SIGNAL(timeout()), this, SLOT(UpdateGuiTimer()));
}

SpectrumDisplayForm::~SpectrumDisplayForm()
{
  // Qt deletes children when parent is deleted

  // Don't worry about deleting Display Plots - they are deleted when parents are deleted
  delete _intValidator;

  delete[] _realFFTDataPoints;
  delete[] _averagedValues;

  for(unsigned int count = 0; count < _historyVector->size(); count++){
    delete[] _historyVector->operator[](count);
  }

  delete _historyVector;

  displayTimer->stop();
  delete displayTimer;
}

void
SpectrumDisplayForm::setSystem( SpectrumGUIClass * newSystem, 
				const uint64_t numFFTDataPoints, 
				const uint64_t numTimeDomainDataPoints )
{
  ResizeBuffers(numFFTDataPoints, numTimeDomainDataPoints);
  
  if(newSystem != NULL){
    _system = newSystem;
    _systemSpecifiedFlag = true;
  }
  else{
    _systemSpecifiedFlag = false;
  }
}

void
SpectrumDisplayForm::newFrequencyData( const SpectrumUpdateEvent* spectrumUpdateEvent)
{
  //_lastSpectrumEvent = (SpectrumUpdateEvent)(*spectrumUpdateEvent);
  const std::complex<float>* complexDataPoints = spectrumUpdateEvent->getFFTPoints();
  const uint64_t numFFTDataPoints = spectrumUpdateEvent->getNumFFTDataPoints();
  const double* realTimeDomainDataPoints = spectrumUpdateEvent->getRealTimeDomainPoints();
  const double* imagTimeDomainDataPoints = spectrumUpdateEvent->getImagTimeDomainPoints();
  const uint64_t numTimeDomainDataPoints = spectrumUpdateEvent->getNumTimeDomainDataPoints();
  const timespec dataTimestamp = spectrumUpdateEvent->getDataTimestamp();
  const bool repeatDataFlag = spectrumUpdateEvent->getRepeatDataFlag();
  const bool lastOfMultipleUpdatesFlag = spectrumUpdateEvent->getLastOfMultipleUpdateFlag();
  const timespec generatedTimestamp = spectrumUpdateEvent->getEventGeneratedTimestamp();

  // REMEMBER: The dataTimestamp is NOT valid when the repeat data flag is true...
  ResizeBuffers(numFFTDataPoints, numTimeDomainDataPoints);

  // Calculate the Magnitude of the complex point
  const std::complex<float>* complexDataPointsPtr = complexDataPoints+numFFTDataPoints/2;
  double* realFFTDataPointsPtr = _realFFTDataPoints;

  double sumMean, localPeakAmplitude, localPeakFrequency;
  const double fftBinSize = (_stopFrequency-_startFrequency) /
    static_cast<double>(numFFTDataPoints);
  localPeakAmplitude = -HUGE_VAL;
  sumMean = 0.0;

  // Run this twice to perform the fftshift operation on the data here as well
  std::complex<float> scaleFactor = std::complex<float>((float)numFFTDataPoints);
  for(uint64_t point = 0; point < numFFTDataPoints/2; point++){
    std::complex<float> pt = (*complexDataPointsPtr) / scaleFactor;
    *realFFTDataPointsPtr = 10.0*log10((pt.real() * pt.real() + pt.imag()*pt.imag()) + 1e-20);

    if(*realFFTDataPointsPtr > localPeakAmplitude) {
      localPeakFrequency = static_cast<float>(point) * fftBinSize;
      localPeakAmplitude = *realFFTDataPointsPtr;
    }
    sumMean += *realFFTDataPointsPtr;
    
    complexDataPointsPtr++;
    realFFTDataPointsPtr++;
  }
  
  // This loop takes the first half of the input data and puts it in the 
  // second half of the plotted data
  complexDataPointsPtr = complexDataPoints;
  for(uint64_t point = 0; point < numFFTDataPoints/2; point++){
    std::complex<float> pt = (*complexDataPointsPtr) / scaleFactor;
    *realFFTDataPointsPtr = 10.0*log10((pt.real() * pt.real() + pt.imag()*pt.imag()) + 1e-20);

    if(*realFFTDataPointsPtr > localPeakAmplitude) {
      localPeakFrequency = static_cast<float>(point) * fftBinSize;
      localPeakAmplitude = *realFFTDataPointsPtr;
    }
    sumMean += *realFFTDataPointsPtr;

    complexDataPointsPtr++;
    realFFTDataPointsPtr++;
  }

  // Don't update the averaging history if this is repeated data
  if(!repeatDataFlag){
    _AverageHistory(_realFFTDataPoints);

    // Only use the local info if we are not repeating data
    _peakAmplitude = localPeakAmplitude;
    _peakFrequency = localPeakFrequency;

    // calculate the spectral mean
    // +20 because for the comparison below we only want to throw out bins
    // that are significantly higher (and would, thus, affect the mean more)
    const double meanAmplitude = (sumMean / numFFTDataPoints) + 20.0;

    // now throw out any bins higher than the mean
    sumMean = 0.0;
    uint64_t newNumDataPoints = numFFTDataPoints;
    for(uint64_t number = 0; number < numFFTDataPoints; number++){
      if (_realFFTDataPoints[number] <= meanAmplitude)
        sumMean += _realFFTDataPoints[number];
      else
        newNumDataPoints--;
    }

    if (newNumDataPoints == 0)             // in the odd case that all
      _noiseFloorAmplitude = meanAmplitude; // amplitudes are equal!
    else
      _noiseFloorAmplitude = sumMean / newNumDataPoints;
  }

  if(lastOfMultipleUpdatesFlag){
    int tabindex = SpectrumTypeTab->currentIndex();
    if(tabindex == d_plot_fft) {
      _frequencyDisplayPlot->PlotNewData(_averagedValues, numFFTDataPoints, 
					 _noiseFloorAmplitude, _peakFrequency, 
					 _peakAmplitude, d_update_time);
    }

    if(tabindex == d_plot_time) {
      _timeDomainDisplayPlot->PlotNewData(realTimeDomainDataPoints, 
					  imagTimeDomainDataPoints, 
					  numTimeDomainDataPoints,
					  d_update_time);
    }
      /*
      if((QGLFormat::hasOpenGL()) && (_useOpenGL)) {
	if( _openGLWaterfall3DFlag == 1 && (tabindex == d_plot_waterfall3d)) {
	  _waterfall3DDisplayPlot->PlotNewData(_realFFTDataPoints, numFFTDataPoints, 
					       d_update_time, dataTimestamp, 
					       spectrumUpdateEvent->getDroppedFFTFrames());
	}
      }
      */
    }

    
    // Tell the system the GUI has been updated
    if(_systemSpecifiedFlag){
      _system->SetLastGUIUpdateTime(generatedTimestamp);
      _system->DecrementPendingGUIUpdateEvents();
    }
}

void
SpectrumDisplayForm::resizeEvent( QResizeEvent *e )
{
  QSize s;
  s.setWidth(FrequencyPlotDisplayFrame->width());
  s.setHeight(FrequencyPlotDisplayFrame->height());
  emit _frequencyDisplayPlot->resizeSlot(&s);

  s.setWidth(TimeDomainDisplayFrame->width());
  s.setHeight(TimeDomainDisplayFrame->height());
  emit _timeDomainDisplayPlot->resizeSlot(&s);

}

void
SpectrumDisplayForm::customEvent( QEvent * e)
{
  if(e->type() == QEvent::User+3){
    if(_systemSpecifiedFlag){
      WindowComboBox->setCurrentIndex(_system->GetWindowType());
      FFTSizeComboBox->setCurrentIndex(_system->GetFFTSizeIndex());
      //FFTSizeComboBox->setCurrentIndex(1);
    }

    // Clear any previous display
    Reset();
  }
  else if(e->type() == 10005){
    SpectrumUpdateEvent* spectrumUpdateEvent = (SpectrumUpdateEvent*)e;
    newFrequencyData(spectrumUpdateEvent);
  }
  else if(e->type() == 10008){
    setWindowTitle(((SpectrumWindowCaptionEvent*)e)->getLabel());
  }
  else if(e->type() == 10009){
    Reset();
    if(_systemSpecifiedFlag){
      _system->ResetPendingGUIUpdateEvents();
    }
  }
  else if(e->type() == 10010){
    _startFrequency = ((SpectrumFrequencyRangeEvent*)e)->GetStartFrequency();
    _stopFrequency = ((SpectrumFrequencyRangeEvent*)e)->GetStopFrequency();
    _centerFrequency  = ((SpectrumFrequencyRangeEvent*)e)->GetCenterFrequency();

    UseRFFrequenciesCB(UseRFFrequenciesCheckBox->isChecked());
  }
}

void
SpectrumDisplayForm::UpdateGuiTimer()  //when timeout, canvas()->update()
{
  // Updates the widget unless updates are disabled or the widget is hidden.

  //update() function does not cause an immediate repaint; instead it schedules a paint event for processing when Qt returns to the main event loop. This permits Qt to optimize for more speed and less flicker than a call to repaint() does.
  _frequencyDisplayPlot->canvas()->update();
  _timeDomainDisplayPlot->canvas()->update();
}


void
SpectrumDisplayForm::AvgLineEdit_valueChanged( int value )
{
  SetAverageCount(value);
}


void
SpectrumDisplayForm::MaxHoldCheckBox_toggled( bool newState )
{
  MaxHoldResetBtn->setEnabled(newState);
  _frequencyDisplayPlot->SetMaxFFTVisible(newState);
  MaxHoldResetBtn_clicked();
}


void
SpectrumDisplayForm::MinHoldCheckBox_toggled( bool newState )
{
  MinHoldResetBtn->setEnabled(newState);
  _frequencyDisplayPlot->SetMinFFTVisible(newState);
  MinHoldResetBtn_clicked();
}


void
SpectrumDisplayForm::MinHoldResetBtn_clicked()
{
  _frequencyDisplayPlot->ClearMinData();
  _frequencyDisplayPlot->replot();
}


void
SpectrumDisplayForm::MaxHoldResetBtn_clicked()
{
  _frequencyDisplayPlot->ClearMaxData();
  _frequencyDisplayPlot->replot();
}


void
SpectrumDisplayForm::TabChanged(int index)
{
  // This might be dangerous to call this with NULL
  resizeEvent(NULL);  
}

void
SpectrumDisplayForm::SetFrequencyRange(const double newCenterFrequency,
				       const double newStartFrequency, 
				       const double newStopFrequency)
{
  double fdiff;
  if(UseRFFrequenciesCheckBox->isChecked()) {
    fdiff = newCenterFrequency;
  }
  else {
    fdiff = std::max(fabs(newStartFrequency), fabs(newStopFrequency));
  }

  if(fdiff > 0) {
    std::string strunits[4] = {"Hz", "kHz", "MHz", "GHz"};
    std::string strtime[4] = {"sec", "ms", "us", "ns"};
    double units10 = floor(log10(fdiff));
    double units3  = std::max(floor(units10 / 3.0), 0.0);
    double units = pow(10, (units10-fmod(units10, 3.0)));
    int iunit = static_cast<int>(units3);
    
    _startFrequency = newStartFrequency;
    _stopFrequency = newStopFrequency;
    _centerFrequency = newCenterFrequency;

    _frequencyDisplayPlot->SetFrequencyRange(_startFrequency,
					     _stopFrequency,
					     _centerFrequency,
					     UseRFFrequenciesCheckBox->isChecked(),
					     units, strunits[iunit]);

    _timeDomainDisplayPlot->SetSampleRate(_stopFrequency - _startFrequency,
					  units, strtime[iunit]);
  }
}

int
SpectrumDisplayForm::GetAverageCount()
{
  return _historyVector->size();
}

void
SpectrumDisplayForm::SetAverageCount(const int newCount)
{
  if(newCount > -1){
    if(newCount != static_cast<int>(_historyVector->size())){
      std::vector<double*>::iterator pos;
      while(newCount < static_cast<int>(_historyVector->size())){
	pos = _historyVector->begin();
	delete[] (*pos);
	_historyVector->erase(pos);
      }

      while(newCount > static_cast<int>(_historyVector->size())){
	_historyVector->push_back(new double[_numRealDataPoints]);
      }
      AverageDataReset();
    }
  }
}

void
SpectrumDisplayForm::_AverageHistory(const double* newBuffer)
{
  if(_numRealDataPoints > 0){
    if(_historyVector->size() > 0){
      memcpy(_historyVector->operator[](_historyEntry), newBuffer,
	     _numRealDataPoints*sizeof(double));

      // Increment the next location to store data
      _historyEntryCount++;
      if(_historyEntryCount > static_cast<int>(_historyVector->size())){
	_historyEntryCount = _historyVector->size();
      }
      _historyEntry = (++_historyEntry)%_historyVector->size();

      // Total up and then average the values
      double sum;
      for(uint64_t location = 0; location < _numRealDataPoints; location++){
	sum = 0;
	for(int number = 0; number < _historyEntryCount; number++){
	  sum += _historyVector->operator[](number)[location];
	}
 	_averagedValues[location] = sum/static_cast<double>(_historyEntryCount);
      }
    }
    else{
      memcpy(_averagedValues, newBuffer, _numRealDataPoints*sizeof(double));
    }
  }
}

void
SpectrumDisplayForm::ResizeBuffers( const uint64_t numFFTDataPoints,
				    const uint64_t /*numTimeDomainDataPoints*/ )
{
  // Convert from Complex to Real for certain Displays
  if(_numRealDataPoints != numFFTDataPoints){
    _numRealDataPoints = numFFTDataPoints;
    delete[] _realFFTDataPoints;
    delete[] _averagedValues;
    
    _realFFTDataPoints = new double[_numRealDataPoints];
    _averagedValues = new double[_numRealDataPoints];
    memset(_realFFTDataPoints, 0x0, _numRealDataPoints*sizeof(double));
    
    const int historySize = _historyVector->size();
    SetAverageCount(0); // Clear the existing history
    SetAverageCount(historySize);
    
    Reset();
  }
}

void
SpectrumDisplayForm::Reset()
{
  AverageDataReset();

}


void
SpectrumDisplayForm::AverageDataReset()
{
  _historyEntry = 0;
  _historyEntryCount = 0;

  memset(_averagedValues, 0x0, _numRealDataPoints*sizeof(double));

  MaxHoldResetBtn_clicked();
  MinHoldResetBtn_clicked();
}


void
SpectrumDisplayForm::closeEvent( QCloseEvent *e )
{
  if(_systemSpecifiedFlag){
    _system->SetWindowOpenFlag(false);
  }

  qApp->processEvents();

  QWidget::closeEvent(e);
}


void
SpectrumDisplayForm::WindowTypeChanged( int newItem )
{
  if(_systemSpecifiedFlag){
   _system->SetWindowType(newItem);
  }
}


void
SpectrumDisplayForm::UseRFFrequenciesCB( bool useRFFlag )
{
  SetFrequencyRange(_centerFrequency, _startFrequency, _stopFrequency);
}


void
SpectrumDisplayForm::FFTComboBoxSelectedCB( const QString &fftSizeString )
{
  if(_systemSpecifiedFlag){
    _system->SetFFTSize(fftSizeString.toLong());
  }
}




void
SpectrumDisplayForm::ToggleTabFrequency(const bool state)  //Enable frequency tab
{
  if(state == true) {
    if(d_plot_fft == -1) {
      SpectrumTypeTab->addTab(FrequencyPage, "Frequency Display");
      d_plot_fft = SpectrumTypeTab->count()-1;
    }
  }
  else {
    SpectrumTypeTab->removeTab(SpectrumTypeTab->indexOf(FrequencyPage));
    d_plot_fft = -1;
  }
}



void
SpectrumDisplayForm::ToggleTabTime(const bool state)    //Enable Time Tab
{
  if(state == true) {
    if(d_plot_time == -1) {
      SpectrumTypeTab->addTab(TimeDomainPage, "Time Domain Display");
      d_plot_time = SpectrumTypeTab->count()-1;
    }
  }
  else {
    SpectrumTypeTab->removeTab(SpectrumTypeTab->indexOf(TimeDomainPage));
    d_plot_time = -1;
  }
}


void
SpectrumDisplayForm::SetTimeDomainAxis(double min, double max)
{
  _timeDomainDisplayPlot->set_yaxis(min, max);
}


void
SpectrumDisplayForm::SetFrequencyAxis(double min, double max)  //!!!!!!!!!!!!!!!!We can use this power min can max
{
  _frequencyDisplayPlot->set_yaxis(min, max);
}

void
SpectrumDisplayForm::SetUpdateTime(double t)
{
  d_update_time = t;
  // QTimer class takes millisecond input
  displayTimer->start(d_update_time*1000);
}
