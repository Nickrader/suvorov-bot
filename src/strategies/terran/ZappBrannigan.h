#pragma once
/* "The Killbots?... A trifle.  It was simply a matter of outsmarting them. ...
Knowing their weakness, I sent wave after wave of my own men at them, until they
reached their limit, and shutdown." -Captain Zapp Brannigan */

#include "Builder.h"
#include "strategies/Strategy.h"

struct Killbots : Strategy {
    Killbots();

    void OnGameStart(Builder* builder_) final;

    void OnStep(Builder* builder_) final;

    void OnUnitIdle(const sc2::Unit* unit_, Builder* builder_) final;

private:
    bool Should_Build_Expansion();
    bool to_build_or_not_to_build = false;

    int number_of_townhalls = 1; // we start with one command center.
    int number_of_barracks = 0; 
};
