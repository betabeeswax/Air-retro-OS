#include "air_arcade_vm.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

using namespace air_arcade;

class TestHost final : public Host {
public:
    struct Rect { int x, y, w, h; uint16_t color; };
    bool keys[7] = {};
    uint16_t background = 0;
    std::vector<Rect> rects;
    std::string text;

    bool buttonDown(Button button) const override {
        return keys[static_cast<unsigned>(button)];
    }
    void clear(uint16_t color) override {
        background = color;
        rects.clear();
    }
    void fillRect(int x, int y, int w, int h, uint16_t color) override {
        rects.push_back({x, y, w, h, color});
    }
    void drawText(int, int, const char* value, uint16_t) override {
        text = value ? value : "";
    }
};

int main() {
    // SET_CONST v0=42; SET_SCORE v0; CREATE_SPRITE 0 at (10,12), 4x4 red;
    // SET_VELOCITY sprite0=(1,0); DRAW; END.
    const uint8_t program[] = {
        1, 0, 42, 0,
        10, 0,
        4, 0, 10, 0, 12, 0, 4, 4, 0x00, 0xf8,
        5, 0, 1, 0, 0, 0,
        11,
        0
    };

    Vm vm;
    assert(vm.load(program, sizeof(program)));
    TestHost host;
    vm.frame(host);

    assert(!vm.faulted());
    assert(vm.variable(0) == 42);
    assert(vm.score() == 42);
    assert(host.background == 0x0000);
    assert(host.rects.size() == 1);
    assert(host.rects[0].x == 11); // velocity is applied once per frame
    assert(host.rects[0].y == 12);
    assert(host.rects[0].color == 0xf800);
    assert(host.text == "SCORE 42");

    // Malformed instructions must stop safely instead of reading past the program.
    const uint8_t malformed[] = {1, 0};
    Vm broken;
    assert(broken.load(malformed, sizeof(malformed)));
    TestHost brokenHost;
    broken.frame(brokenHost);
    assert(broken.faulted());

    // Oversized or empty programs are rejected.
    assert(!broken.load(nullptr, 1));
    assert(!broken.load(program, kMaxProgramBytes + 1));
    return 0;
}
