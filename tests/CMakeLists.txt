find_package(Qt6 REQUIRED COMPONENTS Core Sql Test)

add_executable(glossata_tests
    GameServiceTests.cpp
    photo_fixtures.qrc
    ${CMAKE_SOURCE_DIR}/src/presentation/GameController.cpp
    ${CMAKE_SOURCE_DIR}/src/application/GameService.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/JsonSpeciesCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/SqliteSpeciesCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/application/TaxonomyIndex.cpp
    ${CMAKE_SOURCE_DIR}/src/application/EducationalContent.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/PhotoResolver.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/JsonExplorationMediaCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/persistence/SqliteGameHistoryRepository.cpp
    ${CMAKE_SOURCE_DIR}/resources.qrc
)

target_include_directories(glossata_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/application
    ${CMAKE_SOURCE_DIR}/src/domain
    ${CMAKE_SOURCE_DIR}/src/infrastructure
    ${CMAKE_SOURCE_DIR}/src/presentation
)

target_link_libraries(glossata_tests PRIVATE Qt6::Core Qt6::Gui Qt6::Sql Qt6::Test)

add_test(NAME GlossataTests COMMAND glossata_tests -o ${CMAKE_CURRENT_BINARY_DIR}/core-results.txt,txt)
set_tests_properties(GlossataTests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

add_executable(glossata_ui_tests
    QmlInterfaceTests.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/discord/DiscordRichPresence.cpp
    ${CMAKE_SOURCE_DIR}/src/application/GameService.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/JsonSpeciesCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/SqliteSpeciesCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/application/TaxonomyIndex.cpp
    ${CMAKE_SOURCE_DIR}/src/application/EducationalContent.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/PhotoResolver.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/JsonExplorationMediaCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/persistence/SqliteGameHistoryRepository.cpp
    ${CMAKE_SOURCE_DIR}/src/presentation/GameController.cpp
    ${CMAKE_SOURCE_DIR}/resources.qrc
)
target_include_directories(glossata_ui_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(glossata_ui_tests PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Sql Qt6::Test Qt6::Network)
add_test(NAME GlossataInterfaceTests COMMAND glossata_ui_tests -o ${CMAKE_CURRENT_BINARY_DIR}/ui-test-results.txt,txt)
set_tests_properties(GlossataInterfaceTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_QUICK_CONTROLS_STYLE=Basic"
)

add_executable(glossata_discord_tests
    DiscordRichPresenceTests.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/discord/DiscordRichPresence.cpp
)
target_include_directories(glossata_discord_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(glossata_discord_tests PRIVATE Qt6::Core Qt6::Network Qt6::Test)
add_test(NAME GlossataDiscordTests COMMAND glossata_discord_tests)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME GlossataImportTests COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_import_inaturalist.py)
add_test(NAME GlossataGovernanceTests COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_content_governance.py)
add_test(NAME GlossataReleaseTests COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_release.py)
add_test(NAME GlossataPhylogenyTests COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_phylogeny.py)
add_test(NAME GlossataExplorationMediaTests COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_exploration_media.py)
