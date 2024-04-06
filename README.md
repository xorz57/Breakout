# Breakout

[![Build](https://github.com/xorz57/Breakout/actions/workflows/Build.yml/badge.svg)](https://github.com/xorz57/Breakout/actions/workflows/Build.yml)

![image](https://github.com/xorz57/Breakout/assets/84932056/3a826456-68e5-424f-a6d6-728bbd7447f4)

## Notice

Please note that this project is currently not actively maintained, and the code may not meet the author's standards for quality or best practices. While the project was a valuable learning experience, the author acknowledges that there may be room for improvement in the codebase.

## Future Plans

The author intends to revisit this project in the future to redesign and improve the code. However, there is currently no timeline for when this may happen.

## Input Controls

| Input Device | Controls           | Description                  |
| ------------ | ------------------ | ---------------------------- |
| Keyboard     | Left / Right Arrow | Move the paddle left / right |
| Controller   | Left / Right D-Pad | Move the paddle left / right |
| Keyboard     | ESC                | Pause / Resume the game      |
| Controller   | Start              | Pause / Resume the game      |

## Dependencies

- [entt](https://github.com/skypjack/entt/)
- [glm](https://github.com/g-truc/glm)
- [SDL2](https://github.com/libsdl-org/SDL)

## How to Build

#### Linux & macOS

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh

git clone https://github.com/xorz57/Breakout.git
cd Breakout
cmake -B build -DCMAKE_BUILD_TYPE=Release -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
ctest --build-config Release
```

#### Windows

```powershell
git clone https://github.com/microsoft/vcpkg.git C:/vcpkg
C:/vcpkg/bootstrap-vcpkg.bat
C:/vcpkg/vcpkg.exe integrate install

git clone https://github.com/xorz57/Breakout.git
cd Breakout
cmake -B build -DCMAKE_BUILD_TYPE=Release -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
ctest --build-config Release
```

## License

This project is licensed under the [MIT License](LICENSE).

## Acknowledgments

We would like to express our gratitude to the authors and maintainers of the libraries and tools that Breakout depends on. Their work is greatly appreciated.

## Contact

For questions, suggestions, or feedback, feel free to contact us:

- Email: [xorz57@gmail.com](mailto:xorz57@gmail.com)
- GitHub: [xorz57](https://github.com/xorz57)
