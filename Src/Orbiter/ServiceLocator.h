/* ServiceLocator.h
 * Provides access to stuff.
 *
 * Copyright (c) 2024 Jack Cochran
 * Licensed under the MIT license.
 */

#ifndef SRC_ORBITER_SERVICE_LOCATOR_H__
#define SRC_ORBTIER_SERVICE_LOCATOR_H__

#include <stdexcept>

class Camera;
class Select;
class InputBox;
class PlanetarySystem;
class Config;
class Orbiter;

namespace orbiter {

class ServiceLocator {
public:
    ServiceLocator();

    Camera& getCamera() const;
    Select& getSelectBox() const;
    InputBox& getInputBox() const;
    PlanetarySystem& getPSys() const;
    Config& getConfig() const;
    Orbiter& getOrbiter() const;

    void provideCamera(Camera* cam);
    void provideSelectBox(Select* sel);
    void provideInputBox(InputBox* in);
    void providePSys(PlanetarySystem* psys);
    void provideConfig(Config* conf);
    void provideOrbiter(Orbiter* orb);

private:
    Camera* camera;
    Select* selectBox;
    InputBox* inputBox;
    PlanetarySystem* psys;
    Config* conf;
    Orbiter* orbiter;
};

class ServiceNotFound : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

const ServiceLocator& getService();

}

#endif /* SRC_ORBITER_SERVICE_LOCATOR_H__ */

