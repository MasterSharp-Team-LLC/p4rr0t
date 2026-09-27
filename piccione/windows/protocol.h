#ifndef PICCIONE_PROTOCOL_H
#define PICCIONE_PROTOCOL_H

#include <stdint.h>

#define PICCIONE_MAGIC 0x50344D32  // "P4M2"
#define PICCIONE_DEFAULT_PORT 9876

// Packet Event Types
#define EVENT_TYPE_KEYBOARD     1
#define EVENT_TYPE_MOUSE_MOVE   2
#define EVENT_TYPE_MOUSE_BUTTON 3
#define EVENT_TYPE_MOUSE_WHEEL  4
#define EVENT_TYPE_HEARTBEAT    5

// Mouse Buttons
#define MOUSE_BTN_LEFT    1
#define MOUSE_BTN_RIGHT   2
#define MOUSE_BTN_MIDDLE  3
#define MOUSE_BTN_SIDE    4
#define MOUSE_BTN_EXTRA   5

#pragma pack(push, 1)
struct PiccionePacket {
    uint32_t magic;      // Magic verification bytes
    uint8_t  type;       // EVENT_TYPE_*
    uint8_t  state;      // 0 = Released/Up, 1 = Pressed/Down, 2 = Repeat
    uint16_t code;       // Keycode (Linux EVDEV KEY_* code) or Button ID
    int32_t  dx;         // Relative Mouse X movement or Horizontal Wheel
    int32_t  dy;         // Relative Mouse Y movement or Vertical Wheel
    uint32_t timestamp;  // Timestamp in ms
};
#pragma pack(pop)

#endif // PICCIONE_PROTOCOL_H