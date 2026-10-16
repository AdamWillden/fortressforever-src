#include "cbase.h"

#include "ff_hud_resource_medpacks.h"
#include "c_ff_player.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudResourceMedpacks);

CHudResourceMedpacks::CHudResourceMedpacks(
	const char* pElementName)
	: CHudElement(pElementName),
	BaseClass(NULL, "HudResourceMedpacks"),
	m_wsFull(L"Full"),
	m_wsRegenerating(L"Regenerating...")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_ALWAYS);

	SetHeaderText(L"Medpacks");
	SetHeaderIconChar('c');
	SetUseToggleText(true);
	SetText(m_wsRegenerating);

	m_qiMedpacks = nullptr;
}

CHudResourceMedpacks::~CHudResourceMedpacks()
{
}

void CHudResourceMedpacks::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Medpacks");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Full");
	if (localized)
		m_wsFull = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;
}

void CHudResourceMedpacks::AddPanelSpecificOptions(
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
		"HideText",
		"#HudPanel_Option_HideText",
		false,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"ValueDisplay",
		"#HudPanel_Option_ValueDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
}

KeyValues* CHudResourceMedpacks::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("HideText", false);
	kvPanelSpecificValues->SetBool("DisableBuildTimer", false);

	kvPanelSpecificValues->SetInt("ValueDisplay", DISPLAY_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudResourceMedpacks::ApplyStyleData(
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

	int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	int iHideText
		= GetInt(
			"HideText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountDisplay> valueDisplay
		= GetAmountDisplay(
			"ValueDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iShowPanel != -1)
		m_bShowPanel = (iShowPanel == 1);

	if (iHideText != -1)
	{
		if (iHideText == 1)
			SetUseToggleText(false);
		else
			SetUseToggleText(true);
	}

	if (valueDisplay.has_value())
		m_qiMedpacks->SetAmountDisplay(
			valueDisplay.value());

	ApplyDisplayOptions();
}

void CHudResourceMedpacks::ApplyDisplayOptions()
{
	if (m_bShowPanel)
	{
		if (IsInPreviewMode())
			SetHiddenBits(0);
		else if (m_iMedpackMax > 0)
			SetHiddenBits(HIDEHUD_PLAYERDEAD);
		else
			SetHiddenBits(HIDEHUD_ALWAYS);
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}
}

void CHudResourceMedpacks::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	m_qiMedpacks = AddItem("HudResourceMedpacks");
	m_qiMedpacks->SetLabel("#FF_ITEM_MEDPACKS");
	m_qiMedpacks->SetIcon('c');
	m_qiMedpacks->SetAmountDisplay(DISPLAY_MAX);
	m_qiMedpacks->SetAmountMax(8);

	AddPanelToHudOptions(
		"Medpacks",
		"#HudPanel_Medpacks",
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceMedpacks::VidInit()
{
	m_iMedpackCount = -1;
	m_iMedpackMax = -1;
	m_qiMedpacks->SetAmount(0);
	SetText(m_wsRegenerating);
	ApplyDisplayOptions();
	UpdateFromPlayer(C_FFPlayer::GetLocalFFPlayer());
}

void CHudResourceMedpacks::UpdateFromPlayer(const C_FFPlayer* pPlayer)
{
	if (!pPlayer || !m_qiMedpacks)
		return;

	const int iMedpackCount = pPlayer->GetMedpackCount();
	const int iMedpackMax = pPlayer->GetMedpackMax();
	const bool bStateUnchanged =
		m_iMedpackCount == iMedpackCount
		&& m_iMedpackMax == iMedpackMax;

	if (bStateUnchanged)
		return;

	m_iMedpackCount = iMedpackCount;
	m_iMedpackMax = iMedpackMax;
	m_qiMedpacks->SetAmountMax(MAX(1, m_iMedpackMax));
	m_qiMedpacks->SetAmount(m_iMedpackCount);

	if (m_iMedpackMax > 0 && m_iMedpackCount >= m_iMedpackMax)
		SetText(m_wsFull);
	else
		SetText(m_wsRegenerating);

	ApplyDisplayOptions();
}
