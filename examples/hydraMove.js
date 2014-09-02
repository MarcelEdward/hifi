//
//  hydraMove.js
//  examples
//
//  Created by Brad Hefta-Gaub on 2/10/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  This is an example script that demonstrates use of the Controller and MyAvatar classes to implement
//  avatar flying through the hydra/controller joysticks
//
//  The joysticks (on hydra) will drive the avatar much like a playstation controller. 
// 
//  Pressing the '4' or the 'FWD' button and moving/banking the hand will allow you to  move and fly.  
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var damping = 0.9;
var position = { x: MyAvatar.position.x, y: MyAvatar.position.y, z: MyAvatar.position.z };
var joysticksCaptured = false;
var THRUST_CONTROLLER = 0;
var VIEW_CONTROLLER = 1;
var INITIAL_THRUST_MULTIPLIER = 1.0;
var THRUST_INCREASE_RATE = 1.05;
var MAX_THRUST_MULTIPLIER = 75.0;
var thrustMultiplier = INITIAL_THRUST_MULTIPLIER;
var grabDelta = { x: 0, y: 0, z: 0};
var grabStartPosition = { x: 0, y: 0, z: 0};
var grabDeltaVelocity = { x: 0, y: 0, z: 0};
var grabStartRotation = { x: 0, y: 0, z: 0, w: 1};
var grabCurrentRotation = { x: 0, y: 0, z: 0, w: 1};
var grabbingWithRightHand = false;
var wasGrabbingWithRightHand = false;
var grabbingWithLeftHand = false;
var wasGrabbingWithLeftHand = false;
var movingWithHead = false; 
var headStartPosition, headStartDeltaPitch, headStartFinalPitch, headStartRoll, headStartYaw;
var EPSILON = 0.000001;
var velocity = { x: 0, y: 0, z: 0};
var THRUST_MAG_UP = 100.0;
var THRUST_MAG_DOWN = 100.0;
var THRUST_MAG_FWD = 150.0;
var THRUST_MAG_BACK = 100.0;
var THRUST_MAG_LATERAL = 150.0;
var THRUST_JUMP = 120.0;

var YAW_MAG = 100.0;
var PITCH_MAG = 100.0;
var THRUST_MAG_HAND_JETS = THRUST_MAG_FWD;
var JOYSTICK_YAW_MAG = YAW_MAG;
var JOYSTICK_PITCH_MAG = PITCH_MAG * 0.5;


var LEFT_PALM = 0;
var LEFT_BUTTON_4 = 4;
var LEFT_BUTTON_FWD = 5;
var RIGHT_PALM = 2;
var RIGHT_BUTTON_4 = 10;
var RIGHT_BUTTON_FWD = 11;



function printVector(text, v, decimals) {
    print(text + " " + v.x.toFixed(decimals) + ", " + v.y.toFixed(decimals) + ", " + v.z.toFixed(decimals));
}

var debug = false;
var RED_COLOR = { red: 255, green: 0, blue: 0 };
var GRAY_COLOR = { red: 25, green: 25, blue: 25 };
var defaultPosition = { x: 0, y: 0, z: 0};
var RADIUS = 0.05;
var greenSphere = -1;
var redSphere = -1;
 
function createDebugOverlay() {

    if (greenSphere == -1) {
        greenSphere = Overlays.addOverlay("sphere", {
                                position: defaultPosition,
                                size: RADIUS,
                                color: GRAY_COLOR,
                                alpha: 0.75,
                                visible: true,
                                solid: true,
                                anchor: "MyAvatar"
                                });
        redSphere = Overlays.addOverlay("sphere", {
                                position: defaultPosition,
                                size: RADIUS,
                                color: RED_COLOR,
                                alpha: 0.5,
                                visible: true,
                                solid: true,
                                anchor: "MyAvatar"
                                });
    }
}

function destroyDebugOverlay() {
    if (greenSphere != -1) {
        Overlays.deleteOverlay(greenSphere);
        Overlays.deleteOverlay(redSphere);
        greenSphere = -1;
        redSphere = -1;
    }
}

