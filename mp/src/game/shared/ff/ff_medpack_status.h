#ifndef FF_MEDPACK_STATUS_H
#define FF_MEDPACK_STATUS_H

#include "networkvar.h"

// Owner-only HUD state shared by the server and client.
struct FFMedpackStatus
{
	DECLARE_CLASS_NOBASE(FFMedpackStatus);
	DECLARE_EMBEDDED_NETWORKVAR();

	FFMedpackStatus()
	{
		Clear();
	}

	void Clear()
	{
		m_iCount = 0;
		m_iMax = 0;
	}

	void Set(int iCount, int iMax)
	{
		m_iCount = iCount;
		m_iMax = iMax;
	}

	CNetworkVar(int, m_iCount);
	CNetworkVar(int, m_iMax);
};

#endif
