//------------------------------------------------------------------------
// Copyright(c) 2024 yg331.
//------------------------------------------------------------------------

#pragma once

#include "SVEQ_cids.h"

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui/uidescription/delegationcontroller.h"
#include "vstgui/vstgui.h"
#include "vstgui/plugin-bindings/vst3editor.h"

namespace VSTGUI {
	class _6dBButton : public CTextButton, public Steinberg::FObject, public DelegationController
	{
	public:
		_6dBButton(const VSTGUI::CRect& size, Steinberg::Vst::EditController* editController = nullptr, Steinberg::Vst::ParamID _type = kParamBand1_type);
		~_6dBButton();

		// --- CView methods ---
		void draw(CDrawContext* pContext) override;
		//CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
		//CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
		//CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;

		CLASS_METHODS(_6dBButton, CTextButton)

	private:
		// --- IDependent (FObject) methods ---

		void PLUGIN_API update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) override;

		// --- attributes ---

		Steinberg::Vst::EditController* editController;
		Steinberg::Vst::Parameter* uiParamBand_type;
		Steinberg::Vst::ParamID Id_type;
		double type = 0;
	};

	class EQCurveView : public CView
				, public Steinberg::FObject
				, public DelegationController
	{
	public:
		EQCurveView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* editController = nullptr);
		~EQCurveView();

		// --- CView methods ---
		void draw(CDrawContext* pContext) override;
		//CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
		//CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons) override;
		//CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;

		CLASS_METHODS(EQCurveView, CView)

	private:

		// --- IDependent (FObject) methods ---

		void PLUGIN_API update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) override;

		// --- attributes ---

		Steinberg::Vst::EditController* editController;

		Steinberg::Vst::Parameter* uiParamLevel;

		Steinberg::Vst::Parameter* uiParamBand1_In;
		Steinberg::Vst::Parameter* uiParamBand2_In;
		Steinberg::Vst::Parameter* uiParamBand3_In;
		Steinberg::Vst::Parameter* uiParamBand4_In;
		Steinberg::Vst::Parameter* uiParamBand5_In;
		Steinberg::Vst::Parameter* uiParamBand1_dB;
		Steinberg::Vst::Parameter* uiParamBand2_dB;
		Steinberg::Vst::Parameter* uiParamBand3_dB;
		Steinberg::Vst::Parameter* uiParamBand4_dB;
		Steinberg::Vst::Parameter* uiParamBand5_dB;
		Steinberg::Vst::Parameter* uiParamBand1_Hz;
		Steinberg::Vst::Parameter* uiParamBand2_Hz;
		Steinberg::Vst::Parameter* uiParamBand3_Hz;
		Steinberg::Vst::Parameter* uiParamBand4_Hz;
		Steinberg::Vst::Parameter* uiParamBand5_Hz;
		Steinberg::Vst::Parameter* uiParamBand1_Q;
		Steinberg::Vst::Parameter* uiParamBand2_Q;
		Steinberg::Vst::Parameter* uiParamBand3_Q;
		Steinberg::Vst::Parameter* uiParamBand4_Q;
		Steinberg::Vst::Parameter* uiParamBand5_Q;
		Steinberg::Vst::Parameter* uiParamBand1_6dB;
		Steinberg::Vst::Parameter* uiParamBand2_6dB;
		Steinberg::Vst::Parameter* uiParamBand3_6dB;
		Steinberg::Vst::Parameter* uiParamBand4_6dB;
		Steinberg::Vst::Parameter* uiParamBand5_6dB;
		Steinberg::Vst::Parameter* uiParamBand1_type;
		Steinberg::Vst::Parameter* uiParamBand2_type;
		Steinberg::Vst::Parameter* uiParamBand3_type;
		Steinberg::Vst::Parameter* uiParamBand4_type;
		Steinberg::Vst::Parameter* uiParamBand5_type;

		Steinberg::Vst::Parameter* uiParamHP_In;
		Steinberg::Vst::Parameter* uiParamLP_In;
		Steinberg::Vst::Parameter* uiParamHP_Hz;
		Steinberg::Vst::Parameter* uiParamLP_Hz;
		Steinberg::Vst::Parameter* uiParamHP_degree;
		Steinberg::Vst::Parameter* uiParamLP_degree;

		SVF Band1;
		SVF Band2;
		SVF Band3;
		SVF Band4;
		SVF Band5;
		PassFilter HP;
		PassFilter LP;

		Steinberg::Vst::Sample64 level = 0.0;

		CFontRef	fontID = kNormalFont;
		CColor		fontColor = kBlackCColor;
	};
}

namespace yg331 {

//------------------------------------------------------------------------
//  SVEQController
//------------------------------------------------------------------------
class SVEQController : public Steinberg::Vst::EditControllerEx1, public VSTGUI::VST3EditorDelegate
{
public:
//------------------------------------------------------------------------
	SVEQController () = default;
	~SVEQController () SMTG_OVERRIDE = default;

    // Create function
	static Steinberg::FUnknown* createInstance (void* /*context*/)
	{
		return (Steinberg::Vst::IEditController*)new SVEQController;
	}

	// IPluginBase
	Steinberg::tresult PLUGIN_API initialize (Steinberg::FUnknown* context) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API terminate () SMTG_OVERRIDE;

