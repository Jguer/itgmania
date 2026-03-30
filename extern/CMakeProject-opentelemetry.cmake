set(OPENTELEMETRY_DIR "opentelemetry-cpp")

# Define minimal set of components to build from opentelemetry-cpp
set(WITH_OTLP_GRPC ON CACHE BOOL "Whether to build with OTLP exporter" FORCE)
set(WITH_OTLP_HTTP ON CACHE BOOL "Whether to build with OTLP HTTP exporter" FORCE)
set(WITH_PROMETHEUS OFF CACHE BOOL "Whether to build with Prometheus exporter" FORCE)
set(WITH_ZIPKIN OFF CACHE BOOL "Whether to build with Zipkin exporter" FORCE)
set(WITH_JAEGER OFF CACHE BOOL "Whether to build with Jaeger exporter" FORCE)
set(WITH_NO_GETENV OFF CACHE BOOL "Whether to build with no getenv" FORCE)
set(WITH_ETW OFF CACHE BOOL "Whether to build with ETW tracer" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Whether to build tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Whether to build examples" FORCE)
set(WITH_BENCHMARK OFF CACHE BOOL "Whether to build benchmark program" FORCE)
set(WITH_ABSEIL ON CACHE BOOL "Whether to build with abseil" FORCE)

# Use the system-provided JSON library
set(NLOHMANN_JSON_INCLUDE_DIRS "/usr/include/nlohmann" CACHE PATH "Path to nlohmann/json.hpp" FORCE)
set(NLOHMANN_JSON_IS_EXTERNAL ON CACHE BOOL "Use external nlohmann_json library" FORCE)

# Set the ABI version to 2
set(WITH_ABI_VERSION_1 OFF CACHE BOOL "ABI version 1" FORCE)
set(WITH_ABI_VERSION_2 ON CACHE BOOL "EXPERIMENTAL: ABI version 2 preview" FORCE)

# Histogram/counter exemplars link metrics to traces (Grafana Mimir + Tempo)
set(WITH_METRICS_EXEMPLAR_PREVIEW ON CACHE BOOL "Enable metric exemplars for trace drill-down" FORCE)

# Force static build for OpenTelemetry
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all libraries as static" FORCE)

# Add the library from source
add_subdirectory(${OPENTELEMETRY_DIR} EXCLUDE_FROM_ALL)

# Set the folder property for organization in IDEs
get_property(OPENTELEMETRY_TARGETS DIRECTORY ${OPENTELEMETRY_DIR} PROPERTY BUILDSYSTEM_TARGETS)
foreach(TARGET ${OPENTELEMETRY_TARGETS})
  set_property(TARGET ${TARGET} PROPERTY FOLDER "External Libraries/OpenTelemetry")
endforeach()

# Define a target that the main project can depend on
add_library(opentelemetry INTERFACE)
target_link_libraries(opentelemetry INTERFACE opentelemetry_api opentelemetry_sdk) 