#include "Wahwah8.h"

//-----------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnLoad()
{	
	Bpm = 0;
	StartPos = 0;
	pos = 0;
	xBeat = 0.0f;

	bMasterFX = isMasterFX();

	DeclareParameterSlider(&SliderValue[0],ID_SLIDER_1,"Dry/Wet","D/W",1.0f);
	DeclareParameterSlider(&SliderValue[1],ID_SLIDER_2,"LFO Rate","Rate",0.5f);
	DeclareParameterSlider(&SliderValue[2],ID_SLIDER_3,"Lowpass Frequency","LPF",0.5f);
	DeclareParameterSlider(&SliderValue[3],ID_SLIDER_4,"FilterMod Depth","Depth",1.0f);
	DeclareParameterSlider(&SliderValue[4],ID_SLIDER_5,"Filter Q","Q", 0.25f);
	DeclareParameterSlider(&SliderValue[5],ID_SLIDER_6,"Phase","Phase",0.0f);
	DeclareParameterSwitch(&ShowGUI, ID_SWITCH_1, "Change GUI", "GUI", 1.0f);


	OnParameter(ID_SLIDER_1);
	OnParameter(ID_SLIDER_2);
	OnParameter(ID_SLIDER_3);
	OnParameter(ID_SLIDER_4);
	OnParameter(ID_SLIDER_5);
	OnParameter(ID_SLIDER_6);

	return S_OK;
}
//-----------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnGetPluginInfo(TVdjPluginInfo8 *infos)
{
	infos->PluginName  = "Wahwah8";
	infos->Author      = "DJ CEL";
	infos->Description = "Wah-Wah effect";
	infos->Flags = 0x00;
	
#if (defined(VDJ_WIN))
	infos->Version     = "4.0 (Win64)";
#elif (defined(VDJ_MAC))
	infos->Version     = "4.0 (macos)";
#endif
	
	return S_OK;
}
//---------------------------------------------------------------------------
ULONG VDJ_API CWahwah8::Release()
{
	delete this;
	return 0;
}
//------------------------------------------------------------------------------
// User Interface
//------------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnParameter(int id)
{	
	HRESULT hr;

	switch(id)
	{
		case ID_SWITCH_1:
			hr = SendCommand("effect_show_gui off");
			hr = SendCommand("effect_show_gui on");
			break;

		case ID_SLIDER_1:
			m_Wet = SliderValue[0];
			m_Dry = 1 - m_Wet; 
			break;
	
		case ID_SLIDER_2:
				 if (SliderValue[1]==0.0f)                               Delay=0.125f;
			else if (SliderValue[1]>0.0f     && SliderValue[1]<0.125f)   Delay=0.25f;
			else if (SliderValue[1]>=0.125f  && SliderValue[1]<0.25f)    Delay=0.5f;
			else if (SliderValue[1]>=0.25f   && SliderValue[1]<0.375f)   Delay=1.0f;
			else if (SliderValue[1]>=0.375f  && SliderValue[1]<0.5f)     Delay=2.0f;
			else if (SliderValue[1]>=0.5f    && SliderValue[1]<0.625f)   Delay=4.0f;
			else if (SliderValue[1]>=0.625   && SliderValue[1]<0.75f)    Delay=6.0f;
			else if (SliderValue[1]>=0.75f   && SliderValue[1]<0.875f)   Delay=8.0f;
			else if (SliderValue[1]>=0.875f  && SliderValue[1]<1.0f)     Delay=16.0f;
			else if (SliderValue[1]==1.0f)                               Delay=32.0f;
			
			lfofreq = 1/Delay;
			break;
		
		case ID_SLIDER_3:
			freqofs = ConvertSliderInValue(FREQOFF_MIN, SliderValue[2], FREQOFF_MAX);
			break;		
		
		case ID_SLIDER_4:
			depth = ConvertSliderInValue(DEPTH_MIN, SliderValue[3], DEPTH_MAX);
			break;

		case ID_SLIDER_5:
			Q = ConvertSliderInValue(Q_MIN, SliderValue[4], Q_MAX);
			break;

		case ID_SLIDER_6:
			phase = ConvertSliderInValue(PHASE_MIN, SliderValue[5], PHASE_MAX);
			break;
	}

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnGetParameterString(int id, char *outParam, int outParamSize) 
{
	float value = 0.0f;

	switch(id)
	{
		case ID_SLIDER_1:
			value = m_Wet * 100;
			snprintf(outParam, outParamSize,"%.0f%%", value);
			break;

		case ID_SLIDER_2:
			if (Delay==0.125f) snprintf(outParam, outParamSize,"1/8 beat");
			else if (Delay==0.25f) snprintf(outParam, outParamSize,"1/4 beat");
			else if (Delay==0.5f) snprintf(outParam, outParamSize,"1/2 beat");
			else if (Delay==1.0f) snprintf(outParam, outParamSize,"1 beat");
			else if (Delay==2.0f) snprintf(outParam, outParamSize,"2 beats");
			else if (Delay==4.0f) snprintf(outParam, outParamSize,"4 beats");
			else if (Delay==6.0f) snprintf(outParam, outParamSize,"6 beats");
			else if (Delay==8.0f) snprintf(outParam, outParamSize,"8 beats");
			else if (Delay==16.0f)snprintf(outParam, outParamSize,"16 beats");
			else if (Delay==32.0f) snprintf(outParam, outParamSize,"32 beats");
			break;

		case ID_SLIDER_3:
			snprintf(outParam, outParamSize,"%.0f Hz", Center_Frequency(freqofs));
			break;

		case ID_SLIDER_4:
			value = depth * 100;
			snprintf(outParam, outParamSize,"%.0f%%", value);
			break;

		case ID_SLIDER_5:
			snprintf(outParam, outParamSize, "%.2f", Q);
			break;

		case ID_SLIDER_6:
			snprintf(outParam, outParamSize, "+%.0f%c%c", phase, 0xC2, 0xB0);
			break;
	}

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnGetUserInterface(TVdjPluginInterface8 *pluginInterface)
{
	HRESULT hr;


if (ShowGUI == 1)
{
	pluginInterface->Type = VDJINTERFACE_SKIN;

	TRESOURCEREF rcXML = {};
	TRESOURCEREF rcPNG = {};

#if (defined(VDJ_WIN))
	rcXML.id = IDR_SKINXML;
	rcXML.type = "GUI";
	rcPNG.id = IDR_SKINPNG;
	rcPNG.type = "GUI";
#elif (defined(VDJ_MAC))
	rcXML.name = "FX_GUI.xml";
	rcPNG.name = "FX_GUI.png";
#endif

	DWORD xmlsize = 0;
	LPVOID xmldata = NULL;
	hr = LoadFileFromResource(rcXML, xmlsize, xmldata);
	pluginInterface->Xml = static_cast<char*>(xmldata);

	DWORD pngsize = 0;
	LPVOID pngdata = NULL;
	hr = LoadFileFromResource(rcPNG, pngsize, pngdata);
	pluginInterface->ImageBuffer = pngdata;
	pluginInterface->ImageSize = pngsize;
}
else
{
	pluginInterface->Type = VDJINTERFACE_DEFAULT;
}
	
	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT CWahwah8::LoadFileFromResource(TRESOURCEREF ref, DWORD &size, LPVOID &data)
{
#if (defined(VDJ_WIN))
	if (ref.id==0 || ref.type==NULL) return S_FALSE;
	HRSRC rc = FindResource(hInstance, MAKEINTRESOURCE(ref.id), ref.type);
	if (rc == NULL) return S_FALSE;
	HGLOBAL rcData = LoadResource(hInstance, rc);
	if (rcData == NULL) return S_FALSE;
	size = SizeofResource(hInstance, rc);
	data = LockResource(rcData);
#elif (defined(VDJ_MAC))
    if (ref.name.empty()) return S_FALSE;
    CFBundleRef bundle = (CFBundleRef) hInstance;
    CFStringRef resourceName = CFStringCreateWithCString(kCFAllocatorDefault, ref.name.c_str(), kCFStringEncodingUTF8);
    if (!resourceName) return S_FALSE;
    CFURLRef url = CFBundleCopyResourceURL(bundle, resourceName, NULL, NULL);
    CFRelease(resourceName);
    if (!url) return S_FALSE;

	CFStringRef path = CFURLCopyPath(url);
	CFRelease(url);

	const char *filePath = CFStringGetCStringPtr(path, kCFStringEncodingUTF8);
	std::ifstream dataFile(filePath, std::ios::binary);
	if (dataFile.is_open())
	{
		dataFile.seekg(0,std::ios::end);
		size = (DWORD)dataFile.tellg();
		data = new char[size];
		dataFile.seekg(0,std::ios::beg);
		dataFile.read((char*)data,size);
		dataFile.close();
	}
	CFRelease(path);
#endif

	return S_OK;
}
//------------------------------------------------------------------------------
// Sound processing
//------------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnStart()
{
	InitFilter();

	Bpm = SongBpm?SongBpm:(SampleRate/2); // by default 120bpm
	StartPos = int(SongPosBeats * Bpm);
  	
	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnStop()
{
	StartPos = 0;
	
	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnProcessSamples(float *buffer,int nb)
{
	 in[0] = 0.0f;
     in[1] = 0.0f;
	out[0] = 0.0f;
	out[1] = 0.0f;

	Bpm = SongBpm?SongBpm:(SampleRate/2);
	pos = int(SongPosBeats * Bpm);

	for(int i=0;i<nb;i++) // i from 0 to nb-1
	{
		xBeat = (pos+i-StartPos)/float(Bpm);  // 'x' beat(s) from StartPos

		for(int ch=0;ch<NB_CHAN;ch++) // 0: left channel, 1:right channel
		{
			in[ch] = buffer[2*i+ch];  // interleaved stereo samples (ie [left_1, right_1, left_2, right_2, ... , left_nb, right_nb]) so '2 * nb' samples in the buffer
			
			ComputeFilterCoeff(FILTER::LPF, ch, xBeat);
			
			filter(ch, FILTER_ORDER, in, out);
			
			out[ch] = m_Dry * in[ch] + m_Wet * out[ch];

			// Sound clipping (now done by VDJ)       
			// Limiter(out[ch]);

			buffer[2*i+ch] = out[ch];
		}
		
	}

	return S_OK;
}
//------------------------------------------------------------------------------
HRESULT VDJ_API CWahwah8::OnTransformPosition(double* songPos, double* videoPos, float* volume, float* srcVolume)
{

	return S_OK;
}
//------------------------------------------------------------------------------
bool CWahwah8::isMasterFX()
{
	double qRes;
	HRESULT hr = S_FALSE;

	hr = GetInfo("get_deck 'master' ? true : false", &qRes);

	if(qRes==1.0f) return true;
	else return false;
}
//------------------------------------------------------------------------------
void CWahwah8::Limiter(float sample)
{
	if (sample < -1.0f)          
		sample = float(-1.0f);       
	else if (sample > 1.0f)          
		sample = float(1.0f);
}
//------------------------------------------------------------------------------
float CWahwah8::ConvertSliderInValue(float min, float slider, float max)
{
	return  min + slider * (max - min);
}
//------------------------------------------------------------------------------
void CWahwah8::InitFilter()
{
	//(re)initialise filter
	for(int i=0;i<FILTER_ORDER+1;i++)
	{
		// Samples
		for(int ch=0;ch<NB_CHAN;ch++)
		{
			x[ch][i] = 0.0f;
			y[ch][i] = 0.0f;
		}

		// Coefficients
		if(i==0) // 1st order
		{
			a[0] = 1.0f;
			b[0] = 1.0f;
		}
		else
		{
			a[i] = 0.0f;
			b[i] = 0.0f;
		}
	}
}
//------------------------------------------------------------------------------
void CWahwah8::filter(int ch, int order, float *Input, float *Output)
{
	// Cookbook formulae for audio EQ biquad filter coefficients
	// The most straight forward implementation would be the "Direct Form 1"
	// y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
	//                  - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
    // where n-1 is the filter order, which handles both FIR and IIR filters, 
	// na is the feedback filter order 
	// nb is the feedforward filter order
			
	if(a[0] == 0.00f) // to avoid the divide by 0
	{
		Output[ch] = Input[ch];
	}
	else
	{
		x[ch][0] = Input[ch];
		
		y[ch][0] = b[0] * x[ch][0];

		for(int i=1;i<order+1;i++)
		{
			y[ch][0] += b[i] * x[ch][i];
			y[ch][0] -= a[i] * y[ch][i];
		}
		
		y[ch][0] /= a[0];
		
		
		// We save in memory the samples we need for the filter:
		for(int j=order-1;j>=0;j--)
		{
			x[ch][j+1] = x[ch][j];
			y[ch][j+1] = y[ch][j];
		}
			
		Output[ch] = y[ch][0];
	}
}
//------------------------------------------------------------------------------
void CWahwah8::ComputeFilterCoeff(FILTER type,int chan, float x)
{
	if(freqofs==1.0f)
	{
		frequency = 0.9992f;
	}
	else 
	{
		if(chan==0) // left channel
			frequency = freqofs + (1.0f - freqofs) * depth * LFO(LFOCURVE::SINE, lfofreq, x, 0); // move around freqofs
		else if(chan==1) // right channel
			frequency = freqofs + (1.0f - freqofs) * depth * LFO(LFOCURVE::SINE, lfofreq, x, phase);
	}
		
	if(chan==0 || (chan==1 && phase!=0) ) // useless to recompute RightChan coeffs when phase=0
	{
		f0 = Center_Frequency(frequency);
		
		// Filter:
		w0 = 2 * float(M_PI) * f0 / (float)SampleRate;
		sn = (float) sin(w0);
		cs = (float) cos(w0);
		
		alpha = sn / (2.0f * Q); // Q!=0 to avoid the divide by 0

		switch(type)
		{
			case FILTER::LPF: // Low pass filter: H(s) = 1 / (s^2 + s/Q + 1)
				b[0] = (1.0f - cs) / 2.0f;
				b[1] = (1.0f - cs);
				b[2] = (1.0f - cs) / 2.0f;
				a[0] = 1.0f + alpha;
				a[1] = -2.0f * cs;
				a[2] = 1.0f - alpha;
				break;

			case FILTER::HPF: // High pass filter: H(s) = s^2 / (s^2 + s/Q + 1)
				b[0] =   (1.0f + cs) / 2.0f;
				b[1] = - (1.0f + cs);
				b[2] =   (1.0f + cs) / 2.0f;
				a[0] = 1.0f + alpha;
				a[1] = -2.0f * cs;
				a[2] = 1.0f - alpha;
				break;

			case FILTER::NOTCH: // Notch filter: H(s) = (s^2 + 1) / (s^2 + s/Q + 1)
				b[0] =   1.0f;
				b[1] = - 2.0f * cs;
				b[2] =   1.0f;
				a[0] = 1.0f + alpha;
				a[1] = - 2.0f * cs;
				a[2] = 1.0f - alpha;
				break;
		}

	}
			
}
//------------------------------------------------------------------------------
float CWahwah8::LFO(LFOCURVE type, float freq, float t, float phi)
{
	double ValLFO = 0.0f;
	double phase = 2 * M_PI * double(phi) / 360.0f;
	double x = double(freq) * double(t);

	switch(type)
	{
		case LFOCURVE::SINE:
			ValLFO = (1.0f - sin(2.0f * M_PI * x + phase) ) * 0.5f;
			break;

		case LFOCURVE::SAWTOOTH:
			ValLFO = 2.0f * (x - floor(x + 0.5f));
			break;

		case LFOCURVE::TRIANGLE:
			ValLFO = 2.0f * (x - floor(x + 0.5f)) * pow(-1,floor(x + 0.5f));
			break;
	}
	
	return (float) ValLFO;
}
//------------------------------------------------------------------------------
float CWahwah8::Center_Frequency(float freq)
{
	double value = (double(freq) - 1.0f) * 6.0f;
	float f0 = (float) exp(value) * (float) SampleRate / 2.0f;
	
	return f0;
}

