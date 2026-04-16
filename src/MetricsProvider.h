// MetricsProvider.h
#ifndef METRICS_PROVIDER_H
#define METRICS_PROVIDER_H

#define OPENTELEMETRY_ABI_VERSION_NO 2

#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/logs/logger.h"
#include "opentelemetry/sdk/trace/tracer.h"

class MetricsProvider {
public:
    MetricsProvider();
    ~MetricsProvider();
    // GetCounter
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetCounter();
    // GetHistogram
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetHistogram();
    // return a gauge for each metric
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetGauge(std::string name);
    // network metrics
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetHttpRequestDuration();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetHttpRequestCounter();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetHttpRequestErrorCounter();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetWebSocketOpenDuration();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetWebSocketOpenCounter();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetWebSocketErrorCounter();
    // frame timing
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetFrameTimeHistogram();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetFrameDropCounter();
    // gameplay accuracy / dance points (gauges) and song play counter
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetAccuracyGauge();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetDancePointsActualGauge();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetDancePointsPossibleGauge();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetSongPlaysCounter();
    // End-of-song histograms (low-cardinality labels; exemplars link to song_play span when enabled)
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetSongFinalAccuracyBpsHistogram();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetSongFinalScoreHistogram();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetSongMaxComboHistogram();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetSongPlayDurationMsHistogram();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetSongFinalLifePercentHistogram();
    // signed hit-timing offset (negative = early, positive = late) in ms
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<double>> GetNoteHitOffsetHistogram();
    // hold/roll outcomes + mine events
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetHoldNoteScoresCounter();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetMineEventsCounter();
    // descriptive "current song" info gauge (always recorded as 1)
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetCurrentSongInfoGauge();
    // per-play "last value" gauges for Global Stats max-of-day queries
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetLastPlayScoreGauge();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetLastPlayMaxComboGauge();
    // get tracer
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Tracer> GetTracer();
    // get logger
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::logs::Logger> GetLogger();

    // store observable gauge, counter, and histogram
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_hitGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_hitHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_hitCounter;
    
    // logger member
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::logs::Logger> m_logger;

    // tracer member
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Tracer> m_tracer;

    // New metrics
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_scoreGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_comboGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_maxComboGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_lifeGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_fpsGauge;

    // Network metrics.
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_httpRequestDuration;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_httpRequestCounter;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_httpRequestErrorCounter;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_websocketOpenDuration;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_websocketOpenCounter;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_websocketErrorCounter;

    // Frame timing metrics.
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_frameTimeHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_frameDropCounter;

    // Gameplay accuracy / dance points and completed song plays.
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_accuracyGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_dancePointsActualGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_dancePointsPossibleGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_songPlaysCounter;

    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_songFinalAccuracyBpsHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_songFinalScoreHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_songMaxComboHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_songPlayDurationMsHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_songFinalLifePercentHistogram;

    // Signed note-hit offset histogram (double, supports negative values).
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<double>> m_noteHitOffsetHistogram;

    // Hold/Roll outcome + mine outcome counters.
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_holdNoteScoresCounter;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_mineEventsCounter;

    // "Current song info" gauge: always 1, carries rich labels (title/artist/
    // bpm/length/difficulty) so dashboards can look them up via label_values.
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_currentSongInfoGauge;

    // Per-play "last value" gauges used for max_over_time() global stats.
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_lastPlayScoreGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_lastPlayMaxComboGauge;
};

extern MetricsProvider* METRICS;

#endif