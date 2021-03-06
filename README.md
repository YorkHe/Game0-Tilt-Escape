# Game Information
(Note: fill in this portion with information about your game.)

Title: Metal Gear Solid: Rolling Balls

Author: Yu He (AndrewID: yuhe)

Design Document: [Tilt Escape](http://graphics.cs.cmu.edu/courses/15-466-f18/game0-designs/ishmaelj/)

Screen Shot:

![Screen Shot](screenshot.png)

How to Play:

1. Use `Arrow Key` to tilt the board and move the character
2. Press `Space Key` to hide into a paper box, thus you can prevent yourself from being seen and stop moving.
3. If being seen by enemy or secure camera, the game will be over. Player can press `Enter Key` to restart a new game.
4. After collecting all the green checkpoints in the maze, the final destination will appear.
5. When the character reaches the final destination, the level is cleared.

Difficulties Encountered:

1. Export blender object

    During the very beginning of this project, I spent over three hours in trying to correctly resize an object in blender. It came out to be only because I didn't apply the scaling operation.

2. Collision Detection

    I spent many time trying to figure out the correct method to detect the collision between the moving character and the maze wall. Under the guidance of TA, Mr. Osman, I decided to use index map to represent the maze, and the result is quite promising.

3. Physical simulation of object collision

    The physical simulation of a sliding marble ball is not very easy.

Good Code:

All the objects in the game are encapsulated as individual classes, and initialized using different data, thus the objects can be conveniently initialized in a data-driven way and updated separately.

```
    // Game.hpp
    BigBoss big_boss = BigBoss(-14.0f, 14.0f);
    Enemy enemy_array[3] = {
        Enemy(6.0f, 0.0f, Enemy::DIRECTION::DIRECTION_LEFT),
        Enemy(0.0f, -6.0f, Enemy::DIRECTION::DIRECTION_UP),
        Enemy(10.0f, 10.0f, Enemy::DIRECTION::DIRECTION_DOWN)
    };

    SecurityCamera security_camera_array[3] = {
            SecurityCamera(-6.0f, 12.5f, SecurityCamera::DIRECTION::DIRECTION_DOWN),
            SecurityCamera(-12.0f, -8.0f, SecurityCamera::DIRECTION::DIRECTION_UP),
            SecurityCamera(-1.0f, -12.0f, SecurityCamera::DIRECTION::DIRECTION_RIGHT)
    };

    // Game.cpp
    big_boss.update(elapsed, board);
    for(auto &enemy : enemy_array) {
        enemy.update(elapsed);
        if (enemy.intercept_with(big_boss) && !big_boss.is_box) {
            game_over = true;
            std::cerr << "GAME OVER!" << std::endl;
        }
    }

    for (auto &security_camera: security_camera_array) {
        security_camera.update(elapsed);
        if (security_camera.intercept_with(big_boss) && !big_boss.is_box) {
            game_over = true;
            std::cerr << "GAME OVER!" << std::endl;
        }
    }
```





# Using This Base Code

Before you dive into the code, it helps to understand the overall structure of this repository.
- Files you should read and/or edit:
    - ```main.cpp``` creates the game window and contains the main loop. You should read through this file to understand what it's doing, but you shouldn't need to change things (other than window title and size).
    - ```Game.*pp``` declaration+definition for the Game struct. These files will contain the bulk of your code changes.
    - ```meshes/export-meshes.py``` exports meshes from a .blend file into a format usable by our game runtime. You will need to edit this file to add vertex color export code.
    - ```Jamfile``` responsible for telling FTJam how to build the project. If you add any additional .cpp files or want to change the name of your runtime executable you will need to modify this.
    - ```.gitignore``` ignores the ```objs/``` directory and the generated executable file. You will need to change it if your executable name changes. (If you find yourself changing it to ignore, e.g., your editor's swap files you should probably, instead be investigating making this change in the global git configuration.)
- Files you probably should at least glance at because they are useful:
    - ```read_chunk.hpp``` contains a function that reads a vector of structures prefixed by a magic number. It's surprising how many simple file formats you can create that only require such a function to access.
    - ```data_path.*pp``` contains a helper function that allows you to specify paths relative to the executable (instead of the current working directory). Very useful when loading assets.
	- ```gl_errors.hpp``` contains a function that checks for opengl error conditions. Also, the helpful macro ```GL_ERRORS()``` which calls ```gl_errors()``` with the current file and line number.
- Files you probably don't need to read or edit:
    - ```GL.hpp``` includes OpenGL prototypes without the namespace pollution of (e.g.) SDL's OpenGL header. It makes use of ```glcorearb.h``` and ```gl_shims.*pp``` to make this happen.
    - ```make-gl-shims.py``` does what it says on the tin. Included in case you are curious. You won't need to run it.

## Asset Build Instructions

In order to generate the ```dist/meshes.blob``` file, tell blender to execute the ```meshes/export-meshes.py``` script:

```
blender --background --python meshes/export-meshes.py -- meshes/meshes.blend dist/meshes.blob
```

There is a Makefile in the ```meshes``` directory that will do this for you.

## Runtime Build Instructions

The runtime code has been set up to be built with [FT Jam](https://www.freetype.org/jam/).

### Getting Jam

For more information on Jam, see the [Jam Documentation](https://www.perforce.com/documentation/jam-documentation) page at Perforce, which includes both reference documentation and a getting started guide.

On unixish OSs, Jam is available from your package manager:
```
	brew install ftjam #on OSX
	apt get ftjam #on Debian-ish Linux
```

On Windows, you can get a binary [from sourceforge](https://sourceforge.net/projects/freetype/files/ftjam/2.5.2/ftjam-2.5.2-win32.zip/download),
and put it somewhere in your `%PATH%`.
(Possibly: also set the `JAM_TOOLSET` variable to `VISUALC`.)

### Libraries

This code uses the [libSDL](https://www.libsdl.org/) library to create an OpenGL context, and the [glm](https://glm.g-truc.net) library for OpenGL-friendly matrix/vector types.
On MacOS and Linux, the code should work out-of-the-box if if you have these installed through your package manager.

If you are compiling on Windows or don't want to install these libraries globally there are pre-built library packages available in the
[kit-libs-linux](https://github.com/ixchow/kit-libs-linux),
[kit-libs-osx](https://github.com/ixchow/kit-libs-osx),
and [kit-libs-win](https://github.com/ixchow/kit-libs-win) repositories.
Simply clone into a subfolder and the build should work.

### Building

Open a terminal (or ```x64 Native Tools Command Prompt for VS 2017``` on Windows), change to the directory containing this code, and type:

```
jam
```

That's it. You can use ```jam -jN``` to run ```N``` parallel jobs if you'd like; ```jam -q``` to instruct jam to quit after the first error; ```jam -dx``` to show commands being executed; or ```jam main.o``` to build a specific file (in this case, main.cpp).  ```jam -h``` will print help on additional options.
