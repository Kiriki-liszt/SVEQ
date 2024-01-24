//------------------------------------------------------------------------
// Copyright(c) 2024 yg331.
//------------------------------------------------------------------------

#include "SVEQ_processor.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioprocessoralgo.h"
#include "public.sdk/source/vst/vsthelpers.h"

#define SIMDE_ENABLE_NATIVE_ALIASES
#include "simde/x86/sse2.h"

using namespace Steinberg;

namespace yg331 {
//------------------------------------------------------------------------
// SVEQProcessor
//------------------------------------------------------------------------
SVEQProcessor::SVEQProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kSVEQControllerUID);
}

//------------------------------------------------------------------------
SVEQProcessor::~SVEQProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

	/* If you don't need an event bus, you can remove the next line */
	addEventInput (STR16 ("Event In"), 1);

#define make_2(t) (2 * ((t / 2) + 1))
#define make_4(t) (4 * ((t / 4) + 1))
	
	for (int channel = 0; channel < 2; channel++) {
		calcFilter(96000.0,  0.0, 24000.0, dnTap_21, 240.0, dnSample_21[channel].coef_hf);
		calcFilter(192000.0, 0.0, 24000.0, dnTap_42, 219.99, dnSample_42[channel].coef_hf);

		for (int i = 0; i < (dnTap_21 + 1) / 2; i++) {
			dnSample_21[channel].coef_hf[i] *= 2.0;
		}
		for (int i = 0; i < (dnTap_42 + 1) / 2; i++) {
			dnSample_42[channel].coef_hf[i] *= 4.0;
		}
		Band1[channel].initSVF();
		Band2[channel].initSVF();
		Band3[channel].initSVF();
		Band4[channel].initSVF();
		Band5[channel].initSVF(); 
		HP[channel].initPassFilter(); 
		LP[channel].initPassFilter(); 
	}

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!
	
	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

