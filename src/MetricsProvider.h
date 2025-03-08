// MetricsProvider.h
#ifndef METRICS_PROVIDER_H
#define METRICS_PROVIDER_H

#define OPENTELEMETRY_ABI_VERSION_NO 2

#include "opentelemetry/sdk/metrics/meter.h"

class MetricsProvider {
public:
    MetricsProvider();
    ~MetricsProvider();
    // GetCounter
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetCounter();
    // GetHistogram
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetHistogram();
    // return a gauge for each TapNoteScore
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetGauge(std::string name);
private:
    // store observable gauge, counter, and histogram
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_hitGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_hitHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_hitCounter;
};

extern MetricsProvider* METRICS;

#endif