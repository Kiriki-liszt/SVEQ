//------------------------------------------------------------------------
// Copyright(c) 2024 yg331.
//------------------------------------------------------------------------

#include "SVEQ_controller.h"

#include "vstgui/plugin-bindings/vst3editor.h"

#include "pluginterfaces/base/ustring.h"
#include "base/source/fstring.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "vstgui/plugin-bindings/vst3groupcontroller.h"


#include "vstgui/vstgui.h"
#include "vstgui/vstgui_uidescription.h"
#include "vstgui/uidescription/detail/uiviewcreatorattributes.h"

#include "vstgui/lib/controls/cparamdisplay.h"
#include <cuchar>
#include <cstdio>

using namespace Steinberg;


namespace VSTGUI {
	_6dBButton::_6dBButton(const VSTGUI::CRect& size, Steinberg::Vst::EditController* editController, Steinberg::Vst::ParamID _type)
		: DelegationController(nullptr)
		, CTextButton(size)
		, editController(editController)
	{
		// retrieve the parameter that we are interested in from controller
		Id_type = _type;
		uiParamBand_type = editController->getParameterObject(_type);

		// listen to these parameters (parameter changes will trigger update() )
		if (uiParamBand_type) uiParamBand_type->addDependent(this);
	}

	_6dBButton::~_6dBButton()
	{
		if (uiParamBand_type) uiParamBand_type->removeDependent(this);
	}

