# TrackViz

A simple UE4 plugin to visualize AirSim track records. Just clone this repo into `Plugins\TrackViz` directory in your UE4 project and load the level you want to visualize tracks in.

![default](https://user-images.githubusercontent.com/32840403/50395293-00a62500-0774-11e9-88ce-752a40d1b43d.png)

Use `TrackVizGameMode` as a default game mode to use this plugin. Track records are searched by default in `FPaths::ProjectDir() + TEXT("tracks")`. `FPaths::ProjectDir()` is a root directory in the case of UE4 project and `<project name>` directory in the case of packaged project (right next to `Binaries`, `Content` and `Saved` directories). This directory is searched for `.txt` files with CSV layout. `POS_X`, `POS_Y` and `POS_Z` fields are used as data for track visualization. `PlayerStart` actor is used as a starting point for all tracks. Track colors can be specified right in the name of the `.txt` file (make sure they are either in the beginning or the end of the file name, not counting the extension). Available colors can be found in [UE4 documentation on the `FColor` class](https://api.unrealengine.com/INT/API/Runtime/Core/Math/FColor/index.html) in the "Constants" section. If colors are not specified, they will be assigned automatically. The legend is displayed in the app.

Track directory path and start tracks position is customizable by modifying `TrackVizGameMode` class or by subclassing it with a Blueprint game mode and changing `startPosition`, `tracksDir` and `isRelativePath` properties in the UE4 editor.
