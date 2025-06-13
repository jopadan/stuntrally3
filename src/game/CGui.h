#pragma once
#include "Gui_Def.h"
#include "settings.h"
#include "ChampsXml.h"  // progress
#include "ChallengesXml.h"
#include "CollectXml.h"
#include "CareerXml.h"
#include "CInput.h"

#include "SliderValue.h"
#include "MessageBoxStyle.h"
#include <MyGUI_Enumerator.h>
#include "ICSInputControlSystem.h"
#include <thread>
#include "peerinfo.hpp"
#include "networkcallbacks.hpp"
#include <boost/thread/mutex.hpp>

namespace Ogre {  class SceneNode;  class Root;  class SceneManager;  class RenderWindow;  class Viewport;  class Light;  }
namespace MyGUI{  class MultiList2;  class Slider;  class Message;  class PolygonalSkin;  }
namespace wraps {	class RenderBoxScene;  }
class Scene;  class SplineRoad;  class GAME;  class CHud;  class CData;
class CGuiCom;  class CarInfo;  class GuiPopup;  class App;


//  cars list items - with info for sorting
struct CarL
{
	std::string name;
	const CarInfo* ci;
};


class CGui : public BaseGui
	, public ICS::DetectingBindingListener
	, public GameClientCallback, public MasterClientCallback
{
public:
	App* app =0;  GAME* pGame =0;  SETTINGS* pSet =0;
	Scene* sc =0;  CData* data =0;
	CHud* hud =0;  MyGUI::Gui* mGui =0;  CGuiCom* gcom =0;

	CGui(App* ap1);
	~CGui();

	typedef std::list <std::string> strlist;


	//  🪧 Main Menu
	void InitMainMenu();
	void btnMainMenu(WP);  void tabMainMenu(Tab tab, size_t id);

	Cmb simList;  void comboSim(CMB);
	Cmb diffList;  void comboDiff(CMB);


	///  🎛️ Gui Init
	///-----------------------------------------------------------------------------------------------------------------

	bool bGI = 0;  // gui inited  set values
	void InitGui(), GuiUpdate();
	void InitGuiCar(), InitGuiChamps();

	std::vector<Tab> vSubTabsGame, vSubTabsOpts, vSubTabsMat;
	GuiPopup* popup =0;  // msg with edits

	//  Gui util
	void toggleGui(bool toggle=true);
	void GuiShortcut(EMenu menu, int tab, int subtab=-1, int game=-1);
	void UpdWndTitle();
	void tabGame(Tab, size_t);

	bool loadReadme = 1;  void FillHelpTxt();
	void ImgPrvClk(WP), ImgTerClk(WP), ImgPrvClose(WP);


	//  🚗 Car
	///-----------------------------------------------------------------------------------------------------------------

	MyGUI::IntCoord GetViewSize();

	WP graphV =0, graphS =0;
	MyGUI::PolygonalSkin* graphVel =0,*graphVGrid =0, *graphSSS =0,*graphSGrid =0;


	///  🎨 Car paint  ------
	CK(PaintAdj);  // adjust panel
	WP panPaintAdj =0, panPaintHSV =0, panPaintMix =0;
	CK(PaintNewLine);

	SlV(PaintType);  SlV(PaintRate);
	SlV(PaintFilter);  Txt txPaintFilter =0;
	Tab tbColorType =0;  void tabColorType(Tab, size_t);

	SV svPaintH, svPaintS, svPaintV;  // h s v sliders
	SV svPaintGloss, svPaintRough, svPaintFresnel;  // pbs
	SV svClearCoat, svClearCoatRough;  // refl
	SV svPaint1Mul, svPaint2Mul, svPaint3Mul;
	
	void slPaint(SV*);  // any paint slider
	void SldUpd_Paint(), UpdPaints();
	void UpdPaintSld(bool upd=true);

	void SetPaint(), UpdImgClr();

	Txt txPaintRgb =0;
	Img imgPaint =0, imgPaintCur =0;
	void imgBtnPaint(WP), btnPaintRandom(WP);
	// Btn btPaintRandom =0;

	Tbi tbPlrPaint =0;  Scv scvPaints =0;
	std::vector<Img> imgsPaint;
	std::vector<Txt> txtsPaint;
	
	void UpdPaintImgs(), UpdPaintCur();
	void btnPaintSave(WP), btnPaintLoad(WP), btnPaintLoadDef(WP);
	void btnPaintAdd(WP), btnPaintDel(WP);


	//  🔩 Setup car  ----
	Ck ckCarGear, ckCarRear, ckCarRearInv;  void chkGear(Ck*);
	Ck ckAbs, ckTcs;
	Btn bchAbs =0, bchTcs =0;
	void chkAbs(WP), chkTcs(WP);

	//  ⚫ gui car tire set gravel/asphalt
	int iTireSet = 0;
	void tabTireSet(Tab, size_t);
	void SldUpd_TireSet();
	bool bReloadSim = 1;

	SV svSSSEffect, svSSSVelFactor;  // sss
	SV svSteerRangeSurf, svSteerRangeSim;
	void btnSSSReset(WP), btnSteerReset(WP), slSSS(SV*);


	//  🚗📃 Car list  (all Car = Vehicle)
	///---------------------------------------------------
	const static int colCar[16],colCh[16],colChL[16],colSt[16],colCol[16];

	void CarListUpd(bool resetNotFound=false);
	void AddCarL(std::string name, const CarInfo* ci);
	std::list<CarL> liCar;
	void FillCarList();

	int iCurCar = 0;  // current
	Ogre::String sListCar;
	Img imgCar =0;  Ed carDesc =0;
	Tab tbPlr[2] = {0,0};

	Mli2 carList =0;
	void listCarChng(Mli2, size_t);
	void btnCarView1(WP), btnCarView2(WP);

	void changeCar(), changeTrack();

	//  🚗📉 Car stats
	const static int iCarSt = 10, iDrvSt = 3;
	Img barCarSt[iCarSt], barCarSpeed =0;
	Txt txCarStTxt[iCarSt], txCarStVals[iCarSt],
		txCarSpeed =0, txCarType =0, txCarYear =0,
		txCarRating =0, txCarDiff =0, txCarWidth =0,
		txCarAuthor =0, txTrackAuthor =0,
		txTrkDrivab[iDrvSt] = {0,0,0};
	Img imgTrkDrivab[iDrvSt] = {0,0,0};
	void UpdCarStats(bool car), UpdDrivability(std::string trk, bool track_user);


	///  🕹️ [Input] tab
	///-----------------------------------------------------------------------------------------------------------------

	//  bind events   . . . . .
	void keyBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  SDL_Keycode key,
		ICS::Control::ControlChangingDirection direction) override;

	void joystickAxisBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  int deviceId, int axis,
		ICS::Control::ControlChangingDirection direction) override;

	void joystickButtonBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  int deviceId, unsigned int button,
		ICS::Control::ControlChangingDirection direction) override;

	//  not needed
	void joystickPOVBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  int deviceId, int pov,
		ICS::InputControlSystem::POVAxis axis,
		ICS::Control::ControlChangingDirection direction) override
	{	return;  }

	void mouseAxisBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  ICS::InputControlSystem::NamedAxis axis,
		ICS::Control::ControlChangingDirection direction) override
	{	return;  }

	void mouseButtonBindingDetected(
		ICS::InputControlSystem* ICS, ICS::Control* control,  unsigned int button,
		ICS::Control::ControlChangingDirection direction) override
	{  return;  }
	
	//  init  ----
	void CreateInputTab( int iTab, bool player,
		const std::vector<InputAction>& actions, ICS::InputControlSystem* ICS);
	void InitInputGui();

	//  input gui  ----
	Tab tabInput =0;  void tabInputChg(Tab, size_t);
	Txt txtInpDetail =0;  WP panInputDetail =0;  Btn chOneAxis =0;
	Ed edInputIncrease =0;

	void editInput(Ed), btnInputInv(WP), chkOneAxis(WP);
	void comboInputKeyAllPreset(CMB);
	void UpdateInputBars(), inputDetailBtn(WP);
	bool TabInputId(int* pId);

	//  bind  ----
	void inputBindBtnClicked(WP), inputUnbind(WP), inputBindBtn2(WP, int, int, MyGUI::MouseButton mb);

	enum EBind {  B_Done=0, B_First, B_Second  };
	void UpdateInputButton(Btn button, const InputAction& action, EBind bind = B_Done);

	InputAction* mBindingAction =0;
	Btn mBindingSender =0;

	void notifyInputActionBound(bool complete);
	bool actionIsActive(std::string, std::string);


	///  🛠️ [Tweak]
	///-----------------------------------------------------------------------------------------------------------------

	const static int ciEdCar = 12;
	Ed edCar[ciEdCar] ={0,}, edPerfTest =0, edTweakCol =0;
	Txt txtTweakPath =0, txtTweakTire =0, txtTweakPathCol;

	Tab tabTweak =0, tabEdCar =0;
	void tabCarEdChng(Tab, size_t), tabTweakChng(Tab, size_t);

	///  ⚫ Tire
	Ed edTweakTireSet =0;  void editTweakTireSet(Ed);
	Li liTwkTiresUser =0, liTwkTiresOrig =0;
	void listTwkTiresUser(Li, size_t), listTwkTiresOrig(Li, size_t);
	void btnTweakTireLoad(WP), btnTweakTireReset(WP), btnTweakTireDelete(WP);
	void FillTweakLists();  Ogre::String sTireLoad;

	///  Surface
	Li liTwkSurfaces =0;  void listTwkSurfaces(Li, size_t);
	int idTwkSurf = -1;  void btnTwkSurfPick(WP), updSld_TwkSurf(int id);
	SV svSuFrict, svSuFrictX, svSuFrictY, svSuRollDrag, svSuRollRes;
	SV svSuBumpWave, svSuBumpAmp, svSuBumpWave2, svSuBumpAmp2;
	Cmb cmbSurfTire, cmbSurfType;  void comboSurfTire(CMB), comboSurfType(CMB);

	void TweakToggle();
	void TweakCarSave(),TweakCarLoad(), TweakTireSave();

	void TweakColUpd(bool user), TweakColLoad(),TweakColSave();

	const static Ogre::String csLateral[15][2], csLongit[13][2], csAlign[18][2], sCommon;

	void btnTweakCarSave(WP),  btnTweakCarLoad(WP);
	void btnTweakTireSave(WP), btnTweakColSave(WP);

	bool GetCarPath(std::string* pathCar/*out*/,
		std::string* pathSave/*=0*/, std::string* pathSaveDir/*=0*/,
		std::string carname, bool forceOrig=false);
	void comboDevTrk(CMB);


	//  📉 Graphs  ----------
	Cmb cmbGraphs =0;  void comboGraphs(CMB);  Txt valGraphsType =0;

	SV svTC_r, svTC_xr;
	SV svTE_yf, svTE_xfx, svTE_xfy, svTE_xpow;
	Ck ckTE_Common, ckTE_Reference;  void chkTEupd(Ck*);
	CK(Graphs);


	//  🆕 Startup  ----
	Ck ckBltLines, ckShowPics, ckDevKeys;
	//  📈 debug, other 🌐
	CK(Wireframe);  Ck ckHudOn;
	//  profiler
	Ck ckProfilerTxt, ckBulletDebug, ckBltProfTxt, ckSoundInfo;
	//  car dbg
	Ck ckTireVis;  //, ckCarDbgBars;
	Ck ckCarDbgTxt, ckCarDbgSurf;  void chkHudCreate(Ck*);
	SV svDbgTxtClr, svDbgTxtCnt;
	SV svCarPrv;
	
	SlV(SimQuality);  // game sim freqs
	Txt txSimDetail =0;
	

	/// 📊 Options  game only
	///-----------------------------------------------------------------------------------------------------------------
	
	CK(Reverse);  // 🏞️ track

	//  ✨ Particles, Trails
	SV svParticles, svTrails;
	Ck ckParticles, ckTrails;  void chkParTrl(Ck*);

	//  ⏲️ Hud view
	Ck ckDigits, ckGauges;  void chkHudShow(Ck*);
	SV svSizeGaug;
	SV svTypeGaug, svLayoutGaug;

	//  🔝 Arrow 📍
	CK(Arrow);  CK(Beam);
	SlV(SizeArrow);

	//  🌍 Minimap
	CK(Minimap);  void chkMiniUpd(Ck*);
	Ck ckMiniZoom, ckMiniRot, ckMiniTer, ckMiniBorder;
	SV svSizeMinimap, svZoomMinimap;
	void slHudSize(SV*), slHudCreate(SV*);

	//  🎥 Camera
	Ck ckCamInfo, ckCamTilt, ckCamLoop;
	Ck ckCamBnc;  SV svCamBnc;
	SV svFov, svFovBoost, svFovSm;
	
	//  🚦 Pacenotes
	Ck ckPaceShow;  SV svPaceDist, svPaceSize, svPaceNext;
	SV svPaceNear, svPaceAlpha;
	void slUpd_Pace(SV*);
	
	//  🎗️ Trail
	CK(TrailShow);
	
	//  ⏱️ Times, opp
	Ck ckTimes, ckOpponents, ckOppSort;
	SLV(CountdownTime);  //-

	//  radios
	Btn bRkmh =0, bRmph =0;  // km/h, mph
	void radKmh(WP), radMph(WP), radUpd(bool kmh);


	//  🔨 Game  setup  ----
	Ck ckVegetCollis, ckCarCollis, ckRoadWCollis, ckDynamicObjs, ckDriveOnHoriz;
	SV svNumLaps;  SLV(RplNumViewports);  //-
	SV svDamageDec;
	SV svBmin,svBmax,svBpow,svBperKm,svBaddSec;

	Cmb cmbBoost =0, cmbFlip =0, cmbDamage =0, cmbRewind =0;
	void comboBoost(CMB), comboFlip(CMB), comboDamage(CMB), comboRewind(CMB);


	///  📽️ Replay
	///-----------------------------------------------------------------------------------------------------------------

	Li rplList =0;
	void listRplChng(Li, size_t);
	void updReplaysList();

	//  cur rpl stats gui
	Txt valRplName =0, valRplInfo =0,
		valRplName2 =0,valRplInfo2 =0;

	//  controls percent and time info
	Txt valRplPerc =0, valRplCur =0, valRplLen =0;

	//  gui save
	Ed edRplName =0, edRplDesc =0;
	Ogre::String getRplName();
	void btnRplLoad(WP), btnRplSave(WP), btnRplLoadFile(std::string file);
	void btnRplDelete(WP), btnRplRename(WP);
	bool bLesson =0;

	//  chk, options
	Ck ckRplAutoRec, ckRplBestOnly, ckRplGhost, ckRplParticles;
	Ck ckRplRewind, ckRplGhostOther, ckRplTrackGhost;
	SV svGhoHideDist, svGhoHideDistTrk;
	Ck ckRplHideHudAids;

	//  list filtering
	Btn rbRplCur =0, rbRplAll =0;  // radio
	void btnRplCur(WP),btnRplAll(WP);
	CK(RplGhosts);
	void edRplFind(Ed);  Ogre::String sRplFind;

	//  controls bar buttons
	Btn btRplPl =0;  void UpdRplPlayBtn();
	Sl slRplPos =0;  void slRplPosEv(SL);
	bool bRplBack =0, bRplFwd =0;
	void btnRplToStart(WP),btnRplToEnd(WP), btnRplPlay(WP);
	void btnRplBackDn(WP,int,int,MyGUI::MouseButton), btnRplBackUp(WP,int,int,MyGUI::MouseButton);
	void btnRplFwdDn(WP,int,int, MyGUI::MouseButton), btnRplFwdUp(WP,int,int, MyGUI::MouseButton);
	void msgRplDelete(MyGUI::Message*, MyGUI::MessageBoxStyle);


	//  👈 Hints
	///---------------------------------------------------
	Ck ckShowWelcome;
	const static int iHints;  int iHintCur = 0;
	Ed edHintTitle =0, edHintText =0, rplSubText =0;
	Img imgHint =0, rplSubImg =0;

	void UpdHint(), setHintImg(Img img, int h);
	void btnHintPrev(WP), btnHintNext(WP);
	void btnHintScreen(WP), btnHintInput(WP), btnHintClose(WP);
	void btnHowToBack(WP), btnLesson(WP);

	struct Subtitle  // for replay lessons
	{
		std::string txt;
		float beg, end;  // time
		int hintImg;
		Subtitle(float beginTime, float endTime, int imgId, std::string text)
			:txt(text), beg(beginTime), end(endTime), hintImg(imgId)
		{	}
	};
	std::list<Subtitle> rplSubtitles;


	//  🏁 New Game
	///---------------------------------------------------
	Btn btNewGameCar =0;
	void btnNewGame(WP), btnNewGameStart(WP);

	//  👥 split screen
	void btnNumPlayers(WP), SetNumPlayers(int num);
	Txt valLocPlayers =0;
	Ck ckSplitVert;
	void chkStartOrd(WP);


	///  🏆 Championships, Challenges, Collections
	///-----------------------------------------------------------------------------------------------------------------

	Btn btStTut =0, btStChamp =0, btStChall =0, btStCollect =0;  // start
	Img imgTut =0, imgChamp =0, imgChall =0, imgCollect =0, imgCareer =0;  // ico
	Tab tabChamp =0, tabChall =0, tabCollect =0;  // tabs

	void tabChampType(Tab, size_t), tabChallType(Tab, size_t), tabCollectType(Tab, size_t);

	//  🏞️ stages
	#define ImgTrk 8
	Img imgTrk[ImgTrk] ={0,}, imgTrkBig =0;
	Txt txColDetail[2] ={0,0};

	Txt txTrkName =0;
	Ed edChDesc =0;  WP panCh =0;
	Txt txtCh =0, valCh =0, txtChName =0,
		txtChP[3] ={0,0,0}, valChP[3] ={0,0,0};  // stages info, pass/progress

	void btnStageNext(WP), btnStagePrev(WP);  Txt valStageNum =0;
	void StageListAdd(int n, Ogre::String name, int laps, Ogre::String progress);

	//  📄 games progress xmls
	ProgressXml progress[2];    // championship  [1]= reversed
	ProgressLXml progressL[2];  // challenge
	ProgressCXml progressC;     // collection
	ProgressCareer career;

	Chall* pChall =0;  // current challenge or 0 if not
	void ProgressSaveChamp(bool upgGui=true), ProgressSaveChall(bool upgGui=true);
	void ProgressSaveCollect(bool upgGui=true);
	void ProgressLoadChamp(), ProgressLoadChall(), ProgressLoadCollect();
	
	Ed edStats =0;  // game stats
	void FillGameStats(), btnCloseStats(WP);

	//  load
	void Ch_XmlLoad();
	void Ch_LoadEnd(), UpdChallDetail(int id);
	//  const
	const static Ogre::String StrPrize(int i/*0 none..3 gold*/), strPrize[4],clrPrize[4];
	const static int ciAddPos[3];  const static float cfSubPoints[3];

	//  📃 lists
	Mli2 liStages =0, liNetEnd =0;  void listStageChng(Mli2, size_t);
	Mli2 liChamps =0;  void listChampChng(Mli2, size_t);
	Mli2 liChalls =0;  void listChallChng(Mli2, size_t);
	Mli2 liCollect =0;  void listCollectChng(Mli2, size_t);

	//  🪟 windows
	void btnChampStart(WP), btnChampEndClose(WP), btnChampStageBack(WP), btnChampStageStart(WP);
	void btnChallStart(WP), btnChallEndClose(WP), btnChallStageBack(WP), btnChallStageStart(WP);
	void btnChRestart(WP);  Btn btChRestart =0, btColRestart =0;
	void btnCollectStart(WP);

	Btn btChampStage =0, btChallStage =0;
	Ed edChampStage =0, edChampEnd =0;  Img imgChampStage =0, imgChampEndCup =0;
	Ed edChallStage =0, edChallEnd =0;  Img imgChallStage =0;
	Img imgChallFail =0, imgChallCup =0;
	Txt txChallEndC =0, txChallEndF =0, txChampEndF =0;
	int iChSnd = 0;  // snd id to play

	//  games lists
	void ChampsListUpdate(), ChampFillStageInfo(bool finished), ChampionshipAdvance(float timeCur);
	void ChallsListUpdate(), ChallFillStageInfo(bool finished), ChallengeAdvance(float timeCur);
	void CollectListUpdate(), UpdCollectDetail(int id);
	void FillChampsList(std::vector<int> vIds), FillChallsList(std::vector<int> vIds);  // Ids from champs/challs .all
	void FillCollectList(std::vector<int> vIds);

	Ed edGameInfo =0;  void btnGameInfo(WP), updGameInfo(), UpdGamesTabVis();
	CK(ChampRev);  CK(Ch_All);

	void ReadTrkStatsChamp(Ogre::String track,bool reverse);
	void updGamesListDim();

	//  chall util
	bool IsChallCar(Ogre::String name), IsCollectCar(Ogre::String name);
	bool isChampGui(), isChallGui(), isCollectGui();  void BackFromChs();


	//  🧰 _Tools_  cmd arg
	void ToolGhosts(),ToolGhostsConv(), ToolTestTrkGhosts();

	//  ⛓️ key util
	int LNext(Mli2, int rel, int ofs), LNext(Li, int rel, int ofs),
		LNext(Mli, int rel);  // util next in list
	void LNext(int rel);  void tabPlayer(Tab, size_t);

	const Ogre::String& GetGhostFile(std::string* ghCar=NULL);
	std::string GetRplListDir();


	///  👥 Multiplayer game
	///-----------------------------------------------------------------------------------------------------------------
	void rebuildGameList(), rebuildPlayerList();
	void updateGameInfo(), updateGameSet(), updateGameInfoGUI();
	void setNetGuiHosting(bool enabled);
	void gameListChanged(protocol::GameList list) override;

	void peerConnected(PeerInfo peer) override, peerDisconnected(PeerInfo peer) override;
	void peerInfo(PeerInfo peer) override;
	void peerMessage(PeerInfo peer, std::string msg) override;
	void peerState(PeerInfo peer, uint8_t state) override;
	void gameInfo(protocol::GameInfo game) override;
	void startRace() override, returnToLobby() override;
	void timeInfo(ClientID id, uint8_t lap, double time) override;
	void error(std::string what) override;
	void join(std::string host, std::string port, std::string password);
	void uploadGameInfo();

	mutable boost::mutex netGuiMutex;
	protocol::GameInfo netGameInfo;

	///  multiplayer gui  --------------------
	Tab tabsNet =0;
	WP  panNetServer =0,panNetServer2 =0, panNetGame =0, panNetTrack =0;
	Mli listServers =0, listPlayers =0;
	int iColLock =0, iColHost =0, iColPort =0;  // ids of columns in listServers

	//  upd gui triggers
	bool bRebuildPlayerList =0, bRebuildGameList =0;
	bool bUpdateGameInfo =0, bUpdChat =0;
	bool bStartGame =0, bStartedGame =0;
	void UpdGuiNetw();

	//  chat,msg  ----
	Ed edNetChat =0;  // chat area, set text through sChatBuffer
	MyGUI::UString sChatBuffer,sChatLast1,sChatLast2;  int iChatMove = 0;
	void AddChatMsg(const MyGUI::UString& clr, const MyGUI::UString& msg, bool add=true);

	Ed edNetChatMsg =0;
	Btn btnNetSendMsg =0;  void chatSendMsg();

	//  Net gui
	Btn btnNetRefresh =0, btnNetJoin =0;    void evBtnNetRefresh(WP), evBtnNetJoin(WP),   evBtnNetJoinLockedClose();
	Btn btnNetCreate =0,  btnNetDirect =0;  void evBtnNetCreate(WP),  evBtnNetDirect(WP), evBtnNetDirectClose();
	Btn btnNetReady =0,   btnNetLeave =0;   void evBtnNetReady(WP),   evBtnNetLeave(WP);
	void btnNetEndClose(WP);

	Txt valNetGameInfo =0, valNetPassword =0;
	Ed edNetGameName =0,   edNetPassword =0;   void evEdNetGameName(Ed),   evEdNetPassword(Ed);
	Ed edNetNick =0,       edNetLocalPort =0;  void evEdNetNick(Ed),       evEdNetLocalPort(Ed);
	Ed edNetServerPort =0, edNetServerIP =0;   void evEdNetServerPort(Ed), evEdNetServerIP(Ed);


	//  fonts
	SV svFntWnd, svFntGui, svFntHud, svFntTimes;

	//  🔗 open urls  ----
	Ed edOpenUrl =0;
	void Url(const std::string& url);

	void btnTrackEditor(WP);  // start ed exe
	void btnWelcome(WP), btnWebsite(WP), btnWiki(WP), btnWikiInput(WP);
	void btnOpenChat(WP), btnSources(WP), btnEdTut(WP), btnTransl(WP), btnDonations(WP);
};