	void PLUGIN_API _6dBButton::update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message)
	{
		// if a parameter value is changed (by host or by another control) this
		// method is called

		auto* p = Steinberg::FCast<Steinberg::Vst::Parameter>(changedUnknown);
		if (p)
		{
			if (message == kChanged)
			{
				// update local parameter value and draw()
				if (p->getInfo().id == Id_type) {
					type = p->getNormalized(); setDirty(true);
				}
				/*
				editController->getParameterObject(getTag())->getInfo().title
				std::string str = "";
				char cstr[3] = "\0";
				mbstate_t mbs;
				for (const auto& it : p->getInfo().title) {
					memset(&mbs, 0, sizeof(mbs));//set shift state to the initial state
					memmove(cstr, "\0\0\0", 3);
					c16rtomb(cstr, it, &mbs);
					str.append(std::string(cstr));
				}//for
				setTitle((VSTGUI::UTF8String)str);
				*/
			}
			else if (message == kWillDestroy)
			{
				// stop listening to parameter changes
				if (uiParamBand_type) uiParamBand_type->removeDependent(this);
				uiParamBand_type = nullptr;
			}
		}
	}

	void _6dBButton::draw(VSTGUI::CDrawContext* pContext)
	{
		setMouseEnabled(false);
		SVF func;
		func.type = func._norm_to_type(type);
		if (func.type == SVF::kHS || func.type == SVF::kLS || func.type == SVF::kHP || func.type == SVF::kLP) {
			setMouseEnabled(true);
			CTextButton::draw(pContext);
		}
		//CTextButton::draw(pContext);
	}


	EQCurveView::EQCurveView(const VSTGUI::CRect& size, Steinberg::Vst::EditController* editController)
		: DelegationController(nullptr)
		, CView(size) 
		, editController(editController)
		, HP(PassFilter::Init_HP_Hz(), SVF::kHP)
		, LP(PassFilter::Init_LP_Hz(), SVF::kLP)
	{
		// retrieve the parameter that we are interested in from controller
		uiParamLevel      = editController->getParameterObject(kParamLevel);

		uiParamBand1_In   = editController->getParameterObject(kParamBand1_In);
		uiParamBand2_In   = editController->getParameterObject(kParamBand2_In);
		uiParamBand3_In   = editController->getParameterObject(kParamBand3_In);
		uiParamBand4_In   = editController->getParameterObject(kParamBand4_In);
		uiParamBand5_In   = editController->getParameterObject(kParamBand5_In);
		uiParamBand1_dB   = editController->getParameterObject(kParamBand1_dB);
		uiParamBand2_dB   = editController->getParameterObject(kParamBand2_dB);
		uiParamBand3_dB   = editController->getParameterObject(kParamBand3_dB);
		uiParamBand4_dB   = editController->getParameterObject(kParamBand4_dB);
		uiParamBand5_dB   = editController->getParameterObject(kParamBand5_dB);
		uiParamBand1_Hz   = editController->getParameterObject(kParamBand1_Hz);
		uiParamBand2_Hz   = editController->getParameterObject(kParamBand2_Hz);
		uiParamBand3_Hz   = editController->getParameterObject(kParamBand3_Hz);
		uiParamBand4_Hz   = editController->getParameterObject(kParamBand4_Hz);
		uiParamBand5_Hz   = editController->getParameterObject(kParamBand5_Hz);
		uiParamBand1_Q    = editController->getParameterObject(kParamBand1_Q);
		uiParamBand2_Q    = editController->getParameterObject(kParamBand2_Q);
		uiParamBand3_Q    = editController->getParameterObject(kParamBand3_Q);
		uiParamBand4_Q    = editController->getParameterObject(kParamBand4_Q);
		uiParamBand5_Q    = editController->getParameterObject(kParamBand5_Q);
		uiParamBand1_6dB  = editController->getParameterObject(kParamBand1_6dB);
		uiParamBand2_6dB  = editController->getParameterObject(kParamBand2_6dB);
		uiParamBand3_6dB  = editController->getParameterObject(kParamBand3_6dB);
		uiParamBand4_6dB  = editController->getParameterObject(kParamBand4_6dB);
		uiParamBand5_6dB  = editController->getParameterObject(kParamBand5_6dB);
		uiParamBand1_type = editController->getParameterObject(kParamBand1_type);
		uiParamBand2_type = editController->getParameterObject(kParamBand2_type);
		uiParamBand3_type = editController->getParameterObject(kParamBand3_type);
		uiParamBand4_type = editController->getParameterObject(kParamBand4_type);
		uiParamBand5_type = editController->getParameterObject(kParamBand5_type);

		uiParamHP_In   = editController->getParameterObject(kParamHP_In);
		uiParamLP_In   = editController->getParameterObject(kParamLP_In);
		uiParamHP_Hz   = editController->getParameterObject(kParamHP_Hz);
		uiParamLP_Hz   = editController->getParameterObject(kParamLP_Hz);
		uiParamHP_degree = editController->getParameterObject(kParamHP_degree);
		uiParamLP_degree = editController->getParameterObject(kParamLP_degree);

		// listen to these parameters (parameter changes will trigger update() )
		if (uiParamLevel)      uiParamLevel->     addDependent(this);

		if (uiParamBand1_In)   uiParamBand1_In->  addDependent(this);
		if (uiParamBand2_In)   uiParamBand2_In->  addDependent(this);
		if (uiParamBand3_In)   uiParamBand3_In->  addDependent(this);
		if (uiParamBand4_In)   uiParamBand4_In->  addDependent(this);
		if (uiParamBand5_In)   uiParamBand5_In->  addDependent(this);
		if (uiParamBand1_dB)   uiParamBand1_dB->  addDependent(this);
		if (uiParamBand2_dB)   uiParamBand2_dB->  addDependent(this);
		if (uiParamBand3_dB)   uiParamBand3_dB->  addDependent(this);
		if (uiParamBand4_dB)   uiParamBand4_dB->  addDependent(this);
		if (uiParamBand5_dB)   uiParamBand5_dB->  addDependent(this);
		if (uiParamBand1_Hz)   uiParamBand1_Hz->  addDependent(this);
		if (uiParamBand2_Hz)   uiParamBand2_Hz->  addDependent(this);
		if (uiParamBand3_Hz)   uiParamBand3_Hz->  addDependent(this);
		if (uiParamBand4_Hz)   uiParamBand4_Hz->  addDependent(this);
		if (uiParamBand5_Hz)   uiParamBand5_Hz->  addDependent(this);
		if (uiParamBand1_Q)    uiParamBand1_Q->   addDependent(this);
		if (uiParamBand2_Q)    uiParamBand2_Q->   addDependent(this);
		if (uiParamBand3_Q)    uiParamBand3_Q->   addDependent(this);
		if (uiParamBand4_Q)    uiParamBand4_Q->   addDependent(this);
		if (uiParamBand5_Q)    uiParamBand5_Q->   addDependent(this);
		if (uiParamBand1_6dB)  uiParamBand1_6dB-> addDependent(this);
		if (uiParamBand2_6dB)  uiParamBand2_6dB-> addDependent(this);
		if (uiParamBand3_6dB)  uiParamBand3_6dB-> addDependent(this);
		if (uiParamBand4_6dB)  uiParamBand4_6dB-> addDependent(this);
		if (uiParamBand5_6dB)  uiParamBand5_6dB-> addDependent(this);
		if (uiParamBand1_type) uiParamBand1_type->addDependent(this);
		if (uiParamBand2_type) uiParamBand2_type->addDependent(this);
		if (uiParamBand3_type) uiParamBand3_type->addDependent(this);
		if (uiParamBand4_type) uiParamBand4_type->addDependent(this);
		if (uiParamBand5_type) uiParamBand5_type->addDependent(this);

		if (uiParamHP_In)   uiParamHP_In->addDependent(this);
		if (uiParamLP_In)   uiParamLP_In->addDependent(this);
		if (uiParamHP_Hz)   uiParamHP_Hz->addDependent(this);
		if (uiParamLP_Hz)   uiParamLP_Hz->addDependent(this);
		if (uiParamHP_degree) uiParamHP_degree->addDependent(this);
		if (uiParamLP_degree) uiParamLP_degree->addDependent(this);
	}

	EQCurveView::~EQCurveView()
	{
		if (uiParamLevel)      uiParamLevel->     removeDependent(this);

		if (uiParamBand1_In)   uiParamBand1_In->  removeDependent(this);
		if (uiParamBand2_In)   uiParamBand2_In->  removeDependent(this);
		if (uiParamBand3_In)   uiParamBand3_In->  removeDependent(this);
		if (uiParamBand4_In)   uiParamBand4_In->  removeDependent(this);
		if (uiParamBand5_In)   uiParamBand5_In->  removeDependent(this);
		if (uiParamBand1_dB)   uiParamBand1_dB->  removeDependent(this);
		if (uiParamBand2_dB)   uiParamBand2_dB->  removeDependent(this);
		if (uiParamBand3_dB)   uiParamBand3_dB->  removeDependent(this);
		if (uiParamBand4_dB)   uiParamBand4_dB->  removeDependent(this);
		if (uiParamBand5_dB)   uiParamBand5_dB->  removeDependent(this);
		if (uiParamBand1_Hz)   uiParamBand1_Hz->  removeDependent(this);
		if (uiParamBand2_Hz)   uiParamBand2_Hz->  removeDependent(this);
		if (uiParamBand3_Hz)   uiParamBand3_Hz->  removeDependent(this);
		if (uiParamBand4_Hz)   uiParamBand4_Hz->  removeDependent(this);
		if (uiParamBand5_Hz)   uiParamBand5_Hz->  removeDependent(this);
		if (uiParamBand1_Q)    uiParamBand1_Q->   removeDependent(this);
		if (uiParamBand2_Q)    uiParamBand2_Q->   removeDependent(this);
		if (uiParamBand3_Q)    uiParamBand3_Q->   removeDependent(this);
		if (uiParamBand4_Q)    uiParamBand4_Q->   removeDependent(this);
		if (uiParamBand5_Q)    uiParamBand5_Q->   removeDependent(this);
		if (uiParamBand1_6dB)  uiParamBand1_6dB-> removeDependent(this);
		if (uiParamBand2_6dB)  uiParamBand2_6dB-> removeDependent(this);
		if (uiParamBand3_6dB)  uiParamBand3_6dB-> removeDependent(this);
		if (uiParamBand4_6dB)  uiParamBand4_6dB-> removeDependent(this);
		if (uiParamBand5_6dB)  uiParamBand5_6dB-> removeDependent(this);
		if (uiParamBand1_type) uiParamBand1_type->removeDependent(this);
		if (uiParamBand2_type) uiParamBand2_type->removeDependent(this);
		if (uiParamBand3_type) uiParamBand3_type->removeDependent(this);
		if (uiParamBand4_type) uiParamBand4_type->removeDependent(this);
		if (uiParamBand5_type) uiParamBand5_type->removeDependent(this);

		if (uiParamHP_In)   uiParamHP_In->  removeDependent(this);
		if (uiParamLP_In)   uiParamLP_In->  removeDependent(this);
		if (uiParamHP_Hz)   uiParamHP_Hz->  removeDependent(this);
		if (uiParamLP_Hz)   uiParamLP_Hz->  removeDependent(this);
		if (uiParamHP_degree) uiParamHP_degree->removeDependent(this);
		if (uiParamLP_degree) uiParamLP_degree->removeDependent(this);
	}

	// ...
	/*
	CMouseEventResult MyView::onMouseDown(CPoint& where, const CButtonState& buttons)
	{
		// ...
		// editController->beginEdit(tag);
		// ...
	}


	CMouseEventResult MyView::onMouseMoved(CPoint& where, const CButtonState& buttons)
	{
		// ...
		// editController->performEdit (tag, value);
		// editController->setParamNormalized (tag, value);
		// ...
	}

	CMouseEventResult MyView::onMouseUp(CPoint& where, const CButtonState& buttons)
	{
		// ...
	   // editController->endEdit(tag);
	   // ...

	}
	*/
	void PLUGIN_API EQCurveView::update(Steinberg::FUnknown* changedUnknown,
		Steinberg::int32 message)
	{
		// if a parameter value is changed (by host or by another control) this
		// method is called

		auto* p = Steinberg::FCast<Steinberg::Vst::Parameter>(changedUnknown);
		if (p)
		{
			if (message == kChanged)
			{
				// update local parameter value and draw()
				switch (p->getInfo().id) {
				case kParamLevel:      level            = p->toPlain(p->getNormalized()); setDirty(true); break;

				case kParamBand1_In:   Band1.fParamIn   = (p->getNormalized()); setDirty(true); break;
				case kParamBand2_In:   Band2.fParamIn   = (p->getNormalized()); setDirty(true); break;
				case kParamBand3_In:   Band3.fParamIn   = (p->getNormalized()); setDirty(true); break;
				case kParamBand4_In:   Band4.fParamIn   = (p->getNormalized()); setDirty(true); break;
				case kParamBand5_In:   Band5.fParamIn   = (p->getNormalized()); setDirty(true); break;
				case kParamBand1_dB:   Band1.fParamdB   = (p->getNormalized()); setDirty(true); break;
				case kParamBand2_dB:   Band2.fParamdB   = (p->getNormalized()); setDirty(true); break;
				case kParamBand3_dB:   Band3.fParamdB   = (p->getNormalized()); setDirty(true); break;
				case kParamBand4_dB:   Band4.fParamdB   = (p->getNormalized()); setDirty(true); break;
				case kParamBand5_dB:   Band5.fParamdB   = (p->getNormalized()); setDirty(true); break;
				case kParamBand1_Hz:   Band1.fParamHz   = (p->getNormalized()); setDirty(true); break;
				case kParamBand2_Hz:   Band2.fParamHz   = (p->getNormalized()); setDirty(true); break;
				case kParamBand3_Hz:   Band3.fParamHz   = (p->getNormalized()); setDirty(true); break;
				case kParamBand4_Hz:   Band4.fParamHz   = (p->getNormalized()); setDirty(true); break;
				case kParamBand5_Hz:   Band5.fParamHz   = (p->getNormalized()); setDirty(true); break;
				case kParamBand1_Q:    Band1.fParamQ    = (p->getNormalized()); setDirty(true); break;
				case kParamBand2_Q:    Band2.fParamQ    = (p->getNormalized()); setDirty(true); break;
				case kParamBand3_Q:    Band3.fParamQ    = (p->getNormalized()); setDirty(true); break;
				case kParamBand4_Q:    Band4.fParamQ    = (p->getNormalized()); setDirty(true); break;
				case kParamBand5_Q:    Band5.fParamQ    = (p->getNormalized()); setDirty(true); break;
				case kParamBand1_6dB:  Band1.fParam_6dB = (p->getNormalized()); setDirty(true); break;
				case kParamBand2_6dB:  Band2.fParam_6dB = (p->getNormalized()); setDirty(true); break;
				case kParamBand3_6dB:  Band3.fParam_6dB = (p->getNormalized()); setDirty(true); break;
				case kParamBand4_6dB:  Band4.fParam_6dB = (p->getNormalized()); setDirty(true); break;
				case kParamBand5_6dB:  Band5.fParam_6dB = (p->getNormalized()); setDirty(true); break;
				case kParamBand1_type: Band1.fParamtype = (p->getNormalized()); setDirty(true); break;
				case kParamBand2_type: Band2.fParamtype = (p->getNormalized()); setDirty(true); break;
				case kParamBand3_type: Band3.fParamtype = (p->getNormalized()); setDirty(true); break;
				case kParamBand4_type: Band4.fParamtype = (p->getNormalized()); setDirty(true); break;
				case kParamBand5_type: Band5.fParamtype = (p->getNormalized()); setDirty(true); break;

				case kParamHP_In:   HP.fParamIn   = (p->getNormalized()); setDirty(true); break;
				case kParamLP_In:   LP.fParamIn   = (p->getNormalized()); setDirty(true); break;
				case kParamHP_Hz:   HP.fParamHz   = (p->getNormalized()); setDirty(true); break;
				case kParamLP_Hz:   LP.fParamHz   = (p->getNormalized()); setDirty(true); break;
				case kParamHP_degree: HP.fParamdegree = (p->getNormalized()); setDirty(true); break;
				case kParamLP_degree: LP.fParamdegree = (p->getNormalized()); setDirty(true); break;
				default: break;
				}
			}
			else if (message == kWillDestroy)
			{
				// stop listening to parameter changes
				if (uiParamLevel)      uiParamLevel->     removeDependent(this);

				if (uiParamBand1_In)   uiParamBand1_In->  removeDependent(this);
				if (uiParamBand2_In)   uiParamBand2_In->  removeDependent(this);
				if (uiParamBand3_In)   uiParamBand3_In->  removeDependent(this);
				if (uiParamBand4_In)   uiParamBand4_In->  removeDependent(this);
				if (uiParamBand5_In)   uiParamBand5_In->  removeDependent(this);
				if (uiParamBand1_dB)   uiParamBand1_dB->  removeDependent(this);
				if (uiParamBand2_dB)   uiParamBand2_dB->  removeDependent(this);
				if (uiParamBand3_dB)   uiParamBand3_dB->  removeDependent(this);
				if (uiParamBand4_dB)   uiParamBand4_dB->  removeDependent(this);
				if (uiParamBand5_dB)   uiParamBand5_dB->  removeDependent(this);
				if (uiParamBand1_Hz)   uiParamBand1_Hz->  removeDependent(this);
				if (uiParamBand2_Hz)   uiParamBand2_Hz->  removeDependent(this);
				if (uiParamBand3_Hz)   uiParamBand3_Hz->  removeDependent(this);
				if (uiParamBand4_Hz)   uiParamBand4_Hz->  removeDependent(this);
				if (uiParamBand5_Hz)   uiParamBand5_Hz->  removeDependent(this);
				if (uiParamBand1_Q)    uiParamBand1_Q->   removeDependent(this);
				if (uiParamBand2_Q)    uiParamBand2_Q->   removeDependent(this);
				if (uiParamBand4_Q)    uiParamBand4_Q->   removeDependent(this);
				if (uiParamBand3_Q)    uiParamBand3_Q->   removeDependent(this);
				if (uiParamBand5_Q)    uiParamBand5_Q->   removeDependent(this);
				if (uiParamBand1_6dB)  uiParamBand1_6dB-> removeDependent(this);
				if (uiParamBand2_6dB)  uiParamBand2_6dB-> removeDependent(this);
				if (uiParamBand3_6dB)  uiParamBand3_6dB-> removeDependent(this);
				if (uiParamBand4_6dB)  uiParamBand4_6dB-> removeDependent(this);
				if (uiParamBand5_6dB)  uiParamBand5_6dB-> removeDependent(this);
				if (uiParamBand1_type) uiParamBand1_type->removeDependent(this);
				if (uiParamBand2_type) uiParamBand2_type->removeDependent(this);
				if (uiParamBand3_type) uiParamBand3_type->removeDependent(this);
				if (uiParamBand4_type) uiParamBand4_type->removeDependent(this);
				if (uiParamBand5_type) uiParamBand5_type->removeDependent(this);

				if (uiParamHP_In)   uiParamHP_In->removeDependent(this);
				if (uiParamLP_In)   uiParamLP_In->removeDependent(this);
				if (uiParamHP_Hz)   uiParamHP_Hz->removeDependent(this);
				if (uiParamLP_Hz)   uiParamLP_Hz->removeDependent(this);
				if (uiParamHP_degree) uiParamHP_degree->removeDependent(this);
				if (uiParamLP_degree) uiParamLP_degree->removeDependent(this);

				uiParamLevel      = nullptr;

				uiParamBand1_In   = nullptr;
				uiParamBand2_In   = nullptr;
				uiParamBand3_In   = nullptr;
				uiParamBand4_In   = nullptr;
				uiParamBand5_In   = nullptr;
				uiParamBand1_dB   = nullptr;
				uiParamBand2_dB   = nullptr;
				uiParamBand3_dB   = nullptr;
				uiParamBand4_dB   = nullptr;
				uiParamBand5_dB   = nullptr;
				uiParamBand1_Hz   = nullptr;
				uiParamBand2_Hz   = nullptr;
				uiParamBand3_Hz   = nullptr;
				uiParamBand4_Hz   = nullptr;
				uiParamBand5_Hz   = nullptr;
				uiParamBand1_Q    = nullptr;
				uiParamBand2_Q    = nullptr;
				uiParamBand3_Q    = nullptr;
				uiParamBand4_Q    = nullptr;
				uiParamBand5_Q    = nullptr;
				uiParamBand1_6dB  = nullptr;
				uiParamBand2_6dB  = nullptr;
				uiParamBand3_6dB  = nullptr;
				uiParamBand4_6dB  = nullptr;
				uiParamBand5_6dB  = nullptr;
				uiParamBand1_type = nullptr;
				uiParamBand2_type = nullptr;
				uiParamBand3_type = nullptr;
				uiParamBand4_type = nullptr;
				uiParamBand5_type = nullptr;

				uiParamHP_In = nullptr;
				uiParamLP_In = nullptr;
				uiParamHP_Hz = nullptr;
				uiParamLP_Hz = nullptr;
				uiParamHP_degree = nullptr;
				uiParamLP_degree = nullptr;
			}
		}
		setDirty(true);
	}

	void EQCurveView::draw(VSTGUI::CDrawContext* pContext)
	{
		pContext->setLineWidth(1);
		pContext->setFillColor(VSTGUI::CColor(50, 50, 95, 0));
		pContext->setFrameColor(VSTGUI::CColor(223, 233, 233, 255)); // black borders
		pContext->drawRect(getViewSize(), VSTGUI::kDrawFilledAndStroked);

		double MAX_FREQ = 22000.0;
		double MIN_FREQ = 10.0;
		double FREQ_LOG_MAX = log(MAX_FREQ / MIN_FREQ);
		double DB_EQ_RANGE = 15.0;

		{
			VSTGUI::CRect r(getViewSize());
			pContext->setFrameColor(VSTGUI::CColor(255, 255, 255, 55));
			for (int x = 2; x < 10; x++) {
				VSTGUI::CCoord Hz_10 = r.getWidth() * log(10.0 * x / MIN_FREQ) / FREQ_LOG_MAX;
				const VSTGUI::CPoint _p1(r.left + Hz_10, r.bottom);
				const VSTGUI::CPoint _p2(r.left + Hz_10, r.top);
				pContext->drawLine(_p1, _p2);
			}
			for (int x = 2; x < 10; x++) {
				VSTGUI::CCoord Hz_100 = r.getWidth() * log(100.0 * x / MIN_FREQ) / FREQ_LOG_MAX;
				const VSTGUI::CPoint _p1(r.left + Hz_100, r.bottom);
				const VSTGUI::CPoint _p2(r.left + Hz_100, r.top);
				pContext->drawLine(_p1, _p2);
			}
			for (int x = 2; x < 10; x++) {
				VSTGUI::CCoord Hz_1000 = r.getWidth() * log(1000.0 * x / MIN_FREQ) / FREQ_LOG_MAX;
				const VSTGUI::CPoint _p1(r.left + Hz_1000, r.bottom);
				const VSTGUI::CPoint _p2(r.left + Hz_1000, r.top);
				pContext->drawLine(_p1, _p2);
			}
			
			for (int x = 2; x < 3; x++) {
				VSTGUI::CCoord Hz_10000 = r.getWidth() * log(10000.0 * x / MIN_FREQ) / FREQ_LOG_MAX;
				const VSTGUI::CPoint _p1(r.left + Hz_10000, r.bottom);
				const VSTGUI::CPoint _p2(r.left + Hz_10000, r.top);
				pContext->drawLine(_p1, _p2);
			}
			
			pContext->setFrameColor(VSTGUI::CColor(255, 255, 255, 155));
			{
				VSTGUI::CCoord Hz_100 = r.getWidth() * log(100.0 / MIN_FREQ) / FREQ_LOG_MAX;
				const VSTGUI::CPoint _p1(r.left + Hz_100, r.bottom);
				const VSTGUI::CPoint _p2(r.left + Hz_100, r.top);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord Hz_1000 = r.getWidth() * log(1000.0 / MIN_FREQ) / FREQ_LOG_MAX;
				const VSTGUI::CPoint _p1(r.left + Hz_1000, r.bottom);
				const VSTGUI::CPoint _p2(r.left + Hz_1000, r.top);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord Hz_10000 = r.getWidth() * log(10000.0 / MIN_FREQ) / FREQ_LOG_MAX;
				const VSTGUI::CPoint _p1(r.left + Hz_10000, r.bottom);
				const VSTGUI::CPoint _p2(r.left + Hz_10000, r.top);
				pContext->drawLine(_p1, _p2);
			}
		}

		{
			VSTGUI::CRect r(getViewSize());
			pContext->setFrameColor(VSTGUI::CColor(255, 255, 255, 55));
			{
				VSTGUI::CCoord dB_p15 = r.getHeight() * (1.0 - (((-15.0 / DB_EQ_RANGE) / 2) + 0.5));
				const VSTGUI::CPoint _p1(r.left, r.bottom - dB_p15);
				const VSTGUI::CPoint _p2(r.right, r.bottom - dB_p15);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord dB_p10 = r.getHeight() * (1.0 - (((-10.0 / DB_EQ_RANGE) / 2) + 0.5));
				const VSTGUI::CPoint _p1(r.left, r.bottom - dB_p10);
				const VSTGUI::CPoint _p2(r.right, r.bottom - dB_p10);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord dB_p5 = r.getHeight() * (1.0 - (((-5.0 / DB_EQ_RANGE) / 2) + 0.5));
				const VSTGUI::CPoint _p1(r.left, r.bottom - dB_p5);
				const VSTGUI::CPoint _p2(r.right, r.bottom - dB_p5);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord dB_0 = r.getHeight() * (1.0 - (((.0 / DB_EQ_RANGE) / 2) + 0.5));
				const VSTGUI::CPoint _p1(r.left, r.bottom - dB_0);
				const VSTGUI::CPoint _p2(r.right, r.bottom - dB_0);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord dB_m5 = r.getHeight() * (1.0 - (((5.0 / DB_EQ_RANGE) / 2) + 0.5));
				const VSTGUI::CPoint _p1(r.left, r.bottom - dB_m5);
				const VSTGUI::CPoint _p2(r.right, r.bottom - dB_m5);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord dB_m10 = r.getHeight() * (1.0 - (((10.0 / DB_EQ_RANGE) / 2) + 0.5));
				const VSTGUI::CPoint _p1(r.left, r.bottom - dB_m10);
				const VSTGUI::CPoint _p2(r.right, r.bottom - dB_m10);
				pContext->drawLine(_p1, _p2);
			}
			{
				VSTGUI::CCoord dB_m15 = r.getHeight() * (1.0 - (((15.0 / DB_EQ_RANGE) / 2) + 0.5));
				const VSTGUI::CPoint _p1(r.left, r.bottom - dB_m15);
				const VSTGUI::CPoint _p2(r.right, r.bottom - dB_m15);
				pContext->drawLine(_p1, _p2);
			}
		}


		VSTGUI::CCoord inset = 30;

		VSTGUI::CGraphicsPath* path = pContext->createGraphicsPath();
		if (path)
		{
			VSTGUI::CRect r(getViewSize());
			//r.inset(inset, 0);
			

			/*
			path->beginSubpath(VSTGUI::CPoint(r.left + r.getWidth() / 2, r.top));
			path->addLine(VSTGUI::CPoint(r.left, r.bottom));
			path->addLine(VSTGUI::CPoint(r.right, r.bottom));
			path->closeSubpath();
			*/

			Band1.setSVF(); Band1.makeSVF();
			Band2.setSVF(); Band2.makeSVF();
			Band3.setSVF(); Band3.makeSVF();
			Band4.setSVF(); Band4.makeSVF();
			Band5.setSVF(); Band5.makeSVF();
			HP.setPassFilter(); HP.makePassFilter();
			LP.setPassFilter(); LP.makePassFilter();

			VSTGUI::CCoord y_mid = r.bottom - (r.getHeight() / 2.0);
			path->beginSubpath(VSTGUI::CPoint(r.left-1, y_mid));			
			for (int x = -1; x <= r.getWidth()+1; x++) {
				double tmp = MIN_FREQ * exp(FREQ_LOG_MAX * x / r.getWidth());
				double freq = (std::max)((std::min)(tmp, MAX_FREQ), MIN_FREQ);

				double dB_level = level;

				double dB_1 = 20 * log10(Band1.mag_response(freq));
				double dB_2 = 20 * log10(Band2.mag_response(freq));
				double dB_3 = 20 * log10(Band3.mag_response(freq));
				double dB_4 = 20 * log10(Band4.mag_response(freq));
				double dB_5 = 20 * log10(Band5.mag_response(freq));
				double dB_HP = 20 * log10(HP.mag_response(freq));
				double dB_LP = 20 * log10(LP.mag_response(freq));
				double dB = dB_level + dB_1 + dB_2 + dB_3 + dB_4 + dB_5 + dB_HP + dB_LP;

				double m = 1.0 - (((dB / DB_EQ_RANGE) / 2) + 0.5);
				double scy = m * r.getHeight();
				path->addLine(VSTGUI::CPoint(r.left + x, r.top + scy));
			}
			path->addLine(VSTGUI::CPoint(r.right+1, r.bottom+1));
			path->addLine(VSTGUI::CPoint(r.left-1, r.bottom+1));
			path->closeSubpath();

			pContext->setFrameColor(VSTGUI::kYellowCColor);
			pContext->setDrawMode(VSTGUI::kAntiAliasing);
			pContext->setLineWidth(2);
			pContext->setLineStyle(VSTGUI::kLineSolid);
			pContext->drawGraphicsPath(path, VSTGUI::CDrawContext::kPathStroked);
			path->forget();
		}

		setDirty(false);
	}
}

