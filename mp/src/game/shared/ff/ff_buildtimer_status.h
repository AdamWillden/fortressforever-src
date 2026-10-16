#ifndef FF_BUILDTIMER_STATUS_H
#define FF_BUILDTIMER_STATUS_H

#include "networkvar.h"
#include "ff_buildabledefs.h"

// Owner-only HUD state shared by the server and client.
struct FFBuildTimerStatus
{
	DECLARE_CLASS_NOBASE(FFBuildTimerStatus);
	DECLARE_EMBEDDED_NETWORKVAR();

	FFBuildTimerStatus()
	{
		Clear();
	}

	void Clear()
	{
		m_iType = FF_BUILD_NONE;
		m_flStart = 0;
		m_flEnd = 0;
	}

	void Set(int iBuildType, float flDuration, float flCurrentTime)
	{
		if (iBuildType != FF_BUILD_NONE && flDuration > 0)
		{
			m_iType = iBuildType;
			m_flStart = flCurrentTime;
			m_flEnd = flCurrentTime + flDuration;
		}
		else
		{
			Clear();
		}
	}

	CNetworkVar(int, m_iType);
	CNetworkVar(float, m_flStart);
	CNetworkVar(float, m_flEnd);
};

#endif
