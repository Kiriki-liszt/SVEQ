//------------------------------------------------------------------------
// Copyright(c) 2024 yg331.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

#include "pluginterfaces/base/futils.h"
#include <vector>
#include <cmath>
#include <algorithm>
#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#endif
#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880   // sqrt(2)
#endif


enum {
	kParamBypass = 0,
	kParamZoom,

	kParamLevel,
	kParamOutput,

	kParamBand1_In,
	kParamBand2_In,
	kParamBand3_In,
	kParamBand4_In,
	kParamBand5_In,

	kParamBand1_dB,
	kParamBand2_dB,
	kParamBand3_dB,
	kParamBand4_dB,
	kParamBand5_dB,

	kParamBand1_Hz,
	kParamBand2_Hz,
	kParamBand3_Hz,
	kParamBand4_Hz,
	kParamBand5_Hz,

	kParamBand1_Q,
	kParamBand2_Q,
	kParamBand3_Q,
	kParamBand4_Q,
	kParamBand5_Q,

	kParamBand1_6dB,
	kParamBand2_6dB,
	kParamBand3_6dB,
	kParamBand4_6dB,
	kParamBand5_6dB,

	kParamBand1_type,
	kParamBand2_type,
	kParamBand3_type,
	kParamBand4_type,
	kParamBand5_type,

	kParamHP_In,
	kParamLP_In,
	kParamHP_Hz,
	kParamLP_Hz,
	kParamHP_degree,
	kParamLP_degree
};
typedef enum {
	overSample_1x,
	overSample_2x,
	overSample_4x,
	overSample_8x,
	overSample_num = 3
} overSample;

class SVF {
public:
	enum filter_type
	{
		kBell,
		kLS,
		kHP,
		kHS,
		//kBP,
		kLP,
		kNotch,

		kFltNum = 5
	};

	SVF() :
		dB(0.5), Hz(1.0), Q(1.0), Fs(192000.0),
		gt0(0.0), gt1(0.0), gt2(0.0), gk0(0.0), gk1(0.0),
		m0(0.0), m1(0.0), m2(0.0),
		v0(0.0), v1(0.0), v2(0.0),
		t0(0.0), t1(0.0), t2(0.0),
		ic1eq(0.0), ic2eq(0.0), type(kBell)
	{
		setSVF();
		initSVF();
	};

	SVF(double _Hz) :
		dB(0.5), Hz(_Hz), Q(1.0), Fs(192000.0),
		gt0(0.0), gt1(0.0), gt2(0.0), gk0(0.0), gk1(0.0),
		m0(0.0), m1(0.0), m2(0.0),
		v0(0.0), v1(0.0), v2(0.0),
		t0(0.0), t1(0.0), t2(0.0),
		ic1eq(0.0), ic2eq(0.0), type(kBell)
	{
		fParamHz = _Hz_to_norm(_Hz);
		setSVF();
		initSVF();
	};

	void initSVF() { ic1eq = 0.0; ic2eq = 0.0; };

	void copySVF(SVF* src)
	{
		this->fParamIn = src->fParamIn;
		this->fParamdB = src->fParamdB;
		this->fParamHz = src->fParamHz;
		this->fParamQ = src->fParamQ;
		this->fParam_6dB = src->fParam_6dB;
		this->fParamtype = src->fParamtype;
		this->fParamFs = src->fParamFs;

		this->In = src->In;
		this->dB = src->dB;
		this->Hz = src->Hz;
		this->Q = src->Q;
		this->_6dB = src->_6dB;
		this->type = src->type;
		this->Fs = src->Fs;

		this->A = src->A;
		this->w = src->w;
		this->g = src->g;
		this->k = src->k;
		this->s = src->s;
		this->gt0 = src->gt0;
		this->gt1 = src->gt1;
		this->gt2 = src->gt2;
		this->gk0 = src->gk0;
		this->gk1 = src->gk1;
		this->m0 = src->m0;
		this->m1 = src->m1;
		this->m2 = src->m2;
		return;
	}

	void setSVF()
	{
		In = fParamIn ? 1 : 0;

		dB = (24.0 * fParamdB) - 12.0;

		Hz = _norm_to_Hz(fParamHz);

		double Q_LOG_MAX = log(24.0 / 0.5);
		double tmq = 0.5 * exp(Q_LOG_MAX * fParamQ);
		Q = std::max(std::min(tmq, 24.0), 0.5);

		if (type == kLP || type == kHP || type == kLS || type == kHS) _6dB = fParam_6dB ? 1 : 0;
		else _6dB = 0;

		type = _norm_to_type(fParamtype);

		Fs = fParamFs;
	}

