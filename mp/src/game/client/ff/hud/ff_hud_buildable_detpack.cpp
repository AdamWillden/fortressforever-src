#include "cbase.h"

using namespace vgui;

#include "ff_hud_buildable_detpack.h"

KeyValues* CHudBuildableDetpack::s_kvVisibilityDisplayOptions;

DECLARE_HUDELEMENT(CHudBuildableDetpack);
DECLARE_HUD_MESSAGE(CHudBuildableDetpack, FF_BuildTimer);
DECLARE_HUD_MESSAGE(CHudBuildableDetpack, DetpackMsg);
DECLARE_HUD_MESSAGE(CHudBuildableDetpack, DetpackAvailabilityMsg);

CHudBuildableDetpack::CHudBuildableDetpack(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudBuildableDetpack"),
	m_wsArmed(L"Armed"),
	m_wsArming(L"Arming..."),
	m_wsAvailable(L"Available"),
	m_wsNotAvailable(L"Not Available")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_ALWAYS);

	SetHeaderText(L"Detpack");
	SetHeaderIconChar('5');
	SetUseToggleText(true);
	SetText(m_wsNotAvailable);

	m_qiBuildProgress = nullptr;
	m_qiDetpackTimeLeft = nullptr;

	if (!s_kvVisibilityDisplayOptions)
	{
		KeyValues* kvVisibilityDisplayOptions
			= new KeyValues("Values");

		kvVisibilityDisplayOptions->SetString(
			std::to_string(FFQuantityHelper::NEVER).c_str(),
			"#HudPanel_DisplayNever");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(FFQuantityHelper::ALWAYS).c_str(),
			"#HudPanel_DisplayAlways");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(FFQuantityHelper::ON_BUILD).c_str(),
			"#HudPanel_DisplayOnBuild");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(FFQuantityHelper::IF_BUILT).c_str(),
			"#HudPanel_DisplayIfBuilt");

		s_kvVisibilityDisplayOptions
			= kvVisibilityDisplayOptions;
	}
}

CHudBuildableDetpack::~CHudBuildableDetpack()
{
}

void CHudBuildableDetpack::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Detpack");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Armed");
	if (localized)
		m_wsArmed = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Arming");
	if (localized)
		m_wsArming = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Available");
	if (localized)
		m_wsAvailable = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NotAvailable");
	if (localized)
		m_wsNotAvailable = localized;
}

void CHudBuildableDetpack::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_Visibility", 0);
	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_QuantityDisplay", 1);

	KeyValues* kvShowPanelOptions = s_kvVisibilityDisplayOptions->MakeCopy();
	kvShowPanelOptions->SetString(
		std::to_string(FFQuantityHelper::IF_AVAILABLE).c_str(), "#HudPanel_DisplayIfAvailable");

	AddComboOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		kvShowPanelOptions,
		FFQuantityHelper::ALWAYS,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideText",
		"#HudPanel_Option_HideText",
		false,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"DisableBuildTimer",
		"#HudPanel_Option_HideBuildTimer",
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

