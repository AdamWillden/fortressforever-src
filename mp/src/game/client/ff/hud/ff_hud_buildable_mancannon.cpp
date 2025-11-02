#include "cbase.h"

#include "ff_hud_buildable_mancannon.h"

using namespace vgui;

KeyValues* CHudBuildableManCannon::s_kvVisibilityDisplayOptions;

DECLARE_HUDELEMENT(CHudBuildableManCannon);
DECLARE_HUD_MESSAGE(CHudBuildableManCannon, ManCannonMsg);
DECLARE_HUD_MESSAGE(CHudBuildableManCannon, ManCannonAvailabilityMsg);
DECLARE_HUD_MESSAGE(CHudBuildableManCannon, FF_BuildTimer);

CHudBuildableManCannon::CHudBuildableManCannon(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudBuildableManCannon"),
	m_wsDeployed(L"Deployed"),
	m_wsDeploying(L"Deploying..."),
	m_wsNotAvailable(L"Not Available"),
	m_wsNotDeployed(L"Not Deployed"),
	m_showPanel(ALWAYS),
	m_hideText(NEVER),
	m_bDisableBuildTimer(false)
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_ALWAYS);

	SetHeaderText(L"ManCannon");
	SetHeaderIconChar('6');
	SetUseToggleText(true);
	SetText(m_wsNotAvailable);

	m_qiBuildProgress = nullptr;
	m_qiHealth = nullptr;

	if (!s_kvVisibilityDisplayOptions)
	{
		KeyValues* kvVisibilityDisplayOptions
			= new KeyValues("Values");

		kvVisibilityDisplayOptions->SetString(
			std::to_string(NEVER).c_str(),
			"#HudPanel_DisplayNever");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(ALWAYS).c_str(),
			"#HudPanel_DisplayAlways");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(ON_BUILD).c_str(),
			"#HudPanel_DisplayOnBuild");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(IF_BUILT).c_str(),
			"#HudPanel_DisplayIfBuilt");

		s_kvVisibilityDisplayOptions
			= kvVisibilityDisplayOptions;
	}
}

CHudBuildableManCannon::~CHudBuildableManCannon()
{
}

void CHudBuildableManCannon::ApplySchemeSettings(
	IScheme* pScheme)
{
	FFQuantityPanel::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_ManCannon");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Deployed");
	if (localized)
		m_wsDeployed = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Deploying");
	if (localized)
		m_wsDeploying = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NotDeployed");
	if (localized)
		m_wsNotDeployed = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NotAvailable");
	if (localized)
		m_wsNotAvailable = localized;
}

void CHudBuildableManCannon::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_Visibility", 0);
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_QuantityDisplay", 1);

	KeyValues* kvShowPanelOptions = s_kvVisibilityDisplayOptions->MakeCopy();
	kvShowPanelOptions->SetString(
		std::to_string(IF_AVAILABLE).c_str(), "#HudPanel_DisplayIfAvailable");

	AddComboOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		kvShowPanelOptions,
		ALWAYS,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"HideText",
		"#HudPanel_Option_HideText",
		s_kvVisibilityDisplayOptions->MakeCopy(),
		NEVER,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"DisableBuildTimer",
		"#HudPanel_Option_HideBuildTimer",
		false,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"HealthDisplay",
		"#HudPanel_Option_HealthDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
	AddAlertOptions(kvPanelSpecificOptions, "#HudPanel_Option_DamageAlert", 1);
}

