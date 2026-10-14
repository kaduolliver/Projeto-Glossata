# Glossata Quiz

Educational desktop quiz built with C++17, Qt 6 Quick/QML, a JSON species catalog, and SQLite for local score history.

## Current behavior

- A dark botanical interface provides a home screen, category selection, quiz, answer review, species information, results, statistics, achievements, and profile settings.
- The player selects one of four distinct scientific-name choices and confirms it. Correct/incorrect feedback stays visible until they continue to the next question.
- Games can use the full catalog or a family with at least four distinct species.
- A game starts at question 1 and continues until the player selects **Encerrar partida**. The catalog is shuffled again after all current species have appeared.
- Each question has a fact and one optional extra hint, limited to one reveal per species.
- The final score is saved once to the local SQLite ranking when the player ends a game.
- The ranking has one entry per player name (case insensitive using SQLite NOCASE). A new completed game replaces that name's previous score, even if lower. Existing duplicate entries are consolidated, retaining the latest game.
- The bundled JSON catalog contains 22 species with a fact and an extra hint for each. It can be extended without changing the quiz engine.
- Player profiles are stored locally in SQLite and can be created, renamed, selected, or deleted. The last profile cannot be deleted.
- Each game retains the player name captured at its start; profile changes apply to future games.
- Failed score saves are shown in the UI and can be retried without duplicates. A pending score blocks a new game and remains in memory until saved or the app closes.
- Statistics and six achievement challenges reflect the current game and reset for a new game; the local ranking persists.
- Licensed species photos are still pending. QML/Canvas artwork is explicitly labeled decorative and does not depict the species being asked about.
- The user-supplied temporary branding image is embedded in the Windows executable and window icon. `package/Glossata Quiz.lnk` starts the game with this icon.
- Optional Discord Rich Presence connects locally to the desktop client using the configured Application ID and asset key. See [Discord setup](docs/DISCORD_SETUP.md) to upload the image and test it.

## Build and test on Windows

Run from the project root with the MSYS2 UCRT64 CMake, Ninja, compiler, and Qt 6 packages available:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
$qmlPaths = @(Get-ChildItem resources/qml -Recurse -Filter '*.qml' | ForEach-Object { $_.FullName })
qmllint @qmlPaths --unqualified disable
```

The build exports `build/compile_commands.json` for VS Code C/C++ IntelliSense. Open this project directory as the workspace root.
CTest runs rules/persistence tests, an offscreen UI test, and Discord protocol tests against isolated local fake servers. UI screenshots are written to `build/tests/previews/`. Automated tests do not publish activity to a real Discord account.

## Remaining work

- Add appropriately licensed species images and their attribution metadata.
- Continue reviewing and expanding species facts against reliable taxonomic/natural-history references.
- Add cumulative profile statistics, persistent achievements, and richer ranking filters if desired.

The Windows runtime package and launcher are generated in `package/`.
After building, update it with `powershell -ExecutionPolicy Bypass -File scripts/package.ps1`. This deploys Qt/QML and the additional MSYS2 runtime dependencies while preserving the existing launcher and `qt.conf`.
