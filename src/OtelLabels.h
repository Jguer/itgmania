// OtelLabels.h
//
// Shared helpers for building consistent OpenTelemetry attribute/label maps
// and stable, theme-independent enum names for gameplay telemetry.
//
// Cardinality note: `song_title` is included to power the live-event dashboard
// tabs. This is acceptable for a single-cabinet/live-event deployment but may
// need to be gated behind a preference for long-running production use.
#ifndef OTEL_LABELS_H
#define OTEL_LABELS_H

#include <map>
#include <string>

#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/trace/span.h"

#include "GameConstantsAndTypes.h"
#include "PlayerNumber.h"

namespace otel_labels {

// --- Current-play span registry ---------------------------------------------
// ScreenGameplay starts a long-lived `song_play` span when a chart is loaded
// and ends it when the stage finishes. Other translation units (ScoreKeeper,
// Player, etc.) use these accessors to attach gameplay-time span events like
// `combo.new_max` or `judgment.miss` to that same trace.

using SpanPtr =
    opentelemetry::v2::nostd::shared_ptr<opentelemetry::v2::trace::Span>;

// Register or clear the current `song_play` span for a given player.
// Passing an empty SpanPtr clears the slot.
void SetCurrentPlaySpan(PlayerNumber pn, SpanPtr span);

// Return the currently registered `song_play` span for `pn`, or an empty
// shared_ptr if none is active.
SpanPtr GetCurrentPlaySpan(PlayerNumber pn);

// Convenience: attach an event with string attributes to the current
// per-player play span. No-op if no span is registered. Keeps call sites
// simple and avoids leaking opentelemetry types everywhere.
void AddCurrentPlayEvent(
    PlayerNumber pn, const char* name,
    const std::map<std::string, std::string>& attrs);

// Fill gameplay identity labels into `out`:
//   player_number, song_title, song_group, steps_type, difficulty, meter
// Values are looked up from GAMESTATE; keys missing their source are omitted.
void FillGameplay(std::map<std::string, std::string>& out, PlayerNumber pn);

// Canonical, theme-independent judgment name for a TapNoteScore, e.g.
// "w1", "w2", "miss", "mine_hit", "mine_avoided", "checkpoint_hit".
const char* JudgmentName(TapNoteScore tns);

// Canonical name for a HoldNoteScore: "held", "let_go", "missed", "none".
const char* HnsName(HoldNoteScore hns);

}  // namespace otel_labels

#endif
