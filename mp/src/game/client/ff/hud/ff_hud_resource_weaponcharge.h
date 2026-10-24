#ifndef FF_HUD_RESOURCE_WEAPONCHARGE_H
#define FF_HUD_RESOURCE_WEAPONCHARGE_H

#include "ff_quantitypanel.h"
#include "hudelement.h"
#include "iclientmode.h"

class CHudResourceWeaponCharge : public CHudElement, public vgui::FFQuantityPanel
{
private:
	struct ChargeData
	{
		float m_flCharge = 0.0f;
		float m_flChargeMax = 1.0f;
		char m_cIcon = '?';
	};

	DECLARE_CLASS_SIMPLE(CHudResourceWeaponCharge, vgui::FFQuantityPanel);

public:
	CHudResourceWeaponCharge(
		const char* pElementName,
		int iWeaponID,
		const char* pszTitle,
		bool bShowPanel);
	void Init();
	void VidInit();

protected:
	void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	void AddPanelSpecificOptions(KeyValues* kvOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(KeyValues* kvStyleData, KeyValues* kvDefaults) override;
	void OnTick() override;

private:
	int m_iWeaponID;
	const char* m_pszTitle;
	bool m_bDefaultShowPanel;
	enum PanelVisibility { NEVER = 0, WHEN_ACTIVE = 1, WHILE_EQUIPPED = 2 };
	enum ProgressVisibility { PROGRESS_NEVER = 0, PROGRESS_WHEN_ACTIVE = 1, PROGRESS_ALWAYS = 2 };

	PanelVisibility m_showPanel;
	ProgressVisibility m_showProgress = PROGRESS_ALWAYS;
	vgui::FFQuantityItem* m_pCharge = nullptr;

	bool IsAvailable();
	ChargeData GetChargeData();
};

class CHudResourceSniperCharge : public CHudResourceWeaponCharge
{
public:
	CHudResourceSniperCharge(const char* pElementName);
};

class CHudResourceAssaultCannonCharge : public CHudResourceWeaponCharge
{
public:
	CHudResourceAssaultCannonCharge(const char* pElementName);
};

class CHudResourceJumpgunCharge : public CHudResourceWeaponCharge
{
public:
	CHudResourceJumpgunCharge(const char* pElementName);
};

#endif