KeyValues* CHudBuildableManCannon::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPanelSpecificValues->SetInt("HideText", NEVER);
	kvPanelSpecificValues->SetBool("DisableBuildTimer", false);

	kvPanelSpecificValues->SetInt("HealthDisplay", DISPLAY_MAX);
	AddAlertDefaults(kvPanelSpecificValues);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudBuildableManCannon::ApplyStyleData(
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

	std::optional<AmountDisplay> healthDisplay
		= GetAmountDisplay(
			"HealthDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (healthDisplay.has_value())
	{
		m_qiHealth->SetAmountDisplay(
			healthDisplay.value());
	}

	std::optional<BuildableDisplayOption> showPanel
		= GetBuildableDisplayOption(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<BuildableDisplayOption> hideText
		= GetBuildableDisplayOption(
			"HideText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iDisableBuildTimer
		= GetInt(
			"DisableBuildTimer",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues,
			-1);

	bool bApplyDisplayOptions = false;

	if (showPanel.has_value()
		&& Change(m_showPanel, showPanel.value()))
	{
		bApplyDisplayOptions = true;
	}

	if (hideText.has_value()
		&& Change(m_hideText, hideText.value()))
	{
		bApplyDisplayOptions = true;
	}

	if (iDisableBuildTimer != -1
		&& Change(m_bDisableBuildTimer, iDisableBuildTimer == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (bApplyDisplayOptions || kvStyleData->GetInt("previewMode", -1) != -1)
		ApplyDisplayOptions();
}

std::optional<BuildableDisplayOption> CHudBuildableManCannon::GetBuildableDisplayOption(
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

void CHudBuildableManCannon::ApplyDisplayOptions()
{
	if (IsInPreviewMode())
	{
		SetHiddenBits(m_showPanel == NEVER ? HIDEHUD_ALWAYS : 0);

		SetText(m_wsDeployed);
		SetUseToggleText(m_hideText != ALWAYS);
		SetToggleTextVisible(m_hideText == NEVER);

		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiHealth);
		ShowItem(m_qiHealth);

		return;
	}

	if (m_bDeployed)
		ShowItem(m_qiHealth);
	else
		HideItem(m_qiHealth);

	if (m_bDeploying)
		SetText(m_wsDeploying);
	else if (m_bDeployed)
		SetText(m_wsDeployed);
	else
		UpdateAvailabilityText();

	if (m_bCanHaveManCannon)
	{
		switch (m_showPanel)
		{
		case ALWAYS:
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
			break;
		case IF_AVAILABLE:
			SetHiddenBits((m_bHasManCannon || m_bDeploying || m_bDeployed)
				? DEFAULT_HIDDEN_BITS
				: HIDEHUD_ALWAYS);
			break;
		case IF_BUILT:
			// Show only if deployed (built)
			SetHiddenBits(m_bDeployed
				? DEFAULT_HIDDEN_BITS
				: HIDEHUD_ALWAYS);
			break;
		case ON_BUILD:
			// Show if deploying (building) or deployed (built)
			SetHiddenBits((m_bDeploying || m_bDeployed)
				? DEFAULT_HIDDEN_BITS
				: HIDEHUD_ALWAYS);
			break;
		case NEVER:
		default:
			SetHiddenBits(HIDEHUD_ALWAYS);
			break;
		}
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}

	switch (m_hideText)
	{
	case ALWAYS:
		SetUseToggleText(false);
		break;
	case IF_BUILT:
		SetUseToggleText(true);
		SetToggleTextVisible(!m_bDeployed);
		break;
	case ON_BUILD:
		SetUseToggleText(true);
		SetToggleTextVisible(!m_bDeploying && !m_bDeployed);
		break;
	case NEVER:
	default:
		SetUseToggleText(true);
		SetToggleTextVisible(true);
		break;
	}

	if (m_bDeploying && !m_bDisableBuildTimer)
	{
		EnableItem(m_qiBuildProgress);
		DisableItem(m_qiHealth);
	}
	else
	{
		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiHealth);
	}
}

void CHudBuildableManCannon::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);
	HOOK_HUD_MESSAGE(CHudBuildableManCannon, ManCannonMsg);
	HOOK_HUD_MESSAGE(CHudBuildableManCannon, ManCannonAvailabilityMsg);
	HOOK_HUD_MESSAGE(CHudBuildableManCannon, FF_BuildTimer);

	m_qiHealth = AddItem("Health");
	m_qiHealth->SetLabel("#FF_ITEM_HEALTH");
	m_qiHealth->SetIcon('a');
	m_qiHealth->SetAmountDisplay(DISPLAY_PERCENTAGE);
	m_qiHealth->SetAmountMax(100);

	m_qiBuildProgress = AddItem("BuildProgress");
	m_qiBuildProgress->SetLabel("#FF_ITEM_PROGRESS");
	m_qiBuildProgress->SetIcon('f');
	m_qiBuildProgress->SetAmountDisplay(DISPLAY_PERCENTAGE);

	AddPanelToHudOptions(
		"ManCannon",
		"#HudPanel_ManCannon",
		"Buildables",
		"#HudPanel_Buildables");
}

void CHudBuildableManCannon::VidInit()
{
	ClearAlert();
	m_bDeployed = false;
	m_bDeploying = false;

	UpdateAvailabilityText();

	m_qiHealth->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);

	HideItem(m_qiHealth);
	DisableItem(m_qiBuildProgress);

	ApplyDisplayOptions();
}

void CHudBuildableManCannon::Paint()
{
	if (!IsInPreviewMode() && m_bDeploying)
	{
		float flTimeElapsed
			= gpGlobals->curtime - m_flBuildStartTime;

		if (m_flBuildDuration > flTimeElapsed)
			m_qiBuildProgress->SetAmount(flTimeElapsed);
	}

	FFQuantityPanel::Paint();
}

void CHudBuildableManCannon::UpdateAvailabilityText()
{
	if (!m_bDeploying && !m_bDeployed)
	{
		if (m_bHasManCannon)
			SetText(m_wsNotDeployed);
		else
			SetText(m_wsNotAvailable);
	}
}

void CHudBuildableManCannon::MsgFunc_ManCannonAvailabilityMsg(bf_read& msg)
{
	const bool bCanHave = msg.ReadByte() != 0;
	const bool bHas = msg.ReadByte() != 0;
	SetAvailability(bCanHave, bHas);
}

void CHudBuildableManCannon::SetAvailability(bool bCanHave, bool bHas)
{
	m_bCanHaveManCannon = bCanHave;
	m_bHasManCannon = bHas;
	UpdateAvailabilityText();
	ApplyDisplayOptions();
}

void CHudBuildableManCannon::MsgFunc_ManCannonMsg(
	bf_read& msg)
{
	BuildState_t state
		= static_cast<BuildState_t>(msg.ReadByte());

	const int iHealth = msg.ReadByte();
	const int iMaxHealth = msg.ReadByte();
	SetBuildState(state, iHealth, iMaxHealth);
}

void CHudBuildableManCannon::SetBuildState(BuildState_t state, int iHealth, int iMaxHealth)
{
	if (state == BUILDSTATE_BUILT && m_bDeployed && iHealth < m_qiHealth->GetAmount())
		TriggerAlert();
	else if (state != BUILDSTATE_BUILT)
		ClearAlert();

	m_qiBuildProgress->SetAmount(0.0f);

	switch (state)
	{
		case BUILDSTATE_NOTBUILT:
		{
			m_bDeployed = false;
			m_bDeploying = false;
			UpdateAvailabilityText();
			HideItem(m_qiHealth);
			DisableItem(m_qiBuildProgress);
			break;
		}
		case BUILDSTATE_BUILDING:
		{
			m_bDeployed = false;
			m_bDeploying = true;
			SetText(m_wsDeploying);
			DisableItem(m_qiHealth);
			EnableItem(m_qiBuildProgress);
			break;
		}
		case BUILDSTATE_BUILT:
		{
			m_qiHealth->SetAmountMax(iMaxHealth);
			m_qiHealth->SetAmount(iHealth);
			m_bDeployed = true;
			m_bDeploying = false;
			SetText(m_wsDeployed);
			ShowItem(m_qiHealth);
			DisableItem(m_qiBuildProgress);
			break;
		}
	}

	UpdateAvailabilityText();
	ApplyDisplayOptions();
}

void CHudBuildableManCannon::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();

	if (iBuildType != FF_BUILD_MANCANNON)
		return;

	const float flBuildDuration = msg.ReadFloat();
	SetBuildTimer(flBuildDuration);
}

void CHudBuildableManCannon::SetBuildTimer(float flBuildDuration)
{
	m_bDeploying = true;
	SetText(m_wsDeploying);

	m_flBuildDuration = flBuildDuration;
	m_flBuildStartTime = gpGlobals->curtime;

	m_qiBuildProgress->SetAmount(0.0f);
	m_qiBuildProgress->SetAmountMax(flBuildDuration);

	ApplyDisplayOptions();
}