KeyValues* CHudBuildableDetpack::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityHelper::ALIGN_BOTTOM);

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("ShowPanel", FFQuantityHelper::ALWAYS);
	kvPanelSpecificValues->SetBool("HideText", false);
	kvPanelSpecificValues->SetBool("DisableBuildTimer", false);

	kvPanelSpecificValues->SetInt("ValueDisplay", DISPLAY_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudBuildableDetpack::ApplyStyleData(
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

	std::optional<AmountDisplay> valueDisplay
		= GetAmountDisplay(
			"ValueDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (valueDisplay.has_value())
	{
		m_qiDetpackTimeLeft->SetAmountDisplay(
			valueDisplay.value());
	}

	std::optional<FFQuantityHelper::BuildableDisplayOption> showPanel
		= GetBuildableDisplayOption(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	int iHideText
		= GetInt(
			"HideText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iDisableBuildTimer
		= GetInt(
			"DisableBuildTimer",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	bool bApplyDisplayOptions = false;

	if (showPanel.has_value()
		&& Change(m_showPanel, showPanel.value()))
	{
		bApplyDisplayOptions = true;
	}

	if (iHideText != -1
		&& Change(m_bHideText, iHideText == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (iDisableBuildTimer != -1)
	{
		m_bDisableBuildTimer = iDisableBuildTimer == 1;
	}

	if (bApplyDisplayOptions || kvStyleData->GetInt("previewMode", -1) != -1)
		ApplyDisplayOptions();
}

std::optional<FFQuantityHelper::BuildableDisplayOption> CHudBuildableDetpack::GetBuildableDisplayOption(
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
		? std::optional<FFQuantityHelper::BuildableDisplayOption>()
		: static_cast<FFQuantityHelper::BuildableDisplayOption>(value);
}

void CHudBuildableDetpack::ApplyDisplayOptions()
{
	if (IsInPreviewMode())
	{
		SetHiddenBits(m_showPanel == FFQuantityHelper::NEVER ? HIDEHUD_ALWAYS : 0);
		SetUseToggleText(!m_bHideText);
		SetText(m_wsArmed);
		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiDetpackTimeLeft);
		ShowItem(m_qiDetpackTimeLeft);
		return;
	}

	if (m_bArmed)
		ShowItem(m_qiDetpackTimeLeft);
	else
		HideItem(m_qiDetpackTimeLeft);

	if (m_bArming)
		SetText(m_wsArming);
	else if (m_bArmed)
		SetText(m_wsArmed);
	else
		UpdateAvailabilityText();

	bool bShowPanel = false;
	switch (m_showPanel)
	{
	case FFQuantityHelper::ALWAYS:
		bShowPanel = m_bCanHaveDetpack || m_bHasDetpack || m_bArming || m_bArmed;
		break;
	case FFQuantityHelper::IF_AVAILABLE:
		bShowPanel = m_bHasDetpack || m_bArming || m_bArmed;
		break;
	case FFQuantityHelper::IF_BUILT:
		bShowPanel = m_bArmed;
		break;
	case FFQuantityHelper::ON_BUILD:
		bShowPanel = m_bArming || m_bArmed;
		break;
	case FFQuantityHelper::NEVER:
	default:
		break;
	}

	if (bShowPanel)
		SetHiddenBits(DEFAULT_HIDDEN_BITS);
	else
		SetHiddenBits(HIDEHUD_ALWAYS);

	if (m_bHideText)
		SetUseToggleText(false);
	else
		SetUseToggleText(true);

	if (!m_bArming)
	{
		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiDetpackTimeLeft);
	}
	else if (!m_bDisableBuildTimer)
	{
		EnableItem(m_qiBuildProgress);
		DisableItem(m_qiDetpackTimeLeft);
	}
}

void CHudBuildableDetpack::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);
	HOOK_HUD_MESSAGE(CHudBuildableDetpack, FF_BuildTimer);
	HOOK_HUD_MESSAGE(CHudBuildableDetpack, DetpackMsg);
	HOOK_HUD_MESSAGE(CHudBuildableDetpack, DetpackAvailabilityMsg);

	m_qiBuildProgress = AddItem("BuildProgress");
	m_qiBuildProgress->SetLabel("#FF_ITEM_PROGRESS");
	m_qiBuildProgress->SetIcon('f');
	m_qiBuildProgress->SetAmountDisplay(DISPLAY_PERCENTAGE);

	m_qiDetpackTimeLeft = AddItem("TimeLeft");
	m_qiDetpackTimeLeft->SetLabel("#FF_ITEM_TIMELEFT");
	m_qiDetpackTimeLeft->SetIcon('f');

	AddPanelToHudOptions(
		"Detpack",
		"#HudPanel_Detpack",
		"Buildables",
		"#HudPanel_Buildables");
}

void CHudBuildableDetpack::VidInit()
{
	m_bArmed = false;
	m_bArming = false;

	SetText(m_wsNotAvailable);

	m_qiDetpackTimeLeft->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);

	HideItem(m_qiDetpackTimeLeft);
	DisableItem(m_qiBuildProgress);

	ApplyDisplayOptions();
}

void CHudBuildableDetpack::OnTick()
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

	bool bArmed = pPlayer->GetDetpack() && pPlayer->GetDetpack()->IsBuilt();
	bool bArming = pPlayer->GetDetpack() && !bArmed;

	// Update arming state
	if (bArming != m_bArming)
	{
		m_bArming = bArming;

		// The true value is handled in the
		// build timer for responsive updates

		if (!m_bArming)
			// We assume the player cancelled,
			// which is overridden below if armed.
			SetText(m_wsAvailable);

		ApplyDisplayOptions();
	}

	// Update armed state
	if (bArmed != m_bArmed)
	{
		m_bArmed = bArmed;

		if (m_bArmed)
		{
			SetText(m_wsArmed);
			ShowItem(m_qiDetpackTimeLeft);
		}
		else
		{
			// Exploded or defused
			SetText(m_wsAvailable);
			HideItem(m_qiDetpackTimeLeft);
		}

		ApplyDisplayOptions();
	}

	UpdateAvailabilityText();
}

void CHudBuildableDetpack::Paint()
{
	if(!IsInPreviewMode() && m_bArming)
	{
		float flTimeElapsed
			= gpGlobals->curtime - m_flBuildStartTime;

		if (m_flBuildDuration > flTimeElapsed)
			m_qiBuildProgress->SetAmount(flTimeElapsed);
	}

	if (!IsInPreviewMode() && m_bArmed)
	{
		float flCurTime = gpGlobals->curtime;

		float flDetpackTimeLeft
			= m_flDetonateTime - flCurTime;

		// Prevent showing negative zero time left
		flDetpackTimeLeft
			= max(flDetpackTimeLeft, 0.0f);

		m_qiDetpackTimeLeft->SetAmount(flDetpackTimeLeft);
	}

	BaseClass::Paint();
}

void CHudBuildableDetpack::MsgFunc_DetpackMsg(
	bf_read& msg)
{
	m_flDetonateTime = msg.ReadFloat();
	m_qiDetpackTimeLeft->SetAmountMax(msg.ReadByte());
}

void CHudBuildableDetpack::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();

	if (iBuildType != FF_BUILD_DETPACK)
		return;

	float flBuildDuration = msg.ReadFloat();
	if (flBuildDuration <= 0)
	{
		m_bArming = false;
		ApplyDisplayOptions();
		return;
	}

	m_bArming = true;
	SetText(m_wsArming);

	m_flBuildDuration = flBuildDuration;
	m_flBuildStartTime = gpGlobals->curtime;

	m_qiBuildProgress->SetAmount(0.0f);
	m_qiBuildProgress->SetAmountMax(flBuildDuration);

	ApplyDisplayOptions();
}

void CHudBuildableDetpack::UpdateAvailabilityText()
{
	if (!m_bArming && !m_bArmed)
	{
		if (m_bHasDetpack)
			SetText(m_wsAvailable);
		else
			SetText(m_wsNotAvailable);
	}
}

void CHudBuildableDetpack::MsgFunc_DetpackAvailabilityMsg(bf_read& msg)
{
	m_bCanHaveDetpack = msg.ReadByte() != 0;
	m_bHasDetpack = msg.ReadByte() != 0;
	UpdateAvailabilityText();
	ApplyDisplayOptions();
}