#pragma once
// #include "Gui_Def.h"
#include "BaseApp.h"
#include "Replay.h"
#include "TrackGhost.h"
#include "cardefs.h"
#include "CarPosInfo.h"
#include "PreviewTex.h"

#include <thread>
#include "ICSChannelListener.h"
#include <OgreHlmsDatablock.h>
#include <OgrePrerequisites.h>

namespace Ogre {  class SceneNode;  class SceneManager;  class TextureGpu;  class Root;
	namespace v1 {  class TextAreaOverlayElement;  }
	class Terra;  class HlmsPbsTerraShadows;  class HlmsDatablock;  }
namespace BtOgre  {  class DebugDrawer;  }
class Scene;  class CScene;  class CData;  class CInput;  class GraphView;
class GAME;  class CHud;  class CGui;  class CGuiCom;


class App : public BaseApp,	public ICS::ChannelListener
{
	//  vars
	//  input  ----
	bool keyUp = false, keyDown = false;  // arrows for gui lists
	bool keyPgUp = false, keyPgDown = false;
	bool keyMul = false, keyDiv = false;  // tide edit

public:
	void InitGui();

	//  SR
	void Init(), Load();
	void Destroy();


	//  main
	void createScene01() override;
	void destroyScene() override;

	void update( float timeSinceLast ) override;

	//  events
	void keyPressed( const SDL_KeyboardEvent &arg ) override;
	void keyReleased( const SDL_KeyboardEvent &arg ) override;


	//  🏔️ mtr ids, from ter  . . . 
	int blendMapSize = 512;
	std::vector<char> blendMtr;  // 1st ter only-  // todo
	//; void GetTerMtrIds();  // todo:


public:
	//  🌟 ctor  ----
	App();
	virtual ~App();
	void ShutDown();

	//  scene data
	CData* data =0;
	Scene* sc = 0;
	
	GAME* pGame =0;


	///  🚗 Cars pos Game data  ----------------
	//  new positions info for every CarModel
	PosInfo carPoses[CarPosCnt][MAX_CARS];  // max 16cars
	int iCurPoses[MAX_CARS];  // current index for carPoses queue
	std::map<int,int> carsCamNum;  // picked camera number for cars
	
	void newPoses(float time), newPerfTest(float time);  // vdrift
	void updatePoses(float time);  // ogre
	void UpdThr();
	std::thread* mThread =0;  // 2nd thread for simulation

	void updCarLightsBright();

	//  📽️ Replays  ----------------
	//  replay - full, saved by user
	//  ghost  - saved on best lap
	//  ghPlay - ghost ride replay, loaded if was on disk, replaced when new
	//  ghTrk  - track's ghost
	Replay2 replay, ghost, ghPlay;
	Rewind rewind;  // to take car back in time (after crash etc.)
	TrackGhost ghTrk;

	//  frm - used when playing replay for hud and sounds
	std::vector<ReplayFrame2> frm;  //size:16

	bool isGhost2nd = 0;  // if present (ghost but from other car)
	std::vector<float> vTimeAtChks;  // track ghost's ⏱️ times at road 🔵 checkpoints
	float fLastTime = 1.f;  // trk ghost total time
		

	///  ⏱️ HUD  ----------------
	CHud* hud = 0;
	bool bSizeHUD = true;

	bool isTweakTab();
	BtOgre::DebugDrawer *dbgdraw = 0;  /// blt dbg

	float fLastFrameDT = 0.001f;
	void DoNetworking();


	///  🆕 Create  . . . . . . . . . . . . . . . . . . . . . . . . 
	Ogre::String resCar, resTrk, resDrv;
	void CreateCar(), CreateRoads(), CreateRoadsInt();

	//  📦🏢 Objects
	void CreateObjects(), DestroyObjects(bool clear), ResetObjects();

	//  💎 Collectibles
	void CreateCollects(), DestroyCollects(bool clear), ResetCollects();
	void CreateCollect(int i), DestroyCollect(int i);

	int iCollected = 0, oldCollected = 0;
	int iCollectedPrize = -2;  // -2 ongoing -1 none  0 bronze ..
	void UpdCollects();

	//  🎆 Fields
	void CreateFields(), DestroyFields(bool clear);
	void CreateField(int i), DestroyField(int i);


	//  New Game  ----
	void NewGame(bool force=false, bool perfTest=false);
	void NewGameDoLoad();

	void LoadData();
	bool newGameRpl = 0;

	bool dstTrk = 1;  // destroy track, false if same
	Ogre::String oldTrack;  bool oldTrkUser = 0;

	
	//  ⏳ Loading  states  ----------------
	bool bLoading = 0, bLoadingEnd = 0, bSimulating = 0;  int iLoad1stFrames = 0;
	void LoadCleanUp(), LoadGame(), LoadScene(), LoadCar(),
		LoadTerrain(), LoadRoad(), LoadObjects(), LoadTrees(), 
		LoadView(int c), LoadMisc(),
		AddListenerRnd2Tex();

	enum ELoadState
	{	LS_CLEANUP=0, LS_GAME, LS_SCENE, LS_CAR,
		LS_TERRAIN, LS_ROAD, LS_OBJECTS, LS_TREES,
		LS_VIEW0, LS_VIEW1, LS_VIEW2, LS_VIEW3, LS_VIEW4, LS_VIEW5, LS_VIEW6, LS_VIEW7, LS_VIEW8,
		LS_MISC, LS_ALL  };
	static Ogre::String cStrLoad[LS_ALL+1];
	int curLoadState = 0;
	std::map<int, std::string> loadingStates;

	// float mTimer = 0.f;  // todo: wind,water

	// void recreateReflections();  // call after refl_mode changed
	bool bRecreateFluidsRTT =0;


	//  🕹️ Input  ----------------
	CInput* input =0;

	void channelChanged(ICS::Channel *channel, float currentValue, float previousValue) override;


	//  🎛️ Gui
	//-----------------------------------------------------------------
	PreviewTex prvView,prvRoad,prvTer;  // track tab
	PreviewTex prvStCh;  // champ,chall stage view

	bool bHideHudBeam  =0;  // hides beam when replay or no road
	bool bHideHudArr   =0;	// hides arrow when replay
	bool bHideHudPace  =0;  // hides pacenotes when same or deny by challenge
	bool bHideHudTrail =0;  // hides trail if denied by challenge
	
	bool bRplPlay =0, bRplPause =0, bRplRec =0, bRplWnd =1;  //  game
	int carIdWin =-1, iRplCarOfs =0, iRplSkip =0;

	//  race pos
	int GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints=0);
	float GetCarTimeMul(const std::string& car, const std::string& sim_mode);

	void Ch_NewGame();


	///  📉 Graphs  ~^.
	std::vector<GraphView*> graphs;
	void CreateGraphs(), DestroyGraphs();
	void UpdateGraphs(), GraphsNewVals();

	///  ⚫📉 tire edit
	const static int TireNG;
	int iEdTire = 0, iTireLoad = 0, iUpdTireGr = 0;
	int iCurLat = 0, iCurLong = 0, iCurAlign = 0;
	bool updateTireEdit();

	///  car perf test
	bool bPerfTest = 0;
	EPerfTest iPerfTestStage = PT_StartWait;
	void PerfLogVel(class CAR* pCar, float time);
};
