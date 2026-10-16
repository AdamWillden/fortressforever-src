#ifndef FF_MANCANNON_STATUS_H
#define FF_MANCANNON_STATUS_H

#include "networkvar.h"
#include "ff_buildabledefs.h"

// Owner-only HUD state shared by the server and client.
struct FFManCannonStatus
{
	DECLARE_CLASS_NOBASE(FFManCannonStatus);
	DECLARE_EMBEDDED_NETWORKVAR();

	FFManCannonStatus()
	{
		Clear();
	}

	void Clear()
	{
		m_bCanHave = false;
		m_bHas = false;
		m_iBuildState = BUILDSTATE_NOTBUILT;
		m_iHealth = 0;
		m_flBuildStart = 0;
		m_flBuildEnd = 0;
	}

	void SetAvailability(bool bCanHave, bool bHas)
	{
		m_bCanHave = bCanHave;
		m_bHas = bHas;
	}

	void BeginBuild(float flCurrentTime)
	{
		m_flBuildStart = flCurrentTime;
	}

	void SetBuildState(BuildState_t state, int iHealth, float flBuildEnd)
	{
		m_iBuildState = state;
		m_iHealth = iHealth;
		if (state == BUILDSTATE_BUILDING)
		{
			m_flBuildEnd = flBuildEnd;
		}
		else
		{
			m_flBuildStart = 0;
			m_flBuildEnd = 0;
		}
	}

	CNetworkVar(bool, m_bCanHave);
	CNetworkVar(bool, m_bHas);
	CNetworkVar(int, m_iBuildState);
	CNetworkVar(int, m_iHealth);
	CNetworkVar(float, m_flBuildStart);
	CNetworkVar(float, m_flBuildEnd);
};

#endif
