/* ServiceLocator.cpp
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#include "ServiceLocator.h"

#include <stdexcept>

namespace orbiter {

ServiceLocator::ServiceLocator()
    : camera{nullptr},
      selectBox{nullptr},
      inputBox{nullptr},
      psys{nullptr},
      conf{nullptr}  {   }


Camera& ServiceLocator::getCamera() const {
    if (!camera) {
        throw ServiceNotFound {"Global Camera not found."};
    }
    return *camera;
}

Select& ServiceLocator::getSelectBox() const {
    if (!selectBox) {
        throw ServiceNotFound {"Global SelectBox not found."};
    }
    return *selectBox;
}

InputBox& ServiceLocator::getInputBox() const {
    if (!inputBox) {
        throw ServiceNotFound {"Global InputBox not found."};
    }
    return *inputBox;
}

PlanetarySystem& ServiceLocator::getPSys() const {
    if (!psys) {
        throw ServiceNotFound {"Global PlanetarySystem not found."};
    }
    return *psys;
}

Config& ServiceLocator::getConfig() const {
    if (!conf) {
        throw ServiceNotFound {"Global Config not found."};
    }
    return *conf;
}

Orbiter& ServiceLocator::getOrbiter() const {
    if (!orbiter) {
        throw ServiceNotFound {"Global Orbiter not found."};
    }
    return *orbiter;
}


void ServiceLocator::provideCamera(Camera* cam) {
    camera = cam;
}

void ServiceLocator::provideSelectBox(Select* sel) {
    selectBox = sel;
}

void ServiceLocator::provideInputBox(InputBox* in) {
    inputBox = in;
}

void ServiceLocator::providePSys(PlanetarySystem* psys_) {
    psys = psys_;
}

void ServiceLocator::provideConfig(Config* conf_) {
    conf = conf_;
}

void ServiceLocator::provideOrbiter(Orbiter* orb) {
    orbiter = orb;
}



}

