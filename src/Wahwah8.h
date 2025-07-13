#ifndef WAHWAH8_H
#define WAHWAH8_H

#define _CRT_SECURE_NO_WARNINGS

#include "VdjDsp8.h"

#if (defined(VDJ_WIN))
	#include "resource.h"
#elif (defined(VDJ_MAC))
	#include <string>
	#include <fstream>
	#define sprintf_s snprintf
#endif

#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>


//---------------------------------------------------------------------------
// Class definition
//---------------------------------------------------------------------------
class CWahwah8 : public IVdjPluginPositionDsp8 
{
public:
	HRESULT OnLoad();
	HRESULT OnGetPluginInfo(TVdjPluginInfo8 *infos);
	ULONG   Release();
	HRESULT OnParameter(int id);
	HRESULT OnGetParameterString(int id, char *outParam, int outParamSize);
	HRESULT OnGetUserInterface(TVdjPluginInterface8 *pluginInterface);
	HRESULT OnStart();
	HRESULT OnStop();
	HRESULT OnProcessSamples(float *buffer,int nb);
	HRESULT OnTransformPosition(double* songPos, double* videoPos, float* volume, float* srcVolume);

private:
	typedef enum _ID_Interface
	{
		ID_SLIDER_1,
		ID_SLIDER_2,
		ID_SLIDER_3,
		ID_SLIDER_4,
		ID_SLIDER_5,
		ID_SLIDER_6,
		ID_SWITCH_1
	} ID_Interface;

	enum class LFOCURVE
	{
		SINE,
		SQUARE,
		TRIANGLE,
		SAWTOOTH
	};

	enum class FILTER
	{
		LPF,
		HPF,
		BPF,
		BPF2,
		NOTCH,
		APF,
		PEAKINGEQ,
		LOWSHELF,
		HIGHSHELF
	};

	typedef struct _TRESOURCEREF
	{
		#if (defined(VDJ_WIN))
				int id;
				char* type;
		#elif (defined(VDJ_MAC))
				std::string name;
		#endif
	} TRESOURCEREF;


	#define NB_CHAN 2
	#define FILTER_ORDER 2


	// Effect parameters range
	#define DEPTH_MIN 0.0f
	#define DEPTH_MAX 1.0f
	#define Q_MIN 0.01f  // to avoid the divide by 0 in AdjustCoeff()
	#define Q_MAX 10.0f
	#define FREQOFF_MIN 0.0f
	#define FREQOFF_MAX 1.0f
	#define PHASE_MIN 0.0f
	#define PHASE_MAX 180.0f

	bool isMasterFX();
	void filter(int ch, int order, float *Input, float *Output);
	void InitFilter();
	void ComputeFilterCoeff(FILTER type, int chan, float x);
	float ConvertSliderInValue(float min, float slider, float max);
	float LFO(LFOCURVE type,float frq,float x,float phi); // phi in degree
	float Center_Frequency(float freq);
	HRESULT LoadFileFromResource(TRESOURCEREF ref, DWORD& size, LPVOID& data);
	void Limiter(float sample);

	// Plugin Interface
	float SliderValue[6];
	int ShowGUI;

	// if FX is applied on Master
	bool bMasterFX;

	// LFO
	int Bpm;
	int StartPos;
	int pos;
	float xBeat;
	

	// Public filter parameters 
	float m_Dry,m_Wet;
	float Delay;      // LFO Delay
    	float depth;      // Wah depth 
	float freqofs;    // Wah frequency offset
	float Q;          // Resonance
	float phase;      // Phase between left and right channel (for 3D sound)

	// Private filter parameters
	float lfofreq;
	float out[NB_CHAN], in[NB_CHAN];
	float x[NB_CHAN][FILTER_ORDER+1], y[NB_CHAN][FILTER_ORDER+1];
	float a[FILTER_ORDER+1], b[FILTER_ORDER+1];
	float frequency, f0, w0, sn, cs, alpha;
};

#endif /* WAHWAH8_H */
