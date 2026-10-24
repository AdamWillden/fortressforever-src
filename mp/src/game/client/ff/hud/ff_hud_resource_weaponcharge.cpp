#include "cbase.h"
#include "ff_hud_resource_weaponcharge.h"
#include "hud_macros.h"
#include "c_ff_player.h"
#include "ff_weapon_assaultcannon.h"
#include "ff_weapon_sniperrifle.h"
#include "ff_weapon_jumpgun.h"
#include "ff_utils.h"
#include "ff_shareddefs.h"
#include <vgui/IVGui.h>

using namespace vgui;
using namespace FFQuantityHelper;

DECLARE_HUDELEMENT(CHudResourceSniperCharge);
DECLARE_HUDELEMENT(CHudResourceAssaultCannonCharge);
DECLARE_HUDELEMENT(CHudResourceJumpgunCharge);

CHudResourceSniperCharge::CHudResourceSniperCharge(const char* pElementName)
	: CHudResourceWeaponCharge(
		pElementName,
		FF_WEAPON_SNIPERRIFLE,
		"#HudPanel_SniperCharge",
		true)
{
}

CHudResourceAssaultCannonCharge::CHudResourceAssaultCannonCharge(const char* pElementName)
	: CHudResourceWeaponCharge(
		pElementName,
		FF_WEAPON_ASSAULTCANNON,
		"#HudPanel_AssaultCannonCharge",
		true)
{
}

CHudResourceJumpgunCharge::CHudResourceJumpgunCharge(const char* pElementName)
	: CHudResourceWeaponCharge(
		pElementName,
		FF_WEAPON_JUMPGUN,
		"#HudPanel_JumpgunCharge",
		true)
{
}

CHudResourceWeaponCharge::CHudResourceWeaponCharge(
	const char* pElementName,
	int iWeaponID,
	const char* pszTitle,
	bool bShowPanel)
	: CHudElement(pElementName), BaseClass(NULL, pElementName),
	m_iWeaponID(iWeaponID), m_pszTitle(pszTitle),
	m_bDefaultShowPanel(bShowPanel), m_showPanel(bShowPanel ? WHEN_ACTIVE : NEVER)
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(DEFAULT_HIDDEN_BITS);

	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
		SetUseToggleText(true);
}

