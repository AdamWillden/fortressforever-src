#ifndef FF_HUD_RESOURCE_MEDPACKS_H
#define FF_HUD_RESOURCE_MEDPACKS_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"

class C_FFPlayer;
using namespace FFQuantityHelper;

class CHudResourceMedpacks
	: public CHudElement, public vgui::FFQuantityPanel
{
public:
	void UpdateFromPlayer(const C_FFPlayer* pPlayer);

private:
	DECLARE_CLASS_SIMPLE(CHudResourceMedpacks, vgui::FFQuantityPanel);
	CHudResourceMedpacks(const char* pElementName);
	~CHudResourceMedpacks();

	vgui::FFQuantityItem* m_qiMedpacks;

	std::wstring m_wsFull;
	std::wstring m_wsRegenerating; 

	bool m_bShowPanel = false;
	int m_iMedpackCount = -1;
	int m_iMedpackMax = -1;
	void ApplyDisplayOptions();

	virtual void ApplySchemeSettings(
		vgui::IScheme* pScheme) override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	void Init();
	void VidInit();
};

#endif