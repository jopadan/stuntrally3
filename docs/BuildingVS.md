## About

This page describes how to compile Stunt Rally 3:
- on **Windows** (10 only tested)
- building first **all dependencies from sources**
- using **VS** 2019  
  Could be older but seems that newer versions aren't yet tested for Ogre-Next at least.  
  There is a free Visual Studio Community edition [here](https://visualstudio.microsoft.com/downloads/), it should work okay (didn't check)  
  (_Visual Studio is a big commercial solution though, way too big and bloated_)  

Surely this is a **long** and somewhat **tedious** process, it can take **2 hours** or more to complete ⚠️.  

_SR3 depends on Ogre-Next 3.0, and MyGui fork for it which aren't released or packaged at all._  
_Other alternative could be getting Conan to work, or vcpkg. This is not done for SR3 ([WIP PR here](https://github.com/stuntrally/stuntrally3/pull/1))._  

_An easier project to build is my [ogre3ter-demo](https://github.com/cryham/ogre3ter-demo/) it only uses Ogre-Next._  

## 1. Getting Dependencies

We need tools: [Git](https://git-scm.com/downloads) and [CMake](https://cmake.org/download/) installed.

This table lists all needed dependencies (libraries) for SR3:

| **Library** | Version used | Website                                    | Downloads                                                                 |
|-------------|--------------|--------------------------------------------|---------------------------------------------------------------------------|
| tinyxml2    | 9.0.0        | <https://github.com/leethomason/tinyxml2/> | [Downloads](https://github.com/leethomason/tinyxml2/tags)                 |
| Bullet      | 3.25         | <https://pybullet.org/wordpress/>          | [Downloads](https://github.com/bulletphysics/bullet3/releases)            |
| Boost       | 1.81         | <https://www.boost.org>                    | [Downloads](https://sourceforge.net/projects/boost/files/boost-binaries/) |
| Enet        | 1.3.17       | <https://enet.bespin.org>                  | [Downloads](https://github.com/lsalzman/enet/tags)                        |
| Ogg         | 1.3.5        | <https://xiph.org>                         | [Downloads](https://xiph.org/downloads/)                                  |
| Vorbis      | 1.3.7        | <https://xiph.org>                         | [Downloads](https://xiph.org/downloads/)                                  |
| OpenAL Soft | 1.23.1       | <https://github.com/kcat/openal-soft>      | [tags](https://github.com/kcat/openal-soft/tags)                          |
|             |              |                                            |                                                                           |
| Ogre-Next<br />has SDL2  | 3.0     | <https://www.ogre3d.org>                   | [git repo](https://github.com/OGRECave/ogre-next) not to Download, script |
| MyGui-next  | 3.?          | <http://mygui.info/>                       | [git repo](https://github.com/cryham/mygui-next/)                         |

Newer, latest versions can be used.  
Only Ogre-Next and MyGui-next are special cases, info in own, later sections.

## 2. Building dependencies

### 2.1

**Ogre-Next** sources are downloaded (using Git) by a script, which will also get its dependencies like SDL2 and build them and itself.  
**MyGui-next** is my fork, it needs downloaded .zip with sources or cloned by Git.  
Both are explained later, for now no need to do anything.

### 2.2 

For **Boost**, since I used VS 2019 14.2 (upgrade 2, not 3 yet) I downloaded boost_1_81_0-msvc-14.2-64.exe to match it.  
For me this e.g. 14.2 is shown in *VS Installer* app on tab *Individual components* as: C++ Modules for v142 build tools (x64/x86)  
and are present and picked in Project properties, in General tab, Platform Toolset.  
This is an `.exe` that will extract Boost files to a typed folder.  

### 2.3

**Bullet** is different, it has a file [`build_visual_studio_vr_pybullet_double_dynamic.bat`](https://github.com/bulletphysics/bullet3/blob/master/build_visual_studio_vr_pybullet_double_dynamic.bat).  
Important line in this `.bat` is starting with: `premake4  --dynamic-runtime --double  `  
Meaning bullet will use _double_ types and dynamic DLL for runtime.  
I recommend removing `--double` from it, it's not needed and works slower on old PCs.

Start this `.bat` file, it then will generate a folder bullet3\vs2010\ with `0_Bullet3Solution.sln` file,  
open it and build with your VS (first convert dialog will show).  
You can build whole solution, there is more stuff.  
We definitely need to build: BulletCollision, BulletDynamics, BulletFileLoader, BulletWorldImporter, LinearMath.  
_Those are older bullet 2 libs (bt* names), not the bullet 3 new (b3* names)._

### 2.4

The rest: **tinyxml2, Enet, Ogg, Vorbis, OpenAL Soft**, for each:  
download `.zip` file with sources for latest stable release version and extract in root folder e.g. `c:\dev`  
Resulting in e.g. these dirs:
```
c:\dev\
  bullet3-3.25
  enet-1.3.17
  libogg-1.3.5
  libvorbis-1.3.7
  openal-soft-1.23.1
  tinyxml2-9.0.0
```

These libs have `CMakeLists.txt` file inside, which means we need to:  
**for each lib**: start CMake-Gui, pick sources folder to where you extracted it  
e.g. `c:\dev\enet-1.3.17`, and then new `build` folder inside e.g. to `c:\dev\enet-1.3.17\build`  
After that set, press Configure, twice, and lastly Generate.  
Now there should be a `.sln` file inside build\ dir.  

Open it (with your VS) and build solution.  
We need to build `Release x64` (64 bit compiler, Release configuration), _later if needed Debug for any issue finding._

Many of these libraries will build just as _Static library .lib_. That's okay, _these will link with our exe, not be in own DLL_.  

For **enet** also these system libs were needed to add in Properties - Linker input:  
winmm.lib;Ws2_32.lib;

Very **important** thing is that all should use in:  
Project Properties - C++ - Code Generation - Runtime Library: **Multi Threaded DLL**, and for Debug same but with Debug DLL.  
This was set so in all but best to be sure. _Failing that would lead to many linker issues much later when linking SR3._  


## 3. Building Ogre-Next

Building Ogre-Next from sources, using [scripts](https://github.com/OGRECave/ogre-next/tree/master/Scripts/BuildScripts/output).  

Save the file [build_ogre_Visual_Studio_16_2019_x64.bat](https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_Visual_Studio_16_2019_x64.bat)  
and put it inside our root folder `c:\dev`

We need Ogre-Next branch v3.0 now. So edit `build_ogre_Visual_Studio_16_2019_x64.bat` and replace in this top line:  
`set OGRE_BRANCH_NAME=master"` to `set OGRE_BRANCH_NAME=v3-0` and save file.

Also in same file we have to add `-D OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS=1` 
in 2 lines starting with `cmake -D `...  
So they should start like:  
`cmake -D OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS=1 `...

Then open cmd and start the `.bat`.

This should succeed after a longer while and build Ogre-Next with its dependencies.

If so you can start and check Ogre demos and samples inside:  
  `c:\dev\Ogre\ogre-next\build\bin\Release\`  
  It's good to check if they work before continuing.

### 3.1 (Optional) Ogre components, rebuild
- Now *not needed*, optional step. We need to have below 2 components built with Ogre.
  
  In `c:\dev\Ogre\ogre-next\build\`  
  edit the file `CMakeCache.txt` and be sure the lines:
  ```
  OGRE_BUILD_COMPONENT_ATMOSPHERE:BOOL=ON
  ...
  OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS:BOOL=ON
  ```
  have ON at end. _At least PLANAR_REFLECTIONS is not by default._

- Now **build Ogre** again. So, in all needed configurations, by opening solution  
  `OGRE-Next.sln` from above `build\` dir, and building in VS.  
  This will take less time than first in cmd.  

----
- _Optional note_.  
  This (rebuild Ogre) step is also needed, after updating Ogre sources to latest (`git pull` in `Ogre/ogre-next/`).  
  Caution: if this was done later, after building MyGui and/or StuntRally3,  
  then you need to rebuild also MyGui and then StuntRally3 too.
 
- _Optional_ for Debug only.  
  For Debug build to be usable we have to comment out this `assert`,  
  (needs rebuilding Ogre-Next after),  
  I added // at start in `OgreNode.h` (line was 681, could change):  

```
        virtual_l2 FORCEINLINE const Matrix4 &_getFullTransform() const
        {
#if OGRE_DEBUG_MODE >= OGRE_DEBUG_MEDIUM
            //assert( !mCachedTransformOutOfDate );
#endif
```

## 4. Building MyGui-Next

MyGui-Next is my fork of MyGui on branch `ogre3`.  
I follow its build guide [here](https://github.com/cryham/mygui-next/tree/ogre3).  
_It isn't the latest, and was also based on fork._  

No need to set, it is so by default:  
   - MYGUI_RENDERSYSTEM: 8 - Ogre 3.x
   - MYGUI_USE_FREETYPE: yes
   - all MYGUI_BUILD*: no
   - MYGUI_STATIC: no, MYGUI_DONT_USE_OBSOLETE: no

You could download .zip with sources, but `git clone` is better,  
since you can later do `git pull` to get any updates.

Inside `c:\dev\`
```
git clone https://github.com/cryham/mygui-next --branch ogre3 --single-branch
```
Follow its build guide [here](https://github.com/cryham/mygui-next/tree/ogre3#windows).  
This _needs_ renaming few cmake files in it.

Open CMake-Gui, pick sources `c:\dev\mygui-next`, build to `c:\dev\mygui-next\build`.  
Press Configure twice, then Generate.  

We need Release build, _only if Release fails, then Debug too._  

## 6. Clone SR3

Clone SR3 (this repo) and [SR3 tracks](https://github.com/stuntrally/tracks3) inside `data/tracks`:  
In `c:\dev\`
```
git clone https://github.com/stuntrally/stuntrally3.git sr3
cd sr3/data
git clone https://github.com/stuntrally/tracks3.git tracks
```
## 7. Build SR3

Rename the files, replacing them (best to keep copies):  
`CMakeLists-WindowsRelease.txt` to:  
`CMakeLists.txt`  
and  
`\bin\Release\plugins_Windows.cfg` to:  
`\bin\Release\plugins.cfg`

Open CMake-Gui, pick sources `c:\dev\sr3` and build as `c:\dev\sr3\build`.  

> If doing a Debug build, then do same but use:
> `CMakeLists-WindowsDebug.txt` and  
> `bin\Debug\plugins_Windows.cfg`  
> and choose a _different_ build dir e.g. `c:\dev\sr3\buildDebug`  

Press Configure twice.  
If it fails, then probably need to adjust some paths in CMake files.  
If it succeeded then press Generate.  
Open `c:\dev\sr3\build\StuntRally3.sln` with VS (2019),  
pick `Release x64` configuration and build it.  

This should produce valid *.exe files


## 8. Start StuntRally3

Copy into `c:\dev\sr3\bin\Release\`  
**all** needed `*.dll` files from previously built dependencies, we need:
```
MyGUIEngine.dll
OgreHlmsPbs.dll
OgreHlmsUnlit.dll
OgreMain.dll
OgreOverlay.dll
OgrePlanarReflections.dll
OpenAL32.dll
Plugin_ParticleFX.dll
RenderSystem_Direct3D11.dll (not used yet)
RenderSystem_GL3Plus.dll
SDL2.dll
amd_ags_x64.dll (got from internet)
```

If build succeeded, start:  
`c:\dev\sr3\bin\Release\StuntRally3.exe` - for SR3 game  
`c:\dev\sr3\bin\Release\SR-Editor3.exe` - for SR3 Track Editor  
and if for Debug then inside:  
`c:\dev\sr3\bin\Debug`  

Starting any `.exe` with `cfg` argument will show Ogre config dialog.

If Ogre dialogs shows empty, replace `plugins.cfg` with `plugins_Windows.cfg` again.  

Only `Open GL 3+ Rendering Subsystem` should be used (DX11 fails).  

Logs will be in:  
`C:\Users\USERNAME\AppData\Roaming\stuntrally3\`
