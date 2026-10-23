#ifndef FF_HUD_RESOURCE_WEAPONCHARGE_H
#define FF_HUD_RESOURCE_WEAPONCHARGE_H

#include "ff_quantitypanel.h"
#include "hudelement.h"
#include "iclientmode.h"

class CHudResourceWeaponCharge : public CHudElement, public vgui::FFQuantityPanel
{
	DECLARE_CLASS_SIMPLE(CHudResourceWeaponCharge, vgui::FFQuantityPanel);

public:
	CHudResourceWeaponCharge(
		const char* pElementName,
		int iWeaponID,
		const char* pszTitle,
		bool bShowPanel);
	void Init();
	void VidInit();
	bool ShouldDraw() override;

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
	int m_iShowPanel;
	int m_iShowProgress = 2;
	vgui::FFQuantityItem* m_pCharge = nullptr;
	bool GetCharge(float& flCharge);
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
