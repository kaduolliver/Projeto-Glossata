cmake_minimum_required(VERSION 3.16)

project(GlossataQuiz VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Sql Network)

add_executable(glossata_quiz
    src/app/main.cpp
    src/infrastructure/discord/DiscordRichPresence.cpp
    src/application/GameService.cpp
    src/infrastructure/content/JsonSpeciesCatalog.cpp
    src/infrastructure/content/SqliteSpeciesCatalog.cpp
    src/application/TaxonomyIndex.cpp
    src/infrastructure/persistence/SqliteGameHistoryRepository.cpp
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

target_link_libraries(glossata_quiz PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::Sql Qt6::Network)

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