	void makeSVF()
	{
		A = pow(10.0, dB / 40.0);
		w = Hz * M_PI / Fs;
		g = tan(w);
		double mm = exp(-0.0575 * abs(dB));
		double bk = 1 / (Q * mm);
		k = 2.0 / Q;
		s = M_SQRT2 / log2(Q*0.5 + 1);

		double kdA = bk / A;
		double kmA = bk * A;
		double smA = s * A;
		double gdA = g / sqrt(A);
		double gmA = g * sqrt(A);
		double AmA = A * A;

		// if listen selected, band pass it + Q-proportinal bell

		switch (type)
		{
		case kLP:    m0 = 0;   m1 = 0;   m2 = 1;   break;
		case kHP:    m0 = 1;   m1 = 0;   m2 = 0;   break;
		//case kBP:    m0 = 0;   m1 = 1;   m2 = 0;   break;
		case kNotch: m0 = 1;   m1 = 0;   m2 = 1;   break;
		case kBell:  g = g;   k = kdA; m0 = 1;   m1 = kmA; m2 = 1;   break;
		case kLS:    g = gdA; k = s;   m0 = 1;   m1 = smA; m2 = AmA; break;
		case kHS:    g = gmA; k = s;   m0 = AmA; m1 = smA; m2 = 1;   break;
		default: break;
		}

		if (_6dB) {
			if (type == kLS) { g = tan(w) / A; m0 = 1;   m1 = 0; m2 = AmA; }
			if (type == kHS) { g = tan(w) * A; m0 = AmA; m1 = 0; m2 = 1; }
			k = 1 - g;
		}

		gt0 = 1 / (1 + g * (g + k));
		gk0 = (g + k) * gt0;
		gt1 = g * gt0;
		gk1 = g * gk0;
		gt2 = g * gt1;
		return;
	};


	Steinberg::Vst::Sample64 computeSVF
	(Steinberg::Vst::Sample64 input)
	{
		if (_6dB) {
			v1 = input;
			v2 = gt1 * input + gt0 * ic1eq;
			v0 = gt0 * input - gt0 * ic2eq;

			ic1eq += 2.0 * g * (input - v2);
			ic2eq += 2.0 * g * v0;

			return m0 * v0 + /* m1 * v1 + */ m2 * v2;
		}

		t0 = input - ic2eq;
		v0 = gt0 * t0 - gk0 * ic1eq;
		t1 = gt1 * t0 - gk1 * ic1eq;
		t2 = gt2 * t0 + gt1 * ic1eq;
		v1 = t1 + ic1eq;
		v2 = t2 + ic2eq;
		ic1eq += 2.0 * t1;
		ic2eq += 2.0 * t2;

		return m0 * v0 + m1 * v1 + m2 * v2;
	};

	double mag_response(double freq) {
		if (!In) return 1.0;

		double ONE_OVER_SAMPLE_RATE = 1.0 / Fs;

		// exp(complex(0.0, -2.0 * pi) * frequency / sampleRate)
		double _zr = (0.0) * freq * ONE_OVER_SAMPLE_RATE;
		double _zi = (-2.0 * M_PI) * freq * ONE_OVER_SAMPLE_RATE;

		// z = zr + zi;
		double zr = exp(_zr) * cos(_zi);
		double zi = exp(_zr) * sin(_zi);

		double nr = 0, ni = 0;
		double dr = 0, di = 0;

		if (_6dB != 0) {
			// Numerator complex
			nr = zr * (-m0 /* + m1 * (g - 1) */ + m2 * g) + (m0 /* + m1 * (g + 1) */ + m2 * g);
			ni = zi * (-m0 /* + m1 * (g - 1) */ + m2 * g);

			// Denominator complex
			dr = zr * (g - 1) + (g + 1);
			di = zi * (g - 1);
		}
		else {
			// z * z
			double zsq_r = zr * zr - zi * zi;
			double zsq_i = zi * zr + zr * zi;

			double gsq = g * g;

			// Numerator complex
			double c_nzsq = (m0 + m1 * g + m2 * gsq);
			double c_nz = (m0 * -2 + m2 * 2.0 * gsq);
			double c_n = (m0 + m1 * -g + m2 * gsq);
			nr = zsq_r * c_nzsq + zr * c_nz + c_n;
			ni = zsq_i * c_nzsq + zi * c_nz;

			// Denominator complex
			double c_dzsq = (1 + k * g + gsq);
			double c_dz = (-2 + 2.0 * gsq);
			double c_d = (1 + k * -g + gsq);
			dr = zsq_r * c_dzsq + zr * c_dz + c_d;
			di = zsq_i * c_dzsq + zi * c_dz;
		}

		// Numerator / Denominator
		double norm = dr * dr + di * di;
		double ddr = (nr * dr + ni * di) / norm;
		double ddi = (ni * dr - nr * di) / norm;

		return sqrt(ddr * ddr + ddi * ddi);
	}


