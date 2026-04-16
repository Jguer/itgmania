// OtelLabels.cpp
#include "OtelLabels.h"

#include <string>
#include <utility>

#include "opentelemetry/common/key_value_iterable_view.h"

#include "Difficulty.h"
#include "GameState.h"
#include "Song.h"
#include "Steps.h"
#include "Style.h"
#include "global.h"

namespace otel_labels {

namespace {
// Slot-per-player registry of the currently-active `song_play` span. Set by
// ScreenGameplay when a chart loads, cleared when the stage ends.
SpanPtr g_currentPlaySpans[NUM_PLAYERS];

bool ValidPn(PlayerNumber pn) { return pn >= 0 && pn < NUM_PLAYERS; }
}  // namespace

void SetCurrentPlaySpan(PlayerNumber pn, SpanPtr span) {
  if (!ValidPn(pn)) {
    return;
  }
  g_currentPlaySpans[pn] = std::move(span);
}

SpanPtr GetCurrentPlaySpan(PlayerNumber pn) {
  if (!ValidPn(pn)) {
    return SpanPtr{};
  }
  return g_currentPlaySpans[pn];
}

void AddCurrentPlayEvent(
    PlayerNumber pn, const char* name,
    const std::map<std::string, std::string>& attrs) {
  SpanPtr span = GetCurrentPlaySpan(pn);
  if (!span) {
    return;
  }
  using AttrMap = std::map<std::string, std::string>;
  auto kv = opentelemetry::common::KeyValueIterableView<AttrMap>{attrs};
  span->AddEvent(name, kv);
}

void FillGameplay(std::map<std::string, std::string>& out, PlayerNumber pn) {
  out["player_number"] = std::to_string(pn);

  if (GAMESTATE == nullptr) {
    return;
  }

  const Song* song = GAMESTATE->m_pCurSong;
  if (song != nullptr) {
    out["song_title"] = song->GetTranslitMainTitle();
    out["song_group"] = song->m_sGroupName;
  }

  Steps* steps = GAMESTATE->m_pCurSteps[pn];
  if (steps != nullptr) {
    out["difficulty"] = DifficultyToString(steps->GetDifficulty());
    out["meter"] = std::to_string(steps->GetMeter());
  }

  const Style* style = GAMESTATE->GetCurrentStyle(pn);
  if (style != nullptr) {
    out["steps_type"] = StepsTypeToString(style->m_StepsType);
  }
}

const char* JudgmentName(TapNoteScore tns) {
  switch (tns) {
    case TNS_W1:
      return "w1";
    case TNS_W2:
      return "w2";
    case TNS_W3:
      return "w3";
    case TNS_W4:
      return "w4";
    case TNS_W5:
      return "w5";
    case TNS_Miss:
      return "miss";
    case TNS_HitMine:
      return "mine_hit";
    case TNS_AvoidMine:
      return "mine_avoided";
    case TNS_CheckpointHit:
      return "checkpoint_hit";
    case TNS_CheckpointMiss:
      return "checkpoint_miss";
    case TNS_None:
      return "none";
    case NUM_TapNoteScore:
    case TapNoteScore_Invalid:
      break;
  }
  return "unknown";
}

const char* HnsName(HoldNoteScore hns) {
  switch (hns) {
    case HNS_Held:
      return "held";
    case HNS_LetGo:
      return "let_go";
    case HNS_Missed:
      return "missed";
    case HNS_None:
      return "none";
    case NUM_HoldNoteScore:
    case HoldNoteScore_Invalid:
      break;
  }
  return "unknown";
}

}  // namespace otel_labels
