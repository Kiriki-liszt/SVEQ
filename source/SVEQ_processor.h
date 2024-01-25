//------------------------------------------------------------------------
// Copyright(c) 2024 yg331.
//------------------------------------------------------------------------

#pragma once

#include "SVEQ_cids.h"

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/base/futils.h"

#include <queue>
using namespace Steinberg;
namespace yg331 {

//------------------------------------------------------------------------
//  SVEQProcessor
//------------------------------------------------------------------------
class SVEQProcessor : public Steinberg::Vst::AudioEffect
{
public:
	SVEQProcessor ();
	~SVEQProcessor () SMTG_OVERRIDE;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/) 
	{ 
		return (Steinberg::Vst::IAudioProcessor*)new SVEQProcessor; 
	}

	//--- ---------------------------------------------------------------------
	// AudioEffect overrides:
	//--- ---------------------------------------------------------------------
	/** Called at first after constructor */
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	
	/** Called at the end before destructor */
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;
	
	Steinberg::tresult PLUGIN_API setBusArrangements(
		Steinberg::Vst::SpeakerArrangement* inputs, int32 numIns,
		Steinberg::Vst::SpeakerArrangement* outputs, int32 numOuts
	) SMTG_OVERRIDE;

	/** Switch the Plug-in on/off */
	Steinberg::tresult PLUGIN_API setActive (Steinberg::TBool state) SMTG_OVERRIDE;

	/** Will be called before any process call */
	Steinberg::tresult PLUGIN_API setupProcessing (Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
	
	/** Asks if a given sample size is supported see SymbolicSampleSizes. */
	Steinberg::tresult PLUGIN_API canProcessSampleSize (Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

	/** Gets the current Latency in samples. */
	Steinberg::uint32 PLUGIN_API getLatencySamples() SMTG_OVERRIDE;

	/** Here we go...the process call */
	Steinberg::tresult PLUGIN_API process (Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		
	/** For persistence */
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;

//------------------------------------------------------------------------
protected:
	template <typename SampleType>
	void processSVF (SampleType** inputs,
					 SampleType** outputs,
					 Steinberg::int32 numChannels,
					 Steinberg::Vst::SampleRate getSampleRate,
					 Steinberg::int32 sampleFrames );
	template <typename SampleType>
	void latencyBypass(SampleType** inputs, SampleType** outputs, Steinberg::int32 numChannels, Steinberg::Vst::SampleRate getSampleRate, long long sampleFrames);

	void Fir_x2_dn(Steinberg::Vst::Sample64* in, Steinberg::Vst::Sample64* out, int32 channel);
	void Fir_x4_dn(Steinberg::Vst::Sample64* in, Steinberg::Vst::Sample64* out, int32 channel);

	bool            bBypass = false;
	Steinberg::Vst::ParamValue fLevel = 0.5;
	Steinberg::Vst::ParamValue fOutput = 0.0;
	Steinberg::Vst::ParamValue fZoom = 2.0 / 6.0;

	overSample      fParamOS = overSample_4x;

	SVF Band1[2] = {(SVF::Init_Band1_Hz()), (SVF::Init_Band1_Hz())};
	SVF Band2[2] = {(SVF::Init_Band2_Hz()), (SVF::Init_Band2_Hz())};
	SVF Band3[2] = {(SVF::Init_Band3_Hz()), (SVF::Init_Band3_Hz())};
	SVF Band4[2] = {(SVF::Init_Band4_Hz()), (SVF::Init_Band4_Hz())};
	SVF Band5[2] = {(SVF::Init_Band5_Hz()), (SVF::Init_Band5_Hz())};
	PassFilter HP[2] = { PassFilter(PassFilter::Init_HP_Hz(), SVF::kHP), PassFilter(PassFilter::Init_HP_Hz(), SVF::kHP)};
	PassFilter LP[2] = { PassFilter(PassFilter::Init_LP_Hz(), SVF::kLP), PassFilter(PassFilter::Init_LP_Hz(), SVF::kLP)};


	SVF BP[2] = {(SVF::Init_Band5_Hz()), (SVF::Init_Band5_Hz())};
	double savg[20];

#define maxTap 512
#define halfTap 256
#define dnTap_21 49
#define dnTap_41 103
#define _delay41 28
#define dnTap_42 225

	inline double Ino(double x)
	{
		double d = 0, ds = 1, s = 1;
		do
		{
			d += 2;
			ds *= x * x / (d * d);
			s += ds;
		} while (ds > s * 1e-6);
		return s;
	}

	void calcFilter(double Fs, double Fa, double Fb, int M, double Att, double* dest)
	{
		// Kaiser windowed FIR filter "DIGITAL SIGNAL PROCESSING, II" IEEE Press pp 123-126.

		int Np = (M - 1) / 2;
		double A[maxTap] = { 0, };
		double Alpha;
		double Inoalpha;
		//double H[maxTap] = { 0, };

		A[0] = 2 * (Fb - Fa) / Fs;

		for (int j = 1; j <= Np; j++)
			A[j] = (sin(2.0 * j * M_PI * Fb / Fs) - sin(2.0 * j * M_PI * Fa / Fs)) / (j * M_PI);

		if (Att < 21.0)
			Alpha = 0;
		else if (Att > 50.0)
			Alpha = 0.1102 * (Att - 8.7);
		else
			Alpha = 0.5842 * pow((Att - 21), 0.4) + 0.07886 * (Att - 21);

		Inoalpha = Ino(Alpha);

		for (int j = 0; j <= Np; j++)
			dest[Np - j] = A[j] * Ino(Alpha * sqrt(1.0 - ((double)j * (double)j / ((double)Np * (double)Np)))) / Inoalpha;
		/*
		for (int j = 0; j < Np; j++)
			H[j] = H[M - 1 - j];
		*/
	};

	typedef struct _Flt {
		//double alignas(16) coef[maxTap] = { 0, };
		double alignas(16) coef_hf[halfTap] = { 0, };
		//double alignas(16) buff[maxTap] = { 0, };
		double alignas(16) buff_up[halfTap] = { 0, };
		double alignas(16) buff_dn[halfTap] = { 0, };
	} Flt;

	Flt dnSample_21[2];
	//Flt dnSample_41[2];
	Flt dnSample_42[2];

	int32 latency_Fir_x2 = 12;
	int32 latency_Fir_x4 = _delay41;

	std::queue<double> latency_q[2];
};

//------------------------------------------------------------------------
} // namespace yg331
