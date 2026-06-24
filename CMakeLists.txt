project(Xonix)

find_package(SFML REQUIRED network audio graphics window system)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/images" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

set(SOURCES
    code/main.cpp
code/LoginSystem.cpp
code/FriendSystem.cpp
code/PlayerProfile.cpp
code/Leaderboard.cpp
code/PointSystem.cpp
code/MultiplayerGame.cpp
code/GameRoom.cpp
code/Inventory.cpp
code/AVLTree.cpp
code/SaveGame.cpp
code/Achievements.cpp
   )

#Create executable
add_executable(xonix ${SOURCES})

target_link_libraries(xonix PRIVATE sfml-system sfml-window sfml-graphics sfml-network sfml-audio)