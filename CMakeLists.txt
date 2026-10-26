cmake_minimum_required(VERSION 3.16)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt" GLOSSATA_VERSION LIMIT_COUNT 1)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt")
if(NOT GLOSSATA_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "VERSION.txt must contain X.Y.Z")
endif()
project(ProjetoGlossata VERSION ${GLOSSATA_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Sql Network Multimedia)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_library(glossata_preferences STATIC
    src/presentation/AppPreferences.cpp
    src/presentation/AchievementAudio.cpp
    src/presentation/EffectOutput.cpp
    src/presentation/EffectOutput.h
)
target_include_directories(glossata_preferences PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(glossata_preferences PUBLIC Qt6::Core Qt6::Multimedia)

add_executable(glossata_quiz
    src/app/main.cpp
    src/infrastructure/discord/DiscordRichPresence.cpp
    src/application/GameService.cpp
    src/domain/SessionCheckpoint.cpp
    src/infrastructure/content/JsonSpeciesCatalog.cpp
    src/infrastructure/content/SqliteSpeciesCatalog.cpp
    src/application/TaxonomyIndex.cpp
    src/application/EducationalContent.cpp
    src/infrastructure/content/PhotoResolver.cpp
    src/infrastructure/content/JsonExplorationMediaCatalog.cpp
    src/infrastructure/persistence/SqliteGameHistoryRepository.cpp
    src/infrastructure/persistence/AchievementStorage.cpp
    src/application/Achievements.cpp
    src/infrastructure/persistence/PlayerIdentityMigration.cpp
    src/infrastructure/persistence/PlayerNameMigration.cpp
    src/infrastructure/persistence/SessionCheckpointStorage.cpp
    src/presentation/GameController.cpp
    resources.qrc
)

target_include_directories(glossata_quiz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/src/infrastructure
    ${CMAKE_CURRENT_SOURCE_DIR}/src/presentation
)

target_link_libraries(glossata_quiz PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Sql Qt6::Network glossata_preferences)
target_compile_definitions(glossata_quiz PRIVATE GLOSSATA_APP_VERSION="${PROJECT_VERSION}")
set_target_properties(glossata_quiz PROPERTIES OUTPUT_NAME projeto_glossata)
add_custom_command(TARGET glossata_quiz POST_BUILD
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/distribution.py"
        record-build --binary "$<TARGET_FILE:glossata_quiz>"
    VERBATIM
)

if(WIN32)
    enable_language(RC)
    set(GLOSSATA_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/branding/glossata.ico")
    configure_file(src/app/windows/app.rc.in ${CMAKE_CURRENT_BINARY_DIR}/app.rc @ONLY)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/app.rc PROPERTIES
        OBJECT_DEPENDS "${GLOSSATA_ICON_FILE}"
    )
    target_sources(glossata_quiz PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
    set_target_properties(glossata_quiz PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
configure_file(resources/data/discord.json ${CMAKE_CURRENT_BINARY_DIR}/discord.json COPYONLY)

enable_testing()
add_subdirectory(tests)
