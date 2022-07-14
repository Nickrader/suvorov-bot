// The MIT License (MIT)
//
// Copyright (c) 2017-2022 Alexander Kurbatov

#pragma once

#include "plugins/Plugin.h"

struct Strategy : Plugin {
    explicit Strategy(float attack_limit_);

    void OnStep(Builder*) override;

    void OnUnitCreated(const sc2::Unit* unit_, Builder*) override;

    void Strategy::CleanUpBodies(sc2::Units& units_); 

 protected:
    float m_attack_limit;
    sc2::Units m_units;
    sc2::Units field_units;
};