tresult PLUGIN_API SVEQProcessor::setBusArrangements(
	Vst::SpeakerArrangement* inputs, int32 numIns,
	Vst::SpeakerArrangement* outputs, int32 numOuts)
{
	if (numIns == 1 && numOuts == 1)
	{
		// the host wants Mono => Mono (or 1 channel -> 1 channel)
		if (Vst::SpeakerArr::getChannelCount(inputs[0]) == 1 &&
			Vst::SpeakerArr::getChannelCount(outputs[0]) == 1)
		{
			auto* bus = FCast<Vst::AudioBus>(audioInputs.at(0));
			if (bus)
			{
				// check if we are Mono => Mono, if not we need to recreate the busses
				if (bus->getArrangement() != inputs[0])
				{
					getAudioInput(0)->setArrangement(inputs[0]);
					getAudioInput(0)->setName(STR16("Mono In"));
					getAudioOutput(0)->setArrangement(outputs[0]);
					getAudioOutput(0)->setName(STR16("Mono Out"));
				}
				return kResultOk;
			}
		}
		// the host wants something else than Mono => Mono,
		// in this case we are always Stereo => Stereo
		else
		{
			auto* bus = FCast<Vst::AudioBus>(audioInputs.at(0));
			if (bus)
			{
				tresult result = kResultFalse;

				// the host wants 2->2 (could be LsRs -> LsRs)
				if (Vst::SpeakerArr::getChannelCount(inputs[0]) == 2 &&
					Vst::SpeakerArr::getChannelCount(outputs[0]) == 2)
				{
					getAudioInput(0)->setArrangement(inputs[0]);
					getAudioInput(0)->setName(STR16("Stereo In"));
					getAudioOutput(0)->setArrangement(outputs[0]);
					getAudioOutput(0)->setName(STR16("Stereo Out"));
					result = kResultTrue;
				}
				// the host want something different than 1->1 or 2->2 : in this case we want stereo
				else if (bus->getArrangement() != Vst::SpeakerArr::kStereo)
				{
					getAudioInput(0)->setArrangement(Vst::SpeakerArr::kStereo);
					getAudioInput(0)->setName(STR16("Stereo In"));
					getAudioOutput(0)->setArrangement(Vst::SpeakerArr::kStereo);
					getAudioOutput(0)->setName(STR16("Stereo Out"));
					result = kResultFalse;
				}

				return result;
			}
		}
	}
	return kResultFalse;
}
//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::process (Vst::ProcessData& data)
{
	//--- First : Read inputs parameter changes-----------

    Vst::IParameterChanges* paramChanges = data.inputParameterChanges;

	if (paramChanges)
	{
		int32 numParamsChanged = paramChanges->getParameterCount();

		for (int32 index = 0; index < numParamsChanged; index++)
		{
			Vst::IParamValueQueue* paramQueue = paramChanges->getParameterData(index);

			if (paramQueue)
			{
				Vst::ParamValue value;
				int32 sampleOffset;
				int32 numPoints = paramQueue->getPointCount();

				/*/*/
				if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue) {
					switch (paramQueue->getParameterId()) {
					case kParamLevel:      fLevel = value; break;
					case kParamOutput:     fOutput = value; break;
					case kParamBypass:     bBypass = (value > 0.5f); break;
					case kParamZoom:       fZoom = value; break;
					case kParamBand1_In:   Band1[0].fParamIn = value; break;
					case kParamBand2_In:   Band2[0].fParamIn = value; break;
					case kParamBand3_In:   Band3[0].fParamIn = value; break;
					case kParamBand4_In:   Band4[0].fParamIn = value; break;
					case kParamBand5_In:   Band5[0].fParamIn = value; break;
					case kParamBand1_dB:   Band1[0].fParamdB = value; break;
					case kParamBand2_dB:   Band2[0].fParamdB = value; break;
					case kParamBand3_dB:   Band3[0].fParamdB = value; break;
					case kParamBand4_dB:   Band4[0].fParamdB = value; break;
					case kParamBand5_dB:   Band5[0].fParamdB = value; break;
					case kParamBand1_Hz:   Band1[0].fParamHz = value; break;
					case kParamBand2_Hz:   Band2[0].fParamHz = value; break;
					case kParamBand3_Hz:   Band3[0].fParamHz = value; break;
					case kParamBand4_Hz:   Band4[0].fParamHz = value; break;
					case kParamBand5_Hz:   Band5[0].fParamHz = value; break;
					case kParamBand1_Q:    Band1[0].fParamQ = value; break;
					case kParamBand2_Q:    Band2[0].fParamQ = value; break;
					case kParamBand3_Q:    Band3[0].fParamQ = value; break;
					case kParamBand4_Q:    Band4[0].fParamQ = value; break;
					case kParamBand5_Q:    Band5[0].fParamQ = value; break;
					case kParamBand1_6dB:  Band1[0].fParam_6dB = value; break;
					case kParamBand2_6dB:  Band2[0].fParam_6dB = value; break;
					case kParamBand3_6dB:  Band3[0].fParam_6dB = value; break;
					case kParamBand4_6dB:  Band4[0].fParam_6dB = value; break;
					case kParamBand5_6dB:  Band5[0].fParam_6dB = value; break;
					case kParamBand1_type: Band1[0].fParamtype = value; Band1[0].initSVF(); Band1[1].copySVF(&Band1[0]); break;
					case kParamBand2_type: Band2[0].fParamtype = value; Band2[0].initSVF(); Band2[1].copySVF(&Band2[0]); break;
					case kParamBand3_type: Band3[0].fParamtype = value; Band3[0].initSVF(); Band3[1].copySVF(&Band3[0]); break;
					case kParamBand4_type: Band4[0].fParamtype = value; Band4[0].initSVF(); Band4[1].copySVF(&Band4[0]); break;
					case kParamBand5_type: Band5[0].fParamtype = value; Band5[0].initSVF(); Band5[1].copySVF(&Band5[0]); break;

					case kParamHP_In:   HP[0].fParamIn = value; break;
					case kParamLP_In:   LP[0].fParamIn = value; break;
					case kParamHP_Hz:   HP[0].fParamHz = value; break;
					case kParamLP_Hz:   LP[0].fParamHz = value; break;
					case kParamHP_degree: HP[0].fParamdegree = value; HP[0].initPassFilter(); HP[1].copyPassFilter(&HP[0]); break;
					case kParamLP_degree: LP[0].fParamdegree = value; LP[0].initPassFilter(); LP[1].copyPassFilter(&LP[0]); break;
					}
				}
			}
		}
	}

	if (data.numInputs == 0 || data.numOutputs == 0)
	{
		// nothing to do
		return kResultOk;
	}

	// (simplification) we suppose in this example that we have the same input channel count than
	// the output
	int32 numChannels = data.inputs[0].numChannels;

	//---get audio buffers----------------
	uint32 sampleFramesSize = getSampleFramesSizeInBytes(processSetup, data.numSamples);
	void** in = getChannelBuffersPointer(processSetup, data.inputs[0]);
	void** out = getChannelBuffersPointer(processSetup, data.outputs[0]);
	Vst::SampleRate getSampleRate = processSetup.sampleRate;

	//---check if silence---------------
	// check if all channel are silent then process silent
	if (data.inputs[0].silenceFlags == Vst::getChannelMask(data.inputs[0].numChannels))
	{
		// mark output silence too (it will help the host to propagate the silence)
		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

		// the plug-in has to be sure that if it sets the flags silence that the output buffer are
		// clear
		for (int32 i = 0; i < numChannels; i++)
		{
			// do not need to be cleared if the buffers are the same (in this case input buffer are
			// already cleared by the host)
			if (in[i] != out[i])
			{
				memset(out[i], 0, sampleFramesSize);
			}
		}
	}
	else {

		data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;

		//---in bypass mode outputs should be like inputs-----
		if (bBypass)
		{
			if (data.symbolicSampleSize == Vst::kSample32) {
				latencyBypass<Vst::Sample32>((Vst::Sample32**)in, (Vst::Sample32**)out, numChannels, getSampleRate, data.numSamples);
			}
			else if (data.symbolicSampleSize == Vst::kSample64) {
				latencyBypass<Vst::Sample64>((Vst::Sample64**)in, (Vst::Sample64**)out, numChannels, getSampleRate, data.numSamples);
			}
		}
		else {
			if (data.symbolicSampleSize == Vst::kSample32) {
				processSVF<Vst::Sample32>((Vst::Sample32**)in, (Vst::Sample32**)out, numChannels, getSampleRate, data.numSamples);
			}
			else {
				processSVF<Vst::Sample64>((Vst::Sample64**)in, (Vst::Sample64**)out, numChannels, getSampleRate, data.numSamples);
			}
		}
	}
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----

	
	if      (newSetup.sampleRate <= 48000.0) fParamOS = overSample_4x;
	else if (newSetup.sampleRate <= 96000.0) fParamOS = overSample_2x;
	else fParamOS = overSample_1x;

	if (fParamOS == overSample_4x) {
		Band1[0].fParamFs = newSetup.sampleRate * 4.0;
		Band2[0].fParamFs = newSetup.sampleRate * 4.0;
		Band3[0].fParamFs = newSetup.sampleRate * 4.0;
		Band4[0].fParamFs = newSetup.sampleRate * 4.0;
		Band5[0].fParamFs = newSetup.sampleRate * 4.0;
		HP[0].fParamFs = newSetup.sampleRate * 4.0;
		LP[0].fParamFs = newSetup.sampleRate * 4.0;
	}
	else if (fParamOS == overSample_2x) {
		Band1[0].fParamFs = newSetup.sampleRate * 2.0;
		Band2[0].fParamFs = newSetup.sampleRate * 2.0;
		Band3[0].fParamFs = newSetup.sampleRate * 2.0;
		Band4[0].fParamFs = newSetup.sampleRate * 2.0;
		Band5[0].fParamFs = newSetup.sampleRate * 2.0;
		HP[0].fParamFs = newSetup.sampleRate * 2.0;
		LP[0].fParamFs = newSetup.sampleRate * 2.0;
	}
	else {
		Band1[0].fParamFs = newSetup.sampleRate * 1.0;
		Band2[0].fParamFs = newSetup.sampleRate * 1.0;
		Band3[0].fParamFs = newSetup.sampleRate * 1.0;
		Band4[0].fParamFs = newSetup.sampleRate * 1.0;
		Band5[0].fParamFs = newSetup.sampleRate * 1.0;
		HP[0].fParamFs = newSetup.sampleRate * 1.0;
		LP[0].fParamFs = newSetup.sampleRate * 1.0;
	}

	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
	// by default kSample32 is supported
	if (symbolicSampleSize == Vst::kSample32)
		return kResultTrue;

	// disable the following comment if your processing support kSample64
	if (symbolicSampleSize == Vst::kSample64)
		return kResultTrue;

	return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::setState (IBStream* state)
{
	// called when we load a preset, the model has to be reloaded
	IBStreamer streamer (state, kLittleEndian);
	int32           savedBypass = 0;
	Vst::ParamValue savedZoom = 0.0;
	Vst::ParamValue savedLevel = 0.0;
	Vst::ParamValue savedOutput = 0.0;

	int32           savedBand1_In = 0;
	int32           savedBand2_In = 0;
	int32           savedBand3_In = 0;
	int32           savedBand4_In = 0;
	int32           savedBand5_In = 0;

	Vst::ParamValue savedBand1_dB = 0.0;
	Vst::ParamValue savedBand2_dB = 0.0;
	Vst::ParamValue savedBand3_dB = 0.0;
	Vst::ParamValue savedBand4_dB = 0.0;
	Vst::ParamValue savedBand5_dB = 0.0;
	
	Vst::ParamValue savedBand1_Hz = 0.0;
	Vst::ParamValue savedBand2_Hz = 0.0;
	Vst::ParamValue savedBand3_Hz = 0.0;
	Vst::ParamValue savedBand4_Hz = 0.0;
	Vst::ParamValue savedBand5_Hz = 0.0;
	
	Vst::ParamValue savedBand1_Q = 0.0;
	Vst::ParamValue savedBand2_Q = 0.0;
	Vst::ParamValue savedBand3_Q = 0.0;
	Vst::ParamValue savedBand4_Q = 0.0;
	Vst::ParamValue savedBand5_Q = 0.0;

	int32           savedBand1_6dB = 0;
	int32           savedBand2_6dB = 0;
	int32           savedBand3_6dB = 0;
	int32           savedBand4_6dB = 0;
	int32           savedBand5_6dB = 0;
	
	Vst::ParamValue savedBand1_type = 0.0;
	Vst::ParamValue savedBand2_type = 0.0;
	Vst::ParamValue savedBand3_type = 0.0;
	Vst::ParamValue savedBand4_type = 0.0;
	Vst::ParamValue savedBand5_type = 0.0;

	int32           savedHP_In = 0;
	int32           savedLP_In = 0;
	Vst::ParamValue savedHP_Hz = 0.0;
	Vst::ParamValue savedLP_Hz = 0.0;
	Vst::ParamValue savedHP_degree = 0.0;
	Vst::ParamValue savedLP_degree = 0.0;

	if (streamer.readInt32(savedBypass) == false) return kResultFalse;
	if (streamer.readDouble(savedZoom) == false) return kResultFalse;
	if (streamer.readDouble(savedLevel) == false) return kResultFalse;
	if (streamer.readDouble(savedOutput) == false) return kResultFalse;

	if (streamer.readInt32(savedBand1_In) == false) return kResultFalse;
	if (streamer.readInt32(savedBand2_In) == false) return kResultFalse;
	if (streamer.readInt32(savedBand3_In) == false) return kResultFalse;
	if (streamer.readInt32(savedBand4_In) == false) return kResultFalse;
	if (streamer.readInt32(savedBand5_In) == false) return kResultFalse;

	if (streamer.readDouble(savedBand1_dB) == false) return kResultFalse;
	if (streamer.readDouble(savedBand2_dB) == false) return kResultFalse;
	if (streamer.readDouble(savedBand3_dB) == false) return kResultFalse;
	if (streamer.readDouble(savedBand4_dB) == false) return kResultFalse;
	if (streamer.readDouble(savedBand5_dB) == false) return kResultFalse;
	if (streamer.readDouble(savedBand1_Hz) == false) return kResultFalse;
	if (streamer.readDouble(savedBand2_Hz) == false) return kResultFalse;
	if (streamer.readDouble(savedBand3_Hz) == false) return kResultFalse;
	if (streamer.readDouble(savedBand4_Hz) == false) return kResultFalse;
	if (streamer.readDouble(savedBand5_Hz) == false) return kResultFalse;
	if (streamer.readDouble(savedBand1_Q) == false) return kResultFalse;
	if (streamer.readDouble(savedBand2_Q) == false) return kResultFalse;
	if (streamer.readDouble(savedBand3_Q) == false) return kResultFalse;
	if (streamer.readDouble(savedBand4_Q) == false) return kResultFalse;
	if (streamer.readDouble(savedBand5_Q) == false) return kResultFalse;
	if (streamer.readInt32(savedBand1_6dB) == false) return kResultFalse;
	if (streamer.readInt32(savedBand2_6dB) == false) return kResultFalse;
	if (streamer.readInt32(savedBand3_6dB) == false) return kResultFalse;
	if (streamer.readInt32(savedBand4_6dB) == false) return kResultFalse;
	if (streamer.readInt32(savedBand5_6dB) == false) return kResultFalse;
	if (streamer.readDouble(savedBand1_type) == false) return kResultFalse;
	if (streamer.readDouble(savedBand2_type) == false) return kResultFalse;
	if (streamer.readDouble(savedBand3_type) == false) return kResultFalse;
	if (streamer.readDouble(savedBand4_type) == false) return kResultFalse;
	if (streamer.readDouble(savedBand5_type) == false) return kResultFalse;

	if (streamer.readInt32(savedHP_In) == false) return kResultFalse;
	if (streamer.readInt32(savedLP_In) == false) return kResultFalse;
	if (streamer.readDouble(savedHP_Hz) == false) return kResultFalse;
	if (streamer.readDouble(savedLP_Hz) == false) return kResultFalse;
	if (streamer.readDouble(savedHP_degree) == false) return kResultFalse;
	if (streamer.readDouble(savedLP_degree) == false) return kResultFalse;

	fZoom = savedZoom;
	bBypass = savedBypass > 0;
	fLevel = savedLevel;
	fOutput = savedOutput;

	Band1[0].fParamIn = savedBand1_In > 0;
	Band2[0].fParamIn = savedBand2_In > 0;
	Band3[0].fParamIn = savedBand3_In > 0;
	Band4[0].fParamIn = savedBand4_In > 0;
	Band5[0].fParamIn = savedBand5_In > 0;
	Band1[0].fParamdB = savedBand1_dB;
	Band1[0].fParamHz = savedBand1_Hz;
	Band1[0].fParamQ  = savedBand1_Q;
	Band2[0].fParamdB = savedBand2_dB;
	Band2[0].fParamHz = savedBand2_Hz;
	Band2[0].fParamQ  = savedBand2_Q;
	Band3[0].fParamdB = savedBand3_dB;
	Band3[0].fParamHz = savedBand3_Hz;
	Band3[0].fParamQ  = savedBand3_Q;
	Band4[0].fParamdB = savedBand4_dB;
	Band4[0].fParamHz = savedBand4_Hz;
	Band4[0].fParamQ  = savedBand4_Q;
	Band5[0].fParamdB = savedBand5_dB;
	Band5[0].fParamHz = savedBand5_Hz;
	Band5[0].fParamQ  = savedBand5_Q;
	Band1[0].fParam_6dB = savedBand1_6dB > 0;
	Band2[0].fParam_6dB = savedBand2_6dB > 0;
	Band3[0].fParam_6dB = savedBand3_6dB > 0;
	Band4[0].fParam_6dB = savedBand4_6dB > 0;
	Band5[0].fParam_6dB = savedBand5_6dB > 0;
	Band1[0].fParamtype = savedBand1_type;
	Band2[0].fParamtype = savedBand2_type;
	Band3[0].fParamtype = savedBand3_type;
	Band4[0].fParamtype = savedBand4_type;
	Band5[0].fParamtype = savedBand5_type;

	HP[0].fParamIn = savedHP_In > 0;
	LP[0].fParamIn = savedLP_In > 0;
	HP[0].fParamHz = savedHP_Hz;
	LP[0].fParamHz = savedLP_Hz;
	HP[0].fParamdegree = savedHP_degree;
	LP[0].fParamdegree = savedLP_degree;

	Band1[1].copySVF(&Band1[0]);
	Band2[1].copySVF(&Band2[0]);
	Band3[1].copySVF(&Band3[0]);
	Band4[1].copySVF(&Band4[0]);
	Band5[1].copySVF(&Band5[0]);

	HP[1].copyPassFilter(&HP[0]);
	LP[1].copyPassFilter(&LP[0]);


	if (Vst::Helpers::isProjectState(state) == kResultTrue)
	{
		// we are in project loading context...

		// Example of using the IStreamAttributes interface
		FUnknownPtr<Vst::IStreamAttributes> stream(state);
		if (stream)
		{
			if (Vst::IAttributeList* list = stream->getAttributes())
			{
				// get the full file path of this state
				Vst::TChar fullPath[1024];
				memset(fullPath, 0, 1024 * sizeof(Vst::TChar));
				if (list->getString(Vst::PresetAttributes::kFilePathStringType, fullPath,
					1024 * sizeof(Vst::TChar)) == kResultTrue)
				{
					// here we have the full path ...
				}
			}
		}
	}
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQProcessor::getState (IBStream* state)
{
	// here we need to save the model
	IBStreamer streamer (state, kLittleEndian);

	streamer.writeInt32(bBypass ? 1 : 0);
	streamer.writeDouble(fZoom);
	streamer.writeDouble(fLevel);
	streamer.writeDouble(fOutput);

	streamer.writeInt32(Band1[0].fParamIn ? 1 : 0);
	streamer.writeInt32(Band2[0].fParamIn ? 1 : 0);
	streamer.writeInt32(Band3[0].fParamIn ? 1 : 0);
	streamer.writeInt32(Band4[0].fParamIn ? 1 : 0);
	streamer.writeInt32(Band5[0].fParamIn ? 1 : 0);
	streamer.writeDouble(Band1[0].fParamdB);
	streamer.writeDouble(Band2[0].fParamdB);
	streamer.writeDouble(Band3[0].fParamdB);
	streamer.writeDouble(Band4[0].fParamdB);
	streamer.writeDouble(Band5[0].fParamdB);
	streamer.writeDouble(Band1[0].fParamHz);
	streamer.writeDouble(Band2[0].fParamHz);
	streamer.writeDouble(Band3[0].fParamHz);
	streamer.writeDouble(Band4[0].fParamHz);
	streamer.writeDouble(Band5[0].fParamHz);
	streamer.writeDouble(Band1[0].fParamQ);
	streamer.writeDouble(Band2[0].fParamQ);
	streamer.writeDouble(Band3[0].fParamQ);
	streamer.writeDouble(Band4[0].fParamQ);
	streamer.writeDouble(Band5[0].fParamQ);
	streamer.writeInt32(Band1[0].fParam_6dB ? 1 : 0);
	streamer.writeInt32(Band2[0].fParam_6dB ? 1 : 0);
	streamer.writeInt32(Band3[0].fParam_6dB ? 1 : 0);
	streamer.writeInt32(Band4[0].fParam_6dB ? 1 : 0);
	streamer.writeInt32(Band5[0].fParam_6dB ? 1 : 0);
	streamer.writeDouble(Band1[0].fParamtype);
	streamer.writeDouble(Band2[0].fParamtype);
	streamer.writeDouble(Band3[0].fParamtype);
	streamer.writeDouble(Band4[0].fParamtype);
	streamer.writeDouble(Band5[0].fParamtype);

	streamer.writeInt32(HP[0].fParamIn ? 1 : 0);
	streamer.writeInt32(LP[0].fParamIn ? 1 : 0);
	streamer.writeDouble(HP[0].fParamHz);
	streamer.writeDouble(LP[0].fParamHz);
	streamer.writeDouble(HP[0].fParamdegree);
	streamer.writeDouble(LP[0].fParamdegree);

	return kResultOk;
}

template <typename SampleType>
void SVEQProcessor::latencyBypass(
	SampleType** inputs,
	SampleType** outputs,
	int32 numChannels,
	Vst::SampleRate getSampleRate,
	long long sampleFrames
)
{
	for (int32 channel = 0; channel < numChannels; channel++)
	{
		SampleType* ptrIn = (SampleType*)inputs[channel];
		SampleType* ptrOut = (SampleType*)outputs[channel];
		int32 samples = sampleFrames;

		if (fParamOS == overSample_1x) {
			memcpy(ptrOut, ptrIn, sizeof(SampleType) * sampleFrames);
			continue;
		}

		int32 latency = 0;
		if (fParamOS == overSample_2x) latency = latency_Fir_x2;
		else if (fParamOS == overSample_4x) latency = latency_Fir_x4;

		if (latency != latency_q[channel].size()) {
			int32 diff = latency - (int32)latency_q[channel].size();
			if (diff > 0) {
				for (int i = 0; i < diff; i++) latency_q[channel].push(0.0);
			}
			else {
				for (int i = 0; i < -diff; i++) latency_q[channel].pop();
			}
		}

		while (--samples >= 0)
		{
			double inin = *ptrIn;
			*ptrOut = (SampleType)latency_q[channel].front();
			latency_q[channel].pop();
			latency_q[channel].push(inin);

			ptrIn++;
			ptrOut++;
		}
	}
	return;
}

template <typename SampleType>
void SVEQProcessor::processSVF(
	SampleType** inputs,
	SampleType** outputs,
	Steinberg::int32 numChannels,
	Steinberg::Vst::SampleRate getSampleRate,
	Steinberg::int32 sampleFrames)
{
	int32 oversampling = 1;
	if (fParamOS == overSample_2x) oversampling = 2;
	else if (fParamOS == overSample_4x) oversampling = 4;

	Vst::Sample64 level = (24.0 * fLevel) - 12.0;
	level = pow(10.0, level / 20.0);

	Band1[0].setSVF(); Band1[0].makeSVF();
	Band2[0].setSVF(); Band2[0].makeSVF();
	Band3[0].setSVF(); Band3[0].makeSVF();
	Band4[0].setSVF(); Band4[0].makeSVF();
	Band5[0].setSVF(); Band5[0].makeSVF();

	Band1[1].copySVF(&Band1[0]);
	Band2[1].copySVF(&Band2[0]);
	Band3[1].copySVF(&Band3[0]);
	Band4[1].copySVF(&Band4[0]);
	Band5[1].copySVF(&Band5[0]);

	HP[0].setPassFilter(); HP[0].makePassFilter();
	LP[0].setPassFilter(); LP[0].makePassFilter();
	HP[1].copyPassFilter(&HP[0]);
	LP[1].copyPassFilter(&LP[0]);

	for (int32 channel = 0; channel < numChannels; channel++)
	{
		int32 latency = 0;
		if (fParamOS == overSample_2x) latency = latency_Fir_x2;
		else if (fParamOS == overSample_4x) latency = latency_Fir_x4;

		if (latency != latency_q[channel].size()) {
			int32 diff = latency - (int32)latency_q[channel].size();
			if (diff > 0) for (int i = 0; i < diff; i++) latency_q[channel].push(0.0);
			else for (int i = 0; i < -diff; i++) latency_q[channel].pop();
		}


		SampleType* ptrIn =  (SampleType*)inputs[channel];
		SampleType* ptrOut = (SampleType*)outputs[channel];
		
		int32 samples = sampleFrames;
		
		while (--samples >= 0)
		{
			Vst::Sample64 inputSample = *ptrIn;

			inputSample *= level;

			//latency_q[channel].push(inputSample);

			double up_x[4] = { 0.0, };
			double up_y[4] = { 0.0, };

			up_x[0] = inputSample;
			// Process : CPU 4~5
			for (int i = 0; i < oversampling; i++)
			{
				Vst::Sample64 overSampled = up_x[i];
				
				if (HP[channel].In) overSampled = HP[channel].computePassFilter(overSampled);
				if (LP[channel].In) overSampled = LP[channel].computePassFilter(overSampled);

				if (Band1[channel].In) overSampled = Band1[channel].computeSVF(overSampled);
				if (Band2[channel].In) overSampled = Band2[channel].computeSVF(overSampled);
				if (Band3[channel].In) overSampled = Band3[channel].computeSVF(overSampled);
				if (Band4[channel].In) overSampled = Band4[channel].computeSVF(overSampled);
				if (Band5[channel].In) overSampled = Band5[channel].computeSVF(overSampled);

				up_y[i] = overSampled;
			}
			
			// Downsampling : CPU 5~6
			Vst::Sample64 tmp = 0.0;
			if (fParamOS == overSample_1x)      tmp = up_y[0];
			else if (fParamOS == overSample_2x) Fir_x2_dn(up_y, &tmp, channel);
			else if (fParamOS == overSample_4x) Fir_x4_dn(up_y, &tmp, channel);
			
			inputSample = tmp;

			//if ((fBand1_type == kHP) || (fBand1_type == kLP)) 
			//	inputSample = fBand1_dB * inputSample + latency_q[channel].front(); latency_q[channel].pop();
			
			*ptrOut = (SampleType)inputSample;

			ptrIn++;
			ptrOut++;
		}
		
	}
	return;
}

// 2 in 1 out
void SVEQProcessor::Fir_x2_dn(Vst::Sample64* in, Vst::Sample64* out, int32 channel)
{
	double inter_21[2];


	// SSE + Symmetry
	constexpr auto half_tap = dnTap_21 / 2;
	constexpr auto steps = 2;
	constexpr auto half_tap_up = half_tap - steps + 1;
	constexpr auto half_tap_dn = half_tap - steps + 2;

	constexpr auto _100_192 = sizeof(double) * half_tap_up;
	memmove(dnSample_21[channel].buff_dn + steps, dnSample_21[channel].buff_dn, _100_192);

	constexpr auto _96_99 = sizeof(double) * steps;
	memmove(dnSample_21[channel].buff_dn, dnSample_21[channel].buff_up + 1 + half_tap, _96_99);

	constexpr auto _0_95 = sizeof(double) * half_tap;
	memmove(dnSample_21[channel].buff_up + 1 + steps, dnSample_21[channel].buff_up + 1, _0_95);

	dnSample_21[channel].buff_up[2] = in[0];
	dnSample_21[channel].buff_up[1] = in[1];

	__m128d _acc_out_a = _mm_setzero_pd();
	for (int i = 0; i < half_tap; i += 2) {
		__m128d coef_a = _mm_load_pd(&dnSample_21[channel].coef_hf[i]);
		__m128d buff_a = _mm_load_pd(&dnSample_21[channel].buff_up[i + steps]);
		__m128d buff_b = _mm_loadr_pd(&dnSample_21[channel].buff_dn[half_tap_dn-2 - i]);
		buff_a = _mm_add_pd(buff_a, buff_b);
		__m128d _mul_a = _mm_mul_pd(coef_a, buff_a);
		_acc_out_a = _mm_add_pd(_acc_out_a, _mul_a);
	}
	_mm_store_pd(inter_21, _acc_out_a);
	*out = inter_21[0] + inter_21[1] + dnSample_21[channel].coef_hf[half_tap] * dnSample_21[channel].buff_up[half_tap + steps];
}
// 4 in 1 out
void SVEQProcessor::Fir_x4_dn(Vst::Sample64* in, Vst::Sample64* out, int32 channel)
{
	double inter_42[4];

	// SSE + Symmetry
constexpr auto half_tap = dnTap_42 / 2;
constexpr auto steps = 4;
constexpr auto half_tap_up = half_tap - steps + 1;
constexpr auto half_tap_dn = half_tap - steps + 2;

constexpr auto _100_192 = sizeof(double) * half_tap_up;
	memmove(dnSample_42[channel].buff_dn + steps, dnSample_42[channel].buff_dn, _100_192);

constexpr auto _96_99 = sizeof(double) * steps;
	memmove(dnSample_42[channel].buff_dn, dnSample_42[channel].buff_up + 1 + half_tap, _96_99);

constexpr auto _0_95 = sizeof(double) * half_tap;
	memmove(dnSample_42[channel].buff_up + 1 + steps, dnSample_42[channel].buff_up + 1, _0_95);

	dnSample_42[channel].buff_up[4] = in[0];
	dnSample_42[channel].buff_up[3] = in[1];
	dnSample_42[channel].buff_up[2] = in[2];
	dnSample_42[channel].buff_up[1] = in[3];

	__m128d _acc_out_a = _mm_setzero_pd();
	for (int i = 0; i < half_tap; i += 2) {
		__m128d coef_a = _mm_load_pd(&dnSample_42[channel].coef_hf[i]);
		__m128d buff_a = _mm_load_pd(&dnSample_42[channel].buff_up[i + steps]);
		__m128d buff_b = _mm_loadr_pd(&dnSample_42[channel].buff_dn[half_tap_dn - i]);
		buff_a = _mm_add_pd(buff_a, buff_b);
		__m128d _mul_a = _mm_mul_pd(coef_a, buff_a);
		_acc_out_a = _mm_add_pd(_acc_out_a, _mul_a);
	}
	_mm_store_pd(inter_42, _acc_out_a);
	*out = inter_42[0] + inter_42[1] + dnSample_42[channel].coef_hf[half_tap] * dnSample_42[channel].buff_up[half_tap + steps];
	return;	
	
}
//------------------------------------------------------------------------
} // namespace yg331
