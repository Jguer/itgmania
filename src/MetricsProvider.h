// MetricsProvider.h
#ifndef METRICS_PROVIDER_H
#define METRICS_PROVIDER_H

#define OPENTELEMETRY_ABI_VERSION_NO 2

#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/trace/tracer.h"
#include "opentelemetry/trace/tracer_provider.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/logs/logger.h"
#include "opentelemetry/logs/logger_provider.h"

class MetricsProvider {
public:
    MetricsProvider();
    ~MetricsProvider();
    // Metrics API
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> GetCounter();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> GetHistogram();
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> GetGauge(std::string name);
    
    // Trace API
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Tracer> GetTracer(const std::string& name = "default");
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Span> StartSpan(const std::string& name);
    
    // Logs API
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::logs::Logger> GetLogger(const std::string& name = "default");
    void Log(opentelemetry::v2::logs::Severity severity, const std::string& message);
    
    // Convenience logging methods
    void Info(const char* fmt, ...);
    void Warn(const char* fmt, ...);
    void Error(const char* fmt, ...);
    void Debug(const char* fmt, ...);

private:
    // Metrics members
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> m_hitGauge;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> m_hitHistogram;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> m_hitCounter;
    
    // Trace members
    std::shared_ptr<opentelemetry::v2::trace::TracerProvider> m_tracerProvider;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Tracer> m_defaultTracer;
    
    // Logs members
    std::shared_ptr<opentelemetry::v2::logs::LoggerProvider> m_loggerProvider;
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::logs::Logger> m_defaultLogger;
};

extern MetricsProvider* METRICS;

#endif