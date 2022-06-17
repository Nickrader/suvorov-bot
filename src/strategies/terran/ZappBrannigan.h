#pragma once
/* "The Killbots?... A trifle.  It was simply a matter of outsmarting them. ...
Knowing their weakness, I sent wave after wave of my own men at them, until they
reached their limit, and shutdown." -Captain Zapp Brannigan */

#include "Builder.h"
#include "strategies/Strategy.h"

struct Killbots : Strategy {
    Killbots();

    void OnGameStart(Builder*) final;

    void OnStep(Builder*) final;

    void OnUnitIdle(const sc2::Unit*, Builder*) final;

    void OnUnitCreated(const sc2::Unit*, Builder*);

    //void OnUnitEnterVision(const sc2::Unit*, Builder*);

private:
    bool Should_Build_Expansion();
    bool to_build_or_not_to_build = false;

    int number_of_townhalls = 1; // we start with one command center.
    int number_of_barracks = 0; 
};

// TODO: set rally point when Barracks is started?constructed?Idle?
//  Obviously, when it gets called will be important, but for now,
//  just want to 
