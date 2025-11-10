#pragma once

#include "Ogre.h"
#include "OgreApplicationContext.h"

class KeyHandler : public OgreBites::InputListener { bool keyPressed(const OgreBites::KeyboardEvent& event) override; };