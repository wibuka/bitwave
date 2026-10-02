# Bitwave

A small C sound library for playing audio built on [miniaudio](https://github.com/mackron/miniaudio).

## Basic Example

```c
#include "bitwave.h"
#include <unistd.h>

int main(void) {
    if (!BW_init()) return 1;

    BW_play("music.mp3");
    sleep(5);

    BW_close();
    return 0;
}
```

## Installation

```bash
git clone git@github.com:wibuka/bitwave.git
cd bitwave
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Linking

```bash
gcc -I/path/to/bitwave app.c /path/to/bitwave/build/libbitwave.a -o app -lm -lpthread -ldl
```
