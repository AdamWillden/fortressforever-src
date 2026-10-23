#include "cbase.h"

#include "ff_hud_timer_cloak.h"
#include "c_ff_player.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudTimerCloak);

CHudTimerCloak::CHudTimerCloak(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudTimerCloak")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(DEFAULT_HIDDEN_BITS);
	SetHeaderText(L"Cloak cooldown");
	SetHeaderIconChar('f');
	SetUseToggleText(true);

	m_qiTimer = nullptr;
}

CHudTimerCloak::~CHudTimerCloak()
{
}

void CHudTimerCloak::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Cloak");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Ready");
	if (localized)
		m_wsReady = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Cloaked");
	if (localized)
		m_wsCloaked = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;
}

void CHudTimerCloak::AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions)
{
	AddHeading(kvPanelSpecificOptions, "#GameUI_Panel", 0);
	AddHeading(kvPanelSpecificOptions, "#FF_ITEM_PROGRESS", 1);
	KeyValues* kvProgressVisibility = new KeyValues("Values");
	kvProgressVisibility->SetString("2", "#HudPanel_DisplayAlways");
	kvProgressVisibility->SetString("1", "#HudPanel_DisplayOnCooldown");
	kvProgressVisibility->SetString("0", "#HudPanel_DisplayNever");
	AddComboOption(
		kvPanelSpecificOptions,
		"ShowProgress",
		"#GameUI_Show",
		kvProgressVisibility,
		static_cast<int>(CooldownVisibility::ALWAYS),
		1);

	KeyValues* kvVisibility = new KeyValues("Values");
	kvVisibility->SetString("0", "#HudPanel_DisplayNever");
	kvVisibility->SetString("1", "#HudPanel_DisplayOnCooldown");
	kvVisibility->SetString("2", "#HudPanel_DisplayAlways");
	AddComboOption(
		kvPanelSpecificOptions,
		"Show",
		"#GameUI_Show",
		kvVisibility,
		static_cast<int>(CooldownVisibility::ALWAYS),
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideText",
		"#HudPanel_Option_HideText",
		false,
		0);

	KeyValues* kvMode = new KeyValues("Values");
	kvMode->SetString("0", "#HudPanel_Option_CountUp");
	kvMode->SetString("1", "#HudPanel_Option_CountDown");
	AddComboOption(
		kvPanelSpecificOptions,
		"CountDown",
		"#HudPanel_Option_Mode",
		kvMode,
		0,
		1);
}

KeyValues* CHudTimerCloak::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	KeyValues* kvPanelSpecificValues = new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("Show", static_cast<int>(CooldownVisibility::ALWAYS));
	kvPanelSpecificValues->SetInt("ShowProgress", static_cast<int>(CooldownVisibility::ALWAYS));
	kvPanelSpecificValues->SetBool("CountDown", false);
	kvPanelSpecificValues->SetBool("HideText", false);
	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudTimerCloak::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	BaseClass::ApplyStyleData(kvStyleData, kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);
	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaultStyleData->FindKey("PanelSpecificValues", true);

	int iShowProgress = GetInt("ShowProgress", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iShowProgress != -1)
		m_showProgress = static_cast<CooldownVisibility>(iShowProgress);

	int iShow = GetInt("Show", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iShow != -1)
		m_show = static_cast<CooldownVisibility>(iShow);

	int iHideText = GetInt("HideText", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideText != -1)
		SetUseToggleText(iHideText == 0);

	int iCountDown = GetInt("CountDown", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iCountDown != -1)
		m_bCountDown = iCountDown == 1;

	m_qiTimer->SetLabel(m_bCountDown ? "#FF_ITEM_TIMELEFT" : "#FF_ITEM_PROGRESS");
	m_qiTimer->SetAmountDisplay(m_bCountDown ? DISPLAY_RAW : DISPLAY_PERCENTAGE);
	m_qiTimer->SetAmountDecimalPlaces(m_bCountDown ? 1 : 0);

	ApplyDisplayOptions();
	if (IsInPreviewMode())
	{
		m_qiTimer->SetAmount(SPY_NEXTCLOAK / 2);
		SetText(m_wsRegenerating);
	}
}

void CHudTimerCloak::ApplyDisplayOptions()
{
	UpdateProgressVisibility();
	if (m_show != CooldownVisibility::NEVER)
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
}

void CHudTimerCloak::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);

	m_qiTimer = AddItem("Cooldown");
	m_qiTimer->SetLabel("#FF_ITEM_PROGRESS");
	m_qiTimer->SetIcon('f');
	m_qiTimer->SetAmountMax(SPY_NEXTCLOAK);
	m_qiTimer->SetAmountDisplay(DISPLAY_PERCENTAGE);

	AddPanelToHudOptions(
		"Cloak",
		"#HudPanel_Cloak",
		"Cooldown",
		"#HudPanel_Cooldown");
}

void CHudTimerCloak::VidInit()
{
	m_qiTimer->SetAmount(0);
	ApplyDisplayOptions();
}

bool CHudTimerCloak::ShouldDraw()
{
	if (m_show == CooldownVisibility::NEVER || !CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	if (!engine->IsInGame())
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer)
		return false;

	return pPlayer->GetClassSlot() == CLASS_SPY
		&& !FF_IsPlayerSpec(pPlayer)
		&& FF_HasPlayerPickedClass(pPlayer)
		&& (m_show == CooldownVisibility::ALWAYS
			|| (!pPlayer->IsCloaked() && pPlayer->GetNextCloak() > gpGlobals->curtime));
}

void CHudTimerCloak::Paint()
{
	if (!IsInPreviewMode())
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (!pPlayer)
			return;

		// The cooldown can finish between the visibility check and painting.
		float flTimeLeft = clamp(pPlayer->GetNextCloak() - gpGlobals->curtime, 0.0f, SPY_NEXTCLOAK);
		if (pPlayer->IsCloaked())
		{
			m_qiTimer->SetAmount(0);
			SetText(m_wsCloaked);
		}
		else
		{
			m_qiTimer->SetAmount(m_bCountDown ? flTimeLeft : SPY_NEXTCLOAK - flTimeLeft);
			SetText(flTimeLeft > 0 ? m_wsRegenerating : m_wsReady);
		}
	}

	BaseClass::Paint();
}

void CHudTimerCloak::UpdateProgressVisibility()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	const bool bCooling = IsInPreviewMode()
		|| (pPlayer && !pPlayer->IsCloaked() && pPlayer->GetNextCloak() > gpGlobals->curtime);

	if (m_showProgress == CooldownVisibility::ALWAYS
		|| (m_showProgress == CooldownVisibility::ON_COOLDOWN && bCooling))
		EnableItem(m_qiTimer);
	else
		DisableItem(m_qiTimer);
}

void CHudTimerCloak::OnTick()
{
	UpdateProgressVisibility();
	BaseClass::OnTick();
}
