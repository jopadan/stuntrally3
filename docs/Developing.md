
# Developing Guide

Guide for developing Stunt Rally 3, or other [Ogre-Next](https://github.com/OGRECave/ogre-next) applications.  
Listing useful tools, many links, and general info for SR3 [sources](https://github.com/stuntrally/stuntrally3/tree/main/src) (C++ code).  

This guide can be useful if you want to:
- **develop** SR3, **test** or **fix** a bug
- **learn** more about SR3 code or find something in it
- learn about **Ogre-Next** used in SR3 or other libraries etc
- fork SR3 code and start your own project (license required: [GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html) or newer) based on it

## ToDo

- Many _ToDo:_ tasks are added BTW of topics in this guide file. It may not reflect very latest changes.  
- **Fast** updated list with more items, but *shorter* info, is on [Roadmap](Roadmap.md), also used for planning what to do next.  
- More in **slow** [Tasks list](Tasks.md) with general info (sorted priority).  
- Lastly there are plenty of `todo:` or `fixme` places marked in sources.

### Rendering support

Main supported *Render System* is Open**GL**3+.  
Vulkan seems working, but is not tested often.  
DirectX11 doesn't work, is not tested. It is (at least) missing .hlsl files, for those shaders only in .glsl. Otherwise it could work.

----
# Developing Tools

## IDE

The recommended all-in-one tool (editor for developing sources),  
having (essential) features like: Go to definition, Find references, Debugging, Git integration, etc.

### [Qt Creator](https://www.qt.io/download)
Easier to set up first, CMake is integrated.  
Works fast, but has less options.

### [VSCodium](https://github.com/VSCodium/vscodium/releases)
Used by CryHam, needs a moment to set up first, quick info below.  
And in [my C++ guide](https://cryham.org/cpp-guide/#VSCodium), especially if you're new to C++.  
Search in all files works super fast. Has lots of extensions available.  

Extensions needed at least: 
- clangd by llvm-vs-code-extensions
- CodeLLDB by vadimcn
- CMake by twxs
- CMake Tools by ms-vscode
- Native Debug by webfreak

#### Settings

SR3 has VSCodium config files in subdir `.vscode/` in git repo.  
Some extensions and IDE preferences in `settings.json` and  
in `launch.json` configurations (for binaries) are set up, to run or debug.

### Windows

On Windows developing is **much more** difficult, longer to set up dependencies.  
Thus not recommended. IDE used is Visual Studio.

### Next

Once set up, when starting, follow more from [Building](Building.md).


----
## 🛠️ Debugging Tools

These tools are used last, and are meant for **advanced** developers, surely not needed for beginners.

Advanced, extra, external tools, should be used when testing or developing new (bigger) things in game or editor:
- [**Renderdoc**](https://renderdoc.org/) - For debugging **GPU render**, tracking rendered frame, its draw calls, shaders, textures, effects, workspaces etc.  
[documentation](https://renderdoc.org/docs/index.html), [forum post](https://forums.ogre3d.org/viewtopic.php?p=554959#p554959) with quick help on using it, and [video tutorials](https://www.youtube.com/results?search_query=tutorial+%22renderdoc%22).
- [Valgrind](https://valgrind.org/) - For debugging CPU, memory **leaks** etc.  
To use run e.g. `valgrind --leak-check=full ./sr-editor3`, at end it will list details about leaks.
- leaks, thread **sanitizer** for gcc or clang [video](https://www.youtube.com/watch?v=ZJKBwQ71LN4&list=PLvv0ScY6vfd_ocTP2ZLicgqKnvq50OCXM&index=14), or [ASAN](https://clang.llvm.org/docs/AddressSanitizer.html).
- **Profiler**. E.g.: Intel VTune, AMD uProf, Google Orbit, KDAB/hotspot. Any of them will do.  
For measuring performance, CPU time spent in methods from classes in code.  
Used [KDAB/hotspot](https://github.com/KDAB/hotspot) on Linux, quick tutorial [video](https://www.youtube.com/watch?v=6ogEkQ-vKt4), one [longer](https://www.youtube.com/watch?v=HOR4LiS4uMI). Overview of profiling [tools](https://www.youtube.com/watch?v=GL0GIdj6k2Q), Heaptrack [video](https://www.youtube.com/watch?v=OXqqVSdrSAw).
- WIP Using **Vulkan**.  
It has better support for debugging tools, Ogre-Next has more debug asserts etc, which <del>is</del> / *would be* good for finding errors.  
MyGui **doesn't** work with Vulkan (on Linux). On Windows seems working.  
Mentioned in [Post1](https://forums.ogre3d.org/viewtopic.php?p=553813#p553813), [Post2](https://forums.ogre3d.org/viewtopic.php?p=554446#p554446).  
*It started longer though, shader compilation was much slower*, should be now fast on Ogre-Next (4.0) master.  

--------

# Ogre-Next docs

[Ogre-Next](https://github.com/OGRECave/ogre-next) is the engine used for: rendering, managing 3D scene, also loading resources, logging to `.log` files etc.

- [Manual](https://ogrecave.github.io/ogre-next/api/latest/manual.html) - recommended reading when beginning.
- [Compositor](https://ogrecave.github.io/ogre-next/api/latest/compositor.html) - for effects, RTTs, reflections, etc.
- [Terra](https://ogrecave.github.io/ogre-next/api/latest/_terra_system.html) - details of its new system for Terrain.
- [version comparison](https://www.ogre3d.org/about/what-version-to-choose) of Ogre and Ogre-Next - just for info.  

----
## HLMS, materials, shaders

**HLMS** (High Level Material System) is the part in Ogre-Next responsible for generating shaders used in materials.  
There is no shader graph editor, everything is in big json or text files.  

- [HLMS doc](https://ogrecave.github.io/ogre-next/api/latest/hlms.html) - **long** read, explains all.
- [HLMS shaders](https://ogrecave.github.io/ogre-next/api/latest/hlms.html#HlmsCreationOfShaders).
- [Ogre wiki](https://wiki.ogre3d.org/Ogre+2.1+FAQ#I_m_writing_my_own_Hlms_implementation_or_just_want_to_know_more_about_it._Where_do_I_find_learning_material_resources_) - with links to old forum topics about common tasks.

It is very complex and extensive, it covers plenty of conditions for shader variations (in .any files).  
We have own shaders for:
- **Terra** for terrain, with layers, triplanar, based on PBS.  
Main files in `data/Hlms/Terra/Any`.
- **PBS**, regular shaders for all materials (objects, road,  cars, vegetation etc).  
Main files in [data/Hlms/Pbs/Any/Main](../data/Hlms/Pbs/Any/Main).  
I'm writing **water**/fluids also inside PBS, don't want to split.
- Unlit (no lighting). We should only use this for **Gui**.  
_ToDo:_ **Particles** must be at least: colored by sun diffuse, darker by shadows, also by terrain.  
Small files in `data/Hlms/Unlit/Any`.

Shader code in `.any` files are preferred, universal and these get translated to `.glsl` (or `.hlsl`) at run time.

SR3 own files are prefixed `SR3_`, so all:  
 `data/Hlms/Pbs/Any/[Main/]SR3_*piece*.any`  
 files have our shader code for: fluids, fog, tree wind, vehicle paint.

Continued in own page for [Materials](Materials.md).

--------

# Stunt Rally 3

## config files 📄

[👤] - means file is in user `.config/stuntrally3` dir, and saved by SR3.  
Otherwise, means a static file in `config/` dir, manually edited.  
(❔) - means file has info about itself and syntax, written inside.

Vehicles config
- config/cars.xml - (❔) - list for Vehicles tab with manually set parameters
- data/cars/cameras.xml - (❔) - all follow camera views for game
- data/carsim/[mode]/* -  
*.car - vehicle file (can edit directly in game Alt-Z),  
*_stats.xml - vehicle performance test results (after Ctrl-F5)  
.tire coeffs, suspension

Game modes
- championships.xml - (❔) - for Gui, tutorials too
- challenges.xml - (❔) - with tracks list, pass conditions, game setup etc

Game progress
- progress.xml, progress_rev.xml - [👤] - progress for championships, _rev for reversed
- progressL.xml, progressL_rev.xml - [👤] - same but for challenges

Game config
- input.xml and input_p[1..4].xml - [👤] - for game, common and player input bindings, Gui tab Input

- paint.cfg - current vehicles paint setups
- paints.ini - dynamic list for vehicles on Paints tab

Common config
- fluids.xml - (❔) - buoyancy and other fluid parameters for fluid areas
- presets.xml - (❔) - params for editor lists and game too
- tracks.xml - [👤] - tracks bookmark and rating, from Gui Track tab

Track files, in each track's dir.  
Below * means: nothing or 2,3,4.. etc if more.
- heightmap*.f32 - heightmap(s) for terrain(s). Format is raw 32 bit floats (4B for 1 height). Size always square: 512,1024,2048 etc.
- road*.xml - file(s) for each road/river etc. Has many params and points.
- scene.xml - 1 file for each track, with all editable params from Gui tabs in Track Editor.

## emojis

You can also get familiar with [emojis](../src/emojis.txt) file.  
It lists many emojis used for indentifying code sections, variable blocks in sources, xml configs, etc.  
Done for better orientation, grouping and cooler code in files, especially for big ones with many at once.  
It is possible to search for all related to some component or aspect in sources,  
by searching for an emoji, e.g. 💨 for game boost, ⛰️ for all terrain stuff, 🎯 for all ray casts made, etc.

## subdirs 📂

SR3 code is in `src/` its subdirs are:
- [lib] [btOgre2](https://github.com/Ybalrid/BtOgre2) - for visualisation of bullet debug lines in Ogre-Next (called Ogre 2.1 before)
- **common** - files for both editor and game. Mainly for common Gui, scene (map / track) elements.  
Has also some MyGui classes a bit extended: `MultiList2.*`,  
and new `Slider*.*, Gui_Popup.*` and `GraphView.*` for game Graphs (Tweak F9).
- common/data - has all data structures (mostly `.xml` config files, listed above).
- **editor** - purely editor sources
- **game** - purely game sources
- libs - few single file libraries
- network - game multiplayer code, info in [DesignDoc.txt](../src/network/DesignDoc.txt).  
Also `game-server/`, small program for server game list (not used).
- OgreCommon - [Ogre-Next](https://github.com/OGRECave/ogre-next/) base application classes - slightly changed.
- [lib] [oics](https://sourceforge.net/projects/oics/) - for input configurations, bindings and analog key inputs emulation.
- road - code for unique SR roads, rivers, walls, pipes and transitions, with LODs.
- sound - sound engine, based on [RoR](https://github.com/RigsOfRods/rigs-of-rods/tree/master/source/main/audio)'s, using openal-soft
- Terra - terrain and other components, from Ogre-Next - quite modified (more info below).
- vdrift - SR simulation (based on old [VDrift](https://github.com/VDrift) from about 2010) with a lot of changes and custom code,  
Also has simulation for spaceships, sphere, hovers etc, and for `Buoyancy.*`.

## Doxygen

Documentation for SR3 sources, class list, members, hierarchy, diagrams etc.  
can be generated using [Doxygen](https://www.doxygen.nl/).  
Setup for it is in [Doxyfile](../Doxyfile).  
After installing Doxygen (and its needed deps for diagrams), start by `doxygen`.  
Then access it by opening created [doxygen/html/index.html](doxygen/html/index.html).

----

# Ogre-Next

Ogre-Next has few components: Terra, `Atmosphere` and PlanarReflections in sources form.  
Those are included in SR3, inside `src/Terra/` and modified for our needs.  
The basic application code and setup is also included from Ogre-Next, inside `src/OgreCommon`, modified.  
Many key changes are marked with `//** `, searching it will find all modified places.


## Modified

Components completely new with Ogre-Next, included in SR3, and modified already.

### Atmosphere 🌫️

Only used for **fog** and its params.  
It can render sky atmosphere with sun (and no clouds). We don't use this. We could someday have dynamic sky shaders this way.  
Modified to not change light after its update. And added more params for shaders, fog etc.  

Our fog shader code is in: [SR3_Fog_piece_ps.any](../data/Hlms/Pbs/Any/SR3_Fog_piece_ps.any)  
and is used in Pbs and Terra `.any` there `@insertpiece( applyFog )`.


### Terra ⛰️

The terrain component. Very efficient. Has triplanar already.  

_ToDo:_ But `setDetailTriplanarNormalEnabled(true);` is broken still, has black-white spots.  
Posts: [start](https://forums.ogre3d.org/viewtopic.php?p=554304#p554304) until [end](https://forums.ogre3d.org/viewtopic.php?p=554312#p554312).

#### Comparison

This **changed** completely from old Ogre. Now heightmaps are power of 2, e.g. 1024, not 1025 like old.  
Old SR `heightmap.f32` are converted on 1st load in SR3. This also needs slight pos offset.  
SR3 loads `heightmap.f32` and gets size (e.g. 1024) from file size (`1024*1024*4 B`).  

#### Normalized

Terra by design had _normalized float heights_ from 0.0 to 1.0 only.  
We **don't** use that, changed it, since our Hmap and SR editor use any, real height values.  

**_ToDo:_**  
This **broke** [TerraShadowMapper](../src/Terra/TerraShadowMapper.cpp) shadowmap generation (mostly for heights below 0).  
and terrain **shadowmap** is disabled.  

Good info [in post](https://forums.ogre3d.org/viewtopic.php?p=555308#p555308), how it works and [where (2nd half)](https://forums.ogre3d.org/viewtopic.php?p=555280#p555280) to change.  

Old: BTW it took way too long, 5 sec at start, possibly due to compute shader building.  
It's this `if (!m_bGenerateShadowMap)` and `return;  //**  5 sec delay` below.  
Set earlier `,bGenerateShadowMap( 0 )  //** ter par  //^^ todo: 1 in ed`.

This also **broke** terrain page **visibility**. Likely decreasing Fps, no idea how much.  
Made all visible in `Terra::isVisible` for `if (!bNormalized)`.  
In ctor `Terra::Terra(` setting `bNormalized` to 1 does try to normalize Hmap.  

**_ToDo:_** Call Terra::update when the camera changes for each of splitscreen views.

We use `tdb->setBrdf(TerraBrdf::BlinnPhongLegacyMath);`  
because it looks best, other are bad. No roughness control.

#### Extended

Terra is extended with **blendmap** RTT (from old SR) and its noise.  
Also with emissive, property: `emissive terrain` in shaders.  

Changed skirt to be relative size below, not that default big to lowest point.  
_ToDo:_ Some holes can appear on horizon terrains. Should be higher for them and on horizon.  
Also good for fluid planar reflection when that skirt is visible.

Added _own_ terrain raycast code (for cursor hit pos), since there wasn't one, [topic](https://forums.ogre3d.org/viewtopic.php?t=96602).  
_ToDo:_ For far future. Not a problem, but since we have many, [how to do multiple terras](https://forums.ogre3d.org/viewtopic.php?t=96050).


### PlanarReflection 🪞

Used for water/fluids. Renders flat mirror reflection for them when visible.  
Modified to have more control and detail options for its camera.  
Shader code in [PlanarReflections_piece_ps.any](../data/Hlms/Pbs/Any/PlanarReflections_piece_ps.any), mainly:  
`planarReflUVs`, normal is in `pixelData.normal`, and distortion is added to `pointInPlane` from normal.  
More info in [post](https://forums.ogre3d.org/viewtopic.php?p=554536#p554536).

## Custom

These are using Ogre-Next base components but extend them to our specific rendering purposes.

### HlmsPbs2

Main **shaders** for all materials (except terrain and particles) here:  
[800.PixelShader_piece_ps.any](../data/Hlms/Pbs/Any/Main/800.PixelShader_piece_ps.any)  (modified)  
[800.VertexShader_piece_vs.any](../data/Hlms/Pbs/Any/Main/800.VertexShader_piece_vs.any)  (modified)  
structures with variables passed to them are in:  
[500.Structs_piece_vs_piece_ps.any](../data/Hlms/Pbs/Any/Main/500.Structs_piece_vs_piece_ps.any)  

Keep in mind ([post](https://forums.ogre3d.org/viewtopic.php?p=554100&sid=6798838bbed3be6881aa07bf10012412#p554100)),
that in .any this does not comment: `// @property( hlms_fog )`  
Either add `0 &&` inside, or any letters to make non existing name, or remove `@` too.


## Changed from SR

These have changed when porting SR to SR3 to use Ogre-Next meshes etc.

### Road 🛣️

Original code made by CryHam for: roads, pipes, their transitions and bridge walls, columns, rivers.  
Code inside:
- [Road_Prepass.cpp](../src/road/Road_Prepass.cpp) - needed computations before rebuild.
- [Road_Rebuild.cpp](../src/road/Road_Rebuild.cpp) - main big code, creating geometry vertices and meshes.
- Grid.* - WIP new for paging grid and adding meshes together in cells, for less batches (draw calls).  
Now used for columns to gather more together.

Creating **mesh** code was based on Ogre-Next:  
`/Samples/2.0/ApiUsage/DynamicGeometry/DynamicGeometryGameState.cpp`.  
[Few posts](https://forums.ogre3d.org/viewtopic.php?p=554774#p554774) with info and issues (fixed).  
Road editing is **slower** now, [topic](https://forums.ogre3d.org/viewtopic.php?p=553712#p553712)  
due to going from V2 mesh to v1 computing tangents and back to v2.  
_ToDo:_ For far future. We should compute tangents/binormals ourself.  

_ToDo:_ We don't handle _device lost_ (happens in DirectX when going out of fullscreen),  
it will go bad for all created meshes (road, grass).  

#### Glass pipes ⭕

Glass pipes rendering changed. Old Ogre had 2 passes with opposite culling.  
New does not support passes [[2.3] Dealing with multi pass rendering in ogre next](https://forums.ogre3d.org/viewtopic.php?t=96902), and instead has:  
2 nodes with same mesh, but clones datablock and sets opposite cull in it.  
Code in `pipe glass 2nd item` section of [Road_Mesh.cpp](../src/road/Road_Mesh.cpp).  

Transparent objects are sorted by Ogre-Next so they don't blink randomly like in old Ogre.  
There is though a less noticable issue with order on borders showin elipses between pipe segments [screen here](https://forums.ogre3d.org/viewtopic.php?p=553945&sid=6798838bbed3be6881aa07bf10012412#p553945).  

From [post](https://forums.ogre3d.org/viewtopic.php?p=556887#p556887), only:
- Ogre has an Order Independent Transparency demo with WBOIT method.  
- Ogre-Next has few Global Illumination methods (but not for terrain) and Alpha Hashing transparency.

### Vegetation 🌳🪨

Means models for trees, palms, rocks, bushes, plants etc.  
It is not paged, does not have impostors (like it was in old SR with paged-geometry, *aka the worst library used*).  

Now simply uses Ogre-Next Items that will be automatically HW instanced, to reduce draw calls, also works with mesh LODs.  
All models are placed during track load by code in [CScene_Veget.cpp](../src/common/CScene_Veget.cpp).  
**_ToDo:_** make it parallel on CPU, it is rather slow.

### Objects .mesh

For Vegetation, Objects, etc.  
All models in **`.mesh`** files from old SR, need to be converted to newer version for SR3 using `OgreMeshTool`.  
Provided Python script [ogre-mesh.py](../config/ogre-mesh.py) is helpful for this. Comments inside have more detail.  
It can process whole dir (e.g. new `objects3/` with files to convert) and has command presets, with LODs set up.  

Code for creating in `App::CreateObjects()` in [App_SceneObjects.cpp](../src/common/App_SceneObjects.cpp).

### Grass 🌿

It has pages (auto size from terrain size) and no LoDs.  
Also no fading yet. Started, find: `grow fade away`.  
Currently done simplest way, has mesh pages, with vertices and indices.  
Quite inefficient to render and slow to create mesh.  
Code in [Grass.h](../src/common/Grass.h) and [Grass_Mesh.cpp](../src/common/Grass_Mesh.cpp). Written quickly by CryHam, partly based on [Road_Mesh.cpp](../src/road/Road_Mesh.cpp) code.  

**_ToDo:_** make it parallel on CPU, it is very slow.  
**_ToDo:_** add RTT for density map, read terrain height map in shader, do vertices in shader not on CPU..  
[old topic](https://forums.ogre3d.org/viewtopic.php?t=85626&start=25)  
[some info](https://forums.ogre3d.org/viewtopic.php?p=553666#p553666) (under "How should I add grass?") on how to do it better.  


## SR

Rest of code that was same in SR 2.x. Only small changes when porting to SR3. And new hovering vehicles since 3.1.

### Simulation 🚗

Pose means here position and rotation.  
Game code in [Update_Poses.cpp](../src/game/Update_Poses.cpp) gets data from VDrift simulation (on 2nd thread) calling `newPoses(`.  
Rendering update calls `updatePoses(` to set vehicles (from carModels) to new poses.  
[CarModel](../src/game/CarModel.h) is just the class gathering vehicle's 3d model meshes, particles and other Ogre graphics.  

VDrift simulation, main update code in [cardynamics_update.cpp](../src/vdrift/cardynamics_update.cpp) in `void CARDYNAMICS::UpdateBody(`.  

SR's own code for non-car vehicles are in: `SimulateSpaceship(` 🚀, `SimulateSphere(` 🔘, and all hovering in `SimulateHover(`.  
Key places in code (that change simulation) for such special vehicles are marked with `//V*`.


### Replay 📽️

All files in `src/game/` with `replay` in name, mainly [Replay.h](../src/game/Replay.h) and [Replay.cpp](../src/game/Replay.cpp) and [Gui_Replay.cpp](../src/game/Gui_Replay.cpp) for Gui events.  
Partly code in [Update_Poses.cpp](../src/game/Update_Poses.cpp) for filling replay/ghost data etc.  
More detail in [CGame.h](../src/game/CGame.h) sections with vars for this.  

### Sound 🔉

Sound manager code in `src/sound/`. Uses [OpenALSoft](https://github.com/kcat/openal-soft).  
It is based on [RoR's](https://github.com/RigsOfRods/rigs-of-rods/tree/master/source/main/audio) code, but very reduced. It completely replaced old VDrift's code.

**_ToDo:_** [Task here](Tasks.md#sounds).


----

# 🟢 Ogre-Next - Advanced 🧰🔧

Explaining how to do advanced things, that need extending HLMS, shaders and .cpp code for it too.
Some utility [code](../src/common/AppGui_Util.cpp) with few really basic things that needed methods now.

Some links (few also in [post here](https://forums.ogre3d.org/viewtopic.php?p=554575#p554575)) with good info (may be somewhat random and really old):
- [recent longer 2 replies](https://forums.ogre3d.org/viewtopic.php?p=555022#p555022)
- [wiki links](https://wiki.ogre3d.org/Ogre+2.1+FAQ#I_m_writing_my_own_Hlms_implementation_or_just_want_to_know_more_about_it._Where_do_I_find_learning_material_resources_)
- [done] [Easier way to communicate with hlms shader?](https://forums.ogre3d.org/viewtopic.php?f=25&t=83081) - old, **good** with stuff below too
- [done] [Adding HLMS customisations per datablock](https://forums.ogre3d.org/viewtopic.php?t=84775)
- [done] [Adding Wind to Grass](https://forums.ogre3d.org/viewtopic.php?t=95892)
- [done, old] [custom hlms wind and fog](https://spotcpp.com/creating-a-custom-hlms-to-add-support-for-wind-and-fog/)
- [old] [Compositors, HLMS and depth buffers](https://forums.ogre3d.org/viewtopic.php?p=543364#p543364)
- [[Solved][2.1] Trying to create a new HLMS](https://forums.ogre3d.org/viewtopic.php?f=25&t=83763)
- [done] [basic using in app](https://ogrecave.github.io/ogre-next/api/2.3/_using_ogre_in_your_app.html)

## HLMS shaders output

Shaders are saved to `/home/user/.cache/stuntrally3/shaders`.  
This is useful to know what finally ended in all shader's code (e.g. glsl) after HLMS did its preprocessor work.  
It has its settings on Gui now (that go into `setDebugOutputPath`): Debug Shaders and Debug Properties.  

With Debug Properties set, properties are at top of each shader file,  
this makes all compiling errors have wrong line numbers,  
but at least one can get some idea of what kind of shader it was.  
Other way is putting extra comments in .any and seeing them in .glsl as result.  
Added few, starting with `//** This is `.

## HLMS extending 🌠

Customizing [doc link](https://ogrecave.github.io/ogre-next/api/latest/hlms.html#HlmsCreationOfShadersCustomizing).

From docs: Hlms implementation can be customized:
- Through HlmsListener.  
Example [here](https://github.com/OGRECave/ogre-next/blob/master/Samples/2.0/Tutorials/Tutorial_Hlms01_Customization/Tutorial_Hlms01_MyHlmsListener.h) of adding new float4 and setProperty.  
This allows you to have access to the buffer pass to fill extra information; or bind extra buffers to the shader.
- Overload HlmsPbs. Intro [post](https://forums.ogre3d.org/viewtopic.php?p=554026&sid=6798838bbed3be6881aa07bf10012412#p554026)  
Useful for overriding only specific parts, or adding new functionality that requires storing extra information in a datablock  
(e.g. overload HlmsPbsDatablock to add more variables, and then overload HlmsPbs::createDatablockImpl to create these custom datablocks)
- Directly modify HlmsPbs, HlmsPbsDatablock and the template (.any).

SR3 does all above, we have our `HlmsPbs2`.  
Also a HLMS PBS listener, it's that default `hlmsPbs->setListener( mHlmsPbsTerraShadows );`  
from Terra, this is only for Pbs objects, `hlmsTerra` has no listener.  
(<del>_ToDo:_ adding globalTime to both would need it</del> - no, `atmo.` seems working).  
Done so objects also receive terrain shadows. Only one listener can be used.  

And we have own datablock: `HlmsPbsDb2` with more stuff when needed for: vehicle paint or fluids.

### Adding more uniforms

Adding more params for Pbs shaders and terrain. E.g. `globalTime` and our own height fog like `fogHparams`.  
Easiest to add new in `struct AtmoSettingsGpu` also `struct AtmoSettings` and then fill it in `Atmosphere2Npr::_update` (called each frame).  
This is mostly working, even for shadow casters etc.  

_ToDo:_ [topic](https://forums.ogre3d.org/viewtopic.php?p=535357#p535357) done through HlmsListener using `getPassBufferSize` and `preparePassBuffer`.

This shader file [500.Structs_piece_vs_piece_ps.any](../data/Hlms/Pbs/Any/Main/500.Structs_piece_vs_piece_ps.any) has definitions for:
- `MaterialStructDecl`
- `CONST_BUFFER_STRUCT_BEGIN( PassBuffer` - uniforms that change per pass
- `@piece( VStoPS_block )`
- and lots more

### setProperty 📄

Allows using different code blocks in `.any` shaders (i.e. different shading paths).  
Calling `setProperty` in `HlmsPbs2::calculateHashForPreCreate(` and if needed same in `calculateHashForPreCaster`.  
Our method checks in name start from `.material` and sets a few cases already (grass, sky, water etc and body for vehicles).

See `selected_glow` for blue selection glow in SR editor for objects, road segments etc.  

Code in `AppGui::UpdSelectGlow(` changes "selected" for a `Renderable` and forces `CalculateHashFor`.  
Then code in `HlmsPbs2::calculateHashForPreCreate(` does by `getCustomParameters(` get the value  
and calls `setProperty` for shaders.

_ToDo:_ It works but slow, when selecting it creates a new shader (delay 1st time). Better use some uniform for this.

_ToDo:_ not needed, add renderable->hasCustomParameter( 999.. ) for all to get info in debugger..


### PBS vertex colors 🌈

[Topic](https://forums.ogre3d.org/viewtopic.php?p=554630#p554630), added for road blending, in editor minimap road preview colors etc.

### Adding textures

[topic from 2015](https://forums.ogre3d.org/viewtopic.php?f=25&t=84539)


### Own DataBlocks 🎲

Inside `HlmsPbs2::createDatablockImpl(` we create `HlmsPbsDb2` for vehicle paint params.  
It has own `HlmsPbsDb2::uploadToConstBuffer`, which is where data is uploaded from C++ to GPU material shader buffers.  

Currently can't change total size, must set `mUserValue[` in cpp and get in shader `material.userValue[`.  
_ToDo:_ `body_paint` property not used yet.  
_ToDo:_ `HlmsPbsDb2` clone fails. IIRC this made the 7 duplicates in .json for vehicle paint.

When setting car material need to use `HlmsPbsDb2*`. Set by `CarModel::SetPaint()`.  
Changes need `scheduleConstBufferUpdate()` to apply.

The modified `HlmsPbs2` replaces default `HlmsPbs` in Ogre-Next and reads all from same dirs as Pbs.  
It gathers all above points.  


### Own HLMS

*Rather **not** needed in SR3*  

Fully own HLMS should inherit from Pbs, and (probably?) needs its own dirs with shaders.  
Also own name in `.material` for materials e.g. `hlms water_name pbs` that last `pbs`.  
[Trying to create a new HLMS](https://forums.ogre3d.org/viewtopic.php?f=25&t=83763)

One example is already in Terra. Has also more and difficult code,
e.g. `Fill command Buffers` in `HlmsTerra::fillBuffersFor(`.

Own HLMS could control more, like `MaterialSizeInGpu` (it needs same size in `uploadToConstBuffer`).  
This assert triggering, means those were different:  
`assert( (size_t)( passBufferPtr - startupPtr ) * 4u == mapSize );`


### Assert, other 🧪

A common assert: [mCachedTransformOutOfDate](https://ogrecave.github.io/ogre-next/api/latest/_ogre20_changes.html#AssersionCachedOutOfDate).  
At end of [post](https://forums.ogre3d.org/viewtopic.php?p=554822#p554822) and also debug todo [here](https://forums.ogre3d.org/viewtopic.php?p=555087#p555087).  

This happens many times in Debug build in SR3.  

Probably best way is to set OgreNext CMake option `OGRE_DEBUG_LEVEL_DEBUG` to 1 or 0.  
So that those `OGRE_ASSERT_MEDIUM( !mCachedTransformOutOfDate );` won't happen (break).  
This is already done by `build-sr3-Linux.py`.

Other way is commenting out (with `//`) in Ogre-Next sources. _ToDo:_ fix it?  
For reference these 3 places needed:
```
/Ogre/ogre-next/OgreMain/include/OgreNode.h : 680

    virtual_l2 FORCEINLINE const Matrix4 &_getFullTransform() const
    {
        // OGRE_ASSERT_MEDIUM( !mCachedTransformOutOfDate );
..
OgreNode.cpp : 753

    Quaternion Node::_getDerivedOrientation() const
    {
        // OGRE_ASSERT_MEDIUM( !mCachedTransformOutOfDate );
..
OgreNode.cpp : 765

    Vector3 Node::_getDerivedPosition() const
    {
        // OGRE_ASSERT_MEDIUM( !mCachedTransformOutOfDate );
```

Don't change node pos or rot from inside listeners.  
Or after, need to call `_getFullTransformUpdated(` e.g. for `ndSky`, `light->getParentNode()->_`, camera etc.  
Seems fixed for each frame, but IIRC does assert on new track load or so.  

Also [post](https://forums.ogre3d.org/viewtopic.php?p=554822#p554822), leaking GPU RAM inactive and  
Exception: `Mapping the buffer twice within the same frame detected!`.


----
## Compositor 🪄

### Workspaces 🪄

Workspace is basically the setup, for rendering one player's view target only, like: reflection, shadow, editor minimap, SSAO blur, etc and lastly screen.  
Almost all compositor things (Workspaces, WorkspaceDef and NodeDef) are created from code now.  
Code is in [AppGui_Compositor.cpp](../src/common/AppGui_Compositor.cpp) and utils in [AppGui_CompositorUtil.cpp](../src/common/AppGui_CompositorUtil.cpp.cpp).  
 (telling how to render stuff, also few extra for editor minimap RTT).  
Creating is in `.log`, lines with: `--++ WS add:`, and in cpp code by any `addWorkspace`.  
Also new code in `.log` has lines starting `CC` with `Compositor` etc.

[SR3.compositor](../data/materials/SR3.compositor) has definitions for few other workspaces used.  

Shadows are made completely by code in [AppGui_Shadows.cpp](../src/common/AppGui_Shadows.cpp),  
based on `Samples/2.0/ApiUsage/ShadowMapFromCode/ShadowMapFromCode.cpp`.  
_Todo:_ only 3 PSSM splits work, no other count. ESM (Sh_Soft) is also broken.

Parts of [post](https://forums.ogre3d.org/viewtopic.php?p=553666#p553666) with info.


### Workspace Listener

We do `workspace->addListener(` so that `PlanarReflWorkspaceListener` is updated, for _every_ workspace that can see the reflection (e.g. the cubemap workspaces).  
In code `AddListenerRnd2Tex()`, it's `ReflectListener`, `mWsListener` in [FluidsReflect.h](../src/common/FluidsReflect.h).

`ReflectListener::passEarlyPreExecute` does check which `render_pass` it is from `SR3.compositor` by `identifier` number.  

In editor it also turns off fog for minimap RTTs, and back on for main views.

_Todo:_ use `#define MANUAL_RTT_UPD` so editor won't drop Fps so much (rendering all its RTTs every frame).  
Needs **fix**: in minimap and preview camera all gets dark (shadowed).  
Possibly relevant [link](https://forums.ogre3d.org/viewtopic.php?p=556401#p556401).


## Effects

Compositor is created from cpp, mainly in [AppGui_Compositor.cpp](../src/common/AppGui_Compositor.cpp).  
This gives much more flexibility and access to consts from [RenderConst.h](../src/common/RenderConst.h).

### Done:
- Refractions (water), for all fluids, based on Ogre-Next Sample_Refractions
- SSAO, based on Ogre-Next Sample_Tutorial_SSAO
- Lens flare (sun shines in camera), [shaders](https://www.shadertoy.com/results?query=lens+flare), [more info](http://john-chapman-graphics.blogspot.com/2013/02/pseudo-lens-flare.html?m=1)
- Sunbeams (rays around trees etc), aka godrays [shaders](https://www.shadertoy.com/results?query=tag=godrays&sort=popular)
- WIP GI, using code from Ogre-Next Test_Voxelizer (IFD, VCT)  
  has _no Terrain_, [issue](https://github.com/OGRECave/ogre-next/issues/475), see comments, but really needed for terra, mostly for inside buildings

### _ToDo:_
- Sample_**HDR** (with bloom)
- other GI todo?: InstantRadiosity (main), LocalCubemaps refl, ImageVoxelizer CIVCT
- ReconstructPosFromDepth ? (soft particles) also [topic](https://forums.ogre3d.org/viewtopic.php?t=97096&sid=5b5762532f55a6b74f28e1404b1d54bb), [maybe](https://forums.ogre3d.org/viewtopic.php?t=97059&sid=5b5762532f55a6b74f28e1404b1d54bb)
- Sky_Postprocess to drop sky nodes
- InterpolationLoop (meh, later)
- meh: OpenVR, StereoRendering
- fix setupESM: ShadowMapFromCode, ShadowMapDebugging

[Video Playlist](https://www.youtube.com/playlist?list=PLU7SCFz6w40OJsw3ci6Hbwz2mqL2mqC6c) with related bugs, most fixed.
