#include "cbase.h"

#include "ff_hud_timer_overpressure.h"
#include "c_ff_player.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudTimerOverpressure);

CHudTimerOverpressure::CHudTimerOverpressure(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudTimerOverpressure")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(DEFAULT_HIDDEN_BITS);
	SetHeaderText(L"Overpressure");
	SetHeaderIconChar('f');
	SetUseToggleText(true);
	SetText(m_wsReady);

	m_qiTimer = nullptr;
}

CHudTimerOverpressure::~CHudTimerOverpressure()
{
}

void CHudTimerOverpressure::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Overpressure");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Ready");
	if (localized)
		m_wsReady = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;
}

void CHudTimerOverpressure::AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions)
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

	AddComboOption(
		kvPanelSpecificOptions,
		"ValueDisplay",
		"#HudPanel_Option_ValueDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_PERCENTAGE,
		1);
}

KeyValues* CHudTimerOverpressure::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	KeyValues* kvPanelSpecificValues = new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("Show", static_cast<int>(CooldownVisibility::ALWAYS));
	kvPanelSpecificValues->SetInt("ShowProgress", static_cast<int>(CooldownVisibility::ALWAYS));
	kvPanelSpecificValues->SetBool("HideText", false);
	kvPanelSpecificValues->SetBool("CountDown", false);
	kvPanelSpecificValues->SetInt("ValueDisplay", DISPLAY_PERCENTAGE);
	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudTimerOverpressure::ApplyStyleData(
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
		m_bHideText = iHideText == 1;

	int iCountDown = GetInt("CountDown", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iCountDown != -1)
		m_bCountDown = iCountDown == 1;

	std::optional<AmountDisplay> valueDisplay = GetAmountDisplay(
		"ValueDisplay",
		kvPanelSpecificValues,
		kvDefaultPanelSpecificValues);
	if (valueDisplay.has_value())
		m_valueDisplay = valueDisplay.value();

	m_qiTimer->SetAmountDisplay(m_bCountDown ? DISPLAY_RAW : m_valueDisplay);
	m_qiTimer->SetAmountDecimalPlaces(m_bCountDown ? 1 : 0);
	m_qiTimer->SetLabel(m_bCountDown ? "#FF_ITEM_TIMELEFT" : "#FF_ITEM_PROGRESS");

	ApplyDisplayOptions();
}

void CHudTimerOverpressure::ApplyDisplayOptions()
{
	UpdateProgressVisibility();
	SetUseToggleText(!m_bHideText);

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

void CHudTimerOverpressure::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);

	m_qiTimer = AddItem("Cooldown");
	m_qiTimer->SetLabel("#FF_ITEM_PROGRESS");
	m_qiTimer->SetIcon('f');
	m_qiTimer->SetAmountMax(OVERPRESSURE_DELAY);
	m_qiTimer->SetAmountDisplay(DISPLAY_PERCENTAGE);

	AddPanelToHudOptions(
		"Overpressure",
		"#HudPanel_Overpressure",
		"Cooldown",
		"#HudPanel_Cooldown");
}

void CHudTimerOverpressure::VidInit()
{
	m_qiTimer->SetAmount(m_bCountDown ? 0 : OVERPRESSURE_DELAY);
	SetText(m_wsReady);
	ApplyDisplayOptions();
}

bool CHudTimerOverpressure::ShouldDraw()
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

	if (m_show == CooldownVisibility::ON_COOLDOWN
		&& pPlayer->m_flNextClassSpecificSkill <= gpGlobals->curtime)
		return false;

	return pPlayer->GetClassSlot() == CLASS_HWGUY
		&& !FF_IsPlayerSpec(pPlayer)
		&& FF_HasPlayerPickedClass(pPlayer);
}

void CHudTimerOverpressure::Paint()
{
	if (!IsInPreviewMode())
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (!pPlayer)
			return;

		float flTimeLeft = pPlayer->m_flNextClassSpecificSkill - gpGlobals->curtime;
		if (flTimeLeft > 0)
		{
			m_qiTimer->SetAmount(m_bCountDown ? flTimeLeft : OVERPRESSURE_DELAY - flTimeLeft);
			SetText(m_wsRegenerating);
		}
		else
		{
			m_qiTimer->SetAmount(m_bCountDown ? 0 : OVERPRESSURE_DELAY);
			SetText(m_wsReady);
		}
	}

	BaseClass::Paint();
}

void CHudTimerOverpressure::UpdateProgressVisibility()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	const bool bCooling = IsInPreviewMode()
		|| (pPlayer && pPlayer->m_flNextClassSpecificSkill > gpGlobals->curtime);

	if (m_showProgress == CooldownVisibility::ALWAYS
		|| (m_showProgress == CooldownVisibility::ON_COOLDOWN && bCooling))
		EnableItem(m_qiTimer);
	else
		DisableItem(m_qiTimer);
}

void CHudTimerOverpressure::OnTick()
{
	UpdateProgressVisibility();
	BaseClass::OnTick();
}
