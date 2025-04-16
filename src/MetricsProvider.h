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
};

extern MetricsProvider* METRICS;

#endif