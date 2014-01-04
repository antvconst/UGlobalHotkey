#include <QtCore>
#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>
#include <QApplication>
#endif
#include "hotkeymap.h"
#include "uglobalhotkeys.h"

UGlobalHotkeys::UGlobalHotkeys(QWidget *parent)
    : QWidget(parent)
{
    #if defined(Q_OS_LINUX)
    qApp->installNativeEventFilter(this);
    QWindow wndw;
    void* v = qApp->platformNativeInterface()->nativeResourceForWindow("connection", &wndw);
    X11Connection = (xcb_connection_t*)v;
    X11Wid = xcb_setup_roots_iterator(xcb_get_setup(X11Connection)).data->root;
    X11KeySymbs = xcb_key_symbols_alloc(X11Connection);
    #endif
}

void UGlobalHotkeys::registerHotkey(const QString& keySeq, size_t id) {
    registerHotkey(UKeySequence(keySeq), id);
}

void UGlobalHotkeys::registerHotkey(const UKeySequence& keySeq, size_t id) {
    if (keySeq.Size() == 0) {
        throw UException("Empty hotkeys");
    }
    if (Registered.find(id) != Registered.end()) {
        unregisterHotkey(id);
    }
    #if defined(Q_OS_WIN)
    size_t winMod = 0;
    size_t key = VK_F2;

    for (size_t i = 0; i != keySeq.Size(); i++) {
        if (keySeq[i] == Qt::Key_Control) {
            winMod |= MOD_CONTROL;
        } else if (keySeq[i] == Qt::Key_Alt) {
            winMod |= MOD_ALT;
        } else if (keySeq[i] == Qt::Key_Shift) {
            winMod |= MOD_SHIFT;
        } else if (keySeq[i] == Qt::Key_Meta) {
            winMod |= MOD_WIN;
        } else {
            key = QtKeyToWin(keySeq[i]);
        }
    }

    if (!RegisterHotKey((HWND)winId(), id, winMod, key)) {
        qDebug() << "Error activating hotkey!";
    } else {
        Registered.insert(id);
    }
    #elif defined(Q_OS_LINUX)
    regLinuxHotkey(keySeq, id);
    #endif
}

void UGlobalHotkeys::unregisterHotkey(size_t id) {
    Q_ASSERT(Registered.find(id) != Registered.end() && "Unregistered hotkey");
    #if defined(Q_OS_WIN)
    UnregisterHotKey((HWND)winId(), id);
    #elif defined(Q_OS_LINUX)
    unregLinuxHotkey(id);
    #endif
    Registered.remove(id);
}

void UGlobalHotkeys::unregisterAllHotkeys()
{
#ifdef Q_OS_WIN
    for (size_t id : Registered)
        this->unregisterHotkey(id);
#elif defined(Q_OS_LINUX)
    for (size_t id :Registered.keys())
        this->unregisterHotkey(id);
#endif
}

UGlobalHotkeys::~UGlobalHotkeys() {
    #if defined(Q_OS_WIN)
    for (QSet<size_t>::iterator i = Registered.begin(); i != Registered.end(); i++) {
        UnregisterHotKey((HWND)winId(), *i);
    }
    #elif defined(Q_OS_LINUX)
    xcb_key_symbols_free(X11KeySymbs);
    #endif
}

#if defined(Q_OS_WIN)
bool UGlobalHotkeys::winEvent(MSG * message, long * result) {
    Q_UNUSED(result);
    if (message->message == WM_HOTKEY) {
        size_t id = message->wParam;
        Q_ASSERT(Registered.find(id) != Registered.end() && "Unregistered hotkey");
        emit activated(id);
    }
    return false;
}

bool UGlobalHotkeys::nativeEvent(const QByteArray &eventType,
                                       void *message, long *result)
{
    Q_UNUSED(eventType);
    return winEvent((MSG*)message, result);
}

#elif defined(Q_OS_LINUX)

bool UGlobalHotkeys::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    return linuxEvent(static_cast<xcb_generic_event_t*>(message));
}

bool UGlobalHotkeys::linuxEvent(xcb_generic_event_t *message)
{
    if ( (message->response_type & ~0x80) == XCB_KEY_PRESS ) {
        xcb_key_press_event_t *ev = (xcb_key_press_event_t*)message;
        auto ind = Registered.key( {ev->detail, (ev->state & ~XCB_MOD_MASK_2)} );

        if (ind == 0) // this is not hotkeys
            return false;

        emit activated(ind);
        return true;
    }
    return false;
}

void UGlobalHotkeys::regLinuxHotkey(const UKeySequence &keySeq, size_t id)
{
    UHotkeyData data;
    UKeyData keyData = QtKeyToLinux(keySeq);

    xcb_keycode_t *keyC = xcb_key_symbols_get_keycode(X11KeySymbs, keyData.key);

    data.keyCode = *keyC;
    data.mods = keyData.mods;

    xcb_grab_key(X11Connection, 1, X11Wid, data.mods, data.keyCode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    // NumLk
    xcb_grab_key(X11Connection, 1, X11Wid, data.mods | XCB_MOD_MASK_2, data.keyCode,XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    Registered.insert(id, data);
}

void UGlobalHotkeys::unregLinuxHotkey(size_t id)
{
    UHotkeyData data = Registered.take(id);
    xcb_ungrab_key(X11Connection, data.keyCode, X11Wid, data.mods);
    xcb_ungrab_key(X11Connection, data.keyCode, X11Wid, data.mods | XCB_MOD_MASK_2);
}
#endif