function displayDebug() {
    if (!(grabbingWithRightHand || grabbingWithLeftHand)) {
        if (greenSphere != -1) {
            destroyDebugOverlay();
        }
    } else {
        // update debug indicator
        if (greenSphere == -1) {    
            createDebugOverlay();
        }

        var displayOffset = { x:0, y:0.5, z:-0.5 };

        Overlays.editOverlay(greenSphere, { position: Vec3.sum(grabStartPosition, displayOffset) } );
        Overlays.editOverlay(redSphere, { position: Vec3.sum(Vec3.sum(grabStartPosition, grabDelta), displayOffset), size: RADIUS + (0.25 * Vec3.length(grabDelta)) } );
    }
}

function getJoystickPosition(palm) {
    // returns CONTROLLER_ID position in avatar local frame
    var invRotation = Quat.inverse(MyAvatar.orientation);
    var palmWorld = Controller.getSpatialControlPosition(palm);
    var palmRelative = Vec3.subtract(palmWorld, MyAvatar.position);
    var palmLocal = Vec3.multiplyQbyV(invRotation, palmRelative);
    return palmLocal;
}

// Used by handleGrabBehavior() for managing the grab position changes
function getAndResetGrabDelta() {
    var HAND_GRAB_SCALE_DISTANCE = 2.0;
    var delta = Vec3.multiply(grabDelta, (MyAvatar.scale * HAND_GRAB_SCALE_DISTANCE));
    grabDelta = { x: 0, y: 0, z: 0};
    var avatarRotation = MyAvatar.orientation;
    var result = Vec3.multiplyQbyV(avatarRotation, Vec3.multiply(delta, -1));
    return result;
}

function getGrabRotation() {
    var quatDiff = Quat.multiply(grabCurrentRotation, Quat.inverse(grabStartRotation));
    return quatDiff;
}

// When move button is pressed, process results 
function handleGrabBehavior(deltaTime) {
    // check for and handle grab behaviors
    grabbingWithRightHand = Controller.isButtonPressed(RIGHT_BUTTON_4);
    grabbingWithLeftHand = Controller.isButtonPressed(LEFT_BUTTON_4);
    stoppedGrabbingWithLeftHand = false;
    stoppedGrabbingWithRightHand = false;
    
    if (grabbingWithRightHand && !wasGrabbingWithRightHand) {
        // Just starting grab, capture starting rotation
        grabStartRotation = Controller.getSpatialControlRawRotation(RIGHT_PALM);
        grabStartPosition = getJoystickPosition(RIGHT_PALM);
        if (debug) printVector("start position", grabStartPosition, 3);
    }
    if (grabbingWithRightHand) {
        grabDelta = Vec3.subtract(getJoystickPosition(RIGHT_PALM), grabStartPosition);
        grabCurrentRotation = Controller.getSpatialControlRawRotation(RIGHT_PALM);
    }
    if (!grabbingWithRightHand && wasGrabbingWithRightHand) {
        // Just ending grab, capture velocity
        grabDeltaVelocity = Controller.getSpatialControlVelocity(RIGHT_PALM);
        stoppedGrabbingWithRightHand = true;
    }

    if (grabbingWithLeftHand && !wasGrabbingWithLeftHand) {
        // Just starting grab, capture starting rotation
        grabStartRotation = Controller.getSpatialControlRawRotation(LEFT_PALM);
        grabStartPosition = getJoystickPosition(LEFT_PALM);
        if (debug) printVector("start position", grabStartPosition, 3);
    }

    if (grabbingWithLeftHand) {
        grabDelta = Vec3.subtract(getJoystickPosition(LEFT_PALM), grabStartPosition);
        grabCurrentRotation = Controller.getSpatialControlRawRotation(LEFT_PALM);
    }
    if (!grabbingWithLeftHand && wasGrabbingWithLeftHand) {
        // Just ending grab, capture velocity
        grabDeltaVelocity = Controller.getSpatialControlVelocity(LEFT_PALM);
        stoppedGrabbingWithLeftHand = true;
    }

    grabbing = grabbingWithRightHand || grabbingWithLeftHand;
    stoppedGrabbing = stoppedGrabbingWithRightHand || stoppedGrabbingWithLeftHand;

    if (grabbing) {
    
        var headOrientation = MyAvatar.headOrientation;
        var front = Quat.getFront(headOrientation);
        var right = Quat.getRight(headOrientation);
        var up = Quat.getUp(headOrientation);

        if (debug) {
            printVector("grabDelta: ", grabDelta, 3);
        }

        var thrust = Vec3.multiply(grabDelta, Math.abs(Vec3.length(grabDelta)));

        var THRUST_GRAB_SCALING = 100000.0;
        
        var thrustFront = Vec3.multiply(front, MyAvatar.scale * -thrust.z * THRUST_GRAB_SCALING * deltaTime);
        MyAvatar.addThrust(thrustFront);
        var thrustRight = Vec3.multiply(right, MyAvatar.scale * thrust.x * THRUST_GRAB_SCALING * deltaTime);
        MyAvatar.addThrust(thrustRight);
        var thrustUp = Vec3.multiply(up, MyAvatar.scale * thrust.y * THRUST_GRAB_SCALING * deltaTime);
        MyAvatar.addThrust(thrustUp);
        
        // add some rotation...
        var deltaRotation = getGrabRotation();
        var PITCH_SCALING = 2.5;
        var PITCH_DEAD_ZONE = 2.0;
        var YAW_SCALING = 2.5;
        var ROLL_SCALING = 2.0;     

        var euler = Quat.safeEulerAngles(deltaRotation);
 
        //  Adjust body yaw by roll from controller
        var orientation = Quat.multiply(Quat.angleAxis(((euler.y * YAW_SCALING) + 
                                                        (euler.z * ROLL_SCALING)) * deltaTime, {x:0, y: 1, z:0}), MyAvatar.orientation);
        MyAvatar.orientation = orientation;

        //  Adjust head pitch from controller
        var pitch = 0.0; 
        if (Math.abs(euler.x) > PITCH_DEAD_ZONE) {
            pitch = (euler.x < 0.0) ? (euler.x + PITCH_DEAD_ZONE) : (euler.x - PITCH_DEAD_ZONE);
        }
        MyAvatar.headPitch = MyAvatar.headPitch + (pitch * PITCH_SCALING * deltaTime);
  
        //  TODO: Add some camera roll proportional to the rate of turn (so it feels like an airplane or roller coaster)

    }

    wasGrabbingWithRightHand = grabbingWithRightHand;
    wasGrabbingWithLeftHand = grabbingWithLeftHand;
}

