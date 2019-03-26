# stackwm: a stacking window-manager.

_stackwm_ is a minimal window-manager based around a stack of windows, intended to help focus:

- only the top of the stack is visible.
- new windows go onto the top of the stack.
- you go back down the stack by closing windows.
- you can swap the top two windows, e.g. for checking documentation, but no further.

## howto

opening windows:

- `<S-c>`: create terminal.
- `<S-b>`: open browser.
- `<S-p>`: use [dmenu](https://tools.suckless.org/dmenu/) to open other applications.

managing windows:

- `<S-x>`: swap the top two windows.
- `<S-w>`: close current window.
- `<S-q>`: quit _stackwm_.

## inspiration

- https://dwm.suckless.org
- https://github.com/vardy/aphelia/
- https://github.com/mackstann/tinywm/

## other links

- [a really nice diagram of XEvents](https://jichu4n.com/content/images/2018/10/sB-bXhvvzFJe2u65_YRwARA.png).
