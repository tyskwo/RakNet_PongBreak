cmake_minimum_required(VERSION 2.6)

project(PongBreak)

include_directories(${CMAKE_SOURCE_DIR}/3rdParty/SFML/include)
include_directories(${CMAKE_SOURCE_DIR}/3rdParty/RakNet/source)

link_directories(${CMAKE_SOURCE_DIR}/3rdParty/SFML/lib)
link_directories(${CMAKE_SOURCE_DIR}/3rdParty/RakNet/lib)

add_executable(Client src/client/main.cpp src/client/client.h src/client/client.cpp src/client/PlayerInfo.h src/client/PlayerInfo.cpp)
add_executable(Server src/server/main.cpp src/server/server.h src/server/server.cpp src/server/game.h src/server/game.cpp src/server/GameInfo.h src/server/GameInfo.cpp)

add_custom_command(TARGET Client PRE_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy_directory
 ${CMAKE_SOURCE_DIR}/3rdParty/bin $<TARGET_FILE_DIR:Client>)

add_custom_command(TARGET Server PRE_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy_directory
 ${CMAKE_SOURCE_DIR}/3rdParty/bin $<TARGET_FILE_DIR:Server>)

set(DEP_LIBS
	sfml-system
	sfml-graphics
	sfml-window
	RakNetLibStatic
	Ws2_32.lib)

target_link_libraries(Client ${DEP_LIBS})
target_link_libraries(Server ${DEP_LIBS})