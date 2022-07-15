// The MIT License (MIT)
//
// Copyright (c) 2017-2022 Alexander Kurbatov

#include "Strategy.h"

#include <sc2api/sc2_map_info.h>

#include <algorithm>

#include "Historican.h"
#include "core/API.h"
#include "core/Helpers.h"

namespace {
Historican gHistory("strategy");
}  // namespace

Strategy::Strategy(float attack_limit_) : m_attack_limit(attack_limit_) {}

void Strategy::OnStep(Builder*) {
  if (static_cast<float>(m_units.size()) < m_attack_limit) return;

  if (static_cast<float>(m_units.size()) < m_attack_limit) return;

  auto targets = gAPI->observer().GameInfo().enemy_start_locations;
  gAPI->action().Attack(m_units, targets.front());

  for (auto i : m_units) field_units.push_back(i);

  m_units.clear();
  m_attack_limit = std::min<float>(m_attack_limit * 1.5f, 100.0f);
}

void Strategy::OnUnitCreated(const sc2::Unit* unit_, Builder*) {
  if (!IsCombatUnit()(*unit_)) return;

  gHistory.info() << sc2::UnitTypeToName(unit_->unit_type)
                  << " added to attack group\n";

  m_units.push_back(unit_);
}

void Strategy::CleanUpBodies(sc2::Units& units_) {
  // Clean up dead bodies.
  auto it =
      std::remove_if(units_.begin(), units_.end(),
                     [](const sc2::Unit* unit_) { return !unit_->is_alive; });
  // Return value Past - the - end iterator for the new range of values
  units_.erase(it, units_.end());  // Yes, erasing the orphaned
}
