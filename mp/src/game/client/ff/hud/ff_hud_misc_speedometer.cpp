#include "cbase.h"

#include "ff_hud_misc_speedometer.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudMiscSpeedometer);

CHudMiscSpeedometer::CHudMiscSpeedometer(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudMiscSpeedometer"),
	m_bShowPanel(true),
	m_speedItems(AVERAGE_ONLY)
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);

	SetHeaderIconChar('v');
	SetHeaderText(L"Speedometer");

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
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_General", 0);
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_QuantityDisplay", 1);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		true,
		0);

	AddNumericOption(
		kvPanelSpecificOptions,
		"AveragePeriod",
		"#HudPanel_Option_AveragePeriod",
		1000,
		100,
		5000,
		0);

	KeyValues* kvItems = new KeyValues("Values");
	kvItems->SetString(std::to_string(SPEED_ONLY).c_str(), "#FF_ITEM_SPEED");
	kvItems->SetString(std::to_string(AVERAGE_ONLY).c_str(), "#FF_ITEM_AVERAGE");
	kvItems->SetString(std::to_string(BOTH).c_str(), "#HudPanel_Option_Both");

	AddComboOption(
		kvPanelSpecificOptions,
		"SpeedItems",
		"#HudPanel_Option_Items",
		kvItems,
		AVERAGE_ONLY,
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

	kvPreset->SetInt("x", 638);
	kvPreset->SetInt("y", 424);
	kvPreset->SetInt("alignH", ALIGN_RIGHT);
	kvPreset->SetInt("alignV", ALIGN_BOTTOM);
	kvPreset->SetInt("showHeaderText", 0);
	kvPreset->SetInt("showHeaderIcon", 0);
	kvPreset->SetInt("showPanel", 0);
	kvPreset->SetInt("showPanelBorder", 0);
	kvPreset->SetInt("barWidth", 0);
	kvPreset->SetInt("barHeight", 0);

	kvPreset->FindKey("Bar")->SetInt("show", 0);
	kvPreset->FindKey("BarBorder")->SetInt("show", 0);
	kvPreset->FindKey("BarBackground")->SetInt("show", 0);
	kvPreset->FindKey("Label")->SetInt("show", 0);
	kvPreset->FindKey("Icon")->SetInt("show", 0);
	kvPreset->FindKey("Amount")->SetInt("show", 1);
	kvPreset->FindKey("Amount")->SetInt("size", 20);
	kvPreset->FindKey("Amount")->SetInt("colorMode", COLOR_MODE_FADED);

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetInt("AveragePeriod", 1000);
	kvPanelSpecificValues->SetInt("SpeedItems", AVERAGE_ONLY);

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

	int iPeriod = GetInt("AveragePeriod", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iPeriod != -1)
		m_flAveragePeriod = clamp(iPeriod, 100, 5000) / 1000.0f;

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

	int iSpeedItems
		= GetInt(
			"SpeedItems",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	bool bApplyDisplayOptions = false;

	if (iShowPanel != -1
		&& Change(m_bShowPanel, iShowPanel == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (iSpeedItems != -1
		&& Change(m_speedItems, static_cast<SpeedItems>(iSpeedItems)))
	{
		bApplyDisplayOptions = true;
	}

	if (bApplyDisplayOptions || kvStyleData->GetInt("previewMode", -1) != -1)
		ApplyDisplayOptions();
}


void CHudMiscSpeedometer::ApplyDisplayOptions()
{
	if (m_bShowPanel)
	{
		if (IsInPreviewMode())
			SetHiddenBits(0);
		else
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}

	if (m_speedItems == AVERAGE_ONLY)
		DisableItem(m_qiInstantaneousSpeed);
	else
		EnableItem(m_qiInstantaneousSpeed);

	if (m_speedItems == SPEED_ONLY)
		DisableItem(m_qiAverageSpeed);
	else
		EnableItem(m_qiAverageSpeed);
}

void CHudMiscSpeedometer::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	m_qiInstantaneousSpeed = AddItem("InstantaneousSpeed");
	m_qiInstantaneousSpeed->SetLabel("#FF_ITEM_SPEED");
	m_qiInstantaneousSpeed->SetIcon('a');
	m_qiInstantaneousSpeed->SetAmountDisplay(DISPLAY_RAW);
	m_qiInstantaneousSpeed->SetAmountMax(600);

	m_qiAverageSpeed = AddItem("AverageSpeed");
	m_qiAverageSpeed->SetLabel("#FF_ITEM_AVERAGE");
	m_qiAverageSpeed->SetIcon('f');
	m_qiAverageSpeed->SetAmountDisplay(DISPLAY_RAW);
	m_qiAverageSpeed->SetAmountMax(600);

	AddPanelToHudOptions(
		"Speedometer",
		"#HudPanel_Speedometer",
		"",
		"");
}

void CHudMiscSpeedometer::VidInit()
{
	m_flAverageSpeed = 0.0f;
	m_flNextUpdate = 0.0f;

	m_qiInstantaneousSpeed->SetAmount(0);
	m_qiAverageSpeed->SetAmount(0);

	ApplyDisplayOptions();
}

void CHudMiscSpeedometer::OnThink()
{
	BaseClass::OnThink();

	if (!engine->IsInGame() || IsInPreviewMode())
		return;

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
	const float T = m_flAveragePeriod; // Time to reach 80% of a new speed
	const float alpha          // 0.14866
		= AlphaForTarget(p, T, dt);

	m_flAverageSpeed
		= alpha * flSpeed
		+ (1.0f - alpha) * m_flAverageSpeed;

	if (flSpeed < 0.1f && m_flAverageSpeed < 0.5f)
	{
		m_flAverageSpeed = 0.0f;
	}

	m_flNextUpdate = gpGlobals->curtime + dt;

	m_qiAverageSpeed->SetAmount(m_flAverageSpeed);
}

// Pick alpha to hit fraction p (e.g. 0.8) by time T, given update step dt.
float CHudMiscSpeedometer::AlphaForTarget(float p, float T, float dt) {
	// Clamp to safe ranges
	if (p <= 0.0f) return 0.0f;
	if (p >= 1.0f) return 1.0f;
	if (dt <= 0.0f) return 1.0f;
	return 1.0f - powf(1.0f - p, dt / T);
}