void CHudResourceWeaponCharge::Init()
{
	m_pCharge = AddItem("Charge");
	m_pCharge->SetLabel(m_iWeaponID == FF_WEAPON_ASSAULTCANNON
		? "#HudPanel_WindUp" : "#HudPanel_Charge");
	m_pCharge->SetIcon('f');
	m_pCharge->SetAmountMax(100);
	m_pCharge->SetAmountDisplay(DISPLAY_PERCENTAGE);
	ivgui()->AddTickSignal(GetVPanel(), 16);
	AddPanelToHudOptions(
		FFQuantityPanel::GetName(),
		m_pszTitle,
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceWeaponCharge::VidInit()
{
	m_pCharge->SetAmount(0);
}

void CHudResourceWeaponCharge::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	wchar_t* pszTitle = g_pVGuiLocalize->Find(m_pszTitle);
	if (pszTitle)
		SetHeaderText(pszTitle);

	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
	{
		wchar_t* pszAnchored = g_pVGuiLocalize->Find("#HudPanel_Anchored");
		if (pszAnchored)
			SetText(pszAnchored);
	}
}

void CHudResourceWeaponCharge::AddPanelSpecificOptions(KeyValues* kvOptions)
{
	AddHeading(kvOptions, "#HudPanel_Heading_General", 0);
	AddHeading(kvOptions, "#HudPanel_Heading_QuantityDisplay", 1);

	KeyValues* kvVisibility = new KeyValues("Values");
	kvVisibility->SetString("0", "#HudPanel_DisplayNever");
	kvVisibility->SetString("1", "#HudPanel_DisplayWhenActive");
	kvVisibility->SetString("2", "#HudPanel_DisplayWhileEquipped");

	AddComboOption(
		kvOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		kvVisibility,
		m_bDefaultShowPanel ? WHEN_ACTIVE : NEVER,
		0);

	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
	{
		KeyValues* kvProgress = new KeyValues("Values");
		kvProgress->SetString("2", "#HudPanel_DisplayAlways");
		kvProgress->SetString("1", "#HudPanel_DisplayWhenActive");
		kvProgress->SetString("0", "#HudPanel_DisplayNever");

		AddComboOption(
			kvOptions,
			"ShowProgress",
			"#HudPanel_Option_ShowProgress",
			kvProgress,
			PROGRESS_ALWAYS,
			0);
	}

	AddComboOption(
		kvOptions,
		"ValueDisplay",
		"#HudPanel_Option_ValueDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_PERCENTAGE,
		1);
}

KeyValues* CHudResourceWeaponCharge::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	kvPreset->SetInt("x", 320);
	kvPreset->SetInt("y", 255);
	kvPreset->SetInt("alignH", ALIGN_CENTER);
	kvPreset->SetInt("alignV", ALIGN_TOP);
	kvPreset->SetInt("showPanel", 0);
	if (m_iWeaponID == FF_WEAPON_SNIPERRIFLE)
		kvPreset->SetInt("showPanelBorder", 0);

	kvPreset->SetInt("showHeaderText", 0);
	kvPreset->SetInt("showHeaderIcon", 0);
	kvPreset->SetInt("showText", 0);
	kvPreset->SetInt("barWidth", 60);
	kvPreset->SetInt("barHeight", 4);
	kvPreset->SetInt("barBorderWidth", 1);
	kvPreset->FindKey("Icon")->SetInt("show", 0);
	kvPreset->FindKey("Label")->SetInt("show", 0);
	kvPreset->FindKey("Amount")->SetInt("show", 0);
	kvPreset->FindKey("Bar")->SetInt("colorMode", COLOR_MODE_FADED);
	kvPreset->FindKey("Bar")->SetInt("alpha", 150);
	kvPreset->FindKey("BarBorder")->SetInt("show", 1);
	kvPreset->FindKey("BarBorder")->SetInt("alpha", 200);
	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
	{
		kvPreset->SetInt("x", 580);
		kvPreset->SetInt("y", 325);
		kvPreset->SetInt("alignH", ALIGN_RIGHT);
		kvPreset->SetInt("alignV", ALIGN_BOTTOM);
		kvPreset->SetInt("showPanel", 1);
		kvPreset->SetInt("showHeaderText", 1);
		kvPreset->SetInt("showText", 1);
		kvPreset->SetInt("barWidth", 80);
		kvPreset->SetInt("barHeight", 10);
		kvPreset->FindKey("Label")->SetInt("show", 1);
		kvPreset->FindKey("Amount")->SetInt("show", 1);
	}

	KeyValues* kvValues = new KeyValues("PanelSpecificValues");
	kvValues->SetBool("ShowPanel", m_bDefaultShowPanel);
	kvValues->SetInt("ValueDisplay", DISPLAY_PERCENTAGE);
	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
		kvValues->SetInt("ShowProgress", PROGRESS_ALWAYS);

	kvPreset->AddSubKey(kvValues);
	return kvPreset;
}

void CHudResourceWeaponCharge::ApplyStyleData(KeyValues* kvStyleData, KeyValues* kvDefaults)
{
	BaseClass::ApplyStyleData(kvStyleData, kvDefaults);
	KeyValues* kvValues = kvStyleData->FindKey("PanelSpecificValues", true);
	KeyValues* kvDefaultValues = kvDefaults->FindKey("PanelSpecificValues", true);
	int iShowPanel = GetInt("ShowPanel", kvValues, kvDefaultValues);
	if (iShowPanel != -1)
		m_showPanel = static_cast<PanelVisibility>(clamp(iShowPanel, int(NEVER), int(WHILE_EQUIPPED)));

	int iShowProgress = GetInt("ShowProgress", kvValues, kvDefaultValues);
	if (iShowProgress != -1)
		m_showProgress = static_cast<ProgressVisibility>(clamp(iShowProgress, int(PROGRESS_NEVER), int(PROGRESS_ALWAYS)));

	std::optional<AmountDisplay> display = GetAmountDisplay("ValueDisplay", kvValues, kvDefaultValues);
	if (display.has_value())
		m_pCharge->SetAmountDisplay(display.value());
}

