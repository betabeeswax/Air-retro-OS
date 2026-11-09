#include "air_arcade_vm.h"

#include <cstring>

namespace air_arcade {

Vm::Vm()
    : programLength_(0), pc_(0), background_(0x0000), score_(0), loaded_(false), faulted_(false) {
    std::memset(program_, 0, sizeof(program_));
    reset();
}

bool Vm::load(const uint8_t* bytes, size_t length) {
    if (!bytes || length == 0 || length > sizeof(program_)) return false;
    std::memcpy(program_, bytes, length);
    programLength_ = length;
    loaded_ = true;
    reset();
    return true;
}

void Vm::reset() {
    pc_ = 0;
    std::memset(vars_, 0, sizeof(vars_));
    for (size_t i = 0; i < kMaxSprites; ++i) sprites_[i] = Sprite{};
    background_ = 0x0000;
    score_ = 0;
    faulted_ = false;
}

bool Vm::readU8(uint8_t& value) {
    if (pc_ >= programLength_) return false;
    value = program_[pc_++];
    return true;
}

bool Vm::readU16(uint16_t& value) {
    uint8_t lo, hi;
    if (!readU8(lo) || !readU8(hi)) return false;
    value = static_cast<uint16_t>(lo | (static_cast<uint16_t>(hi) << 8));
    return true;
}

bool Vm::readI16(int16_t& value) {
    uint16_t raw;
    if (!readU16(raw)) return false;
    value = static_cast<int16_t>(raw);
    return true;
}

bool Vm::validVariable(uint8_t index) const { return index < kVariableCount; }
bool Vm::validSprite(uint8_t index) const { return index < kMaxSprites && sprites_[index].active; }

bool Vm::overlaps(const Sprite& a, const Sprite& b) const {
    return a.active && b.active &&
        a.x < b.x + b.width && a.x + a.width > b.x &&
        a.y < b.y + b.height && a.y + a.height > b.y;
}

void Vm::draw(Host& host) {
    host.clear(background_);
    for (size_t i = 0; i < kMaxSprites; ++i) {
        const Sprite& s = sprites_[i];
        if (!s.active) continue;
        host.fillRect(s.x, s.y, s.width, s.height, s.color);
    }
    char scoreText[24];
    // Avoid printf/heap use on the microcontroller.
    const char prefix[] = "SCORE ";
    size_t n = 0;
    for (size_t i = 0; i < sizeof(prefix) - 1; ++i) scoreText[n++] = prefix[i];
    int32_t value = score_ < 0 ? -score_ : score_;
    char digits[12];
    size_t count = 0;
    do { digits[count++] = static_cast<char>('0' + (value % 10)); value /= 10; } while (value && count < sizeof(digits));
    if (score_ < 0) scoreText[n++] = '-';
    while (count) scoreText[n++] = digits[--count];
    scoreText[n] = '\0';
    host.drawText(3, 3, scoreText, 0xffff);
}

void Vm::frame(Host& host) {
    if (!loaded_ || faulted_) return;
    pc_ = 0;
    unsigned executed = 0;
    bool drawRequested = false;

    while (pc_ < programLength_ && executed++ < kFrameInstructionBudget) {
        uint8_t rawOp;
        if (!readU8(rawOp)) { faulted_ = true; break; }
        const Op op = static_cast<Op>(rawOp);

        switch (op) {
        case Op::End:
            pc_ = programLength_;
            break;

        case Op::SetConst: {
            uint8_t dst; int16_t value;
            if (!readU8(dst) || !readI16(value) || !validVariable(dst)) { faulted_ = true; break; }
            vars_[dst] = value;
            break;
        }

        case Op::Add: {
            uint8_t dst, src;
            if (!readU8(dst) || !readU8(src) || !validVariable(dst) || !validVariable(src)) { faulted_ = true; break; }
            vars_[dst] = static_cast<int16_t>(vars_[dst] + vars_[src]);
            break;
        }

        case Op::ReadButton: {
            uint8_t dst, button;
            if (!readU8(dst) || !readU8(button) || !validVariable(dst) || button > static_cast<uint8_t>(Button::Menu)) { faulted_ = true; break; }
            vars_[dst] = host.buttonDown(static_cast<Button>(button)) ? 1 : 0;
            break;
        }

        case Op::CreateSprite: {
            uint8_t id, w, h; int16_t x, y; uint16_t color;
            if (!readU8(id) || !readI16(x) || !readI16(y) || !readU8(w) || !readU8(h) ||
                !readU16(color) || id >= kMaxSprites || w == 0 || h == 0 ||
                w > kScreenWidth || h > kScreenHeight) { faulted_ = true; break; }
            sprites_[id].active = true;
            sprites_[id].x = x;
            sprites_[id].y = y;
            sprites_[id].vx = 0;
            sprites_[id].vy = 0;
            sprites_[id].width = w;
            sprites_[id].height = h;
            sprites_[id].color = color;
            break;
        }

        case Op::SetVelocity: {
            uint8_t id; int16_t vx, vy;
            if (!readU8(id) || !readI16(vx) || !readI16(vy) || !validSprite(id)) { faulted_ = true; break; }
            sprites_[id].vx = vx;
            sprites_[id].vy = vy;
            break;
        }

        case Op::Jump: {
            uint16_t target;
            if (!readU16(target) || target >= programLength_) { faulted_ = true; break; }
            pc_ = target;
            break;
        }

        case Op::JumpIfZero: {
            uint8_t variable; uint16_t target;
            if (!readU8(variable) || !readU16(target) || !validVariable(variable) || target >= programLength_) { faulted_ = true; break; }
            if (vars_[variable] == 0) pc_ = target;
            break;
        }

        case Op::Collide: {
            uint8_t a, b; uint16_t target;
            if (!readU8(a) || !readU8(b) || !readU16(target) ||
                !validSprite(a) || !validSprite(b) || target >= programLength_) { faulted_ = true; break; }
            if (overlaps(sprites_[a], sprites_[b])) pc_ = target;
            break;
        }

        case Op::SetBackground: {
            uint16_t color;
            if (!readU16(color)) { faulted_ = true; break; }
            background_ = color;
            break;
        }

        case Op::SetScore: {
            uint8_t variable;
            if (!readU8(variable) || !validVariable(variable)) { faulted_ = true; break; }
            score_ = vars_[variable];
            break;
        }

        case Op::Draw:
            drawRequested = true;
            break;

        default:
            faulted_ = true;
            break;
        }

        if (faulted_) break;
    }

    if (executed > kFrameInstructionBudget) faulted_ = true;

    for (size_t i = 0; i < kMaxSprites; ++i) {
        Sprite& s = sprites_[i];
        if (!s.active) continue;
        s.x = static_cast<int16_t>(s.x + s.vx);
        s.y = static_cast<int16_t>(s.y + s.vy);
        if (s.x < 0) s.x = 0;
        if (s.y < 0) s.y = 0;
        if (s.x + s.width > kScreenWidth) s.x = static_cast<int16_t>(kScreenWidth - s.width);
        if (s.y + s.height > kScreenHeight) s.y = static_cast<int16_t>(kScreenHeight - s.height);
    }

    if (drawRequested || !faulted_) draw(host);
}

int16_t Vm::variable(size_t index) const {
    return index < kVariableCount ? vars_[index] : 0;
}

} // namespace air_arcade
