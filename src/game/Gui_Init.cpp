#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "paths.h"
#include "CGame.h"
#include "CGui.h"
#include "MultiList2.h"
#include "Slider.h"
#include "Gui_Popup.h"
#include "TracksXml.h"

#include <OgreOverlay.h>
#include <OgreWindow.h>
#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace MyGUI;
//using namespace Ogre;
using namespace std;


///  🎛️ Gui Init
//---------------------------------------------------------------------------------------------------------------------

void CGui::InitGui()
{
	LogO(Ogre::String(":::# Init Gui ----"));
	mGui = app->mGui;
	gcom->mGui = mGui;
	Check::pGUI = mGui;  SliderValue::pGUI = mGui;
	Check::bGI = &bGI;   SliderValue::bGI = &bGI;

	popup->mGui = mGui;
	popup->mPlatform = app->mPlatform;

	if (!mGui)  return;
	Ogre::Timer ti;
	int i;

	//  new widgets
	FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	FactoryManager::getInstance().registerFactory<Slider>("Widget");


	///  Load .layout files
	auto Load = [&](string file)
	{
		auto v = LayoutManager::getInstance().loadLayout(file + ".layout");
		app->vwGui.insert(app->vwGui.end(), v.begin(), v.end());
	};
	Load("Common");  Load("MaterialEditor");
	Load("Game_Main");  Load("Game");  Load("Game_Utils");
	Load("Game_Help");  Load("Game_Options");
	Load("Game_Replay");  Load("Game_Tweak");


	//  🪟 main windows  [----]
	app->mWMainMenu = fWnd("MainMenuWnd");  // main menus 3 levels
	app->mWMainSetup = fWnd("MainSetupWnd");
	app->mWMainGames = fWnd("MainGamesWnd");
	app->mWndGameInfo = fWP("GameInfoWnd");

	app->mWndGame = fWnd("GameWnd");  app->mWndReplays = fWnd("ReplaysWnd");
	app->mWndHowTo = fWnd("HowToPlayWnd");
	app->mWndHelp = fWnd("HelpWnd");  app->mWndOpts = fWnd("OptionsWnd");  // common


	//  dialog wnds  --[]--
	app->mWndMaterials = fWnd("MaterialsWnd");  app->mWndTrkFilt = fWnd("TrackFilterWnd");
	app->mWndWelcome = fWnd("WelcomeWnd");
	app->mWndStats = fWnd("StatsWnd");  app->mWndStats->setVisible(false);

	app->mWndChampStage = fWnd("WndChampStage");  app->mWndChampStage->setVisible(false);
	app->mWndChampEnd   = fWnd("WndChampEnd");    app->mWndChampEnd->setVisible(false);
	app->mWndChallStage = fWnd("WndChallStage");  app->mWndChallStage->setVisible(false);
	app->mWndChallEnd   = fWnd("WndChallEnd");    app->mWndChallEnd->setVisible(false);

	app->mWndNetEnd = fWnd("WndNetEnd");  app->mWndNetEnd->setVisible(false);
	app->mWndTweak = fWnd("WndTweak");    app->mWndTweak->setVisible(false);
	app->mWndTweak->setPosition(0,90);

	//  center dialog wnds  --[]--
	const int wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();
	IntSize ws;
	#define center(wnd)  ws = wnd->getSize();  wnd->setPosition((wx - ws.width) / 2, (wy - ws.height) / 2);
	center(app->mWndChampStage)  center(app->mWndChampEnd)
	center(app->mWndChallStage)  center(app->mWndChallEnd)
	center(app->mWndNetEnd)


	//  for find defines
	Btn btn, bchk;  Cmb cmb;
	Slider* sl;  SV* sv;  Ck* ck;


	//  Tabs  --------
	Tab tab,sub;
	fTabW("TabWndGame");     app->mTabsGame = tab;  Tev(tab, Game);
	fTabW("TabWndReplays");  app->mTabsRpl = tab;
	fTabW("TabWndOptions");  app->mTabsOpts = tab;
	fTabW("TabWndHelp");     app->mTabsHelp = tab;
	fTabW("TabWndMat");      app->mTabsMat = tab;

	//  get sub tabs
	vSubTabsGame.clear();
	for (i=0; i < app->mTabsGame->getItemCount(); ++i)
	{
		sub = (Tab)gcom->FindSubTab(app->mTabsGame->getItemAt(i));
		vSubTabsGame.push_back(sub);
	}
	vSubTabsOpts.clear();
	for (i=0; i < app->mTabsOpts->getItemCount(); ++i)
	{
		sub = (Tab)gcom->FindSubTab(app->mTabsOpts->getItemAt(i));
		vSubTabsOpts.push_back(sub);
	}
	vSubTabsMat.clear();
	for (i=0; i < app->mTabsMat->getItemCount(); ++i)
	{
		sub = (Tab)gcom->FindSubTab(app->mTabsMat->getItemAt(i));
		vSubTabsMat.push_back(sub);
	}

	if (pSet->iMenu >= MN_Tutorial && pSet->iMenu <= MN_Chall)
		app->mTabsGame->setIndexSelected(TAB_Champs);


	//  replay  ----
	app->mWndRpl = fWnd("RplControlsWnd");
	app->mWndRplTxt = fWnd("RplLessonTextWnd");
	rplSubText = fEd("RplLessonText");  rplSubImg = fImg("RplLessonImg");


	///  🎛️ Gui common init  ---
	InitMainMenu();
	gcom->GuiInitAll();


	toggleGui(false);
	app->updMouse();
	gcom->bnQuit->setVisible(app->isFocGui);


	///  👈 Welcome, HowToPlay, Hints  ----
	ck= &ckShowWelcome;  ck->Init("chkHintShow", &pSet->show_welcome);

	edHintTitle = fEd("HintTitle");  edHintText = fEd("HintText");
	imgHint = fImg("HintImg");  UpdHint();

	Btn("btnHintPrev", btnHintPrev);     Btn("btnHintNext", btnHintNext);
	Btn("btnHintScreen",btnHintScreen);  Btn("btnHintInput", btnHintInput);
	Btn("btnHintClose", btnHintClose);

	app->mWndWelcome->setVisible(pSet->show_welcome && !pSet->autostart);

	//  How to play  >> >
	Btn("btnHowToBack", btnHowToBack);
	for (int i=1; i <= 7; ++i)
	{	Btn("BtnLesson"+toStr(i), btnLesson);  }

	edStats = fEd("EdStats");  // game stats
	Btn("CloseStats", btnCloseStats);
	FillGameStats();


	///  🎚️ Sliders
	//------------------------------------------------------------------------

	//  ⏲️ Hud view sizes  ----
	sv= &svSizeGaug;	sv->Init("SizeGaug",	&pSet->size_gauges,    0.1f, 0.3f,  1.f, 3,4);  sv->DefaultF(0.19f);  Sev(HudSize);
	sv= &svTypeGaug;	sv->Init("TypeGaug",	&pSet->gauges_type,    0, 5);  sv->DefaultI(5);  Sev(HudCreate);
	//sv= &svLayoutGaug;	sv->Init("LayoutGaug",	&pSet->gauges_layout,  0, 2);  sv->DefaultI(1);  Sev(HudCreate);

	sv= &svSizeMinimap;	sv->Init("SizeMinimap",	&pSet->size_minimap,   0.05f, 0.3f, 1.f, 3,4);  sv->DefaultF(0.165f);  Sev(HudSize);
	sv= &svZoomMinimap;	sv->Init("ZoomMinimap",	&pSet->zoom_minimap,   0.9f, 4.f,   1.f, 2,4);  sv->DefaultF(1.6f);    Sev(HudSize);
	sv= &svSizeArrow;	sv->Init("SizeArrow",   &pSet->size_arrow,     0.1f, 0.5f,  1.f, 3,4);  sv->DefaultF(0.26f);  Sev(SizeArrow);
	Slv(CountdownTime,  pSet->gui.pre_time / 0.5f /10.f);  sl->mfDefault = 4.f /10.f;


	//  📊 Options  ----
	sv= &svParticles;	sv->Init("Particles",	&pSet->particles_len, 0.f, 4.f, 2.f);  sv->DefaultF(1.5f);
	sv= &svTrails;		sv->Init("Trails",		&pSet->trails_len,    0.f, 4.f, 2.f);  sv->DefaultF(3.f);


	///  ✅ View Checks
	//------------------------------------------------------------------------
	ck= &ckReverse;		ck->Init("ReverseOn",	&pSet->gui.track_reversed);  Cev(Reverse);

	//  📊 Options  ----
	ck= &ckParticles;	ck->Init("ParticlesOn", &pSet->particles);   Cev(ParTrl);
	ck= &ckTrails;		ck->Init("TrailsOn",    &pSet->trails);      Cev(ParTrl);

	//  ⏱️ Hud  ----
	ck= &ckDigits;		ck->Init("Digits",      &pSet->show_digits);   Cev(HudShow);
	ck= &ckGauges;		ck->Init("Gauges",      &pSet->show_gauges);   Cev(HudShow);

	ck= &ckArrow;		ck->Init("Arrow",       &pSet->check_arrow);   Cev(Arrow);
	ck= &ckBeam;		ck->Init("ChkBeam",     &pSet->check_beam);    Cev(Beam);

	//  🌍 minimap
	ck= &ckMinimap;		ck->Init("Minimap",     &pSet->trackmap);      Cev(Minimap);
	// ck= &ckMiniZoom;	ck->Init("MiniZoom",    &pSet->mini_zoomed);   Cev(MiniUpd);  // fixme
	ck= &ckMiniRot;		ck->Init("MiniRot",     &pSet->mini_rotated);
	// ck= &ckMiniTer;		ck->Init("MiniTer",     &pSet->mini_terrain);  Cev(MiniUpd);  // todo
	// ck= &ckMiniBorder;	ck->Init("MiniBorder",  &pSet->mini_border);   Cev(MiniUpd);

	//  🎥 camera
	ck= &ckCamInfo;		ck->Init("CamInfo",     &pSet->show_cam);   Cev(HudShow);
	ck= &ckCamTilt;		ck->Init("CamTilt",     &pSet->cam_tilt);
	ck= &ckCamLoop;		ck->Init("CamLoop",     &pSet->cam_loop_chng);

	ck= &ckCamBnc;		ck->Init("CamBounce",   &pSet->cam_bounce);
	sv= &svCamBnc;		sv->Init("CamBnc",		&pSet->cam_bnc_mul, 0.f, 2.f);

	sv= &svFov;			sv->Init("Fov",			&pSet->fov_min,   50.f, 180.f, 1.f, 1,4);  sv->DefaultF(90.f);
	sv= &svFovBoost;	sv->Init("FovBoost",	&pSet->fov_boost,  0.f, 15.f, 1.f, 1,4);  sv->DefaultF(5.f);
	sv= &svFovSm;		sv->Init("FovSm",		&pSet->fov_smooth, 0.f, 15.f, 1.5f);  sv->DefaultF(5.f);

	//  🚦 pacenotes
	ck= &ckPaceShow;	ck->Init("ChkPace",		&pSet->pace_show);
	sv= &svPaceNext;	sv->Init("PaceNext",	&pSet->pace_next,   2,8);  sv->DefaultI(4);
	sv= &svPaceDist;	sv->Init("PaceDist",	&pSet->pace_dist,   20.f,1000.f, 2.f, 0,3);  sv->DefaultF(200.f);

	sv= &svPaceSize;	sv->Init("PaceSize",	&pSet->pace_size,   0.1f,2.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	sv= &svPaceNear;	sv->Init("PaceNear",	&pSet->pace_near,   0.1f,2.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	sv= &svPaceAlpha;	sv->Init("PaceAlpha",	&pSet->pace_alpha,  0.3f,1.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	//slUpd_Pace(0);
	ck= &ckTrailShow;	ck->Init("ChkTrail",	&pSet->trail_show);  Cev(TrailShow);


	//  kmh/mph radio
	Btn("kmh", radKmh);  bRkmh = btn;  bRkmh->setStateSelected(!pSet->show_mph);
	Btn("mph", radMph);	 bRmph = btn;  bRmph->setStateSelected( pSet->show_mph);

	//  ⏱️ Times, opp
	ck= &ckTimes;		ck->Init("Times",       &pSet->show_times);      Cev(HudShow);
	// ck= &ckOpponents;	ck->Init("Opponents",   &pSet->show_opponents);  Cev(HudShow);
	// ck= &ckOppSort;		ck->Init("OppSort",     &pSet->opplist_sort);


	//  📉 Graphs  ------------------------------------------------------------
	valGraphsType = fTxt("GraphsTypeVal");
	Cmb(cmb, "CmbGraphsType", comboGraphs);  cmbGraphs = cmb;
	if (cmb)
	{	cmb->removeAllItems();
		for (i=0; i < Gh_ALL; ++i)
			cmb->addItem(csGraphNames[i]);
		cmb->setIndexSelected(pSet->graphs_type);
	}
	valGraphsType->setCaption(toStr(pSet->graphs_type));


	//  📈 debug, other 🌐  ------------------------------------------------------------
	ck= &ckWireframe;	ck->Init("Wireframe",   &app->bWireframe);  Cev(Wireframe);
	ck= &ckHudOn;		ck->Init("HudOn",       &pSet->hud_on);

	sv= &svDbgTxtCnt;	sv->Init("DbgTxtCnt",	&pSet->car_dbgtxtcnt, 0, 8);
	ck= &ckProfilerTxt;	ck->Init("ProfilerTxt", &pSet->profilerTxt);  Cev(HudShow);
	ck= &ckSoundInfo;	ck->Init("SoundInfo",   &pSet->sounds_info);

	ck= &ckBulletDebug;	ck->Init("BulletDebug", &pSet->bltDebug);
	ck= &ckBltLines;	ck->Init("BltLines",	&pSet->bltLines);
 	ck= &ckBltProfTxt;	ck->Init("BltProfTxt",  &pSet->bltProfilerTxt);  // fixme?

 // ck= &ckCarDbgBars;	ck->Init("CarDbgBars",  &pSet->car_dbgbars);   Cev(HudShow);  //-
	ck= &ckCarDbgTxt;	ck->Init("CarDbgTxt",   &pSet->car_dbgtxt);    Cev(HudShow);
	ck= &ckCarDbgSurf;	ck->Init("CarDbgSurf",  &pSet->car_dbgsurf);   Cev(HudShow);

 	ck= &ckTireVis;		ck->Init("CarTireVis",  &pSet->car_tirevis);   Cev(HudCreate);
	ck= &ckGraphs;		ck->Init("Graphs",		&pSet->show_graphs);   Cev(Graphs);

	sv= &svDbgTxtClr;	sv->Init("DbgTxtClr",	&pSet->car_dbgtxtclr, 0, 1);  //-
	sv= &svDbgTxtCnt;	sv->Init("DbgTxtCnt",	&pSet->car_dbgtxtcnt, 0, 8);


	//  🔧 Tweak  ------------------------------------------------------------
	ck= &ckDevKeys;		ck->Init("DevKeys",		&pSet->dev_keys);
	sv= &svCarPrv;		sv->Init("CarPrv",		&gPar.carPrv, 0, 3);

	//  quick Tracks  --------
	vector<string> vt;
	for (const auto& ti : data->tracks->trks)
		// if (ti.test || ti.testC)  // only Test-
		{	auto sc = gcom->GetSceneryColor(ti.name);
			vt.push_back(sc + ti.name);
		}

	const char* ch = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXY";  // no Z
	int len = strlen(ch), h = app->mWindow->getHeight()/2;   // can go too much- 630-
	for (int i=0; i < len; ++i)
	{
		string s = "DevTrk";
		char c = ch[i];
		s += string{c};

		Cmb(cmb, s, comboDevTrk);
		cmb->setMaxListLength(h);

		cmb->addItem("");
		for (const auto& t : vt)
			cmb->addItem(t);
	
		auto f = pSet->dev_tracks.find(c);
		if (f != pSet->dev_tracks.end())
		{	string t = (*f).second;  auto sc = gcom->GetSceneryColor(t);
			cmb->setCaption(sc + t);
	}	}

	//  simulation
	txSimDetail = fTxt("SimDetail");
	sv= &svSimQuality;
		sv->strMap[0] = TR("#{GraphicsAll_Low}");   sv->strMap[1] = TR("#{GraphicsAll_Medium}");
		sv->strMap[2] = TR("#{GraphicsAll_High}");  sv->strMap[3] = TR("#{GraphicsAll_Ultra}");
							sv->Init("SimQuality",	&pSet->g.sim_quality, 0,3);  Sev(SimQuality);
	slSimQuality(0);


	//  🔨 Game  ------------------------------------------------------------
	ck= &ckVegetCollis;		ck->Init("VegetCollis",		&pSet->gui.collis_veget);
	ck= &ckCarCollis;		ck->Init("CarCollis",		&pSet->gui.collis_cars);
	ck= &ckRoadWCollis;		ck->Init("RoadWCollis",		&pSet->gui.collis_roadw);
	ck= &ckDynamicObjs;		ck->Init("DynamicObjects",	&pSet->gui.dyn_objects);
	ck= &ckDriveOnHoriz;	ck->Init("DriveOnHorizons",	&pSet->gui.drive_horiz);

	Cmb(cmb, "CmbBoost", comboBoost);	cmbBoost = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{Never}"));		cmb->addItem(TR("#{FuelLap}"));
		cmb->addItem(TR("#{FuelTime}"));	cmb->addItem(TR("#{Always}"));
		cmb->setIndexSelected(pSet->gui.boost_type);

	Cmb(cmb, "CmbFlip", comboFlip);		cmbFlip = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{Never}"));		cmb->addItem(TR("#{FuelBoost}"));
		cmb->addItem(TR("#{Always}"));
		cmb->setIndexSelected(pSet->gui.flip_type);

	Cmb(cmb, "CmbDamage", comboDamage);	cmbDamage = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{None}"));		cmb->addItem(TR("#{Reduced}"));
		cmb->addItem(TR("#{Normal}"));
		cmb->setIndexSelected(pSet->gui.damage_type);

	Cmb(cmb, "CmbRewind", comboRewind);	cmbRewind = cmb;
		cmb->removeAllItems();
		cmb->addItem(TR("#{None}"));		cmb->addItem(TR("#{Always}"));
		cmb->addItem(TR("#{GoBackTime}"));
		//cmb->addItem(TR("#{FuelLap}"));		cmb->addItem(TR("#{FuelTime}"));
		cmb->setIndexSelected(pSet->gui.rewind_type);

	sv= &svDamageDec;	sv->Init("DamageDec",	&pSet->gui.damage_dec, 0.f, 100.f, 1.f, 0,1, 1.f, " %");  sv->DefaultF(40.f);
	sv= &svBmin;	sv->Init("Bmin",	&pSet->gui.boost_min,     0.f, 10.f,1.f, 1,3);  sv->DefaultF(2.f);
	sv= &svBmax;	sv->Init("Bmax",	&pSet->gui.boost_max,     2.f, 20.f,1.f, 1,3);  sv->DefaultF(11.f);
	sv= &svBpow;	sv->Init("Bpow",	&pSet->gui.boost_power,   0.f, 2.5f,1.f, 1,3);  sv->DefaultF(0.8f);
	sv= &svBperKm;	sv->Init("BperKm",	&pSet->gui.boost_per_km,  0.f, 4.f, 1.f, 1,3);  sv->DefaultF(1.f);
	sv= &svBaddSec;	sv->Init("BaddSec",	&pSet->gui.boost_add_sec, 0.f, 1.f, 1.f, 2,4);  sv->DefaultF(0.1f);


	//  👥 Split screen  ------------------------------------------------------------
	for (int i=0; i < MAX_Players; ++i)
	{	Btn("btnPlayers"+toStr(i+1), btnNumPlayers);  }

	tbPlr[0] = fTab("SubTabPlayer");   Tev(tbPlr[0], Player);
	tbPlr[1] = fTab("SubTabPlayer2");  Tev(tbPlr[1], Player);
	ck= &ckSplitVert;	ck->Init("chkSplitVertically",  &pSet->split_vertically);
	btnNumPlayers(0);  // hide tabs

	Chk("chkStartOrderRev", chkStartOrd, pSet->gui.start_order);
	valLocPlayers = fTxt("valLocPlayers");
	if (valLocPlayers)
		valLocPlayers->setCaption(toStr(pSet->gui.local_players));

	sv= &svNumLaps;  sv->Init("NumLaps",  &pSet->gui.num_laps, 1,10, 1.3f);  sv->DefaultI(2);


	//  🆕 Startup  ------------------------------------------------------------
	// ck= &ckShowPics;	ck->Init("ShowPictures",&pSet->loadingbackground);


	//  📽️ Replays  ------------------------------------------------------------
	Btn("RplLoad",   btnRplLoad);    Btn("RplSave",   btnRplSave);
	Btn("RplDelete", btnRplDelete);  Btn("RplRename", btnRplRename);
	//  settings
	ck= &ckRplAutoRec;		ck->Init("RplChkAutoRec",	 &app->bRplRec);
	ck= &ckRplBestOnly;		ck->Init("RplChkBestOnly",	 &pSet->rpl_bestonly);
	ck= &ckRplGhost;		ck->Init("RplChkGhost",		 &pSet->rpl_ghost);
	ck= &ckRplParticles;	ck->Init("RplChkParticles",	 &pSet->rpl_ghostpar);

	ck= &ckRplRewind;		ck->Init("RplChkRewind",	 &pSet->rpl_ghostrewind);
	ck= &ckRplGhostOther;	ck->Init("RplChkGhostOther", &pSet->rpl_ghostother);
	ck= &ckRplTrackGhost;	ck->Init("RplChkTrackGhost", &pSet->rpl_trackghost);
	Slv(RplNumViewports, (pSet->rpl_numViews-1) / 3.f);
	sv= &svGhoHideDist;		sv->Init("GhoHideDist",		 &pSet->ghoHideDist,    0.f, 30.f, 1.5f, 1,4);  sv->DefaultF(5.f);
	sv= &svGhoHideDistTrk;	sv->Init("GhoHideDistTrk",	 &pSet->ghoHideDistTrk, 0.f, 30.f, 1.5f, 1,4);  sv->DefaultF(5.f);
	ck= &ckRplParticles;	ck->Init("RplChkParticles",	 &pSet->rpl_ghostpar);
	ck= &ckRplHideHudAids;  ck->Init("RplChkHideHudAids",&pSet->rpl_hideHudAids);

	//  radios, filter
	ck= &ckRplGhosts;	ck->Init("RplBtnGhosts",  &pSet->rpl_listghosts);  Cev(RplGhosts);
	Btn("RplBtnAll", btnRplAll);  rbRplAll = btn;
	Btn("RplBtnCur", btnRplCur);  rbRplCur = btn;
	btn = pSet->rpl_listview == 0 ? rbRplAll : rbRplCur;
	if (btn)  btn->setStateSelected(true);


	if (app->mWndRpl)
	{	//  replay controls
		Btn("RplToStart", btnRplToStart);  Btn("RplToEnd", btnRplToEnd)
		Btn("RplPlay", btnRplPlay);  btRplPl = btn;
		btn = fBtn("RplBack");		btn->eventMouseButtonPressed += newDelegate(this, &CGui::btnRplBackDn);  btn->eventMouseButtonReleased += newDelegate(this, &CGui::btnRplBackUp);
		btn = fBtn("RplForward");	btn->eventMouseButtonPressed += newDelegate(this, &CGui::btnRplFwdDn);   btn->eventMouseButtonReleased += newDelegate(this, &CGui::btnRplFwdUp);

		//  info
		slRplPos = (Slider*)app->mWndRpl->findWidget("RplSlider");
		if (slRplPos)  slRplPos->eventValueChanged += newDelegate(this, &CGui::slRplPosEv);

		valRplPerc = fTxt("RplPercent");
		valRplCur = fTxt("RplTimeCur");
		valRplLen = fTxt("RplTimeLen");
	}

	//  text desc, stats
	valRplName = fTxt("RplName");  valRplName2 = fTxt("RplName2");
	valRplInfo = fTxt("RplInfo");  valRplInfo2 = fTxt("RplInfo2");
	edRplName = fEd("RplNameEdit");
	//edRplDesc = fEd("RplDesc");

	rplList = fLi("RplList");
	Lev(rplList, RplChng);  // todo: lazy load..
	updReplaysList();

	Ed ed;
	Edt(ed,"RplFind",edRplFind);


	///  🚗 Car  ----
	InitGuiCar();


	///  📡 Multiplayer net
	//------------------------------------------------------------------------
	int c=0;
	tabsNet = fTab("SubTabNet");

	//  server, games
	listServers = fMli("MListServers");
	Mli ml = listServers;
		ml->addColumn("#C0FFC0"+TR("#{Game name}"), 200);  ++c;
		ml->addColumn("#50FF50"+TR("#{Track}"), 200);  ++c;
		ml->addColumn("#80FFC0"+TR("#{Laps}"), 70);  ++c;
		ml->addColumn("#FFFF00"+TR("#{Players}"), 70);  ++c;
		ml->addColumn("#80FFFF"+TR("#{Collis.}"), 90);  ++c;
		ml->addColumn("#A0D0FF"+TR("#{Simulation}"), 100);  ++c;
		ml->addColumn("#A0D0FF"+TR("#{Boost}"), 110);  ++c;
		ml->addColumn("#FF6060"+TR("#{Locked}"), 80);  iColLock = c;  ++c;
		ml->addColumn("#FF9000"+TR("#{NetHost}"), 220);  iColHost = c;  ++c;
		ml->addColumn("#FFB000"+TR("#{NetPort}"), 100);  iColPort = c;  ++c;

	Btn("btnNetRefresh",evBtnNetRefresh); btnNetRefresh = btn;
	Btn("btnNetJoin",   evBtnNetJoin);    btnNetJoin = btn;
	Btn("btnNetCreate", evBtnNetCreate);  btnNetCreate = btn;
	Btn("btnNetDirect", evBtnNetDirect);  btnNetDirect = btn;

	//  game, players
	Edt(edNetGameName, "edNetGameName", evEdNetGameName);  edNetGameName->setCaption(pSet->netGameName);
	valNetPassword = fTxt("valNetPassword");
	Edt(edNetPassword, "edNetPassword", evEdNetPassword);

	listPlayers = fMli("MListPlayers");
	ml = listPlayers;
		ml->addColumn("#80C0FF"+TR("#{Player}"), 140);
		ml->addColumn("#F08080"+TR("#{Vehicle}"), 80);
		ml->addColumn("#C0C060"+TR("#{Peers}"), 60);
		ml->addColumn("#60F0F0"+TR("#{Ping}"), 80);
		ml->addColumn("#40F040"+TR("#{NetReady}"), 60);

	Btn("btnNetReady", evBtnNetReady);  btnNetReady = btn;
	Btn("btnNetLeave", evBtnNetLeave);	btnNetLeave = btn;

	//  panels to hide tabs
	panNetServer  = fWP("panelNetServer");   panNetServer->setVisible(false);
	panNetServer2 = fWP("panelNetServer2");  panNetServer2->setVisible(false);
	panNetGame = fWP("panelNetGame");      panNetGame->setVisible(true);

	//  chat
	edNetChat = fEd("edNetChat");  // chat area
	edNetChatMsg = fEd("edNetChatMsg");  // user text
	//  track,game text
	valNetGameInfo = fTxt("valNetGameInfo");

	//  settings
	Edt(edNetNick,		"edNetNick",		evEdNetNick);		edNetNick->setCaption(		pSet->nickname);
	Edt(edNetServerIP,	"edNetServerIP",	evEdNetServerIP);	edNetServerIP->setCaption(	pSet->master_server_address);
	Edt(edNetServerPort,"edNetServerPort",	evEdNetServerPort);	edNetServerPort->setCaption(toStr(pSet->master_server_port));
	Edt(edNetLocalPort,	"edNetLocalPort",	evEdNetLocalPort);	edNetLocalPort->setCaption(	toStr(pSet->local_port));


	///  🕹️ Input tab  -------
	InitInputGui();


	///  🏞️ Tracks list, text, chg btn
	//------------------------------------------------------------------------
	gcom->panTrkDesc[0] = fWP("panTrkDesc0");
	gcom->trkDesc[0] = fEd("TrackDesc0");  gcom->trkAdvice[0] = fEd("TrackAdvice0");
	gcom->sListTrack = pSet->gui.track;

	gcom->GuiInitTrack();

	//  📡 multi panel cover
	Tbi trkTab = fTbi("TabTrack");
	trkTab->setColour(Colour(0.8f,0.96f,1.f));
	const IntCoord& tc = trkTab->getCoord();

	WP wp = trkTab->createWidget<Widget>(
		"PanelSkin", 0,0,tc.width*0.66f,tc.height, Align::Default/*, "Popup", "panelNetTrack"*/);
	wp->setColour(Colour(0.8f,0.96f,1.f));
	wp->setAlpha(0.8f);  wp->setVisible(false);
	panNetTrack = wp;
	//<UserString key="RelativeTo" value="OptionsWnd"/>


	//  🏁 New Game  ----
	for (i=0; i <= 3; ++i)
	{	Btn("NewGame"+toStr(i), btnNewGame);  if (i==1)  btNewGameCar = btn;  }


	//  🏆 Champs, Challs, Collects etc  ----
	InitGuiChamps();


	//  📡 multi end list  ------
	Btn("btnNetEndClose", btnNetEndClose);

	Mli2 li = app->mWndNetEnd->createWidget<MultiList2>("MultiListBox",4,42,632,360, Align::Left | Align::VStretch);
	li->setInheritsAlpha(false);  li->setColour(Colour(0.8,0.9,1,1));  li->setAlpha(0.7);
	li->removeAllColumns();
	li->addColumn("", 40);  //N
	li->addColumn(TR("#{TBPlace}"), 60);	li->addColumn(TR("#{NetNickname}"), 180);
	li->addColumn(TR("#{TBTime}"), 120);	li->addColumn(TR("#{TBBest}"), 120);
	li->addColumn(TR("#{TBLap}"), 60);
	liNetEnd = li;


	//  🔧 Tweak mtr  ---------------------
	gcom->InitGuiTweakMtr();
	gcom->updTweakMtr();

	gcom->ChangeTrackView();


	//  fonts
	sv= &svFntWnd;		sv->Init("FntWnd",		&pSet->font_wnds, -0.1f, 0.2f);  sv->DefaultF(0.f);
	sv= &svFntGui;		sv->Init("FntGui",		&pSet->font_gui,   0.8f, 1.2f);  sv->DefaultF(1.f);
	sv= &svFntHud;		sv->Init("FntHud",		&pSet->font_hud,   0.6f, 1.5f);  sv->DefaultF(1.f);
	sv= &svFntTimes;	sv->Init("FntTimes",	&pSet->font_times, 0.6f, 1.5f);  sv->DefaultF(1.f);


	//  user dir
	Ed edUserDir = fEd("EdUserDir");
	edUserDir->setCaption(PATHS::UserConfigDir());

	edOpenUrl = fEd("OpenUrl");
	edOpenUrl->setCaption("https://cryham.org/stuntrally/");

	//  🔗 open url btns  -------------
	Btn("BtnTrackEditor", btnTrackEditor);
	Btn("OpenWelcome", btnWelcome);   Btn("OpenWebsite", btnWebsite);  Btn("OpenSources", btnSources);
	Btn("OpenChat",    btnOpenChat);  Btn("OpenDonations", btnDonations);
	//  wiki
	Btn("OpenWiki",  btnWiki);   Btn("OpenWikiInput", btnWikiInput);
	Btn("OpenEdTut", btnEdTut);  Btn("OpenTransl", btnTransl);  // mplr?


	UpdWndTitle();
	bGI = true;  // gui inited, gui events can now save vals

	LogO(Ogre::String(":::* Time Init Gui: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
