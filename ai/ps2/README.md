# Tic Tac Toe

## Compilation
Invoke CMake with a generator of your preference to generate appropriate build files, then use the selected build system.

Example in-source release build with ninja as the generator
```
cmake -B ./ -GNinja -DCMAKE_BUILD_TYPE=Release
ninja
```

## Running
CMake will build two executables:
 - ttt_server - the game server that coordinates the game. 2 clients must connect to the server to start a game.
 - ttt_client - the game client.
