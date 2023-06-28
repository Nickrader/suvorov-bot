// The MIT License (MIT)
//
// Copyright (c) 2017-2022 Alexander Kurbatov

#include "ChatterBox.h"
#include "Version.h"
#include "core/API.h"

#include <string>

void ChatterBox::OnGameStart(Builder*) {
    std::string hello(
        std::string(PROJECT_NAME) +
        " v" + PROJECT_VERSION +
        " by @" + PROJECT_AUTHOR);

    gAPI->action().SendMessage(hello);
    gAPI->action().SendMessage("gl hf");

    gAPI->action().SendMessage("Type 'target' to see targeting boxes.\n\
        The purple box indicates A-Move targeting position, not unit.");
}

void ChatterBox::OnStep(Builder*) {
}
