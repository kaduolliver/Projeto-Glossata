find_package(Qt6 REQUIRED COMPONENTS Core Sql Test)

add_executable(glossata_tests
    GameServiceTests.cpp
    ${CMAKE_SOURCE_DIR}/src/presentation/GameController.cpp
    ${CMAKE_SOURCE_DIR}/src/application/GameService.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/JsonSpeciesCatalog.cpp
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

target_link_libraries(glossata_tests PRIVATE Qt6::Core Qt6::Sql Qt6::Test)

add_test(NAME GlossataTests COMMAND glossata_tests)

add_executable(glossata_ui_tests
    QmlInterfaceTests.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/discord/DiscordRichPresence.cpp
    ${CMAKE_SOURCE_DIR}/src/application/GameService.cpp
    ${CMAKE_SOURCE_DIR}/src/infrastructure/content/JsonSpeciesCatalog.cpp
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
