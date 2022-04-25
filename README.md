# Wordle

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/d91244e94a37417fbd1649ae5f2def6f)](https://www.codacy.com/gh/ngagesdk/wordle/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=ngagesdk/wordle&amp;utm_campaign=Badge_Grade)
[![CMake](https://github.com/ngagesdk/wordle/actions/workflows/cmake.yml/badge.svg)](https://github.com/ngagesdk/wordle/actions/workflows/cmake.yml)

A portable clone of Wordle designed for the Nokia N-Gage.

[![Wordle](https://raw.githubusercontent.com/ngagesdk/wordle/master/media/promo-tn.jpg)](https://raw.githubusercontent.com/ngagesdk/wordle/master/media/promo.jpg?raw=true "Wordle")

## Features

- Multiple language settings such as English, Russian (СЛОВО), German
  (WÖRDL) and Finnish (SANIS).

- Play the NYT's Daily Word!

- Enjoy a stress-free game with as many tries as you want in endless
  mode.

- Tailored for the N-Gage, but highly portable as the entire game is
  written in C89 and only depends on [SDL
  2.0.x](https://github.com/libsdl-org/SDL).

- A web version of the game can be found on my website: [N-Gage
  Wordle](https://mupf.dev/wordle/).

## Compiling

First clone the repository:
```bash
git clone https://github.com/ngagesdk/wordle.git
```

### N-Gage

The easiest way to get Wordle up and running is Visual Studio 2022 with
[C++ CMake tools for
Windows](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio)
installed.  If you have installed and set up the [N-Gage
SDK.](https://github.com/ngagesdk/ngage-toolchain) in advance, simply
open the cloned repository via `File -> Open -> Folder`.  Everything
else is set up automatically.

### Other Platforms

Wordle can also be compiled for other platforms such as Linux, Windows
or any other platform [supported by
SDL2](https://wiki.libsdl.org/Installation#supported_platforms).  For
Windows, I recommend using [MSYS2](https://www.msys2.org/).

Wordle can be compiled with the included CMake configuration. E.g:
```bash
mkdir build
cd build
cp ../res/data.pfs .
cmake -DBUILD_ON_ALT_PLATFORM=ON ..
make
./wordle.exe
```

## Licence and Credits

- Special thanks to Josh Wardle for developing this brilliant game in
  the first place.

- Packed file loader by [Daniel
  Monteiro](https://montyontherun.itch.io/).

- stb by Sean Barrett is licensed under "The MIT License".  See the file
  [LICENSE](https://github.com/nothings/stb/blob/master/LICENSE) for
  details.

- The application menu icon has been designed by geekahedron from the
  [Wordleverse](https://discord.com/invite/FdQKzenz) Discord community.

- The finnish word list is based on the "nykysuomen sanalista" by
  [Kotus](https://kaino.kotus.fi/sanat/nykysuomi/), licensed under a [CC
  BY 3.0](https://creativecommons.org/licenses/by/3.0/deed.fi) license.

- [ASCII Bitmap Font
  "cellphone"](https://opengameart.org/content/ascii-bitmap-font-cellphone)
  by domsson.

- This project's source code is, unless stated otherwise, licensed under
  the "The MIT License".  See the file [LICENSE.md](LICENSE.md) for
  details.
