// MetricsProvider.cpp
#include "MetricsProvider.h"

#include "opentelemetry/exporters/prometheus/exporter_factory.h"
#include "opentelemetry/exporters/prometheus/exporter_options.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/aggregation/default_aggregation.h"
#include "opentelemetry/sdk/metrics/aggregation/histogram_aggregation.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/view/instrument_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/meter_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/view_factory.h"


MetricsProvider* METRICS = nullptr;

namespace metrics_sdk      = opentelemetry::sdk::metrics;
namespace common           = opentelemetry::common;
namespace metrics_exporter = opentelemetry::exporter::metrics;
namespace metrics_api      = opentelemetry::metrics;

MetricsProvider::MetricsProvider()
{ 
	std::string name{"itgmania"};
	std::string addr{"https://localhost:2222/metrics"};
	metrics_exporter::PrometheusExporterOptions opts;
	if (!addr.empty())
	{
		opts.url = addr;
	}
	std::puts("PrometheusExporter itgmania program running ...");

	std::string version{"1.2.0"};
	std::string schema{"https://opentelemetry.io/schemas/1.2.0"};

	auto prometheus_exporter = metrics_exporter::PrometheusExporterFactory::Create(opts);

	// Initialize and set the global MeterProvider
	auto u_provider = metrics_sdk::MeterProviderFactory::Create();
	auto *p         = static_cast<metrics_sdk::MeterProvider *>(u_provider.get());

	p->AddMetricReader(std::move(prometheus_exporter));

	// counter view
	std::string counter_name = name + "_counter";
	std::string counter_unit = "unit";

	auto instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
		metrics_sdk::InstrumentType::kCounter, counter_name, counter_unit);

	auto meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

	auto sum_view = metrics_sdk::ViewFactory::Create(counter_name, "description", counter_unit,
													metrics_sdk::AggregationType::kSum);

	p->AddView(std::move(instrument_selector), std::move(meter_selector), std::move(sum_view));

	// histogram view
	std::string histogram_name = name + "_histogram";
	std::string histogram_unit = "unit";

	auto histogram_instrument_selector = metrics_sdk::InstrumentSelectorFactory::Create(
		metrics_sdk::InstrumentType::kHistogram, histogram_name, histogram_unit);

	auto histogram_meter_selector = metrics_sdk::MeterSelectorFactory::Create(name, version, schema);

	auto histogram_view = metrics_sdk::ViewFactory::Create(
		histogram_name, "description", histogram_unit, metrics_sdk::AggregationType::kHistogram);

	p->AddView(std::move(histogram_instrument_selector), std::move(histogram_meter_selector),
				std::move(histogram_view));

	std::shared_ptr<opentelemetry::metrics::MeterProvider> provider(std::move(u_provider));
	metrics_api::Provider::SetMeterProvider(provider);
}

MetricsProvider::~MetricsProvider()
{
  std::shared_ptr<metrics_api::MeterProvider> none;
  metrics_api::Provider::SetMeterProvider(none);
}