	static double getFreqMax() { return 41000.0; };
	static double getFreqMin() { return 20.0; };
	static double getdBMax() { return 12.0; };
	static double getdBMin() { return -12.0; };
	static double getQMax() { return 24.0; };
	static double getQMin() { return 0.5; };

	static double Init_Band1_Hz() { return 80.0; };
	static double Init_Band2_Hz() { return 200.0; };
	static double Init_Band3_Hz() { return 2000.0; };
	static double Init_Band4_Hz() { return 6000.0; };
	static double Init_Band5_Hz() { return 16000.0; };

	static filter_type _norm_to_type(Steinberg::Vst::ParamValue value) 
	{
		return static_cast<filter_type>(Steinberg::FromNormalized<Steinberg::Vst::ParamValue>(value, kFltNum));
	};
	static Steinberg::Vst::ParamValue _type_to_norm(filter_type type) 
	{
		return Steinberg::ToNormalized<Steinberg::Vst::ParamValue>(type, kFltNum);
	};
	static Steinberg::Vst::ParamValue _dB_to_norm(double _dB)
	{
		return (_dB + getdBMax()) / (getdBMax() - getdBMin());
	};
	static Steinberg::Vst::ParamValue _Hz_to_norm(double _Hz)
	{
		return log(_Hz / getFreqMin()) / log(getFreqMax() / getFreqMin());
	};
	static double _norm_to_Hz(Steinberg::Vst::ParamValue paramValue)
	{
		double FREQ_LOG_MAX = log(getFreqMax() / getFreqMin());
		double tmp = getFreqMin() * exp(FREQ_LOG_MAX * paramValue);
		return std::max(std::min(tmp, getFreqMax()), getFreqMin());
	};
	static Steinberg::Vst::ParamValue _Q_to_norm(double _Q)
	{
		return log(_Q / getQMin()) / log(getQMax() / getQMin());
	};

	Steinberg::Vst::ParamValue fParamIn = 1.0;
	Steinberg::Vst::ParamValue fParamdB = _dB_to_norm(0.0);
	Steinberg::Vst::ParamValue fParamHz = _Hz_to_norm(1000.0);
	Steinberg::Vst::ParamValue fParamQ = _Q_to_norm(1.414);
	Steinberg::Vst::ParamValue fParam_6dB = 0.0;
	Steinberg::Vst::ParamValue fParamtype = _type_to_norm(kBell);
	Steinberg::Vst::Sample64   fParamFs = 192000.0;
	
	filter_type type;
	int In = 1;

private:
	double dB;
	double Hz;
	double Q;
	double Fs;
	int _6dB;

	double A, w, g, k, s;
	double gt0, gt1, gt2;
	double gk0, gk1;
	double m0, m1, m2;

	double v0, v1, v2;
	double t0, t1, t2;
	double ic1eq;
	double ic2eq;
};

class PassFilter{
public:

	enum filter_degree
	{
		_6dBoct,
		_12dBoct,
		_18dBoct,
		_24dBoct,
		_36dBoct,
		kDegreeNum = 4
	};

