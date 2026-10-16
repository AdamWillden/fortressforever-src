#include "cbase.h"

#include "ff_hud_timer_build.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudTimerBuild);

CHudTimerBuild::CHudTimerBuild(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudTimerBuild")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_ALWAYS);

	m_flDuration = 0.0f;
	m_flStartTime = 0.0f;
	m_bInvertScale = false;
	m_bShowPanel = true;

	m_iBuildType = FF_BUILD_NONE;

	m_qiTimer = nullptr;
}

CHudTimerBuild::~CHudTimerBuild()
{
}

void CHudTimerBuild::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddBooleanOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		true,
		0);

	KeyValues* kvInvertScaleOptions
		= new KeyValues("Values");

	kvInvertScaleOptions->SetString(
		"0",
		"#HudPanel_Option_CountUp");
	kvInvertScaleOptions->SetString(
		"1",
		"#HudPanel_Option_CountDown");

	AddComboOption(
		kvPanelSpecificOptions,
		"InvertScale",
		"#HudPanel_Option_Mode",
		kvInvertScaleOptions,
		1);
}

KeyValues* CHudTimerBuild::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= BaseClass::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("InvertScale", false);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	kvPreset->SetInt("showHeaderText", 0);

	kvPreset->SetInt("barWidth", 100);
	kvPreset->SetInt("barHeight", 15);

	kvPreset->SetInt("headerIconSize", 5);
	kvPreset->SetInt("headerIconAnchorPosition", ANCHORPOS_MIDDLELEFT);
	kvPreset->SetInt("headerIconAlignHoriz", ALIGN_RIGHT);
	kvPreset->SetInt("headerIconAlignVert", ALIGN_CENTER);
	kvPreset->SetInt("headerIconX", -2);
	kvPreset->SetInt("headerIconY", 0);

	kvPreset->SetInt("x", 320);
	kvPreset->SetInt("y", 380);
	kvPreset->SetInt("alignH", ALIGN_CENTER);
	kvPreset->SetInt("alignV", ALIGN_TOP);

	return kvPreset;
}

void CHudTimerBuild::ApplyStyleData(
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
	int iInvertScale
		= GetInt(
			"InvertScale",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iShowPanel != -1)
		m_bShowPanel = (iShowPanel == 1);

	if (iInvertScale != -1)
	{
		m_bInvertScale = (iInvertScale == 1);

		m_qiTimer->SetLabel(
			m_bInvertScale
			? "#FF_ITEM_TIMELEFT"
			: "#FF_ITEM_PROGRESS");


		m_qiTimer->SetAmountDisplay(
			m_bInvertScale
			? DISPLAY_RAW
			: DISPLAY_PERCENTAGE);
	}

	ApplyDisplayOptions();
}

void CHudTimerBuild::ApplyDisplayOptions()
{
	if (m_bShowPanel)
	{
		if (IsInPreviewMode())
			SetHiddenBits(0);
		else if (m_iBuildType != FF_BUILD_NONE
			&& gpGlobals->curtime < m_flStartTime + m_flDuration)
			SetHiddenBits(HIDEHUD_PLAYERDEAD);
		else
			SetHiddenBits(HIDEHUD_ALWAYS);
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}
}

void CHudTimerBuild::VidInit()
{
	wchar_t* tempString = g_pVGuiLocalize->Find("#HudPanel_Timer");

	if (!tempString)
		tempString = L"Timer";
	else
		wcsupr(tempString);

	SetHeaderText(tempString);
	SetHeaderIconChar('f');
	m_iBuildType = FF_BUILD_NONE;
	m_flStartTime = 0;
	m_flDuration = 0;
	m_qiTimer->SetAmount(0);
	ApplyDisplayOptions();
	UpdateFromPlayer(C_FFPlayer::GetLocalFFPlayer());
}

void CHudTimerBuild::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	m_qiTimer = AddItem("HudTimerBuildTimeLeft");
	m_qiTimer->SetIcon('f');
	m_qiTimer->SetAmount(0);
	m_qiTimer->SetAmountDecimalPlaces(1);
	m_qiTimer->SetLabel("#FF_ITEM_PROGRESS");

	// This is the correct value but gets reset on build below
	m_qiTimer->SetAmountMax(3.5f);

	AddPanelToHudOptions(
		"Build",
		"#HudPanel_Build",
		"Timers",
		"#HudPanel_Timers");
}

void CHudTimerBuild::Paint()
{
	const float flTimeElapsed = clamp(gpGlobals->curtime - m_flStartTime, 0.0f, m_flDuration);
	if (m_bInvertScale)
		m_qiTimer->SetAmount(m_flDuration - flTimeElapsed);
	else
		m_qiTimer->SetAmount(flTimeElapsed);

	ApplyDisplayOptions();
	BaseClass::Paint();
}

void CHudTimerBuild::UpdateFromPlayer(const C_FFPlayer* pPlayer)
{
	if (!pPlayer || !m_qiTimer)
		return;

	const FFBuildTimerStatus& status = pPlayer->GetBuildTimerStatus();

	const int iBuildType = status.m_iType;
	const float flStartTime = status.m_flStart;
	const float flDuration = MAX(0.0f, status.m_flEnd - flStartTime);
	const bool bStateUnchanged =
		m_iBuildType == iBuildType
		&& m_flStartTime == flStartTime
		&& m_flDuration == flDuration;

	if (bStateUnchanged)
		return;

	m_iBuildType = iBuildType;
	m_flStartTime = flStartTime;
	m_flDuration = flDuration;

	switch (m_iBuildType)
	{
	case FF_BUILD_DISPENSER:
		SetHeaderIconChar('4');
		break;
	case FF_BUILD_SENTRYGUN:
		SetHeaderIconChar('1');
		break;
	case FF_BUILD_DETPACK:
		SetHeaderIconChar('5');
		break;
	case FF_BUILD_MANCANNON:
		SetHeaderIconChar('6');
		break;
	default:
		SetHeaderIconChar('f');
		break;
	}

	m_qiTimer->SetAmountMax(MAX(0.001f, m_flDuration));
	if (m_bInvertScale)
		m_qiTimer->SetAmount(m_flDuration);
	else
		m_qiTimer->SetAmount(0);

	ApplyDisplayOptions();
}
