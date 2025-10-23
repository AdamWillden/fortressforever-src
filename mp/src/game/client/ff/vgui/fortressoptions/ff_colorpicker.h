#ifndef FF_COLORPICKER_H
#define FF_COLORPICKER_H

#include <vgui_controls/Button.h>
#include "ff_colorpickerdialog.h"
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Panel.h>
#include "ff_inputslider.h"

namespace vgui
{
	class FFColorPicker : public Panel
	{
		DECLARE_CLASS_SIMPLE(FFColorPicker, Panel);
	public:
		FFColorPicker(Panel* parent, const char* name, Panel* pActionSignalTarget = NULL, bool bColorModeIntensity = false);
		FFColorPicker(Panel* parent, const char* name, const char* dialogButtonText = NULL, Panel* pActionSignalTarget = NULL, const char* pCmd = NULL, bool bColorModeIntensity = false);
		FFColorPicker(Panel* parent, const char* name, const wchar_t* dialogButtonText = NULL, Panel* pActionSignalTarget = NULL, const char* pCmd = NULL, bool bColorModeIntensity = false);

		void SetColorPickerDialog(FFColorPickerDialog* newDialog);

		int GetRedComponentValue();
		int GetGreenComponentValue();
		int GetBlueComponentValue();
		int GetAlphaComponentValue();
		int GetColorMode();

		void SetRedComponentValue(int iRed);
		void SetGreenComponentValue(int iGreen);
		void SetBlueComponentValue(int iBlue);
		void SetAlphaComponentValue(int iAlpha);
		void SetColorMode(int iColorMode);

		void SetValue(int iRed, int iGreen, int iBlue, int iAlpha);
		void GetValue(int& iRed, int& iGreen, int& iBlue, int& iAlpha);

		/*virtual void AddActionSignalTarget(Panel* messageTarget);
		virtual void AddActionSignalTarget(VPANEL messageTarget);
		virtual void RemoveActionSignalTarget(Panel* oldTarget);*/

	private:
		ImagePanel* m_pColorBackground;
		Button* m_pDialogButton;

		CFFInputSlider* m_pRed;
		CFFInputSlider* m_pGreen;
		CFFInputSlider* m_pBlue;
		CFFInputSlider* m_pAlpha;
		ComboBox* m_pColorMode;

		FFColorPickerDialog* m_pDialog;
		int m_iTeamColorPreview;
		int m_iRed;
		int m_iGreen;
		int m_iBlue;
		int m_iAlpha;
		int m_iColorMode;

		virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

		void UpdateColor();
		void RecalculateColorPreview();
		void SetRGBComponentsEnabled(bool bState);

		void OnTick(void);

		MESSAGE_FUNC_PARAMS(OnRedComponentChanged, "RedComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnGreenComponentChanged, "GreenComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnBlueComponentChanged, "BlueComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnAlphaComponentChanged, "AlphaComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnColorModeChanged, "ColorModeChanged", data);
		MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data);
		MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);
	};
};
#endif