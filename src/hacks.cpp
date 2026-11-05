#include "state.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/GJGameLevel.hpp>

using namespace geode::prelude;
using namespace cl;

// Noclip + instant respawn
class $modify(CLPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        if (opt("noclip") && obj != m_anticheatSpike) return;
        PlayLayer::destroyPlayer(player, obj);
        if (opt("instant-respawn") && !opt("noclip")) {
            this->stopActionByTag(0x10);
            this->delayedResetLevel();
        }
    }
};

// Speedhack
class $modify(CLScheduler, CCScheduler) {
    void update(float dt) {
        if (opt("speed-enabled")) dt *= static_cast<float>(speedVal());
        CCScheduler::update(dt);
    }
};

// Unlock all icons / colors
class $modify(CLGameManager, GameManager) {
    bool isIconUnlocked(int id, IconType type) {
        if (opt("unlock-all")) return true;
        return GameManager::isIconUnlocked(id, type);
    }
    bool isColorUnlocked(int id, UnlockType type) {
        if (opt("unlock-all")) return true;
        return GameManager::isColorUnlocked(id, type);
    }
};

// Safe mode: не сохраняем прогресс при читах
class $modify(CLLevel, GJGameLevel) {
    void savePercentage(int percent, bool practice, int clicks, int attempts, bool valid) {
        if (opt("safe-mode") && cheating()) return;
        GJGameLevel::savePercentage(percent, practice, clicks, attempts, valid);
    }
};
