#include "KeyHandler.h"

bool KeyHandler::keyPressed(const OgreBites::KeyboardEvent& event) {
    if (event.keysym.sym == OgreBites::SDLK_ESCAPE) Ogre::Root::getSingleton().queueEndRendering();

    return true;
}