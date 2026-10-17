#ifndef FF_HUD_MEASURE_H
#define FF_HUD_MEASURE_H

// Temporary A/B instrumentation. Indices keep lookups out of the timed region.
enum FFHudMeasureId
{
    HUD_MEASURE_BUILDTIMER,
    HUD_MEASURE_MEDPACKS,
    HUD_MEASURE_MANCANNON,
    HUD_MEASURE_TIMER_SET,
    HUD_MEASURE_MANCANNON_TIMER,
    HUD_MEASURE_SEND_TIMER,
    HUD_MEASURE_SEND_MEDPACKS,
    HUD_MEASURE_SEND_MANCANNON,
    HUD_MEASURE_COUNT
};

bool FFHudMeasureEnabled();
void FFHudMeasureRecord(FFHudMeasureId id, double seconds, bool changed, int payloadBytes);

class FFHudMeasureScope
{
public:
    explicit FFHudMeasureScope(FFHudMeasureId id, int payloadBytes = 0)
        : m_id(id), m_enabled(FFHudMeasureEnabled()), m_changed(false),
          m_payloadBytes(payloadBytes), m_start(0)
    {
        if (m_enabled)
            m_start = Plat_FloatTime();
    }
    ~FFHudMeasureScope()
    {
        if (m_enabled)
            FFHudMeasureRecord(m_id, Plat_FloatTime() - m_start, m_changed, m_payloadBytes);
    }
    void Changed() { m_changed = true; }

private:
    FFHudMeasureId m_id;
    bool m_enabled;
    bool m_changed;
    int m_payloadBytes;
    double m_start;
};

#endif
