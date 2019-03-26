#include <X11/Xlib.h>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>

// config.

constexpr const char* terminal = "stterm &";
constexpr const char* browser = "firefox &";

void quit();
void close_window();
void start_terminal();
void start_browser();
void start_dmenu();
void swap_stack_head();

constexpr auto mod_key = Mod4Mask;

static std::map<std::string, void (*)()> keys{
    {"q", quit},           {"w", close_window},  {"x", swap_stack_head},
    {"c", start_terminal}, {"b", start_browser}, {"p", start_dmenu},
};

// logging.

void log_info(const std::string& msg) { std::cerr << msg << "." << std::endl; }

[[noreturn]] void log_fatal(const std::string& msg) {
  std::cerr << "died: " << msg << "." << std::endl;
  exit(1);
}

using StreamFuncP = std::unique_ptr<std::ostringstream,
                                    std::function<void(std::ostringstream*)>>;
StreamFuncP log_stream(void (*level)(const std::string&)) {
  return StreamFuncP(new std::ostringstream,
                     [=](std::ostringstream* stream) { level(stream->str()); });
}
#define LOG(level) *(log_stream(log_##level))

// code.

static Display* display;
static Window root_window;

static int screen_width;
static int screen_height;

static std::list<Window> stack;
void push(Window w) { stack.push_front(w); }
void pop(Window w) { stack.remove(w); }

void refocus() {
  if (stack.empty()) {
    return;
  }
  Window w = stack.front();
  XRaiseWindow(display, w);
  XMoveResizeWindow(display, w, 0, 0, screen_width, screen_height);
  XSetInputFocus(display, w, RevertToPointerRoot, CurrentTime);
}

// KeyPress handlers.

unsigned int keycode(const std::string& name) {
  return XKeysymToKeycode(display, XStringToKeysym(name.c_str()));
}

void quit() { XCloseDisplay(display); }
void start_terminal() { system(terminal); }
void start_browser() { system(browser); }
void start_dmenu() { system("dmenu_run"); }
void close_window() {
  if (stack.empty()) {
    return;
  }
  XKillClient(display, stack.front());
}
void swap_stack_head() {
  if (stack.size() < 2) {
    return;
  }
  Window a = stack.front();
  stack.pop_front();
  Window b = stack.front();
  stack.pop_front();
  stack.push_front(a);
  stack.push_front(b);
  refocus();
}

// XEvent handlers.

void on_keypress(const XKeyEvent& e) {
  for (const auto& key_and_func : keys) {
    if (e.keycode == keycode(key_and_func.first)) {
      key_and_func.second();
    }
  }
}

void on_configurerequest(const XConfigureRequestEvent& e) {
  XWindowChanges changes;
  changes.x = e.x;
  changes.y = e.y;
  changes.width = e.width;
  changes.height = e.height;
  changes.border_width = e.border_width;
  changes.sibling = e.above;
  changes.stack_mode = e.detail;
  XConfigureWindow(display, e.window, e.value_mask, &changes);
}

void on_maprequest(const XMapRequestEvent& e) {
  XMapWindow(display, e.window);
  push(e.window);
  refocus();
}

void on_unmapnotify(const XUnmapEvent& e) {
  pop(e.window);
  refocus();
}
void on_destroynotify(const XDestroyWindowEvent& e) {
  pop(e.window);
  refocus();
}

void init_keys();
void on_mappingnotify(XMappingEvent& e) {
  XRefreshKeyboardMapping(&e);
  if (e.request == MappingKeyboard) {
    init_keys();
  }
}

// init.

int x_error_handler(Display* d, XErrorEvent* e) {
  char buf[100];
  XGetErrorText(d, e->error_code, buf, sizeof(buf));
  LOG(fatal) << "XError: " << buf;
  return 0;
}

void init_keys() {
  LOG(info) << "grabbing keys";
  XUngrabKey(display, AnyKey, AnyModifier, root_window);
  for (const auto& key_and_func : keys) {
    XGrabKey(display, keycode(key_and_func.first), mod_key,
             DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
  }
  LOG(info) << "grabbed keys";
}

void init_x() {
  display = XOpenDisplay(NULL);
  if (!display) {
    LOG(fatal) << "failed to open the display";
  }
  root_window = DefaultRootWindow(display);
  screen_width = DisplayWidth(display, DefaultScreen(display));
  screen_height = DisplayHeight(display, DefaultScreen(display));

  XSelectInput(display, root_window,
               SubstructureNotifyMask | SubstructureRedirectMask);

  XSetErrorHandler(x_error_handler);

  init_keys();

  XSync(display, False);

  LOG(info) << "setup complete";
}

// main event loop.

int main() {
  init_x();

  XEvent e;
  for (;;) {
    XSync(display, False);
    XNextEvent(display, &e);

#define HANDLE(event, code)             \
  case event:                           \
    LOG(info) << "starting " << #event; \
    code;                               \
    LOG(info) << "finished " << #event; \
    break

    switch (e.type) {
      HANDLE(KeyPress, on_keypress(e.xkey));
      HANDLE(ConfigureRequest, on_configurerequest(e.xconfigurerequest));
      HANDLE(MapRequest, on_maprequest(e.xmaprequest));
      HANDLE(UnmapNotify, on_unmapnotify(e.xunmap));
      HANDLE(DestroyNotify, on_destroynotify(e.xdestroywindow));
      HANDLE(MappingNotify, on_mappingnotify(e.xmapping));
    }
#undef HANDLE
  }

  return 0;
}
