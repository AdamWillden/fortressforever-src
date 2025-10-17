#include "cbase.h"

#include "ff_hud_misc_speedometer.h"


DECLARE_HUDELEMENT(CHudMiscSpeedometer);

CHudMiscSpeedometer::CHudMiscSpeedometer(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudMiscSpeedometer"),
	m_bShowPanel(ALWAYS),
	m_bDisableInstantaneousSpeed(false),
	m_bDisableAverageSpeed(false)
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NOTMEDIC);

	SetHeaderText(L"Speedometer");
	SetHeaderIconChar('6');

	m_qiInstantaneousSpeed = nullptr;
	m_qiAverageSpeed = nullptr;
}

CHudMiscSpeedometer::~CHudMiscSpeedometer()
{
}

void CHudMiscSpeedometer::ApplySchemeSettings(
	IScheme* pScheme)
{
	FFQuantityPanel::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Speedometer");
	if (localized)
		SetHeaderText(localized);
}

void CHudMiscSpeedometer::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddBooleanOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		true,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"DisableInstantaneousSpeed",
		"#HudPanel_Option_HideInstantaneousSpeed",
		false,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"DisableAverageSpeed",
		"#HudPanel_Option_HideAverageSpeed",
		false,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"InstantaneousSpeedDisplay",
		"#HudPanel_Option_InstantaneousSpeedDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_RAW,
		1);

	AddComboOption(
		kvPanelSpecificOptions,
		"AverageSpeedDisplay",
		"#HudPanel_Option_AverageSpeedDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_RAW,
		1);
}

KeyValues* CHudMiscSpeedometer::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 420);
	kvPreset->SetInt("alignH", ALIGN_RIGHT);
	kvPreset->SetInt("alignV", ALIGN_TOP);

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPanelSpecificValues->SetInt("HideText", NEVER);
	kvPanelSpecificValues->SetBool("DisableInstantaneousSpeed", false);
	kvPanelSpecificValues->SetBool("DisableAverageSpeed", false);

	kvPanelSpecificValues->SetInt("InstantaneousSpeedDisplay", DISPLAY_RAW);
	kvPanelSpecificValues->SetInt("AverageSpeedDisplay", DISPLAY_RAW);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudMiscSpeedometer::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	FFQuantityPanel::ApplyStyleData(
		kvStyleData,
		kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);

	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaultStyleData->FindKey("PanelSpecificValues", true);

	std::optional<AmountDisplay> instantaneousSpeedDisplay
		= GetAmountDisplay(
			"InstantaneousSpeedDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<AmountDisplay> averageSpeedDisplay
		= GetAmountDisplay(
			"AverageSpeedDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (instantaneousSpeedDisplay.has_value())
	{
		m_qiInstantaneousSpeed->SetAmountDisplay(
			instantaneousSpeedDisplay.value());
	}

	if (averageSpeedDisplay.has_value())
	{
		m_qiAverageSpeed->SetAmountDisplay(
			averageSpeedDisplay.value());
	}

	int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iDisableInstantaneousSpeed
		= GetInt(
			"DisableInstantaneousSpeed",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iDisableAverageSpeed
		= GetInt(
			"DisableAverageSpeed",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	bool bApplyDisplayOptions = false;

	if (iShowPanel != -1
		&& Change(m_bShowPanel, iShowPanel == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (iDisableInstantaneousSpeed != -1
		&& Change(m_bDisableInstantaneousSpeed, iDisableInstantaneousSpeed == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (iDisableAverageSpeed != -1
		&& Change(m_bDisableAverageSpeed, iDisableAverageSpeed == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (bApplyDisplayOptions)
		ApplyDisplayOptions();
}

std::optional<BuildableDisplayOption> CHudMiscSpeedometer::GetBuildableDisplayOption(
	const char* keyName,
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	int value
		= GetInt(
			keyName,
			kvStyleData,
			kvDefaultStyleData,
			-1);

	return value == -1
		? std::optional<BuildableDisplayOption>()
		: static_cast<BuildableDisplayOption>(value);
}

void CHudMiscSpeedometer::ApplyDisplayOptions()
{
	if (m_bShowPanel)
	{
		SetHiddenBits(HIDEHUD_PLAYERDEAD);
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}
}

void CHudMiscSpeedometer::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	m_qiInstantaneousSpeed = AddItem("Health");
	m_qiInstantaneousSpeed->SetLabel("#FF_ITEM_SPEED");
	m_qiInstantaneousSpeed->SetIcon('a');
	m_qiInstantaneousSpeed->SetAmountDisplay(DISPLAY_RAW);
	m_qiInstantaneousSpeed->SetAmountMax(600);

	m_qiAverageSpeed = AddItem("BuildProgress");
	m_qiAverageSpeed->SetLabel("#FF_ITEM_AVERAGE");
	m_qiAverageSpeed->SetIcon('f');
	m_qiAverageSpeed->SetAmountDisplay(DISPLAY_RAW);
	m_qiAverageSpeed->SetAmountMax(600);

	AddPanelToHudOptions(
		"Speedometer",
		"#HudPanel_Speedometer",
		"Misc",
		"#HudPanel_Misc");
}

void CHudMiscSpeedometer::VidInit()
{
	m_qiInstantaneousSpeed->SetAmount(0);
	m_qiAverageSpeed->SetAmount(0);

	ApplyDisplayOptions();
}

void CHudMiscSpeedometer::OnThink()
{
	C_FFPlayer* pPlayer
		= C_FFPlayer::GetLocalFFPlayerOrAnyObserverTarget();

	if (!pPlayer)
		return;

	if (gpGlobals->curtime < m_flNextUpdate) {
		return;
	}

	Vector vecVelocity
		= pPlayer->GetAbsVelocity();

	float flSpeed
		= FastSqrt(
			vecVelocity.x * vecVelocity.x
			+ vecVelocity.y * vecVelocity.y);

	m_qiInstantaneousSpeed->SetAmount(round(flSpeed));

	const float dt = 0.1f;     // your m_flNextUpdate step
	const float p = 0.8f;      // 80% responsiveness
	const float T = 1.0f;      // within 1 second
	const float alpha          // 0.14866
		= AlphaForTarget(p, T, dt);

	m_flAvgerageSpeed
		= alpha * flSpeed
		+ (1.0f - alpha) * m_flAvgerageSpeed;

	m_flNextUpdate = gpGlobals->curtime + dt;

	m_qiAverageSpeed->SetAmount(m_flAvgerageSpeed);
}

// Pick alpha to hit fraction p (e.g. 0.8) by time T, given update step dt.
float CHudMiscSpeedometer::AlphaForTarget(float p, float T, float dt) {
	// Clamp to safe ranges
	if (p <= 0.0f) return 0.0f;
	if (p >= 1.0f) return 1.0f;
	if (dt <= 0.0f) return 1.0f;
	return 1.0f - powf(1.0f - p, dt / T);
}
