// MetricsProvider.cpp
#include "MetricsProvider.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "ver.h"

#include "opentelemetry/common/attribute_value.h"
#include "opentelemetry/exporters/otlp/otlp_environment.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_options.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_log_record_exporter_options.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_options.h"
#include "opentelemetry/metrics/meter_provider.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/aggregation/default_aggregation.h"
#include "opentelemetry/sdk/metrics/aggregation/histogram_aggregation.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_options.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_context.h"
#include "opentelemetry/sdk/metrics/meter_context_factory.h"
#ifdef ENABLE_METRICS_EXEMPLAR_PREVIEW
#	include "opentelemetry/sdk/metrics/exemplar/filter_type.h"
#endif
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/metric_reader.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"
#include "opentelemetry/sdk/metrics/state/filtered_ordered_attribute_map.h"
#include "opentelemetry/sdk/metrics/view/instrument_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/meter_selector_factory.h"
#include "opentelemetry/sdk/metrics/view/view_factory.h"
#include "opentelemetry/sdk/metrics/view/view_registry_factory.h"
#include "opentelemetry/sdk/resource/resource.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/sdk/logs/logger_provider.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "opentelemetry/sdk/logs/exporter.h"
#include "opentelemetry/sdk/logs/processor.h"
#include "opentelemetry/sdk/logs/batch_log_record_processor_factory.h"
#include "opentelemetry/sdk/logs/batch_log_record_processor_options.h"
#include "opentelemetry/sdk/trace/batch_span_processor_factory.h"
#include "opentelemetry/sdk/trace/batch_span_processor_options.h"
#include "opentelemetry/sdk/trace/samplers/always_off.h"
#include "opentelemetry/sdk/trace/samplers/always_on.h"
#include "opentelemetry/sdk/trace/samplers/trace_id_ratio.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>

MetricsProvider* METRICS = nullptr;

namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace common = opentelemetry::common;
namespace metrics_api = opentelemetry::metrics;
namespace otlp_exporter = opentelemetry::exporter::otlp;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace logs_api = opentelemetry::logs;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace_api = opentelemetry::trace;

namespace
{
std::string BuildSessionId()
{
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch())
		.count();
	auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());

	std::ostringstream stream;
	stream << std::hex << now << "-" << thread_id;
	return stream.str();
}

double GetTraceSampleRate()
{
	auto rate = PREFSMAN ? PREFSMAN->m_fOTelTraceSampleRate.Get() : 1.0f;
	return std::clamp(static_cast<double>(rate), 0.0, 1.0);
}
} // namespace

MetricsProvider::MetricsProvider()
{
	const std::string version{"1.2.0"};
	const std::string schema{"https://opentelemetry.io/schemas/1.2.0"};
	const std::string name{"itgmania"};
	const std::string session_id = BuildSessionId();
	const bool telemetry_enabled = PREFSMAN && PREFSMAN->m_bOTelEnabled.Get();
	const bool metrics_enabled =
		telemetry_enabled && PREFSMAN->m_bOTelMetricsEnabled.Get() &&
		!PREFSMAN->m_sOTLPMetricsURL.Get().empty();
	const bool logs_enabled =
		telemetry_enabled && PREFSMAN->m_bOTelLogsEnabled.Get() &&
		!PREFSMAN->m_sOTLPLogsURL.Get().empty();
	const bool traces_enabled =
		telemetry_enabled && PREFSMAN->m_bOTelTracesEnabled.Get() &&
		!PREFSMAN->m_sOTLPTracesURL.Get().empty();

	auto resource_attributes = opentelemetry::v2::sdk::resource::ResourceAttributes{
		{"service.name", "itgmania"},
		{"service.version", product_version},
		{"service.instance.id", session_id},
		{"service.namespace", "itgmania"},
		{"schema.url", schema},
	};
	auto resource = opentelemetry::v2::sdk::resource::Resource::Create(resource_attributes);

	if (metrics_enabled)
	{
		otlp_exporter::OtlpGrpcMetricExporterOptions exporter_options;
		exporter_options.endpoint = PREFSMAN->m_sOTLPMetricsURL.Get();
		auto exporter = otlp_exporter::OtlpGrpcMetricExporterFactory::Create(exporter_options);

		metrics_sdk::PeriodicExportingMetricReaderOptions reader_options;
		reader_options.export_interval_millis = std::chrono::milliseconds(2000);
		reader_options.export_timeout_millis = std::chrono::milliseconds(2000);
		auto reader =
			metrics_sdk::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), reader_options);

		auto views = metrics_sdk::ViewRegistryFactory::Create();
		auto context = metrics_sdk::MeterContextFactory::Create(std::move(views), resource);
