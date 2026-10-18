#include "cbase.h"

using namespace vgui;

#include "ff_hud_buildable_dispenser.h"
#include "ff_buildable_dispenser.h"

using namespace FFQuantityHelper;

DECLARE_HUDELEMENT(CHudBuildableDispenser);
DECLARE_HUD_MESSAGE(CHudBuildableDispenser, DispenserMsg);

CHudBuildableDispenser::CHudBuildableDispenser(
	const char* pElementName)
	: CHudBuildable(
		pElementName,
		FF_BUILD_DISPENSER,
		"HudBuildableDispenser",
		"#HudPanel_Dispenser")
{
	m_iCellRequirement = FF_BUILDCOST_DISPENSER;
}

CHudBuildableDispenser::~CHudBuildableDispenser()
{
}

void CHudBuildableDispenser::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	CHudBuildable::AddPanelSpecificOptions(
		kvPanelSpecificOptions);

	KeyValues* kvHideCells = nullptr;
	KeyValues* kvCellsDisplay = nullptr;
	for (KeyValues* kvOption = kvPanelSpecificOptions->GetFirstSubKey(); kvOption;)
	{
		KeyValues* kvNext = kvOption->GetNextKey();
		const char* pszName = kvOption->GetString("name");

		if (Q_stricmp(pszName, "HideCells") == 0)
		{
			kvHideCells = kvOption;
			kvHideCells->SetInt("defaultValue", NEVER);
			kvHideCells->SetInt("group", 1);
			kvPanelSpecificOptions->RemoveSubKey(kvOption);
		}
		else if (Q_stricmp(pszName, "CellsDisplay") == 0)
		{
			kvCellsDisplay = kvOption;
			kvCellsDisplay->SetInt("group", 2);
			kvPanelSpecificOptions->RemoveSubKey(kvOption);
		}
		else if (kvOption->GetInt("group") == 1)
		{
			kvOption->SetInt("group", 2);
		}
		else if (Q_stricmp(kvOption->GetName(), "Heading") == 0)
		{
			kvOption->SetString("text", "#HudPanel_Heading_General");
		}

		kvOption = kvNext;
	}

	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_Visibility", 1);
	AddSpacer(kvPanelSpecificOptions, 1);

	AddComboOption(
		kvPanelSpecificOptions,
		"AmmoDisplay",
		"#HudPanel_Option_AmmoDisplay",
		CHudBuildable::s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		2);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideAmmo",
		"#HudPanel_Option_HideAmmo",
		false,
		1);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideArmor",
		"#HudPanel_Option_HideArmor",
		true,
		1);
	AddComboOption(
		kvPanelSpecificOptions,
		"ArmorDisplay",
		"#HudPanel_Option_ArmorDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		2);

	kvPanelSpecificOptions->AddSubKey(kvHideCells);
	kvPanelSpecificOptions->AddSubKey(kvCellsDisplay);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideNails",
		"#HudPanel_Option_HideNails",
		true,
		1);
	AddComboOption(
		kvPanelSpecificOptions,
		"NailsDisplay",
		"#HudPanel_Option_NailsDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		2);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideShells",
		"#HudPanel_Option_HideShells",
		true,
		1);
	AddComboOption(
		kvPanelSpecificOptions,
		"ShellsDisplay",
		"#HudPanel_Option_ShellsDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		2);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideRockets",
		"#HudPanel_Option_HideRockets",
		false,
		1);
	AddComboOption(
		kvPanelSpecificOptions,
		"RocketsDisplay",
		"#HudPanel_Option_RocketsDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		2);

	AddAlertOptions(kvPanelSpecificOptions, "#HudPanel_Option_DamageAlert", 0);
}

KeyValues* CHudBuildableDispenser::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= CHudBuildable::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= kvPreset->FindKey("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("AmmoDisplay", DISPLAY_MAX);
	kvPanelSpecificValues->SetBool("HideAmmo", false);
	kvPanelSpecificValues->SetInt("HideCells", NEVER);
	kvPanelSpecificValues->SetBool("HideArmor", true);
	kvPanelSpecificValues->SetInt("ArmorDisplay", DISPLAY_MAX);
	kvPanelSpecificValues->SetBool("HideNails", true);
	kvPanelSpecificValues->SetInt("NailsDisplay", DISPLAY_MAX);
	kvPanelSpecificValues->SetBool("HideShells", true);
	kvPanelSpecificValues->SetInt("ShellsDisplay", DISPLAY_MAX);
	kvPanelSpecificValues->SetBool("HideRockets", false);
	kvPanelSpecificValues->SetInt("RocketsDisplay", DISPLAY_MAX);

	return kvPreset;
}

