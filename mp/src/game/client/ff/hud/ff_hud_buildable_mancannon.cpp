#include "cbase.h"
#include "ff_hud_measure.h"

#include "ff_hud_buildable_mancannon.h"

using namespace vgui;

KeyValues* CHudBuildableManCannon::s_kvVisibilityDisplayOptions;

DECLARE_HUDELEMENT(CHudBuildableManCannon);

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
	if (m_bCanHaveManCannon)
	{
		switch (m_showPanel)
		{
		case ALWAYS:
			SetHiddenBits(HIDEHUD_PLAYERDEAD);
			break;
		case IF_BUILT:
			// Show only if deployed (built)
			SetHiddenBits(m_bDeployed
				? HIDEHUD_PLAYERDEAD
				: HIDEHUD_ALWAYS);
			break;
		case ON_BUILD:
			// Show if deploying (building) or deployed (built)
			SetHiddenBits((m_bDeploying || m_bDeployed)
				? HIDEHUD_PLAYERDEAD
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
	ivgui()->AddTickSignal(GetVPanel(), 250);

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
	m_bCanHaveManCannon = false;
	m_bHasManCannon = false;
	m_iHealth = -1;
	m_flBuildStartTime = 0;
	m_flBuildDuration = 0;
	m_bDeployed = false;
	m_bDeploying = false;

	SetText(m_wsNotDeployed);

	m_qiHealth->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);

	HideItem(m_qiHealth);
	DisableItem(m_qiBuildProgress);

	ApplyDisplayOptions();
	UpdateFromPlayer(C_FFPlayer::GetLocalFFPlayer());
}

void CHudBuildableManCannon::Paint()
{
	if (m_bDeploying)
	{
		float flTimeElapsed
			= gpGlobals->curtime - m_flBuildStartTime;

		m_qiBuildProgress->SetAmount(clamp(flTimeElapsed, 0.0f, m_flBuildDuration));
	}

	FFQuantityPanel::Paint();
}

void CHudBuildableManCannon::UpdateFromPlayer(const C_FFPlayer* pPlayer)
{
	FFHudMeasureScope measure(HUD_MEASURE_MANCANNON);
	if (!pPlayer || !m_qiHealth || !m_qiBuildProgress)
		return;

	bool bDeploying = pPlayer->GetManCannonBuildState() == BUILDSTATE_BUILDING;
	bool bDeployed = pPlayer->GetManCannonBuildState() == BUILDSTATE_BUILT;
	float flStart = pPlayer->GetManCannonBuildStart();
	float flDuration = MAX(0.0f, pPlayer->GetManCannonBuildEnd() - flStart);

	const bool bStateUnchanged =
		m_bCanHaveManCannon == pPlayer->CanHaveManCannon()
		&& m_bHasManCannon == pPlayer->HasManCannon()
		&& m_bDeploying == bDeploying
		&& m_bDeployed == bDeployed
		&& m_iHealth == pPlayer->GetManCannonHealth()
		&& m_flBuildStartTime == flStart
		&& m_flBuildDuration == flDuration;

	if (bStateUnchanged)
		return;

	measure.Changed();

	m_bCanHaveManCannon = pPlayer->CanHaveManCannon();
	m_bHasManCannon = pPlayer->HasManCannon();
	m_bDeploying = bDeploying;
	m_bDeployed = bDeployed;
	m_iHealth = pPlayer->GetManCannonHealth();
	m_flBuildStartTime = flStart;
	m_flBuildDuration = flDuration;

	if (m_bDeployed)
	{
		ShowItem(m_qiHealth);
		SetText(m_wsDeployed);
	}
	else
	{
		HideItem(m_qiHealth);

		if (m_bDeploying)
		{
			SetText(m_wsDeploying);
		}
		else if (m_bHasManCannon)
		{
			SetText(m_wsNotDeployed);
		}
		else
		{
			SetText(m_wsNotAvailable);
		}
	}

	m_qiHealth->SetAmount(m_iHealth);

	m_qiBuildProgress->SetAmountMax(MAX(0.001f, m_flBuildDuration));
	m_qiBuildProgress->SetAmount(0.0f);

	ApplyDisplayOptions();
}