#ifdef ENABLE_METRICS_EXEMPLAR_PREVIEW
		context->SetExemplarFilter(metrics_sdk::ExemplarFilterType::kTraceBased);
#endif
		context->AddMetricReader(std::move(reader));

		auto provider = metrics_sdk::MeterProviderFactory::Create(std::move(context));
		std::shared_ptr<opentelemetry::metrics::MeterProvider> shared_provider(std::move(provider));
		metrics_api::Provider::SetMeterProvider(shared_provider);
	}

	auto meter_provider = metrics_api::Provider::GetMeterProvider();
	auto meter = meter_provider->GetMeter(name, version);
	m_hitHistogram = meter->CreateUInt64Histogram(
		"itgmania_note_hit_timing_histogram",
		"Distribution of note hit timings (ms) relative to perfect",
		"ms");
	m_hitCounter = meter->CreateUInt64Counter(
		"itgmania_note_hits_total",
		"Total number of notes hit in the current session.",
		"count");
	m_hitGauge = meter->CreateInt64Gauge(
		"itgmania_current_song_note_hits",
		"Number of notes hit in the current song.",
		"count");
	m_scoreGauge = meter->CreateInt64Gauge(
		"itgmania_current_score",
		"Current EX Score or similar for the player.",
		"points");
	m_comboGauge = meter->CreateInt64Gauge(
		"itgmania_current_combo",
		"Current unbroken combo count.",
		"notes");
	m_maxComboGauge = meter->CreateInt64Gauge(
		"itgmania_max_combo",
		"Maximum combo achieved in the current song.",
		"notes");
	m_lifeGauge = meter->CreateInt64Gauge(
		"itgmania_life_bar",
		"Current fill level of the dance gauge/life bar.",
		"percent");
	m_fpsGauge = meter->CreateInt64Gauge(
		"itgmania_fps",
		"Current frames per second of the game.",
		"fps");
	m_httpRequestDuration = meter->CreateUInt64Histogram(
		"itgmania_http_request_duration_ms",
		"Duration of outbound HTTP requests.",
		"ms");
	m_httpRequestCounter = meter->CreateUInt64Counter(
		"itgmania_http_requests_total",
		"Total number of outbound HTTP requests.",
		"count");
	m_httpRequestErrorCounter = meter->CreateUInt64Counter(
		"itgmania_http_request_errors_total",
		"Total number of failed outbound HTTP requests.",
		"count");
	m_websocketOpenDuration = meter->CreateUInt64Histogram(
		"itgmania_websocket_open_duration_ms",
		"WebSocket session open duration.",
		"ms");
	m_websocketOpenCounter = meter->CreateUInt64Counter(
		"itgmania_websocket_open_total",
		"Total number of websocket open events.",
		"count");
	m_websocketErrorCounter = meter->CreateUInt64Counter(
		"itgmania_websocket_error_total",
		"Total number of websocket error events.",
		"count");
	m_frameTimeHistogram = meter->CreateUInt64Histogram(
		"itgmania_frame_time_ms",
		"Gameplay loop frame time.",
		"ms");
	m_frameDropCounter = meter->CreateUInt64Counter(
		"itgmania_frame_drops_total",
		"Number of gameplay frames that exceeded the slow-frame threshold.",
		"count");
	m_accuracyGauge = meter->CreateInt64Gauge(
		"itgmania_accuracy_percent",
		"Current dance points accuracy in basis points (percent * 100) vs possible so far.",
		"basis_points");
	m_dancePointsActualGauge = meter->CreateInt64Gauge(
		"itgmania_dance_points_actual",
		"Actual dance points accumulated so far in the current song.",
		"points");
	m_dancePointsPossibleGauge = meter->CreateInt64Gauge(
		"itgmania_dance_points_possible",
		"Possible dance points judged so far in the current song.",
		"points");
	m_songPlaysCounter = meter->CreateUInt64Counter(
		"itgmania_song_plays_total",
		"Number of completed song/stage plays recorded for telemetry.",
		"count");
	m_songFinalAccuracyBpsHistogram = meter->CreateUInt64Histogram(
		"itgmania_song_final_accuracy_bps",
		"Final dance-points accuracy per completed play (basis points: 10000 = 100%).",
		"basis_points");
	m_songFinalScoreHistogram = meter->CreateUInt64Histogram(
		"itgmania_song_final_score",
		"Final score per completed play.",
		"points");
	m_songMaxComboHistogram = meter->CreateUInt64Histogram(
		"itgmania_song_max_combo",
		"Maximum combo per completed play.",
		"notes");
	m_songPlayDurationMsHistogram = meter->CreateUInt64Histogram(
		"itgmania_song_play_duration_ms",
		"Stage gameplay duration per completed play.",
		"ms");
	m_songFinalLifePercentHistogram = meter->CreateUInt64Histogram(
		"itgmania_song_final_life_percent",
		"Life gauge at end of play (0-100).",
		"percent");
	m_noteHitOffsetHistogram = meter->CreateDoubleHistogram(
		"itgmania_note_hit_offset_ms",
		"Signed note hit timing offset (negative = early, positive = late).",
		"ms");
	m_holdNoteScoresCounter = meter->CreateUInt64Counter(
		"itgmania_hold_note_scores_total",
		"Hold / Roll outcomes (held, let_go, missed).",
		"count");
	m_mineEventsCounter = meter->CreateUInt64Counter(
		"itgmania_mine_events_total",
		"Mine interactions (hit, avoided).",
		"count");
	m_currentSongInfoGauge = meter->CreateInt64Gauge(
		"itgmania_current_song_info",
		"Always 1 while a song is selected; labels describe the currently-loaded chart.",
		"info");
	m_lastPlayScoreGauge = meter->CreateInt64Gauge(
		"itgmania_last_play_score",
		"Final score of the most recent completed play. Query with max_over_time for per-range bests.",
		"points");
	m_lastPlayMaxComboGauge = meter->CreateInt64Gauge(
		"itgmania_last_play_max_combo",
		"Max combo of the most recent completed play. Query with max_over_time for per-range bests.",
		"notes");

	if (logs_enabled)
	{
		otlp_exporter::OtlpGrpcLogRecordExporterOptions log_exporter_options;
		log_exporter_options.endpoint = PREFSMAN->m_sOTLPLogsURL.Get();
		auto log_exporter = otlp_exporter::OtlpGrpcLogRecordExporterFactory::Create(log_exporter_options);
		logs_sdk::BatchLogRecordProcessorOptions log_batch_options;
		auto log_processor = logs_sdk::BatchLogRecordProcessorFactory::Create(
			std::move(log_exporter), log_batch_options);
		auto logger_provider = logs_sdk::LoggerProviderFactory::Create(std::move(log_processor), resource);
		std::shared_ptr<logs_api::LoggerProvider> shared_logger_provider(std::move(logger_provider));
		logs_api::Provider::SetLoggerProvider(shared_logger_provider);
		m_logger = shared_logger_provider->GetLogger(name, version);
	}
	else if (const auto logger_provider = logs_api::Provider::GetLoggerProvider())
	{
		m_logger = logger_provider->GetLogger(name, version);
	}

	if (traces_enabled)
	{
		otlp_exporter::OtlpGrpcExporterOptions trace_exporter_options;
		trace_exporter_options.endpoint = PREFSMAN->m_sOTLPTracesURL.Get();
		auto trace_exporter = otlp_exporter::OtlpGrpcExporterFactory::Create(trace_exporter_options);
		trace_sdk::BatchSpanProcessorOptions span_batch_options;
		auto span_processor = trace_sdk::BatchSpanProcessorFactory::Create(
			std::move(trace_exporter), span_batch_options);

		const double sample_rate = GetTraceSampleRate();
		std::unique_ptr<trace_sdk::Sampler> sampler =
			std::make_unique<trace_sdk::AlwaysOnSampler>();
		if (sample_rate <= 0.0)
		{
			sampler = std::make_unique<trace_sdk::AlwaysOffSampler>();
		}
		else if (sample_rate < 1.0)
		{
			sampler = std::make_unique<trace_sdk::TraceIdRatioBasedSampler>(sample_rate);
		}

		auto tracer_provider_unique = trace_sdk::TracerProviderFactory::Create(
			std::move(span_processor),
			resource,
			std::move(sampler));
		std::shared_ptr<trace_api::TracerProvider> tracer_provider(std::move(tracer_provider_unique));
		trace_api::Provider::SetTracerProvider(tracer_provider);
		m_tracer = tracer_provider->GetTracer(name, version);
	}
	else if (const auto tracer_provider = trace_api::Provider::GetTracerProvider())
	{
		m_tracer = tracer_provider->GetTracer(name, version);
	}
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
	if (name == "hitGauge") return m_hitGauge;
	if (name == "scoreGauge") return m_scoreGauge;
	if (name == "comboGauge") return m_comboGauge;
	if (name == "maxComboGauge") return m_maxComboGauge;
	if (name == "lifeGauge") return m_lifeGauge;
	if (name == "fpsGauge") return m_fpsGauge;
	return opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>>();
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetCounter()
{
	return m_hitCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetHttpRequestDuration()
{
	return m_httpRequestDuration;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetHttpRequestCounter()
{
	return m_httpRequestCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetHttpRequestErrorCounter()
{
	return m_httpRequestErrorCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetWebSocketOpenDuration()
{
	return m_websocketOpenDuration;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetWebSocketOpenCounter()
{
	return m_websocketOpenCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetWebSocketErrorCounter()
{
	return m_websocketErrorCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetFrameTimeHistogram()
{
	return m_frameTimeHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetFrameDropCounter()
{
	return m_frameDropCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> MetricsProvider::GetAccuracyGauge()
{
	return m_accuracyGauge;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> MetricsProvider::GetDancePointsActualGauge()
{
	return m_dancePointsActualGauge;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> MetricsProvider::GetDancePointsPossibleGauge()
{
	return m_dancePointsPossibleGauge;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetSongPlaysCounter()
{
	return m_songPlaysCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetSongFinalAccuracyBpsHistogram()
{
	return m_songFinalAccuracyBpsHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetSongFinalScoreHistogram()
{
	return m_songFinalScoreHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetSongMaxComboHistogram()
{
	return m_songMaxComboHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetSongPlayDurationMsHistogram()
{
	return m_songPlayDurationMsHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<uint64_t>> MetricsProvider::GetSongFinalLifePercentHistogram()
{
	return m_songFinalLifePercentHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Histogram<double>> MetricsProvider::GetNoteHitOffsetHistogram()
{
	return m_noteHitOffsetHistogram;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetHoldNoteScoresCounter()
{
	return m_holdNoteScoresCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Counter<uint64_t>> MetricsProvider::GetMineEventsCounter()
{
	return m_mineEventsCounter;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> MetricsProvider::GetCurrentSongInfoGauge()
{
	return m_currentSongInfoGauge;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> MetricsProvider::GetLastPlayScoreGauge()
{
	return m_lastPlayScoreGauge;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::metrics::Gauge<int64_t>> MetricsProvider::GetLastPlayMaxComboGauge()
{
	return m_lastPlayMaxComboGauge;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::logs::Logger> MetricsProvider::GetLogger()
{
	return m_logger;
}

opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Tracer> MetricsProvider::GetTracer()
{
	return m_tracer;
}
