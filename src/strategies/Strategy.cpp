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
  if (static_cast<float>(m_units.size()) < m_attack_limit &&
      gAPI->observer().GetFoodUsed() < 190)
    return;

  auto targets = gAPI->observer().GameInfo().enemy_start_locations;
  gAPI->action().Attack(m_units, targets.front());
  TransferToField(m_units);
  m_attack_limit = std::min<float>(m_attack_limit * 1.5f, 100.0f);
}

void Strategy::OnUnitCreated(const sc2::Unit* unit_, Builder*) {
  if (!IsCombatUnit()(*unit_)) return;

  gHistory.info() << sc2::UnitTypeToName(unit_->unit_type)
                  << " added to attack group\n";

  m_units.push_back(unit_);
}

void Strategy::SetAttackLimit(float attack_limit_) {
  m_attack_limit = attack_limit_;
}

float Strategy::GetAttackLimit() { return m_attack_limit; }

void Strategy::CleanUpBodies(sc2::Units& units_) {
  // Clean up dead bodies.
  auto it =
      std::remove_if(units_.begin(), units_.end(),
                     [](const sc2::Unit* unit_) { return !unit_->is_alive; });

  units_.erase(it, units_.end());
}

void Strategy::TransferToField(sc2::Units& units_) {
  for (auto i : units_) field_units.push_back(i);
  units_.clear();
}
