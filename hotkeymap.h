#include <QtCore>
#include "uglobal.h"

#if defined(Q_OS_WIN)
size_t QtKeyToWin(size_t key) {
    // TODO: other maping or full keys list

    if (key >= 0x01000030 && key <= 0x01000047) {
        return VK_F1 + (key - Qt::Key_F1);
    }

    return key;
}
#elif defined(Q_OS_LINUX)
#include "ukeysequence.h"
#include "xcb/xcb.h"
#include "xcb/xcb_keysyms.h"
#include "X11/keysym.h"

struct UKeyData {
    int key;
    int mods;
};

UKeyData QtKeyToLinux(const UKeySequence &keySeq)
{
    UKeyData data = {0, 0};

    auto key = keySeq.GetSimpleKeys();
    if (key.size() > 0)
        data.key = key[0];
    else
        throw UException("Invalid hotkey");

    // Key conversion
    // Qt's F keys need conversion
    if (data.key >= Qt::Key_F1 && data.key <= Qt::Key_F35) {
        const size_t DIFF = Qt::Key_F1 - XK_F1;
        data.key -= DIFF;
    } else if (data.key >= Qt::Key_Space && data.key <= Qt::Key_QuoteLeft) {
        // conversion is not necessary, if the value in the range Qt::Key_Space - Qt::Key_QuoteLeft
    } else {
        throw UException("Invalid hotkey: key conversion is not defined");
    }

    // Modifiers conversion
    auto mods = keySeq.GetModifiers();

    for (auto i : mods) {
        if (i == Qt::Key_Shift)
            data.mods |= XCB_MOD_MASK_SHIFT;
        else if (i == Qt::Key_Control)
            data.mods |= XCB_MOD_MASK_CONTROL;
        else if (i == Qt::Key_Alt)
            data.mods |= XCB_MOD_MASK_1;
        else if (i == Qt::Key_Meta)
            data.mods |= XCB_MOD_MASK_4; // !
    }

    return data;
}
#endif