namespace yg331 {
//------------------------------------------------------------------------
// LogRangeParameter Declaration
//------------------------------------------------------------------------
class LogRangeParameter : public Vst::RangeParameter
{
public:
	using RangeParameter::RangeParameter;
	Vst::ParamValue toPlain(Vst::ParamValue _valueNormalized) const SMTG_OVERRIDE;
	Vst::ParamValue toNormalized(Vst::ParamValue plainValue) const SMTG_OVERRIDE;
	void toString(Vst::ParamValue _valueNormalized, Vst::String128 string) const SMTG_OVERRIDE;
};
//------------------------------------------------------------------------
// LogRangeParameter Implementation
//------------------------------------------------------------------------
Vst::ParamValue LogRangeParameter::toPlain(Vst::ParamValue _valueNormalized) const
{
	double FREQ_LOG_MAX = log(getMax() / getMin());
	double tmp = getMin() * exp(FREQ_LOG_MAX * _valueNormalized);
	double freq = (std::max)((std::min)(tmp, getMax()), getMin());
	return freq;
	//return _valueNormalized * (getMax() - getMin()) + getMin();
}

//------------------------------------------------------------------------
Vst::ParamValue LogRangeParameter::toNormalized(Vst::ParamValue plainValue) const
{
	SMTG_ASSERT(getMax() - getMin() != 0);
	double FREQ_LOG_MAX = log(getMax() / getMin());
	return log(plainValue / getMin()) / FREQ_LOG_MAX;
	//return (plainValue - getMin()) / (getMax() - getMin());
}

void LogRangeParameter::toString(Vst::ParamValue _valueNormalized, Vst::String128 string) const
{
	{
		//Parameter::toString(toPlain(_valueNormalized), string);
		UString wrapper(string, str16BufferSize(Vst::String128));
		{
			if (!wrapper.printFloat(toPlain(_valueNormalized), precision))
				string[0] = 0;
			wrapper.append(STR16(" "));
			wrapper.append(getInfo().units);
		}
	}
	/*
	char text[32];
	if (_valueNormalized >= 0.505)
	{
		snprintf(text, 32, "R %d", int32((_valueNormalized - 0.5f) * 200 + 0.5f));
	}
	else if (_valueNormalized <= 0.495)
	{
		snprintf(text, 32, "L %d", int32((0.5f - _valueNormalized) * 200 + 0.5f));
	}
	else
	{
		strcpy(text, "C");
	}

	Steinberg::UString(string, 128).fromAscii(text);
	*/
}

//------------------------------------------------------------------------
// LinRangeParameter Declaration
//------------------------------------------------------------------------
class LinRangeParameter : public Vst::RangeParameter
{
public:
	using RangeParameter::RangeParameter;
	void toString(Vst::ParamValue _valueNormalized, Vst::String128 string) const SMTG_OVERRIDE;
};
//------------------------------------------------------------------------
// LinRangeParameter Implementation
//------------------------------------------------------------------------
void LinRangeParameter::toString(Vst::ParamValue _valueNormalized, Vst::String128 string) const
{
	{
		//Parameter::toString(toPlain(_valueNormalized), string);
		UString wrapper(string, str16BufferSize(Vst::String128));
		{
			if (!wrapper.printFloat(toPlain(_valueNormalized), precision))
				string[0] = 0;
			wrapper.append(STR16(" "));
			wrapper.append(getInfo().units);
		}
	}
}

//------------------------------------------------------------------------
// SVEQController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated

