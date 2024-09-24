# Sputnik Engine
An unfinished experimental 2D game engine written from scratch in C++ using SDL2.

At the time I was coding this engine I only used a quite popular but limited 2D game engine so I wanted to make my own
with features that I would use in a game, such as camera zooming in/out, camera rotating, audio filters
(for example, an underwater filter or an echo filter for a cave). It's fun making your own engine, I suggest other people
to try that themselves.

The reason this engine was never finished is because I found the engine that fits all my needs: [Godot Engine](https://godotengine.org/),
an open-source MIT-licensed game engine that supports both 2D and 3D games, camera zooming, camera rotating, audio filters,
advanced effects for 2D and much more, and then I lost motivation for completing this project, but at least the very basics of a 2D engine
is done and it has some nice features: camera zooming is done, camera rotating is also supported, and there's an underwater audio filter
thanks to [SoLoud audio library](https://solhsa.com/soloud/).

It was a nice experience making my own game engine, and now I know more about how game engines operate,
and I'm grateful to the libraries that I used for providing such possibilities for anyone to make a game or a whole game engine
using them.

This engine uses the following libraries: [SDL2](https://www.libsdl.org/), [SDL2_gpu](https://github.com/grimfang4/sdl-gpu) and [SoLoud audio library](https://solhsa.com/soloud/).

I don't own the assets (except for code that I wrote) that is used in the example game included in the repository.
The sprites and Sonic the Hedgehog characters are copyrighted by Sega and Sonic Team.
Some music is also copyrighted by Sega, but some other music was made by [Karl Brueggemann](https://www.youtube.com/@KGB525) and is available on his YouTube channel
(the music wasn't made exclusively for this engine or this example game, I used music that is publically available on his YouTube channel).
