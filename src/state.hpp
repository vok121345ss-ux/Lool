#pragma once
#include <Geode/Geode.hpp>
#include <fstream>
#include <vector>
#include <filesystem>

namespace cl {
    enum class Mode { Idle, Record, Play };
    struct Action { int frame; bool down; int button; bool p1; };
    struct State {
        Mode mode = Mode::Idle;
        int frame = 0;
        size_t idx = 0;
        std::vector<Action> actions;
    };

    inline State& st() { static State s; return s; }
    inline bool opt(char const* key) { return geode::Mod::get()->getSettingValue<bool>(key); }
    inline double speedVal() { return geode::Mod::get()->getSettingValue<double>("speed"); }

    inline bool cheating() {
        return opt("noclip")
            || (opt("speed-enabled") && speedVal() != 1.0)
            || st().mode == Mode::Play;
    }

    inline std::filesystem::path macroPath() {
        return geode::Mod::get()->getSaveDir() / "macro.txt";
    }
    inline void saveMacro() {
        std::ofstream f(macroPath());
        for (auto& a : st().actions)
            f << a.frame << ' ' << a.down << ' ' << a.button << ' ' << a.p1 << '\n';
    }
    inline void loadMacro() {
        st().actions.clear();
        std::ifstream f(macroPath());
        int fr, d, b, p;
        while (f >> fr >> d >> b >> p)
            st().actions.push_back({fr, d != 0, b, p != 0});
    }
}
