#include "pch.h"
#include "GraphicsSystem.h"
#include "paths.h"
#ifdef SR_EDITOR
	#include "CApp.h"
#else
	#include "CGame.h"
#endif
#include <OgreRoot.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreConfigFile.h>
#include <OgreWindow.h>

#include <Compositor/OgreCompositorWorkspace.h>
#include <OgreArchiveManager.h>
#include <OgreHlmsManager.h>
#include "OgreHlmsTerra.h"
#include "TerraWorkspaceListener.h"

//  declares WinMain / main
#include "MainEntryPointHelper.h"
#include "AndroidSystems.h"
#include "MainEntryPoints.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
	#include "OSX/macUtils.h"
	#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
		#include "System/iOS/iOSUtils.h"
	#else
		#include "System/OSX/OSXUtils.h"
	#endif
#endif
using namespace Ogre;


class GameGraphicsSystem final : public GraphicsSystem
{
	TerraWorkspaceListener *mTerraWorkspaceListener;

	/*void stopCompositor() override
	{
		if( mWorkspace )
			mWorkspace->removeListener( mTerraWorkspaceListener );
		delete mTerraWorkspaceListener;
		mTerraWorkspaceListener = 0;
	}*/

	//-----------------------------------------------------------------------------
	CompositorWorkspace *setupCompositor() override
	{
		return 0;
	}

	//-----------------------------------------------------------------------------
	void setupResources() override
	{
		GraphicsSystem::setupResources();
	}

	//  🌠 Hlms
	//-----------------------------------------------------------------------------
	void registerHlms() override
	{
		GraphicsSystem::registerHlms();

		String rootDir = PATHS::Data()+"/";

		RenderSystem *rs = mRoot->getRenderSystem();
		String name = rs->getName(), syntax = "GLSL";
		if (name == "OpenGL ES 2.x Rendering Subsystem")  syntax = "GLSLES";
		if (name == "Direct3D11 Rendering Subsystem")     syntax = "HLSL";
		else if (name == "Metal Rendering Subsystem")     syntax = "Metal";

		String mainPath;  StringVector paths;

		ArchiveManager &archiveManager = ArchiveManager::getSingleton();
		HlmsManager *hlmsManager = mRoot->getHlmsManager();
		{
			//  Create & Register HlmsTerra
			//  Get the path to all the subdirectories used by HlmsTerra
			HlmsTerra::getDefaultPaths( mainPath, paths );
			Archive* aTerra = archiveManager.load(
				rootDir + mainPath, getMediaReadArchiveType(), true );

			ArchiveVec dirs;
			for (auto it = paths.begin(); it != paths.end(); ++it )
			{
				Archive* aLib = archiveManager.load(
					rootDir + *it, getMediaReadArchiveType(), true );
				dirs.push_back( aLib );
			}

			//  Create and register the terra Hlms
			hlmsTerra = OGRE_NEW HlmsTerra( aTerra, &dirs );
			hlmsManager->registerHlms( hlmsTerra );
			hlmsTerra->setDebugOutputPath(mDebugShaders, mDebugProperties, PATHS::ShadersDir()+"/");
		}

		//  Add Terra's piece files that customize the PBS implementation.
		//  These pieces are coded so that they will be activated when
		//  we set the HlmsPbsTerraShadows listener and there's an active Terra
		//  see App::createScene01
		Hlms *hlmsPbs = hlmsManager->getHlms( HLMS_PBS );
		Archive *aPbs = hlmsPbs->getDataFolder();
		
		ArchiveVec libPbs = hlmsPbs->getPiecesLibraryAsArchiveVec();
		libPbs.push_back(
			ArchiveManager::getSingletonPtr()->load(
				rootDir + "Hlms/Terra/" + syntax + "/PbsTerraShadows",
				getMediaReadArchiveType(), true ) );
		hlmsPbs->reloadFrom( aPbs, &libPbs );
	}
	//-----------------------------------------------------------------------------


public:
	//  🌟 ctor  ----
	GameGraphicsSystem( App* app,
			GameState *gameState,
			String logCfgPath = String(""),
			String cachePath = String(""),
			String resourcePath = String(""),
			String pluginsPath = String("./") )
		: GraphicsSystem(
			app, gameState,
			logCfgPath,
			cachePath,
			resourcePath,
			pluginsPath )
		, mTerraWorkspaceListener( 0 )
	{
		mResourcePath = resourcePath;
	}
};


//  🌟 main App ctor
//-----------------------------------------------------------------------------
void MainEntryPoints::createSystems(
		App *app,
		GameState **outGraphicsGameState,
		GraphicsSystem **outGraphicsSystem,
		GameState **outLogicGameState,
		LogicSystem **outLogicSystem)
{
	std::cout << "Init :: pathmanager\n";
	PATHS::Init();
	std::cout << PATHS::info.str();

	std::cout << "Init :: new GraphicsSystem\n";

	GameGraphicsSystem *graphicsSystem = new GameGraphicsSystem(
		app, app,
		PATHS::UserConfigDir()+"/",
		PATHS::CacheDir()+"/",
		PATHS::GameConfigDir()+"/",
		String("./") );

	app->mGraphicsSystem = graphicsSystem;

	*outGraphicsGameState = app;
	*outGraphicsSystem = graphicsSystem;
}

//  💥 destroy
void MainEntryPoints::destroySystems(
		GameState *graphicsGameState,
		GraphicsSystem *graphicsSystem,
		GameState *logicGameState,
		LogicSystem *logicSystem)
{
	delete graphicsSystem;
	//- delete graphicsGameState;  // deleting app later, at end of main
}


//-----------------------------------------------------------------------------
const char* MainEntryPoints::getWindowTitle()
{
#ifdef SR_EDITOR
	return "Stunt Rally 3 Track Editor";
#else
	return "Stunt Rally 3";
#endif
}

//  🚀 main
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMainApp( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow )
#else
int mainApp( int argc, const char *argv[] )
#endif
{
	return MainEntryPoints::mainAppSingleThreaded( DEMO_MAIN_ENTRY_PARAMS );
	// return MainEntryPoints::mainAppMultiThreaded( DEMO_MAIN_ENTRY_PARAMS );  // todo ?
}
#endif
