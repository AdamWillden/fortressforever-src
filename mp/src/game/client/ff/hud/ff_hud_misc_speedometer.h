#ifndef FF_HUDMISC_SPEEDOMETER_H
#define FF_HUDMISC_SPEEDOMETER_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

using namespace vgui;
using namespace FFQuantityHelper;

class CHudMiscSpeedometer
    : public CHudElement, public FFQuantityPanel
{
private:
    DECLARE_CLASS_SIMPLE(CHudMiscSpeedometer, FFQuantityPanel);
    CHudMiscSpeedometer(const char* pElementName);
    ~CHudMiscSpeedometer();

    FFQuantityItem* m_qiInstantaneousSpeed;
    FFQuantityItem* m_qiAverageSpeed;

    float m_flNextUpdate = 0.0f;
    float m_flAvgerageSpeed = 0.0f;

    // HUD customization options
    bool m_bShowPanel;
    bool m_bDisableInstantaneousSpeed;
    bool m_bDisableAverageSpeed;

    virtual void ApplySchemeSettings(IScheme* pScheme) override;

	virtual void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	virtual KeyValues* GetDefaultStyleData() override;
	virtual void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);
    void ApplyDisplayOptions();

    static std::optional<BuildableDisplayOption> GetBuildableDisplayOption(
        const char* keyName,
        KeyValues* kvStyleData,
        KeyValues* kvDefaultStyleData);

    void Init();
    void VidInit();

    void OnThink() override;

    float AlphaForTarget(float p, float T, float dt);
};

#endif