var HEAD_MOVE_DEAD_ZONE = 0.0;
var HEAD_STRAFE_DEAD_ZONE = 0.0;
var HEAD_ROTATE_DEAD_ZONE = 0.0; 
var HEAD_THRUST_FWD_SCALE = 12000.0;
var HEAD_THRUST_STRAFE_SCALE = 1000.0;
var HEAD_YAW_RATE = 2.0;
var HEAD_PITCH_RATE = 1.0;
var HEAD_ROLL_THRUST_SCALE = 75.0;
var HEAD_PITCH_LIFT_THRUST = 3.0;

function moveWithHead(deltaTime) {
    if (movingWithHead) {
        var deltaYaw = MyAvatar.getHeadFinalYaw() - headStartYaw;
        var deltaPitch = MyAvatar.getHeadDeltaPitch() - headStartDeltaPitch;

        var bodyLocalCurrentHeadVector = Vec3.subtract(MyAvatar.getHeadPosition(), MyAvatar.position);
        bodyLocalCurrentHeadVector = Vec3.multiplyQbyV(Quat.angleAxis(-deltaYaw, {x:0, y: 1, z:0}), bodyLocalCurrentHeadVector);
        var headDelta = Vec3.subtract(bodyLocalCurrentHeadVector, headStartPosition);
        headDelta = Vec3.multiplyQbyV(Quat.inverse(Camera.getOrientation()), headDelta);
        headDelta.y = 0.0;   //  Don't respond to any of the vertical component of head motion

        //  Thrust based on leaning forward and side-to-side
        if (Math.abs(headDelta.z) > HEAD_MOVE_DEAD_ZONE) {
            MyAvatar.addThrust(Vec3.multiply(Quat.getFront(Camera.getOrientation()), -headDelta.z * HEAD_THRUST_FWD_SCALE * deltaTime));
        }
        if (Math.abs(headDelta.x) > HEAD_STRAFE_DEAD_ZONE) {
            MyAvatar.addThrust(Vec3.multiply(Quat.getRight(Camera.getOrientation()), headDelta.x * HEAD_THRUST_STRAFE_SCALE * deltaTime));
        }
        if (Math.abs(deltaYaw) > HEAD_ROTATE_DEAD_ZONE) {
            var orientation = Quat.multiply(Quat.angleAxis(deltaYaw * HEAD_YAW_RATE * deltaTime, {x:0, y: 1, z:0}), MyAvatar.orientation);
            MyAvatar.orientation = orientation;
        }
        //  Thrust Up/Down based on head pitch
        MyAvatar.addThrust(Vec3.multiply({ x:0, y:1, z:0 }, (MyAvatar.getHeadFinalPitch() - headStartFinalPitch) * HEAD_PITCH_LIFT_THRUST * deltaTime));
        //  For head trackers, adjust pitch by head pitch
        MyAvatar.headPitch += deltaPitch * HEAD_PITCH_RATE * deltaTime; 
        //  Thrust strafe based on roll ange 
        MyAvatar.addThrust(Vec3.multiply(Quat.getRight(Camera.getOrientation()), -(MyAvatar.getHeadFinalRoll() - headStartRoll) * HEAD_ROLL_THRUST_SCALE * deltaTime));
    }
}

