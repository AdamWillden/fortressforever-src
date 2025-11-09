#include "cbase.h"

#include "ff_hud_buildable_mancannon.h"

KeyValues* CHudBuildableManCannon::s_kvVisibilityDisplayOptions;

DECLARE_HUDELEMENT(CHudBuildableManCannon);
DECLARE_HUD_MESSAGE(CHudBuildableManCannon, ManCannonMsg);
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

	SetHiddenBits(HIDEHUD_PLAYERDEAD);
	SetEnabled(false);

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

void CHudBuildableManCannon::SetHasManCannon(
	bool bHasManCannon)
{
	if (m_bHasManCannon == bHasManCannon)
		return;

	m_bHasManCannon = bHasManCannon;

	if (m_bDeploying || m_bDeployed)
		return;

	SetText(
		m_bHasManCannon
		? m_wsNotDeployed
		: m_wsNotAvailable);
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
	AddComboOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		s_kvVisibilityDisplayOptions->MakeCopy(),
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

	if (bApplyDisplayOptions)
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
	switch (m_showPanel)
	{
	case ALWAYS:
		SetHiddenBits(HIDEHUD_PLAYERDEAD);
		break;
	case IF_BUILT:
		// Show only if deployed (built)
		SetHiddenBits(
			m_bDeployed
			? HIDEHUD_PLAYERDEAD
			: HIDEHUD_ALWAYS);
		break;
	case ON_BUILD:
		// Show if deploying (building) or deployed (built)
		SetHiddenBits(
			(m_bDeploying || m_bDeployed)
			? HIDEHUD_PLAYERDEAD
			: HIDEHUD_ALWAYS);
		break;
	case NEVER:
	default:
		SetHiddenBits(HIDEHUD_ALWAYS);
		break;
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

	if (!m_bDeploying)
	{
		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiHealth);
	}
	else if (!m_bDisableBuildTimer)
	{
		EnableItem(m_qiBuildProgress);
		DisableItem(m_qiHealth);
	}
}

void CHudBuildableManCannon::Init()
{
	HOOK_HUD_MESSAGE(CHudBuildableManCannon, ManCannonMsg);
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
	FFQuantityPanel::OnTick();

	m_bDeployed = false;
	m_bDeploying = false;

	SetText(m_wsNotDeployed);

	m_qiHealth->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);

	HideItem(m_qiHealth);
	DisableItem(m_qiBuildProgress);

	ApplyDisplayOptions();
}

void CHudBuildableManCannon::Paint()
{
	if (m_bDeploying)
	{
		float flTimeElapsed
			= gpGlobals->curtime - m_flBuildStartTime;

		if (m_flBuildDuration > flTimeElapsed)
			m_qiBuildProgress->SetAmount(flTimeElapsed);
	}

	FFQuantityPanel::Paint();
}

void CHudBuildableManCannon::MsgFunc_ManCannonMsg(
	bf_read& msg)
{
	BuildState_t state
		= static_cast<BuildState_t>(msg.ReadByte());

	int iHealth
		= static_cast<int>(msg.ReadByte());

	m_qiHealth->SetAmount(iHealth);
	m_qiBuildProgress->SetAmount(0.0f);

	switch (state)
	{
	case BUILDSTATE_NOTBUILT:
	{
		m_bDeployed = false;
		m_bDeploying = false;
		SetText(
			m_bHasManCannon
			? m_wsNotDeployed
			: m_wsNotAvailable);
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
		m_bDeployed = true;
		m_bDeploying = false;
		SetText(m_wsDeployed);
		ShowItem(m_qiHealth);
		DisableItem(m_qiBuildProgress);
		break;
	}
	}

	ApplyDisplayOptions();
}

void CHudBuildableManCannon::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();

	if (iBuildType != FF_BUILD_MANCANNON)
		return;

	m_bDeploying = true;
	SetText(m_wsDeploying);

	float flBuildDuration = msg.ReadFloat();

	m_flBuildDuration = flBuildDuration;
	m_flBuildStartTime = gpGlobals->curtime;

	m_qiBuildProgress->SetAmount(0.0f);
	m_qiBuildProgress->SetAmountMax(flBuildDuration);

	ApplyDisplayOptions();
}