#pragma once

#include <cstddef>
#include <cstdint>

namespace air_arcade {

constexpr int kScreenWidth = 240;
constexpr int kScreenHeight = 135;
constexpr int kMaxSprites = 8;
constexpr int kVariableCount = 16;
constexpr int kMaxProgramBytes = 4096;
constexpr int kFrameInstructionBudget = 256;

enum class Button : uint8_t {
    Up = 0, Down, Left, Right, A, B, Menu
};

class Host {
public:
    virtual ~Host() = default;
    virtual bool buttonDown(Button button) const = 0;
    virtual void clear(uint16_t rgb565) = 0;
    virtual void fillRect(int x, int y, int width, int height, uint16_t rgb565) = 0;
    virtual void drawText(int x, int y, const char* text, uint16_t rgb565) = 0;
};

enum class Op : uint8_t {
    End = 0,
    SetConst = 1,       // u8 variable, i16 value
    Add = 2,             // u8 destination, u8 source
    ReadButton = 3,      // u8 destination, u8 Button
    CreateSprite = 4,    // u8 id, i16 x, i16 y, u8 w, u8 h, u16 color
    SetVelocity = 5,     // u8 id, i16 vx, i16 vy
    Jump = 6,            // u16 absolute byte offset
    JumpIfZero = 7,      // u8 variable, u16 absolute byte offset
    Collide = 8,         // u8 sprite A, u8 sprite B, u16 target if overlapping
    SetBackground = 9,   // u16 rgb565
    SetScore = 10,       // u8 variable
    Draw = 11            // draw current sprites and score
};

struct Sprite {
    bool active = false;
    int16_t x = 0;
    int16_t y = 0;
    int16_t vx = 0;
    int16_t vy = 0;
    uint8_t width = 8;
    uint8_t height = 8;
    uint16_t color = 0xffff;
};

class Vm {
public:
    Vm();

    // Copies a compact AIR Arcade bytecode program. Returns false for invalid/oversized input.
    bool load(const uint8_t* bytes, size_t length);
    void reset();
    void frame(Host& host);

    int16_t variable(size_t index) const;
    int32_t score() const { return score_; }
    bool faulted() const { return faulted_; }

private:
    bool readU8(uint8_t& value);
    bool readU16(uint16_t& value);
    bool readI16(int16_t& value);
    bool validVariable(uint8_t index) const;
    bool validSprite(uint8_t index) const;
    bool overlaps(const Sprite& a, const Sprite& b) const;
    void draw(Host& host);

    uint8_t program_[kMaxProgramBytes];
    size_t programLength_;
    int16_t vars_[kVariableCount];
    Sprite sprites_[kMaxSprites];
    uint16_t background_;
    int32_t score_;
    bool loaded_;
    bool faulted_;
};

} // namespace air_arcade
