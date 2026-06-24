# Xonix

A C++ recreation of the classic Xonix arcade game, rebuilt with SFML and extended into a full multiplayer platform — player accounts, real-time multiplayer, leaderboards, achievements, and an unlockable theme system backed by a self-balancing AVL tree.

## Features

- **Classic Xonix gameplay** — claim territory on the grid while evading enemies
- **Multiplayer mode** — real-time competitive play over the network (SFML Network)
- **Account system** — registration and login with password strength validation
- **Friend system** — add and manage friends
- **Leaderboard** — global score ranking across players
- **Player profiles** — match history, including wins/losses and points earned
- **Achievements** — unlockable milestones (score thresholds, power-up usage, etc.)
- **Theme system** — unlockable cosmetic themes, stored and retrieved via a custom AVL tree implementation
- **Save / load** — persist and resume game state mid-session
- **Matchmaking** — room-based queue system using leaderboard rank

## Tech Stack

- C++
- SFML (graphics, window, system, network, audio)
- CMake

## Project Structure

```
GROUP11_PROJECT/
├── code/             # Game logic and systems (login, multiplayer, leaderboard, achievements, AVL tree, etc.)
├── images/           # Sprites, tiles, and fonts
├── Report/           # Project report
└── CMakeLists.txt
```

## Getting Started

### Prerequisites
- CMake (3.x or later)
- A C++ compiler (MSVC, MinGW, or g++)
- [SFML](https://www.sfml-dev.org/) installed and discoverable by CMake

### Build
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Run
```bash
./xonix
```
Run it from inside the `build` folder so it can find the `images/` assets copied there during the build.

## Authors

- Abdul Raffay (24i-2008)
- Muhammad Hamza Adil (24i-2130)

Built as a semester project at FAST University, Islamabad.