// Update for joysticks and move button
function flyWithHydra(deltaTime) {
    var thrustJoystickPosition = Controller.getJoystickPosition(THRUST_CONTROLLER);
    
    if (thrustJoystickPosition.x != 0 || thrustJoystickPosition.y != 0) {
        if (thrustMultiplier < MAX_THRUST_MULTIPLIER) {
            thrustMultiplier *= 1 + (deltaTime * THRUST_INCREASE_RATE);
        }
        var headOrientation = MyAvatar.headOrientation;

        var front = Quat.getFront(headOrientation);
        var right = Quat.getRight(headOrientation);
        var up = Quat.getUp(headOrientation);
    
        var thrustFront = Vec3.multiply(front, MyAvatar.scale * THRUST_MAG_HAND_JETS * 
                                        thrustJoystickPosition.y * thrustMultiplier * deltaTime);
        MyAvatar.addThrust(thrustFront);
        var thrustRight = Vec3.multiply(right, MyAvatar.scale * THRUST_MAG_HAND_JETS * 
                                        thrustJoystickPosition.x * thrustMultiplier * deltaTime);
        MyAvatar.addThrust(thrustRight);
    } else {
        thrustMultiplier = INITIAL_THRUST_MULTIPLIER;
    }

    // View Controller
    var viewJoystickPosition = Controller.getJoystickPosition(VIEW_CONTROLLER);
    if (viewJoystickPosition.x != 0 || viewJoystickPosition.y != 0) {

        // change the body yaw based on our x controller
        var orientation = MyAvatar.orientation;
        var deltaOrientation = Quat.fromPitchYawRollDegrees(0, (-1 * viewJoystickPosition.x * JOYSTICK_YAW_MAG * deltaTime), 0);
        MyAvatar.orientation = Quat.multiply(orientation, deltaOrientation);

        // change the headPitch based on our x controller
        //pitch += viewJoystickPosition.y * JOYSTICK_PITCH_MAG * deltaTime;
        var newPitch = MyAvatar.headPitch + (viewJoystickPosition.y * JOYSTICK_PITCH_MAG * deltaTime);
        MyAvatar.headPitch = newPitch;
    }
    handleGrabBehavior(deltaTime);
    moveWithHead(deltaTime);
    displayDebug();

}
    
Script.update.connect(flyWithHydra);
Controller.captureJoystick(THRUST_CONTROLLER);
Controller.captureJoystick(VIEW_CONTROLLER);

// Map keyPress and mouse move events to our callbacks
function scriptEnding() {
    // re-enabled the standard application for touch events
    Controller.releaseJoystick(THRUST_CONTROLLER);
    Controller.releaseJoystick(VIEW_CONTROLLER);
}
Script.scriptEnding.connect(scriptEnding);

Controller.keyPressEvent.connect(function(event) {
    if (event.text == "SPACE" && !movingWithHead) {
        movingWithHead = true;
        headStartPosition = Vec3.subtract(MyAvatar.getHeadPosition(), MyAvatar.position);
        headStartDeltaPitch = MyAvatar.getHeadDeltaPitch();
        headStartFinalPitch = MyAvatar.getHeadFinalPitch();
        headStartRoll = MyAvatar.getHeadFinalRoll();
        headStartYaw = MyAvatar.getHeadFinalYaw(); 
    }                        
});
Controller.keyReleaseEvent.connect(function(event) {
    if (event.text == "SPACE") {
        movingWithHead = false;
    }                        
});

