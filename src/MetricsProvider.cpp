// MetricsProvider.cpp
#include "MetricsProvider.h"
#include "RageUtil.h"
#include "PrefsManager.h"

#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_options.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/common/global_log_handler.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_context_factory.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_options.h"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_options.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

MetricsProvider* METRICS = nullptr;

namespace metrics_sdk      = opentelemetry::sdk::metrics;
namespace common           = opentelemetry::common;
namespace metrics_api      = opentelemetry::metrics;
namespace otlp_exporter    = opentelemetry::exporter::otlp;
namespace logs_sdk         = opentelemetry::sdk::logs;
namespace logs_api         = opentelemetry::logs;
namespace trace_sdk        = opentelemetry::sdk::trace;
namespace trace_api        = opentelemetry::trace;

MetricsProvider::MetricsProvider()
{ 
	otlp_exporter::OtlpGrpcMetricExporterOptions exporter_options;
	exporter_options.endpoint = PREFSMAN->m_sOTLPMetricsURL.Get();
	auto exporter = otlp_exporter::OtlpGrpcMetricExporterFactory::Create(exporter_options);
	std::string version{"1.2.0"};
	std::string schema{"https://opentelemetry.io/schemas/1.2.0"};
	std::string name{"itgmania"};

	// Initialize and set the global MeterProvider
	metrics_sdk::PeriodicExportingMetricReaderOptions reader_options;
	reader_options.export_interval_millis = std::chrono::milliseconds(1000);
	reader_options.export_timeout_millis  = std::chrono::milliseconds(500);

	auto reader =
		metrics_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), reader_options);

	auto context = metrics_sdk::MeterContextFactory::Create();
	context->AddMetricReader(std::move(reader));

	auto u_provider = metrics_sdk::MeterProviderFactory::Create(std::move(context));
	std::shared_ptr<opentelemetry::metrics::MeterProvider> provider(std::move(u_provider));

	metrics_api::Provider::SetMeterProvider(provider);

	// create metrics
	opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter(name, "1.2.0");
	m_hitHistogram = meter->CreateUInt64Histogram("hitHistogram", "hits", "unit");
	m_hitCounter = meter->CreateUInt64Counter("hitCounter", "total hits of session", "unit");
	m_hitGauge = meter->CreateInt64Gauge("hitGauge", "hits over the current song", "unit");

	// Initialize logger
	otlp_exporter::OtlpGrpcLogRecordExporterOptions log_exporter_options;
	log_exporter_options.endpoint = PREFSMAN->m_sOTLPLogsURL.Get();
	
	// Set debug logging via the SDK's log handler if needed
	opentelemetry::sdk::common::internal_log::GlobalLogHandler::SetLogLevel(
		opentelemetry::sdk::common::internal_log::LogLevel::Debug);
	
	auto log_exporter = otlp_exporter::OtlpGrpcLogRecordExporterFactory::Create(log_exporter_options);
	
	// Create a processor for the logger
	auto log_processor = logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(log_exporter));
	
	// Create a LoggerProvider with the processor
	auto logger_provider = logs_sdk::LoggerProviderFactory::Create(std::move(log_processor));
	
	// Set as the global LoggerProvider
	std::shared_ptr<logs_api::LoggerProvider> shared_logger_provider(std::move(logger_provider));
	logs_api::Provider::SetLoggerProvider(shared_logger_provider);
	
	// Create a logger
	m_logger = shared_logger_provider->GetLogger(name, version);

	// Initialize tracer
	otlp_exporter::OtlpGrpcExporterOptions trace_exporter_options;
	trace_exporter_options.endpoint = PREFSMAN->m_sOTLPTracesURL.Get();
	
	auto trace_exporter = otlp_exporter::OtlpGrpcExporterFactory::Create(trace_exporter_options);
	
	// Create a span processor
	auto span_processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(trace_exporter));
	
	// Create a TracerProvider with the processor
	auto tracer_provider_unique = trace_sdk::TracerProviderFactory::Create(std::move(span_processor));
	
	// Convert to shared_ptr to set global provider
	std::shared_ptr<trace_api::TracerProvider> tracer_provider(std::move(tracer_provider_unique));
	
	// Set as the global TracerProvider
	trace_api::Provider::SetTracerProvider(tracer_provider);
	
	// Create a tracer
	m_tracer = tracer_provider->GetTracer(name, version);
}

MetricsProvider::~MetricsProvider()
{
  // Clean up meter provider
  std::shared_ptr<metrics_api::MeterProvider> none;
  metrics_api::Provider::SetMeterProvider(none);
  
  // Clean up logger provider
  std::shared_ptr<logs_api::LoggerProvider> none_logger;
  logs_api::Provider::SetLoggerProvider(none_logger);

  // Clean up tracer provider
  std::shared_ptr<trace_api::TracerProvider> none_tracer;
  trace_api::Provider::SetTracerProvider(none_tracer);
}

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

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::logs::Logger> MetricsProvider::GetLogger()
{
    return m_logger;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Tracer> MetricsProvider::GetTracer()
{
    return m_tracer;
}