void CHudBuildableDispenser::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	CHudBuildable::ApplyStyleData(
		kvStyleData,
		kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);

	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaultStyleData->FindKey("PanelSpecificValues", true);

	std::optional<AmountDisplay> cellsDisplay
		= GetAmountDisplay(
			"CellsDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<AmountDisplay> ammoDisplay
		= GetAmountDisplay(
			"AmmoDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (cellsDisplay.has_value())
	{
		m_qiCells->SetAmountDisplay(
			cellsDisplay.value());
	}

	if (ammoDisplay.has_value())
	{
		m_qiAmmo->SetAmountDisplay(
			ammoDisplay.value());
	}

	int iHideAmmo = GetInt("HideAmmo", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideAmmo != -1)
		m_bHideAmmo = iHideAmmo == 1;

	int iHideArmor = GetInt("HideArmor", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideArmor != -1)
		m_hideStoredItems[0] = iHideArmor == 1;

	std::optional<AmountDisplay> armorDisplay = GetAmountDisplay("ArmorDisplay", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (armorDisplay.has_value())
		m_storedItems[0]->SetAmountDisplay(armorDisplay.value());

	int iHideNails = GetInt("HideNails", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideNails != -1)
		m_hideStoredItems[1] = iHideNails == 1;

	std::optional<AmountDisplay> nailsDisplay = GetAmountDisplay("NailsDisplay", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (nailsDisplay.has_value())
		m_storedItems[1]->SetAmountDisplay(nailsDisplay.value());

	int iHideShells = GetInt("HideShells", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideShells != -1)
		m_hideStoredItems[2] = iHideShells == 1;

	std::optional<AmountDisplay> shellsDisplay = GetAmountDisplay("ShellsDisplay", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (shellsDisplay.has_value())
		m_storedItems[2]->SetAmountDisplay(shellsDisplay.value());

	int iHideRockets = GetInt("HideRockets", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideRockets != -1)
		m_hideStoredItems[3] = iHideRockets == 1;

	std::optional<AmountDisplay> rocketsDisplay = GetAmountDisplay("RocketsDisplay", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (rocketsDisplay.has_value())
		m_storedItems[3]->SetAmountDisplay(rocketsDisplay.value());

	ApplyDisplayOptions();
}

void CHudBuildableDispenser::Init()
{
	HOOK_HUD_MESSAGE(CHudBuildableDispenser, DispenserMsg);

	InitItemHealth();
	InitItemBuildProgress();

	m_qiAmmo = AddItem("Ammo");
	m_qiAmmo->SetLabel("#FF_ITEM_AMMO");
	m_qiAmmo->SetIcon('r');
	m_qiAmmo->SetAmountDisplay(DISPLAY_PERCENTAGE);
	m_qiAmmo->SetAmountMax(100);

	InitItemCells();
	m_qiCells->SetIntensityLimits(
		0,
		(int)(FF_BUILDCOST_DISPENSER / 3),
		(int)(FF_BUILDCOST_DISPENSER / 3) * 2,
		FF_BUILDCOST_DISPENSER);
	m_qiCells->SetIntensityAmountScaled(false);

	m_storedItems[0] = AddItem("Armor");
	m_storedItems[0]->SetLabel("#FF_ITEM_ARMOR");
	m_storedItems[0]->SetAmountDisplay(DISPLAY_MAX);
	DisableItem(m_storedItems[0]);

	m_storedItems[1] = AddItem("Nails");
	m_storedItems[1]->SetLabel("#FF_ITEM_NAILS");
	m_storedItems[1]->SetAmountDisplay(DISPLAY_MAX);
	DisableItem(m_storedItems[1]);

	m_storedItems[2] = AddItem("Shells");
	m_storedItems[2]->SetLabel("#FF_ITEM_SHELLS");
	m_storedItems[2]->SetAmountDisplay(DISPLAY_MAX);
	DisableItem(m_storedItems[2]);

	m_storedItems[3] = AddItem("Rockets");
	m_storedItems[3]->SetLabel("#FF_ITEM_ROCKETS");
	m_storedItems[3]->SetAmountDisplay(DISPLAY_MAX);
	DisableItem(m_storedItems[3]);

	CHudBuildable::Init();
}

void CHudBuildableDispenser::VidInit()
{
	CHudBuildable::VidInit();

	SetHeaderIconChar('4');

	m_qiAmmo->SetAmount(0);

	DisableItem(m_qiAmmo);
}

int CHudBuildableDispenser::GetMaxCells()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	C_FFDispenser* pDispenser = pPlayer ? pPlayer->GetDispenser() : nullptr;
	if (pDispenser && pDispenser->IsBuilt())
		return pDispenser->GetCells() + pDispenser->NeedsCells();

	return FF_BUILDCOST_DISPENSER;
}

CFFBuildableObject* CHudBuildableDispenser::GetBuildable()
{
	return C_FFPlayer::GetLocalFFPlayer()
		->GetDispenser();
}

void CHudBuildableDispenser::OnBuiltChanged(
	bool bBuilt)
{
	CHudBuildable::OnBuiltChanged(bBuilt);

	m_iCellRequirement = bBuilt ? 0 : FF_BUILDCOST_DISPENSER;
	m_qiCells->SetAmountMax(GetMaxCells());
	m_qiCells->SetIntensityAmountScaled(bBuilt);
	if (bBuilt)
		m_qiCells->SetIntensityLimits(0, 33, 66, 100);
	else
		m_qiCells->SetIntensityLimits(
			0,
			FF_BUILDCOST_DISPENSER / 3,
			FF_BUILDCOST_DISPENSER * 2 / 3,
			FF_BUILDCOST_DISPENSER);
}

void CHudBuildableDispenser::MsgFunc_DispenserMsg(
	bf_read& msg)
{
	int iHealth = (int)msg.ReadByte();
	int iAmmo = (int)msg.ReadByte();

	SetHealth(iHealth);
	m_qiAmmo->SetAmount(iAmmo);
}

void CHudBuildableDispenser::ApplyDisplayOptions()
{
	CHudBuildable::ApplyDisplayOptions();
	const bool bShowStored = m_bBuilt || IsInPreviewMode();
	if (bShowStored && !m_bHideAmmo)
		EnableItem(m_qiAmmo);
	else
		DisableItem(m_qiAmmo);

	for (int i = 0; i < 4; ++i)
	{
		if (bShowStored && !m_hideStoredItems[i])
			EnableItem(m_storedItems[i]);
		else
			DisableItem(m_storedItems[i]);
	}
}

void CHudBuildableDispenser::OnTick()
{
	m_qiCells->SetAmountMax(GetMaxCells());
	CHudBuildable::OnTick();
	if (IsInPreviewMode())
	{
		m_qiAmmo->SetAmount(60);
		m_qiCells->SetAmountMax(100);
		m_qiCells->SetAmount(60);
		for (int i = 0; i < 4; ++i)
		{
			m_storedItems[i]->SetAmountMax(100);
			m_storedItems[i]->SetAmount(60);
		}

		return;
	}

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer || pPlayer->GetClassSlot() != CLASS_ENGINEER)
		return;

	C_FFDispenser* pDispenser = pPlayer->GetDispenser();
	if (!pDispenser || !pDispenser->IsBuilt())
		return;

	m_qiAmmo->SetAmount(pDispenser->GetAmmoPercent());
	m_storedItems[0]->SetAmountMax(pDispenser->GetArmor() + pDispenser->NeedsArmor());
	m_storedItems[0]->SetAmount(pDispenser->GetArmor());
	m_qiCells->SetAmountMax(pDispenser->GetCells() + pDispenser->NeedsCells());
	m_qiCells->SetAmount(pDispenser->GetCells());
	m_storedItems[1]->SetAmountMax(pDispenser->GetNails() + pDispenser->NeedsNails());
	m_storedItems[1]->SetAmount(pDispenser->GetNails());
	m_storedItems[2]->SetAmountMax(pDispenser->GetShells() + pDispenser->NeedsShells());
	m_storedItems[2]->SetAmount(pDispenser->GetShells());
	m_storedItems[3]->SetAmountMax(pDispenser->GetRockets() + pDispenser->NeedsRockets());
	m_storedItems[3]->SetAmount(pDispenser->GetRockets());
}