	// EditController
	Steinberg::tresult PLUGIN_API setComponentState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::IPlugView* PLUGIN_API createView (Steinberg::FIDString name) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getState (Steinberg::IBStream* state) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API setParamNormalized (Steinberg::Vst::ParamID tag,
                                                      Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamStringByValue (Steinberg::Vst::ParamID tag,
                                                         Steinberg::Vst::ParamValue valueNormalized,
                                                         Steinberg::Vst::String128 string) SMTG_OVERRIDE;
	Steinberg::tresult PLUGIN_API getParamValueByString (Steinberg::Vst::ParamID tag,
                                                         Steinberg::Vst::TChar* string,
                                                         Steinberg::Vst::ParamValue& valueNormalized) SMTG_OVERRIDE;
VSTGUI::CView* createCustomView (VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes& attributes,
									 const VSTGUI::IUIDescription* description, VSTGUI::VST3Editor* editor) override
	{
		if (VSTGUI::UTF8StringView(name) == "EQCurveView")
		{
			VSTGUI::CRect size(VSTGUI::CPoint(30, 10), VSTGUI::CPoint(620, 150));
			customView_EQCurve = new VSTGUI::EQCurveView(size, this);
			return customView_EQCurve;
		}
		if (VSTGUI::UTF8StringView(name) == "Band1_6dB")
		{
			VSTGUI::CRect size(VSTGUI::CPoint(30, 10), VSTGUI::CPoint(50, 50));
			CV_6dBButton[0] = (new VSTGUI::_6dBButton(size, this, kParamBand1_type));
			return CV_6dBButton[0];
		}
		if (VSTGUI::UTF8StringView(name) == "Band2_6dB")
		{
			VSTGUI::CRect size(VSTGUI::CPoint(30, 10), VSTGUI::CPoint(50, 50));
			CV_6dBButton[1] = (new VSTGUI::_6dBButton(size, this, kParamBand2_type));
			return CV_6dBButton[1];
		}
		if (VSTGUI::UTF8StringView(name) == "Band3_6dB")
		{
			VSTGUI::CRect size(VSTGUI::CPoint(30, 10), VSTGUI::CPoint(50, 50));
			CV_6dBButton[2] = (new VSTGUI::_6dBButton(size, this, kParamBand3_type));
			return CV_6dBButton[2];
		}
		if (VSTGUI::UTF8StringView(name) == "Band4_6dB")
		{
			VSTGUI::CRect size(VSTGUI::CPoint(30, 10), VSTGUI::CPoint(50, 50));
			CV_6dBButton[3] = (new VSTGUI::_6dBButton(size, this, kParamBand4_type));
			return CV_6dBButton[3];
		}
		if (VSTGUI::UTF8StringView(name) == "Band5_6dB")
		{
			VSTGUI::CRect size(VSTGUI::CPoint(30, 10), VSTGUI::CPoint(50, 50));
			CV_6dBButton[4] = (new VSTGUI::_6dBButton(size, this, kParamBand5_type));
			return CV_6dBButton[4];
		}
		return nullptr;
	}
	VSTGUI::CView* verifyView (VSTGUI::CView* view, const VSTGUI::UIAttributes& attributes,
							   const VSTGUI::IUIDescription* description, VSTGUI::VST3Editor* editor) override
	{
		return view;
	}
	VSTGUI::IController* createSubController (VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description,
									  VSTGUI::VST3Editor* editor) override
	{
		return nullptr;
	}

	void PLUGIN_API update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) SMTG_OVERRIDE;
	void editorAttached(Steinberg::Vst::EditorView* editor) SMTG_OVERRIDE;
	void editorRemoved(Steinberg::Vst::EditorView* editor) SMTG_OVERRIDE;

 	//---Interface---------
	DEFINE_INTERFACES
		// Here you can add more supported VST3 interfaces
		// DEF_INTERFACE (Vst::IXXX)
	END_DEFINE_INTERFACES (EditController)
    DELEGATE_REFCOUNT (EditController)

//------------------------------------------------------------------------
protected:
typedef std::vector<Steinberg::Vst::EditorView*> EditorVector;
	EditorVector editors;
	struct ZoomFactor {
		const Steinberg::tchar* title;
		double factor;

		ZoomFactor(const Steinberg::tchar* title, double factor) : title(title), factor(factor) {}
	};
	typedef std::vector<ZoomFactor> ZoomFactorVector;
	ZoomFactorVector zoomFactors;

	Steinberg::tchar* Filter_types[SVF::kFltNum + 1] = {
		(Steinberg::tchar*)STR("Bell"),
		(Steinberg::tchar*)STR("LowShelf"),
		(Steinberg::tchar*)STR("HighPass"),
		(Steinberg::tchar*)STR("HighShelf"),
		(Steinberg::tchar*)STR("LowPass"),
		//(Steinberg::tchar*)STR("BandPass"),
		(Steinberg::tchar*)STR("Notch")
	};

	Steinberg::tchar* Filter_degree[PassFilter::kDegreeNum + 1] = {
		(Steinberg::tchar*)STR("6"), (Steinberg::tchar*)STR("12"),
		(Steinberg::tchar*)STR("18"), (Steinberg::tchar*)STR("24"),
		(Steinberg::tchar*)STR("36")
	};

	VSTGUI::EQCurveView* customView_EQCurve = nullptr;
	VSTGUI::_6dBButton* CV_6dBButton[5] = { nullptr, };

};

//------------------------------------------------------------------------
} // namespace yg331
