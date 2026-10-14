# Glossata Quiz

Educational desktop quiz built with C++17, Qt 6 Quick/QML, and SQLite. Explore the traditional five educational kingdoms: Animalia, Plantae, Fungi, Protista, and Monera.

## Current behavior

- A dark botanical interface provides a home screen, kingdom selection, a taxonomic explorer with clickable breadcrumbs, quiz, answer review, species information, results, statistics, achievements, and profile settings.
- The player selects one of four distinct choices at the same taxonomic rank and confirms it. Correct/incorrect feedback stays visible until they continue to the next question.
- Games can use the mixed catalog or descendants of a selected group. Identification levels are offered only when at least four distinct valid answers exist.
- A game starts at question 1 and continues until **Voltar → Encerrar partida**. Canceling the confirmation preserves the question, selection and hint. Confirming saves once and returns to the previous explorer.
- Questions use a neutral observation prompt and an optional sourced biological hint. Unreviewed facts and generic import descriptions are kept as historical data and are not used as clues.
- The final score is saved once to the local SQLite ranking when the player ends a game.
- The ranking has one entry per player name (case insensitive using SQLite NOCASE). A new completed game replaces that name's previous score, even if lower. Existing duplicate entries are consolidated, retaining the latest game.
- The offline catalog contains 122 active species: 42 Animalia and 20 each in Plantae, Fungi, Protista and Monera. Scientific taxonomy and source identifiers remain separate from the educational grouping. Former organisms explicitly replaced by the user are archived with their original IDs.
- Player profiles are stored locally in SQLite and can be created, renamed, selected, or deleted. The last profile cannot be deleted.
- Each game retains the player name captured at its start; profile changes apply to future games.
- Failed score saves are shown in the UI and can be retried without duplicates. A pending score blocks a new game and remains in memory until saved or the app closes.
- Statistics and six achievement challenges reflect the current game and reset for a new game; the local ranking persists.
- All 122 active organisms have a validated local image; there are 120 distinct image files. The resolver checks source identity, credits, license, SHA-256, resource existence, decoding and minimum resolution. Every question and its review use the same photo snapshot.
- The visual catalog has 102 organisms approved for at least one rank. Monera retains its 20 organisms and uses clearly labeled supporting photographs and classification clues; these images do not establish species identity. Mixed genus play currently includes 101 records across the five educational kingdoms.
- The explorer loads 16 sourced taxon introductions, rounded selectors and styled breadcrumbs. Taxonomic performance groups real answers by educational kingdom and source phylum/division ID.
- Species information provides 122 Portuguese profiles with an introduction, distribution or documented occurrence, morphology, conservation, plain-language curiosity and hint, plus feeding/nutrition where documented. Scientific classification and dated references expand within the same dark interface. See [species profiles](docs/FICHAS_ESPECIES.md).
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

## Strategic hints and exact points

The existing Hint button offers taxonomic, etymological and morphological assistance. Correct answers award 6, 3 or 2 integer units (one point = 6); combined hints use the lowest cap. Opening the panel costs nothing. Correct-answer counts, accuracy and achievements remain separate from points. Existing history is migrated exactly once, and new per-answer records are saved atomically with the latest result per player. [Technical report, scientific coverage and limitations](docs/DICAS_E_PONTUACAO.md).

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

See the [current replacements and Monera policy](docs/SUBSTITUICOES_CATALOGO.md), [refinement history](docs/REFINAMENTO_IDENTIFICACAO.md), [expansion history](docs/CINCO_REINOS.md), [photo credits](docs/PHOTO_CREDITS.md), and [project context](PROJECT_CONTEXT.md).

## Remaining work

- Continue reviewing and expanding species facts against reliable taxonomic/natural-history references.
- Broaden representation beyond the initial Protozoa and Bacteria samples within Protista and Monera.
- Add cumulative profile statistics, persistent achievements, and richer ranking filters if desired.

The Windows runtime package and launcher are generated in `package/`.
After building, update it with `powershell -ExecutionPolicy Bypass -File scripts/package.ps1`. This deploys Qt/QML and the additional MSYS2 runtime dependencies while preserving the existing launcher and `qt.conf`.

Compiled executables, Qt DLLs, import caches, generated screenshots, and personal SQLite databases are excluded from Git. A fresh clone must be built and packaged locally.