	PassFilter(double _Hz, SVF::filter_type _type)
	{
		if (_type == SVF::kHP) fParamHz = _Hz_to_norm_HP(_Hz);
		else fParamHz = _Hz_to_norm_LP(_Hz);
		_6dB_1.fParamHz = fParamHz;
		_12dB_1.fParamHz = fParamHz;
		_12dB_2.fParamHz = fParamHz;
		_12dB_3.fParamHz = fParamHz;

		fParamtype = SVF::_type_to_norm(_type);
		_6dB_1.fParamtype = fParamtype;
		_12dB_1.fParamtype = fParamtype;
		_12dB_2.fParamtype = fParamtype;
		_12dB_3.fParamtype = fParamtype;

		_6dB_1.fParam_6dB = 1;
		_12dB_1.fParam_6dB = 0;
		_12dB_2.fParam_6dB = 0;
		_12dB_3.fParam_6dB = 0;

		fParamFs = 192000.0;
	};

	void initPassFilter() {
		_6dB_1.initSVF();
		_12dB_1.initSVF();
		_12dB_2.initSVF();
		_12dB_3.initSVF();
	};

	void copyPassFilter(PassFilter* src)
	{
		this->fParamdegree = src->fParamdegree;
		this->fParamHz = src->fParamHz;
		this->fParamIn = src->fParamIn;
		this->fParamtype = src->fParamtype;
		this->fParamFs = src->fParamFs;
		this->degree = src->degree;
		this->In = src->In;
	
		_6dB_1.copySVF(&(src->_6dB_1));
		_12dB_1.copySVF(&(src->_12dB_1));
		_12dB_2.copySVF(&(src->_12dB_2));
		_12dB_3.copySVF(&(src->_12dB_3));

		//setPassFilter();
	}

	void setPassFilter()
	{
		In = fParamIn ? 1 : 0;
		degree = _norm_to_degree(fParamdegree);

		_6dB_1.fParamIn = fParamIn;
		_12dB_1.fParamIn = fParamIn;
		_12dB_2.fParamIn = fParamIn;
		_12dB_3.fParamIn = fParamIn;

		double __Hz = 0;
		if (SVF::_norm_to_type(fParamtype) == SVF::kHP) __Hz = SVF::_Hz_to_norm(_norm_to_Hz_HP(fParamHz));
		else __Hz = SVF::_Hz_to_norm(_norm_to_Hz_LP(fParamHz));
		_6dB_1.fParamHz = __Hz;
		_12dB_1.fParamHz = __Hz;
		_12dB_2.fParamHz = __Hz;
		_12dB_3.fParamHz = __Hz;

		switch (degree)
		{
		case PassFilter::_6dBoct:
			break;
		case PassFilter::_12dBoct:
			_12dB_1.fParamQ = SVF::_Q_to_norm(M_SQRT2);
			break;
		case PassFilter::_18dBoct:
			_12dB_1.fParamQ = SVF::_Q_to_norm(2.0);
			break;
		case PassFilter::_24dBoct:
			#define _24dBoct_1 1.08239220029239402443 // sqrt(2) * sqrt(2 - sqrt(2))
			#define _24dBoct_2 2.61312592975275315155 // sqrt(2) * sqrt(2 + sqrt(2))
			_12dB_1.fParamQ = SVF::_Q_to_norm(_24dBoct_1);
			_12dB_2.fParamQ = SVF::_Q_to_norm(_24dBoct_2);
			break;
		case PassFilter::_36dBoct:
			#define _36dBoct_1 1.03527618041008273586 // sqrt(6.0) - sqrt(2.0)
			#define _36dBoct_2 M_SQRT2                // sqrt(2.0)
			#define _36dBoct_3 3.86370330515627280477 // sqrt(6.0) + sqrt(2.0)
			_12dB_1.fParamQ = SVF::_Q_to_norm(_36dBoct_1);
			_12dB_2.fParamQ = SVF::_Q_to_norm(_36dBoct_2);
			_12dB_3.fParamQ = SVF::_Q_to_norm(_36dBoct_3);
			break;
		default:
			break;
		}

		_6dB_1.setSVF();
		_12dB_1.setSVF();
		_12dB_2.setSVF();
		_12dB_3.setSVF();
	}

	void makePassFilter()
	{
		_6dB_1.makeSVF();
		_12dB_1.makeSVF();
		_12dB_2.makeSVF();
		_12dB_3.makeSVF();
	}

