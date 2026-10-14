# Glossata Quiz

Educational desktop quiz built with C++17, Qt 6 Quick/QML, and SQLite. Explore the traditional five educational kingdoms: Animalia, Plantae, Fungi, Protista, and Monera.

## Current behavior

- A dark botanical interface provides a home screen, kingdom selection, a taxonomic explorer with clickable breadcrumbs, quiz, answer review, species information, results, statistics, achievements, and profile settings.
- The player selects one of four distinct choices at the same taxonomic rank and confirms it. Correct/incorrect feedback stays visible until they continue to the next question.
- Games can use the mixed catalog or descendants of a selected group. Identification levels are offered only when at least four distinct valid answers exist.
- A game starts at question 1 and continues until the player selects **Encerrar partida**. The catalog is shuffled again after all current species have appeared.
- Each question has a fact and one optional extra hint, limited to one reveal per species.
- The final score is saved once to the local SQLite ranking when the player ends a game.
- The ranking has one entry per player name (case insensitive using SQLite NOCASE). A new completed game replaces that name's previous score, even if lower. Existing duplicate entries are consolidated, retaining the latest game.
- The offline catalog contains 122 species: the original 22 Lepidoptera plus 20 new species per educational kingdom. Scientific taxonomy and source identifiers remain separate from the educational grouping.
- Player profiles are stored locally in SQLite and can be created, renamed, selected, or deleted. The last profile cannot be deleted.
- Each game retains the player name captured at its start; profile changes apply to future games.
- Failed score saves are shown in the UI and can be retried without duplicates. A pending score blocks a new game and remains in memory until saved or the app closes.
- Statistics and six achievement challenges reflect the current game and reset for a new game; the local ranking persists.
- The 100 imported photographs are individually licensed CC BY or CC0, with author, source, and license links. Only reviewed photographs enable visual questions; Monera uses textual taxonomic clues. QML/Canvas fallback artwork is explicitly labeled decorative.
- The user-supplied temporary branding image is embedded in the Windows executable and window icon. `package/Glossata Quiz.lnk` starts the game with this icon.
- Optional Discord Rich Presence connects locally to the desktop client using the configured Application ID and asset key. See [Discord setup](docs/DISCORD_SETUP.md) to upload the image and test it.

## Build and test on Windows

Run from the project root with the MSYS2 UCRT64 CMake, Ninja, compiler, Qt 6 Core/Gui/Quick/Sql/Network/Test packages, and Python 3.11+ available:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
$qmlPaths = @(Get-ChildItem resources/qml -Recurse -Filter '*.qml' | ForEach-Object { $_.FullName })
qmllint @qmlPaths --unqualified disable
```

The build exports `build/compile_commands.json` for VS Code C/C++ IntelliSense. Open this project directory as the workspace root.
CTest runs four suites: rules/persistence, an offscreen UI test, Discord protocol tests against isolated local fake servers, and offline import/provenance tests. UI screenshots are written to `build/tests/previews/`. Automated tests do not publish activity to a real Discord account.

## Catalog and documentation

| Educational kingdom | Species |
|---|---:|
| Animalia | 42 |
| Plantae | 20 |
| Fungi | 20 |
| Protista | 20 |
| Monera | 20 |

The importer uses Python's standard library and caches iNaturalist API responses. Reimporting with the same target is idempotent:

```powershell
python scripts/import_inaturalist.py --per-kingdom 20 --max-pages 5
python scripts/import_inaturalist.py --offline --per-kingdom 20
```

The target is the total number of imported species per kingdom. Increase it to request additional records. Playing and browsing the catalog do not require internet access; completing a new import offline requires previously cached responses and photos.

See [implementation and limitations](docs/CINCO_REINOS.md), [photo credits](docs/PHOTO_CREDITS.md), and [project context](PROJECT_CONTEXT.md).

## Remaining work

- Continue reviewing and expanding species facts against reliable taxonomic/natural-history references.
- Broaden representation beyond the initial Protozoa and Bacteria samples within Protista and Monera.
- Add cumulative profile statistics, persistent achievements, and richer ranking filters if desired.

The Windows runtime package and launcher are generated in `package/`.
After building, update it with `powershell -ExecutionPolicy Bypass -File scripts/package.ps1`. This deploys Qt/QML and the additional MSYS2 runtime dependencies while preserving the existing launcher and `qt.conf`.

Compiled executables, Qt DLLs, import caches, generated screenshots, and personal SQLite databases are excluded from Git. A fresh clone must be built and packaged locally.
