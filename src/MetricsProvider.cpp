// MetricsProvider.cpp
#include "MetricsProvider.h"

#include "opentelemetry/common/attribute_value.h"
#include "opentelemetry/exporters/otlp/otlp_environment.h"
#include "opentelemetry/exporters/otlp/otlp_http.h"
#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h"
#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h"
#include "opentelemetry/exporters/prometheus/exporter_factory.h"
#include "opentelemetry/exporters/prometheus/exporter_options.h"
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

MetricsProvider* METRICS = nullptr;

namespace metrics_sdk      = opentelemetry::sdk::metrics;
namespace common           = opentelemetry::common;
namespace metrics_exporter = opentelemetry::exporter::metrics;
namespace metrics_api      = opentelemetry::metrics;
namespace otlp_exporter = opentelemetry::exporter::otlp;

MetricsProvider::MetricsProvider()
{ 
	otlp_exporter::OtlpHttpMetricExporterOptions exporter_options;
	auto exporter = otlp_exporter::OtlpHttpMetricExporterFactory::Create(exporter_options);
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
}

MetricsProvider::~MetricsProvider()
{
  std::shared_ptr<metrics_api::MeterProvider> none;
  metrics_api::Provider::SetMeterProvider(none);
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
