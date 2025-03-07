// MetricsProvider.cpp
#include "MetricsProvider.h"

#include "opentelemetry/common/attribute_value.h"
#include "opentelemetry/exporters/otlp/otlp_environment.h"
#include "opentelemetry/exporters/otlp/otlp_http.h"
#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h"
#include "opentelemetry/exporters/otlp/otlp_http_log_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_log_exporter_options.h"
#include "opentelemetry/exporters/otlp/otlp_http_trace_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_trace_exporter_options.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/common/global_log_handler.h"
#include "opentelemetry/sdk/metrics/aggregation/default_aggregation.h"
#include "opentelemetry/sdk/metrics/aggregation/histogram_aggregation.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_context.h"
#include "opentelemetry/sdk/metrics/meter_context_factory.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/metric_reader.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/sdk/metrics/state/filtered_ordered_attribute_map.h"
#include "opentelemetry/sdk/metrics/view/instrument_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/meter_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/view_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/batch_span_processor_factory.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/trace/provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/processor.h"
#include "opentelemetry/sdk/logs/batch_log_record_processor_factory.h"
#include "opentelemetry/logs/provider.h"

MetricsProvider* METRICS = nullptr;

namespace metrics_sdk      = opentelemetry::sdk::metrics;
namespace common           = opentelemetry::common;
namespace metrics_exporter = opentelemetry::exporter::metrics;
namespace metrics_api      = opentelemetry::metrics;
namespace otlp_exporter    = opentelemetry::exporter::otlp;
namespace trace_sdk        = opentelemetry::sdk::trace;
namespace trace_api        = opentelemetry::trace;
namespace logs_sdk         = opentelemetry::sdk::logs;
namespace logs_api         = opentelemetry::logs;

MetricsProvider::MetricsProvider()
{ 
	std::string version{"1.2.0"};
	std::string schema{"https://opentelemetry.io/schemas/1.2.0"};
	std::string name{"itgmania"};
	std::string endpoint{"http://localhost:4317"};

	// Initialize and set the global MeterProvider
	{
		otlp_exporter::OtlpHttpMetricExporterOptions exporter_options;
		exporter_options.url = endpoint;
		auto exporter = otlp_exporter::OtlpHttpMetricExporterFactory::Create(exporter_options);

		metrics_sdk::PeriodicExportingMetricReaderOptions reader_options;
		reader_options.export_interval_millis = std::chrono::milliseconds(1000);
		reader_options.export_timeout_millis  = std::chrono::milliseconds(500);

		auto reader = metrics_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), reader_options);

		auto context = metrics_sdk::MeterContextFactory::Create();
		context->AddMetricReader(std::move(reader));

		auto u_provider = metrics_sdk::MeterProviderFactory::Create(std::move(context));
		std::shared_ptr<metrics_api::MeterProvider> provider(std::move(u_provider));

		metrics_api::Provider::SetMeterProvider(provider);

		// create metrics
		opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter(name, version);
		m_hitHistogram = meter->CreateUInt64Histogram("hitHistogram", "hits", "unit");
		m_hitCounter = meter->CreateUInt64Counter("hitCounter", "total hits of session", "unit");
		m_hitGauge = meter->CreateInt64Gauge("hitGauge", "hits over the current song", "unit");
	}

	// Initialize and set the global TracerProvider
	{
		otlp_exporter::OtlpHttpTraceExporterOptions exporter_options;
		exporter_options.url = endpoint;
		auto exporter = otlp_exporter::OtlpHttpTraceExporterFactory::Create(exporter_options);
		
		auto processor = trace_sdk::BatchSpanProcessorFactory::Create(std::move(exporter));
		auto provider = trace_sdk::TracerProviderFactory::Create(std::move(processor));
		
		m_tracerProvider = std::shared_ptr<trace_api::TracerProvider>(std::move(provider));
		trace_api::Provider::SetTracerProvider(m_tracerProvider);
		
		// Create default tracer
		m_defaultTracer = m_tracerProvider->GetTracer(name, version);
	}

	// Initialize and set the global LoggerProvider
	{
		otlp_exporter::OtlpHttpLogExporterOptions exporter_options;
		exporter_options.url = endpoint;
		auto exporter = otlp_exporter::OtlpHttpLogExporterFactory::Create(exporter_options);
		
		auto processor = logs_sdk::BatchLogRecordProcessorFactory::Create(std::move(exporter));
		auto provider = logs_sdk::LoggerProviderFactory::Create(std::move(processor));
		
		m_loggerProvider = std::shared_ptr<logs_api::LoggerProvider>(std::move(provider));
		logs_api::Provider::SetLoggerProvider(m_loggerProvider);
		
		// Create default logger
		m_defaultLogger = m_loggerProvider->GetLogger(name, version);
	}
}

MetricsProvider::~MetricsProvider()
{
	// Clean up providers in reverse order of initialization
	std::shared_ptr<logs_api::LoggerProvider> none_logger;
	logs_api::Provider::SetLoggerProvider(none_logger);
	
	std::shared_ptr<trace_api::TracerProvider> none_tracer;
	trace_api::Provider::SetTracerProvider(none_tracer);
	
	std::shared_ptr<metrics_api::MeterProvider> none_meter;
	metrics_api::Provider::SetMeterProvider(none_meter);
}

// Metrics API implementations
opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetHistogram()
{
	return m_hitHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> MetricsProvider::GetGauge(std::string name)
{
	return m_hitGauge;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetCounter()
{
    return m_hitCounter;
}

// Trace API implementations
opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Tracer> MetricsProvider::GetTracer(const std::string& name)
{
	if (name == "default") {
		return m_defaultTracer;
	}
	return m_tracerProvider->GetTracer(name, "1.2.0");
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Span> MetricsProvider::StartSpan(const std::string& name)
{
	return m_defaultTracer->StartSpan(name);
}

// Logs API implementations
opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::logs::Logger> MetricsProvider::GetLogger(const std::string& name)
{
	if (name == "default") {
		return m_defaultLogger;
	}
	return m_loggerProvider->GetLogger(name, "1.2.0");
}

void MetricsProvider::Log(opentelemetry::v2::logs::Severity severity, const std::string& message)
{
	m_defaultLogger->Log(severity, message);
}

// Convenience logging methods with variable arguments support
void MetricsProvider::Info(const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    Log(opentelemetry::v2::logs::Severity::kInfo, buffer);
}

void MetricsProvider::Warn(const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    Log(opentelemetry::v2::logs::Severity::kWarn, buffer);
}

void MetricsProvider::Error(const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    Log(opentelemetry::v2::logs::Severity::kError, buffer);
}

void MetricsProvider::Debug(const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    Log(opentelemetry::v2::logs::Severity::kDebug, buffer);
}
