# Centi - TUI Text Editor

Centi is my personal experiment with writing a terminal-based text editor, loosely inspired by my personal nvim configuration. It's written in C++ 17 and currently only supports linux systems.

## Goals

- [ ] Multiple platforms
    - [x] Linux
    - [ ] Windows
    - [ ] SDL2
- [ ] Vim bindings:
    - [x] General movemement
    - [ ] Motions
    - [ ] Macros
- [ ] Theming
- [ ] Builtin terminal
- [ ] File browser
- [ ] Git integration
- [ ] LSP support

## Resources Used
- https://viewsourcecode.org/snaptoken/kilo/index.html
- https://github.com/charlesnicholson/nanoprintf
- headers under `include/sl` are from my c++ support library 'syslib', ripped from https://github.com/DeanoBurrito/northport