bool CHudResourceWeaponCharge::IsAvailable()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer || FF_IsPlayerSpec(pPlayer) || !FF_HasPlayerPickedClass(pPlayer))
		return false;

	CFFWeaponBase* pWeapon = pPlayer->GetActiveFFWeapon();
	if (!pWeapon || pWeapon->GetWeaponID() != m_iWeaponID)
		return false;

	return true;
}

CHudResourceWeaponCharge::ChargeData CHudResourceWeaponCharge::GetChargeData()
{
	CFFWeaponBase* pWeapon
		= C_FFPlayer::GetLocalFFPlayer()
		->GetActiveFFWeapon();

	float flCharge = 0, flChargeMax = 0;
	char cIcon = '?';

	switch (m_iWeaponID)
	{
	case FF_WEAPON_SNIPERRIFLE:
	{
		CFFWeaponSniperRifle* pRifle = static_cast<CFFWeaponSniperRifle*>(pWeapon);

		// Charge starts at the rifle's minimum damage while the shot is held.
		if (!pRifle->IsInFire())
		{
			flCharge = 0.0f;
		}
		else
		{
			flCharge = clamp(gpGlobals->curtime - pRifle->GetFireStartTime(), 1.0f, FF_SNIPER_MAXCHARGE);
		}
		flChargeMax = FF_SNIPER_MAXCHARGE;
		cIcon = 's';
		break;
	}

	case FF_WEAPON_ASSAULTCANNON:
	{
		flCharge = static_cast<CFFWeaponAssaultCannon*>(pWeapon)->m_flChargeTime;
		flChargeMax = FF_AC_WINDUPTIME;
		cIcon = 'b';
		break;
	}

	case FF_WEAPON_JUMPGUN:
	{
		cIcon = 'j';
		flCharge = static_cast<CFFWeaponJumpgun*>(pWeapon)->GetClampedCharge();
		flChargeMax = JUMPGUN_CHARGEUPTIME;
		break;
	}
	}

	flCharge = clamp(flCharge, 0.0f, flChargeMax);

	return {
		flCharge,
		flChargeMax,
		cIcon
	};
}

void CHudResourceWeaponCharge::OnTick()
{
	const bool bPreview = IsInPreviewMode();
	const bool bAvailable = engine->IsInGame() && IsAvailable();
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	const bool bAnchored = m_iWeaponID == FF_WEAPON_ASSAULTCANNON
		&& pPlayer && pPlayer->m_bAssaultCannonAnchored;

	ChargeData charge;
	if (bAvailable)
		charge = GetChargeData();
	else if (bPreview)
	{
		charge.m_flCharge = 0.6f;
		charge.m_cIcon = m_iWeaponID == FF_WEAPON_JUMPGUN ? 'j'
			: m_iWeaponID == FF_WEAPON_ASSAULTCANNON ? 'b' : 's';
	}

	const bool bActive = charge.m_flCharge > 0.0f || bAnchored;
	const bool bShow = m_showPanel != NEVER
		&& (bPreview || (bAvailable && (m_showPanel == WHILE_EQUIPPED || bActive)));
	SetHiddenBits(bShow ? (bPreview ? 0 : DEFAULT_HIDDEN_BITS) : HIDEHUD_ALWAYS);

	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
		SetToggleTextVisible(bPreview || bAnchored);

	if (bAvailable || bPreview)
	{
		m_pCharge->SetAmountMax(charge.m_flChargeMax);
		m_pCharge->SetAmount(charge.m_flCharge);
		m_pCharge->SetIcon(charge.m_cIcon);
	}

	if (m_showProgress == PROGRESS_ALWAYS
		|| (m_showProgress == PROGRESS_WHEN_ACTIVE && (bPreview || charge.m_flCharge > 0.0f)))
		EnableItem(m_pCharge);
	else
		DisableItem(m_pCharge);

	BaseClass::OnTick();
}
