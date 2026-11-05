#include "state.hpp"
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;
using namespace cl;

class $modify(CLBaseLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        auto& s = st();
        if (s.mode == Mode::Play) return; // реальные клики блокируются при воспроизведении
        if (s.mode == Mode::Record && PlayLayer::get())
            s.actions.push_back({s.frame, down, button, isPlayer1});
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void processCommands(float dt) {
        auto& s = st();
        if (s.mode == Mode::Play) {
            while (s.idx < s.actions.size() && s.actions[s.idx].frame <= s.frame) {
                auto& a = s.actions[s.idx++];
                GJBaseGameLayer::handleButton(a.down, a.button, a.p1); // оригинал
            }
        }
        GJBaseGameLayer::processCommands(dt);
        ++s.frame;
    }
};

class $modify(CLReplayPlay, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        if (m_isPracticeMode) return; // макросы пишутся с начала уровня
        auto& s = st();
        s.frame = 0;
        s.idx = 0;
        if (s.mode == Mode::Record) s.actions.clear();
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        auto& s = st();
        if (s.mode == Mode::Record) {
            saveMacro();
            s.mode = Mode::Idle;
            Notification::create("Макрос сохранён", NotificationIcon::Success)->show();
        } else if (s.mode == Mode::Play) {
            s.mode = Mode::Idle;
        }
    }
};

class $modify(CLPause, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        auto win = CCDirector::get()->getWinSize();
        auto menu = CCMenu::create();
        menu->setID("cheatlib-menu");
        menu->setPosition({win.width * 0.1f, win.height * 0.5f});

        auto add = [&](char const* label, float y, std::function<void()> fn) {
            auto spr = ButtonSprite::create(label);
            spr->setScale(.6f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [fn](CCMenuItemSpriteExtra*) { fn(); });
            btn->setPosition({0.f, y});
            menu->addChild(btn);
        };

        add("Record", 40.f, [this] {
            auto& s = st();
            s.mode = Mode::Record;
            s.actions.clear();
            Notification::create("Запись...", NotificationIcon::Info)->show();
            this->onRestart(nullptr);
        });
        add("Play", 0.f, [this] {
            auto& s = st();
            if (s.actions.empty()) loadMacro();
            if (s.actions.empty()) {
                Notification::create("Нет макроса", NotificationIcon::Error)->show();
                return;
            }
            s.mode = Mode::Play;
            Notification::create("Воспроизведение", NotificationIcon::Info)->show();
            this->onRestart(nullptr);
        });
        add("Stop", -40.f, [] {
            auto& s = st();
            if (s.mode == Mode::Record) saveMacro();
            s.mode = Mode::Idle;
            Notification::create("Остановлено", NotificationIcon::Success)->show();
        });

        this->addChild(menu);
    }
};