	//---do not forget to call parent ------
	tresult result = EditControllerEx1::initialize (context);
	if (result != kResultOk)
	{
		return result;
	}

	// Here you could register some parameters
int32 stepCount;
	int32 flags;
	int32 tag;
	Vst::ParamValue defaultVal;
	Vst::ParamValue minPlain;
	Vst::ParamValue maxPlain;
	Vst::ParamValue defaultPlain;


	tag = kParamBypass;
	stepCount = 1;
	defaultVal = Init_Bypass ? 1 : 0;
	flags = Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass;
	parameters.addParameter(STR16("Bypass"), nullptr, stepCount, defaultVal, flags, tag);


	if (zoomFactors.empty())
	{
		Vst::ParamValue zoom_coef = 1.0;
		zoomFactors.push_back(ZoomFactor(STR("50%"), zoom_coef * 0.5));  // 0/6
		zoomFactors.push_back(ZoomFactor(STR("75%"), zoom_coef * 0.75)); // 1/6
		zoomFactors.push_back(ZoomFactor(STR("100%"), zoom_coef * 1.0));  // 2/6
		zoomFactors.push_back(ZoomFactor(STR("125%"), zoom_coef * 1.25)); // 3/6
		zoomFactors.push_back(ZoomFactor(STR("150%"), zoom_coef * 1.5));  // 4/6
		zoomFactors.push_back(ZoomFactor(STR("175%"), zoom_coef * 1.75)); // 5/6
		zoomFactors.push_back(ZoomFactor(STR("200%"), zoom_coef * 2.0));  // 6/6
	}

