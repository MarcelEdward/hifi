//
//  SixenseManager.h
//  input-plugins/src/input-plugins
//
//  Created by Andrzej Kapolka on 11/15/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SixenseManager_h
#define hifi_SixenseManager_h

#ifdef HAVE_SIXENSE
    #include <glm/glm.hpp>
    #include <glm/gtc/quaternion.hpp>
    #include "sixense.h"

#ifdef __APPLE__
    #include <QCoreApplication>
    #include <qlibrary.h>
#endif

#endif

#include "InputPlugin.h"
#include "InputDevice.h"

class QLibrary;

const unsigned int BUTTON_0 = 1U << 0; // the skinny button between 1 and 2
const unsigned int BUTTON_1 = 1U << 5;
const unsigned int BUTTON_2 = 1U << 6;
const unsigned int BUTTON_3 = 1U << 3;
const unsigned int BUTTON_4 = 1U << 4;
const unsigned int BUTTON_FWD = 1U << 7;
const unsigned int BUTTON_TRIGGER = 1U << 8;

const bool DEFAULT_INVERT_SIXENSE_MOUSE_BUTTONS = false;

// Handles interaction with the Sixense SDK (e.g., Razer Hydra).
class SixenseManager : public InputPlugin, public InputDevice {
    Q_OBJECT
public:
    enum JoystickAxisChannel {
        AXIS_Y_POS = 1U << 0,
        AXIS_Y_NEG = 1U << 3,
        AXIS_X_POS = 1U << 4,
        AXIS_X_NEG = 1U << 5,
        BACK_TRIGGER = 1U << 6,
    };
    
    enum JointChannel {
        LEFT_HAND = 0,
        RIGHT_HAND,
    };

    SixenseManager();
    
    static SixenseManager& getInstance();

    // Plugin functions
    virtual bool isSupported() const override;
    virtual bool isJointController() const override { return true; }
    const QString& getName() const { return NAME; }

    virtual void activate() override;
    virtual void deactivate() override;

    virtual void pluginFocusOutEvent() override { focusOutEvent(); }
    virtual void pluginUpdate(float deltaTime, bool jointsCaptured) override { update(deltaTime, jointsCaptured); }

    // Device functions
    virtual void registerToUserInputMapper(UserInputMapper& mapper) override;
    virtual void assignDefaultInputMapping(UserInputMapper& mapper) override;
    virtual void update(float deltaTime, bool jointsCaptured) override;
    virtual void focusOutEvent() override;

    bool getInvertButtons() const { return _invertButtons; }
    void setInvertButtons(bool invertSixenseButtons) { _invertButtons = invertSixenseButtons; }
    
    UserInputMapper::Input makeInput(unsigned int button, int index);
    UserInputMapper::Input makeInput(JoystickAxisChannel axis, int index);
    UserInputMapper::Input makeInput(JointChannel joint);

public slots:
    void setFilter(bool filter);

private:    
    void handleButtonEvent(unsigned int buttons, int index);
    void handleAxisEvent(float x, float y, float trigger, int index);
    void handlePoseEvent(glm::vec3 position, glm::quat rotation, int index);

    void updateCalibration(void* controllers);
    
    int _calibrationState;

    // these are calibration results
    glm::vec3 _neckBase;    // midpoint between controllers during X-axis calibration
    glm::quat _orbRotation; // rotates from orb frame into body frame
    float _armLength;

    // these are measured values used to compute the calibration results
    quint64 _lockExpiry;
    glm::vec3 _averageLeft;
    glm::vec3 _averageRight;
    glm::vec3 _reachLeft;
    glm::vec3 _reachRight;
    glm::vec3 _reachUp;
    glm::vec3 _reachForward;
    float _lastDistance;
    
#ifdef __APPLE__
    QLibrary* _sixenseLibrary;
#endif
    
    bool _hydrasConnected;

    bool _invertButtons = DEFAULT_INVERT_SIXENSE_MOUSE_BUTTONS;

    static const QString NAME;
};

#endif // hifi_SixenseManager_h