	Steinberg::Vst::Sample64 computePassFilter
	(Steinberg::Vst::Sample64 input)
	{
		switch (degree)
		{
		case PassFilter::_6dBoct:
			return _6dB_1.computeSVF(input);
			break;
		case PassFilter::_12dBoct:
			return _12dB_1.computeSVF(input);
			break;
		case PassFilter::_18dBoct:
			return _12dB_1.computeSVF(_6dB_1.computeSVF(input));
			break;
		case PassFilter::_24dBoct:
			return _12dB_1.computeSVF(_12dB_2.computeSVF(input));
			break;
		case PassFilter::_36dBoct:
			return _12dB_1.computeSVF(_12dB_2.computeSVF(_12dB_3.computeSVF(input)));
			break;
		default:
			return input;
			break;
		}
	};

	double mag_response(double freq) {
		switch (degree)
		{
		case PassFilter::_6dBoct:
			return _6dB_1.mag_response(freq);
			break;
		case PassFilter::_12dBoct:
			return _12dB_1.mag_response(freq);
			break;
		case PassFilter::_18dBoct:
			return _12dB_1.mag_response(freq) * _6dB_1.mag_response(freq);
			break;
		case PassFilter::_24dBoct:
			return _12dB_1.mag_response(freq) * _12dB_2.mag_response(freq);
			break;
		case PassFilter::_36dBoct:
			return _12dB_1.mag_response(freq) * _12dB_2.mag_response(freq) * _12dB_3.mag_response(freq);
			break;
		default:
			return 1.0;
			break;
		}
	}

	static double Init_HP_Hz() { return 20.0; };
	static double Init_LP_Hz() { return 20000.0; };

	static double getFreqMax_HP() { return 400.0; };
	static double getFreqMin_HP() { return 20.0; };

	static double getFreqMax_LP() { return 41000.0; };
	static double getFreqMin_LP() { return 1000.0; };

	static Steinberg::Vst::ParamValue _Hz_to_norm_HP(double _Hz)
	{
		return log(_Hz / getFreqMin_HP()) / log(getFreqMax_HP() / getFreqMin_HP());
	};
	static double _norm_to_Hz_HP(Steinberg::Vst::ParamValue paramValue)
	{
		double FREQ_LOG_MAX = log(getFreqMax_HP() / getFreqMin_HP());
		double tmp = getFreqMin_HP() * exp(FREQ_LOG_MAX * paramValue);
		return std::max(std::min(tmp, getFreqMax_HP()), getFreqMin_HP());
	};
	
	static Steinberg::Vst::ParamValue _Hz_to_norm_LP(double _Hz)
	{
		return log(_Hz / getFreqMin_LP()) / log(getFreqMax_LP() / getFreqMin_LP());
	};
	static double _norm_to_Hz_LP(Steinberg::Vst::ParamValue paramValue)
	{
		double FREQ_LOG_MAX_LP = log(getFreqMax_LP() / getFreqMin_LP());
		double tmp_LP = getFreqMin_LP() * exp(FREQ_LOG_MAX_LP * paramValue);
		return tmp_LP;//std::max(std::min(tmp_LP, getFreqMax_LP()), getFreqMin_LP());
	};
	static filter_degree _norm_to_degree(Steinberg::Vst::ParamValue value)
	{
		return static_cast<filter_degree>(Steinberg::FromNormalized<Steinberg::Vst::ParamValue>(value, kDegreeNum));
	};
	static Steinberg::Vst::ParamValue _degree_to_norm(filter_degree type)
	{
		return Steinberg::ToNormalized<Steinberg::Vst::ParamValue>(type, kDegreeNum);
	};

	SVF _6dB_1;
	SVF _12dB_1, _12dB_2, _12dB_3;
	Steinberg::Vst::ParamValue fParamIn = 0.0;
	Steinberg::Vst::ParamValue fParamHz;
	Steinberg::Vst::ParamValue fParamtype;
	Steinberg::Vst::ParamValue fParamFs;
	Steinberg::Vst::ParamValue fParamdegree = _degree_to_norm(_12dBoct);
	filter_degree degree = _12dBoct;
	int In = 0;
};

namespace yg331 {
//------------------------------------------------------------------------
static const Steinberg::FUID kSVEQProcessorUID (0xFD13F195, 0x41CF5F22, 0xAC337E9F, 0x2FF7054F);
static const Steinberg::FUID kSVEQControllerUID (0xFF415DC6, 0xC29B50CC, 0x9F58529A, 0x305DA421);

#define SVEQVST3Category "Fx"

const bool
	Init_Bypass = false;

const Steinberg::Vst::ParamValue
    Init_Zoom = 2.0;

//------------------------------------------------------------------------
} // namespace yg331