	Vst::StringListParameter* zoomParameter = new Vst::StringListParameter(STR("Zoom"), kParamZoom);
	for (ZoomFactorVector::const_iterator it = zoomFactors.begin(), end = zoomFactors.end(); it != end; ++it)
	{
		zoomParameter->appendString(it->title);
	}
	zoomParameter->setNormalized(zoomParameter->toNormalized(Init_Zoom)); // toNorm(2) == 100%
	zoomParameter->addDependent(this);
	parameters.addParameter(zoomParameter);



	flags = Vst::ParameterInfo::kCanAutomate;

	minPlain = -12.0;
	maxPlain = 12.0;
	defaultPlain = 0.0;
	stepCount = 0;

	tag = kParamLevel;
	auto* ParamIn = new LinRangeParameter(STR16("Level"), tag, STR16("dB"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	ParamIn->setPrecision(2);
	parameters.addParameter(ParamIn);

	tag = kParamOutput;
	auto* ParamOut = new Vst::RangeParameter(STR16("Output"), tag, STR16("dB"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	ParamOut->setPrecision(2);
	parameters.addParameter(ParamOut);


	stepCount = 1;
	defaultVal = Init_Bypass ? 1 : 0;
	parameters.addParameter(STR16("Band1_In"), nullptr, stepCount, defaultVal, flags, kParamBand1_In);
	parameters.addParameter(STR16("Band2_In"), nullptr, stepCount, defaultVal, flags, kParamBand2_In);
	parameters.addParameter(STR16("Band3_In"), nullptr, stepCount, defaultVal, flags, kParamBand3_In);
	parameters.addParameter(STR16("Band4_In"), nullptr, stepCount, defaultVal, flags, kParamBand4_In);
	parameters.addParameter(STR16("Band5_In"), nullptr, stepCount, defaultVal, flags, kParamBand5_In);


	minPlain = SVF::getdBMin();
	maxPlain = SVF::getdBMax();
	defaultPlain = 0.0;
	stepCount = 0;

	auto* Band1_dB = new LinRangeParameter(STR("Band1_dB"), kParamBand1_dB, STR("dB"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band2_dB = new LinRangeParameter(STR("Band2_dB"), kParamBand2_dB, STR("dB"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band3_dB = new LinRangeParameter(STR("Band3_dB"), kParamBand3_dB, STR("dB"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band4_dB = new LinRangeParameter(STR("Band4_dB"), kParamBand4_dB, STR("dB"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band5_dB = new LinRangeParameter(STR("Band5_dB"), kParamBand5_dB, STR("dB"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	Band1_dB->setPrecision(2);
	Band2_dB->setPrecision(2);
	Band3_dB->setPrecision(2);
	Band4_dB->setPrecision(2);
	Band5_dB->setPrecision(2);
	parameters.addParameter(Band1_dB);
	parameters.addParameter(Band2_dB);
	parameters.addParameter(Band3_dB);
	parameters.addParameter(Band4_dB);
	parameters.addParameter(Band5_dB);

	minPlain = SVF::getFreqMin();
	maxPlain = SVF::getFreqMax();
	stepCount = 0;

	auto* Band1_Hz = new LogRangeParameter(STR("Band1_Hz"), kParamBand1_Hz, STR("Hz"), minPlain, maxPlain, SVF::Init_Band1_Hz(), stepCount, flags);
	auto* Band2_Hz = new LogRangeParameter(STR("Band2_Hz"), kParamBand2_Hz, STR("Hz"), minPlain, maxPlain, SVF::Init_Band2_Hz(), stepCount, flags);
	auto* Band3_Hz = new LogRangeParameter(STR("Band3_Hz"), kParamBand3_Hz, STR("Hz"), minPlain, maxPlain, SVF::Init_Band3_Hz(), stepCount, flags);
	auto* Band4_Hz = new LogRangeParameter(STR("Band4_Hz"), kParamBand4_Hz, STR("Hz"), minPlain, maxPlain, SVF::Init_Band4_Hz(), stepCount, flags);
	auto* Band5_Hz = new LogRangeParameter(STR("Band5_Hz"), kParamBand5_Hz, STR("Hz"), minPlain, maxPlain, SVF::Init_Band5_Hz(), stepCount, flags);
	Band1_Hz->setPrecision(0);
	Band2_Hz->setPrecision(0);
	Band3_Hz->setPrecision(0);
	Band4_Hz->setPrecision(0);
	Band5_Hz->setPrecision(0);
	parameters.addParameter(Band1_Hz);
	parameters.addParameter(Band2_Hz);
	parameters.addParameter(Band3_Hz);
	parameters.addParameter(Band4_Hz);
	parameters.addParameter(Band5_Hz);


	minPlain = SVF::getQMin();
	maxPlain = SVF::getQMax();
	defaultPlain = 1.414;
	stepCount = 0;

	auto* Band1_Q = new LogRangeParameter(STR16("Band1_Q"), kParamBand1_Q, STR16("Q"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band2_Q = new LogRangeParameter(STR16("Band2_Q"), kParamBand2_Q, STR16("Q"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band3_Q = new LogRangeParameter(STR16("Band3_Q"), kParamBand3_Q, STR16("Q"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band4_Q = new LogRangeParameter(STR16("Band4_Q"), kParamBand4_Q, STR16("Q"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	auto* Band5_Q = new LogRangeParameter(STR16("Band5_Q"), kParamBand5_Q, STR16("Q"), minPlain, maxPlain, defaultPlain, stepCount, flags);
	Band1_Q->setPrecision(2);
	Band2_Q->setPrecision(2);
	Band3_Q->setPrecision(2);
	Band4_Q->setPrecision(2);
	Band5_Q->setPrecision(2);
	parameters.addParameter(Band1_Q);
	parameters.addParameter(Band2_Q);
	parameters.addParameter(Band3_Q);
	parameters.addParameter(Band4_Q);
	parameters.addParameter(Band5_Q);

	stepCount = 1;
	defaultVal = 0;
	parameters.addParameter(STR16("Band1_6dB"), nullptr, stepCount, defaultVal, flags, kParamBand1_6dB);
	parameters.addParameter(STR16("Band2_6dB"), nullptr, stepCount, defaultVal, flags, kParamBand2_6dB);
	parameters.addParameter(STR16("Band3_6dB"), nullptr, stepCount, defaultVal, flags, kParamBand3_6dB);
	parameters.addParameter(STR16("Band4_6dB"), nullptr, stepCount, defaultVal, flags, kParamBand4_6dB);
	parameters.addParameter(STR16("Band5_6dB"), nullptr, stepCount, defaultVal, flags, kParamBand5_6dB);


	auto* Band1_type = new Vst::StringListParameter(STR16("Band1_type"), kParamBand1_type, STR16(""), flags);
	auto* Band2_type = new Vst::StringListParameter(STR16("Band2_type"), kParamBand2_type, STR16(""), flags);
	auto* Band3_type = new Vst::StringListParameter(STR16("Band3_type"), kParamBand3_type, STR16(""), flags);
	auto* Band4_type = new Vst::StringListParameter(STR16("Band4_type"), kParamBand4_type, STR16(""), flags);
	auto* Band5_type = new Vst::StringListParameter(STR16("Band5_type"), kParamBand5_type, STR16(""), flags);

	for (int i = 0; i < SVF::kFltNum + 1; i++) {
		Band1_type->appendString(Filter_types[i]);
		Band2_type->appendString(Filter_types[i]);
		Band3_type->appendString(Filter_types[i]);
		Band4_type->appendString(Filter_types[i]);
		Band5_type->appendString(Filter_types[i]);
	}

	Band1_type->setNormalized(SVF::_type_to_norm(SVF::kBell));
	Band2_type->setNormalized(SVF::_type_to_norm(SVF::kBell));
	Band3_type->setNormalized(SVF::_type_to_norm(SVF::kBell));
	Band4_type->setNormalized(SVF::_type_to_norm(SVF::kBell));
	Band5_type->setNormalized(SVF::_type_to_norm(SVF::kBell));
	
	parameters.addParameter(Band1_type);
	parameters.addParameter(Band2_type);
	parameters.addParameter(Band3_type);
	parameters.addParameter(Band4_type);
	parameters.addParameter(Band5_type);


	
	stepCount = 1;
	defaultVal = 0 ? 1 : 0;
	parameters.addParameter(STR16("HP_In"), nullptr, stepCount, defaultVal, flags, kParamHP_In);
	parameters.addParameter(STR16("LP_In"), nullptr, stepCount, defaultVal, flags, kParamLP_In);

	stepCount = 0;
	auto* HP_Hz = new LogRangeParameter(STR("HP_Hz"), kParamHP_Hz, STR("Hz"), PassFilter::getFreqMin_HP(), PassFilter::getFreqMax_HP(), PassFilter::Init_HP_Hz(), stepCount, flags);
	auto* LP_Hz = new LogRangeParameter(STR("LP_Hz"), kParamLP_Hz, STR("Hz"), PassFilter::getFreqMin_LP(), PassFilter::getFreqMax_LP(), PassFilter::Init_LP_Hz(), stepCount, flags);
	HP_Hz->setPrecision(0);
	LP_Hz->setPrecision(0);
	parameters.addParameter(HP_Hz);
	parameters.addParameter(LP_Hz);

	auto* HP_degree = new Vst::StringListParameter(STR16("HP_degree"), kParamHP_degree, STR16(""), flags);
	auto* LP_degree = new Vst::StringListParameter(STR16("LP_degree"), kParamLP_degree, STR16(""), flags);
	for (int i = 0; i < PassFilter::kDegreeNum + 1; i++) {
		HP_degree->appendString(Filter_degree[i]);
		LP_degree->appendString(Filter_degree[i]);
	}
	HP_degree->setNormalized(PassFilter::_degree_to_norm(PassFilter::_12dBoct));
	LP_degree->setNormalized(PassFilter::_degree_to_norm(PassFilter::_12dBoct));
	parameters.addParameter(HP_degree);
	parameters.addParameter(LP_degree);

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::terminate ()
{
	// Here the Plug-in will be de-instantiated, last possibility to remove some memory!

	//---do not forget to call parent ------
	return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::setComponentState (IBStream* state)
{
	// Here you get the state of the component (Processor part)
	if (!state)
		return kResultFalse;
IBStreamer streamer(state, kLittleEndian);

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


	setParamNormalized(kParamBypass, savedBypass ? 1 : 0);
	setParamNormalized(kParamZoom, savedZoom);
	setParamNormalized(kParamLevel, savedLevel);
	setParamNormalized(kParamOutput, savedOutput);

	setParamNormalized(kParamBand1_In, savedBand1_In ? 1 : 0);
	setParamNormalized(kParamBand2_In, savedBand2_In ? 1 : 0);
	setParamNormalized(kParamBand3_In, savedBand3_In ? 1 : 0);
	setParamNormalized(kParamBand4_In, savedBand4_In ? 1 : 0);
	setParamNormalized(kParamBand5_In, savedBand5_In ? 1 : 0);
	setParamNormalized(kParamBand1_dB, savedBand1_dB);
	setParamNormalized(kParamBand2_dB, savedBand2_dB);
	setParamNormalized(kParamBand3_dB, savedBand3_dB);
	setParamNormalized(kParamBand4_dB, savedBand4_dB);
	setParamNormalized(kParamBand5_dB, savedBand5_dB);
	setParamNormalized(kParamBand1_Hz, savedBand1_Hz);
	setParamNormalized(kParamBand2_Hz, savedBand2_Hz);
	setParamNormalized(kParamBand3_Hz, savedBand3_Hz);
	setParamNormalized(kParamBand4_Hz, savedBand4_Hz);
	setParamNormalized(kParamBand5_Hz, savedBand5_Hz);
	setParamNormalized(kParamBand1_Q, savedBand1_Q);
	setParamNormalized(kParamBand2_Q, savedBand2_Q);
	setParamNormalized(kParamBand3_Q, savedBand3_Q);
	setParamNormalized(kParamBand4_Q, savedBand4_Q);
	setParamNormalized(kParamBand5_Q, savedBand5_Q);
	setParamNormalized(kParamBand1_6dB, savedBand1_6dB ? 1 : 0);
	setParamNormalized(kParamBand2_6dB, savedBand2_6dB ? 1 : 0);
	setParamNormalized(kParamBand3_6dB, savedBand3_6dB ? 1 : 0);
	setParamNormalized(kParamBand4_6dB, savedBand4_6dB ? 1 : 0);
	setParamNormalized(kParamBand5_6dB, savedBand5_6dB ? 1 : 0);
	setParamNormalized(kParamBand1_type, savedBand1_type);
	setParamNormalized(kParamBand2_type, savedBand2_type);
	setParamNormalized(kParamBand3_type, savedBand3_type);
	setParamNormalized(kParamBand4_type, savedBand4_type);
	setParamNormalized(kParamBand5_type, savedBand5_type);

	setParamNormalized(kParamHP_In, savedHP_In ? 1 : 0);
	setParamNormalized(kParamLP_In, savedLP_In ? 1 : 0);
	setParamNormalized(kParamHP_Hz, savedHP_Hz);
	setParamNormalized(kParamLP_Hz, savedLP_Hz);
	setParamNormalized(kParamHP_degree, savedHP_degree);
	setParamNormalized(kParamLP_degree, savedLP_degree);
	
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::setState (IBStream* state)
{
	// Here you get the state of the controller

	return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::getState (IBStream* state)
{
	// Here you are asked to deliver the state of the controller (if needed)
	// Note: the real state of your plug-in is saved in the processor

	return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API SVEQController::createView (FIDString name)
{
	// Here the Host wants to open your editor (if you have one)
	if (FIDStringsEqual (name, Vst::ViewType::kEditor))
	{
		// create your editor here and return a IPlugView ptr of it
		auto* view = new VSTGUI::VST3Editor (this, "view", "SVEQ_editor.uidesc");
		view->setZoomFactor(1.0);
		setKnobMode(Steinberg::Vst::KnobModes::kLinearMode);
		return view;
	}
	return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
	// called by host to update your parameters
	tresult result = EditControllerEx1::setParamNormalized (tag, value);
	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
	// called by host to get a string for given normalized value of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API SVEQController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
	// called by host to get a normalized value from a string representation of a specific parameter
	// (without having to set the value!)
	return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

void PLUGIN_API SVEQController::update(FUnknown* changedUnknown, int32 message)
{
	EditControllerEx1::update(changedUnknown, message);

	// GUI Resizing
	// check 'zoomtest' code at
	// https://github.com/steinbergmedia/vstgui/tree/vstgui4_10/vstgui/tests/uidescription%20vst3/source

	Vst::Parameter* param = FCast<Vst::Parameter>(changedUnknown);
	if (!param)
		return;

	if (param->getInfo().id == kParamZoom)
	{
		size_t index = static_cast<size_t> (param->toPlain(param->getNormalized()));

		if (index >= zoomFactors.size())
			return;

		for (EditorVector::const_iterator it = editors.begin(), end = editors.end(); it != end; ++it)
		{
			VSTGUI::VST3Editor* editor = dynamic_cast<VSTGUI::VST3Editor*>(*it);
			if (editor)
				editor->setZoomFactor(zoomFactors[index].factor);
		}
	}
}
//------------------------------------------------------------------------
void SVEQController::editorAttached(Steinberg::Vst::EditorView* editor)
{
	editors.push_back(editor);
}

//------------------------------------------------------------------------
void SVEQController::editorRemoved(Steinberg::Vst::EditorView* editor)
{
	editors.erase(std::find(editors.begin(), editors.end(), editor));
}
//------------------------------------------------------------------------
} // namespace yg331
