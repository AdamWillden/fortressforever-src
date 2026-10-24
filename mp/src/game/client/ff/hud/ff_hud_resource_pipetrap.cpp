#include "cbase.h"

using namespace vgui;
#include "ff_shareddefs.h"

#include "ff_hud_resource_pipetrap.h"

enum {
	RESET_PIPES = 0,
	INCREMENT_PIPES,
	DECREMENT_PIPES
};

DECLARE_HUDELEMENT(CHudResourcePipeTrap);
DECLARE_HUD_MESSAGE(CHudResourcePipeTrap, PipeMsg);

CHudResourcePipeTrap::CHudResourcePipeTrap(
	const char* pElementName)
	: CHudElement(pElementName),
	BaseClass(NULL, "HudResourcePipeTrap"),
	m_wsDeployed(L"Deployed"),
	m_wsNeedAmmo(L"Need ammo!"),
	m_wsNotDeployed(L"Not Deployed")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_ALWAYS);

	SetHeaderText(L"Pipebombs");
	SetHeaderIconChar('o');
	SetUseToggleText(true);
	SetText(m_wsNotDeployed);

	m_qiPipeLaid = nullptr;
}

CHudResourcePipeTrap::~CHudResourcePipeTrap()
{
}

void CHudResourcePipeTrap::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_PipeTrap");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Deployed");
	if (localized)
		m_wsDeployed = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NeedAmmo");
	if (localized)
		m_wsNeedAmmo = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NotDeployed");
	if (localized)
		m_wsNotDeployed = localized;
}

void CHudResourcePipeTrap::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_General", 0);
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_QuantityDisplay", 1);

	KeyValues* kvVisibility = new KeyValues("Values");
	kvVisibility->SetString("0", "#HudPanel_DisplayNever");
	kvVisibility->SetString("2", "#HudPanel_DisplayIfDeployed");
	kvVisibility->SetString("1", "#HudPanel_DisplayAlways");

	AddComboOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		kvVisibility,
		1,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"ShowPipes",
		"#HudPanel_Option_ShowPipes",
		kvVisibility->MakeCopy(),
		IF_DEPLOYED,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"ShowText",
		"#HudPanel_Option_ShowText",
		kvVisibility->MakeCopy(),
		ALWAYS,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"ValueDisplay",
		"#HudPanel_Option_ValueDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
}

KeyValues* CHudResourcePipeTrap::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= BaseClass::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPanelSpecificValues->SetInt("ShowPipes", IF_DEPLOYED);
	kvPanelSpecificValues->SetInt("ShowText", ALWAYS);

	kvPanelSpecificValues->SetInt("ValueDisplay", DISPLAY_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudResourcePipeTrap::ApplyStyleData(
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

	std::optional<PanelVisibility> showPanel
		= GetPanelVisibility(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<PanelVisibility> showPipes
		= GetPanelVisibility(
			"ShowPipes",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<PanelVisibility> showText
		= GetPanelVisibility(
			"ShowText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<AmountDisplay> valueDisplay
		= GetAmountDisplay(
			"ValueDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (showPanel.has_value())
		m_showPanel = showPanel.value();

	if (showPipes.has_value())
		m_showPipes = showPipes.value();

	if (showText.has_value())
		m_showText = showText.value();

	// Preserve assignments saved with the former Hide Text checkbox.
	if (!kvPanelSpecificValues->FindKey("ShowText")
		&& kvPanelSpecificValues->FindKey("HideText"))
	{
		m_showText = kvPanelSpecificValues->GetBool("HideText") ? NEVER : ALWAYS;
	}

	if (valueDisplay.has_value())
		m_qiPipeLaid->SetAmountDisplay(
			valueDisplay.value());

	ApplyDisplayOptions();
}

std::optional<CHudResourcePipeTrap::PanelVisibility> CHudResourcePipeTrap::GetPanelVisibility(
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
		? std::optional<PanelVisibility>()
		: static_cast<PanelVisibility>(value);
}

void CHudResourcePipeTrap::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 200);
	HOOK_HUD_MESSAGE(CHudResourcePipeTrap, PipeMsg);

	m_qiPipeLaid = AddItem("HudResourcePipeTrapLaid");
	m_qiPipeLaid->SetLabel("#FF_ITEM_PIPES");
	m_qiPipeLaid->SetIcon('o');
	m_qiPipeLaid->SetAmountDisplay(DISPLAY_MAX);
	m_qiPipeLaid->SetAmountMax(FF_MAX_PIPEBOMBS);

	AddPanelToHudOptions(
		"PipeTrap",
		"#HudPanel_PipeTrap",
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourcePipeTrap::VidInit()
{
	m_iNumPipes = 0;
	m_bHasPipeLauncher = false;
	ApplyDisplayOptions();
	SetText(m_wsNotDeployed);
	m_qiPipeLaid->SetAmount(0);
}

void CHudResourcePipeTrap::OnTick()
{
	BaseClass::OnTick();
	if (IsInPreviewMode())
		return;

	if (!engine->IsInGame())
		return;

	C_FFPlayer* pPlayer
		= C_FFPlayer::GetLocalFFPlayer();

	if (!pPlayer)
		return;

	const bool bHasPipeLauncher
		= pPlayer->Weapon_OwnsThisType("ff_weapon_pipelauncher") != nullptr;
	if (bHasPipeLauncher != m_bHasPipeLauncher)
	{
		m_bHasPipeLauncher = bHasPipeLauncher;
		ApplyDisplayOptions();
	}

	if (m_iNumPipes > 0)
	{
		SetText(m_wsDeployed);
	}
	else
	{
		SetText(m_wsNotDeployed);
	}

	// Allow overriding deploy status with ammo status
	if (pPlayer->GetAmmoCount(AMMO_ROCKETS) <= 0)
		SetText(m_wsNeedAmmo);
}

void CHudResourcePipeTrap::MsgFunc_PipeMsg(bf_read& msg)
{
	int iIncrementPipes = (int)msg.ReadByte();
	switch (iIncrementPipes)
	{
	case INCREMENT_PIPES:
		m_iNumPipes++;
		break;
	case DECREMENT_PIPES:
		m_iNumPipes--;
		break;
	case RESET_PIPES:
	default:
		m_iNumPipes = 0;
		break;
	}

	m_iNumPipes = max(0, m_iNumPipes);
	m_qiPipeLaid->SetAmount(m_iNumPipes);
	ApplyDisplayOptions();
}

void CHudResourcePipeTrap::ApplyDisplayOptions()
{
	const bool bDeployed = IsInPreviewMode() || m_iNumPipes > 0;

	SetUseToggleText(m_showText != NEVER);
	SetToggleTextVisible(m_showText == ALWAYS || (m_showText == IF_DEPLOYED && bDeployed));

	if (m_showPipes == ALWAYS || (m_showPipes == IF_DEPLOYED && bDeployed))
		EnableItem(m_qiPipeLaid);
	else
		DisableItem(m_qiPipeLaid);

	ShowItem(m_qiPipeLaid);

	if (IsInPreviewMode())
	{
		SetHiddenBits(m_showPanel != NEVER ? 0 : HIDEHUD_ALWAYS);
		SetText(m_wsDeployed);
		return;
	}

	m_qiPipeLaid->SetAmount(m_iNumPipes);
	if (m_iNumPipes > 0)
	{
		SetText(m_wsDeployed);
	}
	else
	{
		SetText(m_wsNotDeployed);
	}

	if (m_showPanel != NEVER
		&& (m_showPanel != IF_DEPLOYED || m_iNumPipes > 0)
		&& (m_bHasPipeLauncher || m_iNumPipes > 0))
	{
		SetHiddenBits(DEFAULT_HIDDEN_BITS);
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}
}