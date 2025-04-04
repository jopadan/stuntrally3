#include "pch.h"
#include "enums.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "RenderConst.h"
#include "GuiCom.h"
#include "CScene.h"
#include "paths.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "Road.h"
#include "PaceNotes.h"
#include "MultiList2.h"
// #include "../sdl4ogre/sdlcursormanager.hpp"
// #include "../sdl4ogre/sdlinputwrapper.hpp"
// #include <OgreTerrain.h>
// #include <OgreOverlay.h>
// #include <OgreOverlayElement.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
// #include <OgreViewport.h>
#include <OgreGpuResource.h>
#include <OgreTextureGpuManager.h>
#include <MyGUI.h>
#include <vector>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


///  Update all Gui controls
///  (after track Load, or Copy)
///  basing on values in scene and road
//..........................................................................................................
void CGui::SetGuiFromXmls()
{
	if (!app->mWndEdit)  return;
	bGI = false;
	
	Vector3 c{0,0,0};
	#define _Ed(name, val)  ed##name->setCaption(toStr(val))
	#define _Clr(name, val)  c = val.GetRGB1();  clr##name->setColour(Colour(c.x,c.y,c.z))
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) )
	

	//  ⛅ Sky
	//-----------------------------------------------
	btnSky->setCaption(sc->skyMtr);
	String s = sc->skyMtr;  s = s.substr(4, s.length());  // sel on pick list
	for (size_t i=0; i < liSky->getItemCount(); ++i)  // no #clr, no sky/
		if (liSky->getSubItemNameAt(1, i).substr(7) == s.substr(4))
			liSky->setIndexSelected(i);

	svSkyYaw.Upd();  svSunPitch.Upd();  svSunYaw.Upd();
	_Clr(Amb, sc->lAmb);  _Clr(Diff, sc->lDiff);  _Clr(Spec, sc->lSpec);
	_Clr(Fog, sc->fogClr);  _Clr(Fog2, sc->fogClr2);  _Clr(FogH, sc->fogClrH);
	
	svFogStart.Upd();	svFogEnd.Upd();
	svFogHStart.Upd();	svFogHEnd.Upd();
	svFogHeight.Upd();	svFogHDensity.Upd();  svFogHDmg.Upd();
	
	for (int i=0; i < NumWeather; ++i)
	{	svRainRate[i].Upd();
		_Cmb(cmbRain[i], sc->rainName[i]);
	}
	_Cmb(cmbReverbs, sc->sReverbs);  UpdRevDescr();
	
	//  ⛰️ Terrain
	//-----------------------------------------------
	SetGuiTerFromXml();

	_Cmb(cmbParDust, sc->sParDust);	_Cmb(cmbParMud,  sc->sParMud);
	_Cmb(cmbParSmoke,sc->sParSmoke);

	//  🌳🪨🌿 Vegetation
	//-----------------------------------------------
	svGrassDens.Upd();  svTreesDens.Upd();
	// _Ed(TrPage, sc->trPage);  _Ed(TrDist, sc->trDist);
	// _Ed(GrPage, sc->grPage);  _Ed(GrDist, sc->grDist);  _Ed(TrImpDist, sc->trDistImp);
	svTrRdDist.Upd();  svGrDensSmooth.Upd();

	tabGrLayers(tabsGrLayers, idGrLay);
	tabPgLayers(tabsPgLayers, idPgLay);

	//  🛣️ Road
	//-----------------------------------------------
	SetGuiRoadFromXml();

	//  🚗 Game
	//-----------------------------------------------
	ckDenyReversed.Upd(&sc->denyReversed);
	ckTiresAsphalt.Upd(&sc->asphalt);
	ckTerrainEmissive.Upd(&td().emissive);
	ckNoWrongChks.Upd(&sc->noWrongChks);
	SldUpd_Game();
	UpdEdInfo();
	
	//  ⚫💭 Surface
	//-----------------------------------------------
	UpdSurfList();
	listSurf(surfList, idSurf);
	
	//  RoR export cfg
	//-----------------------------------------------
	svRoR_AmbAdd.Upd();  svRoR_Amb.Upd();  svRoR_Diff.Upd();  svRoR_Spec.Upd();
	svRoR_FogMul.Upd();

	svRoR_Water.Upd(); 	svRoR_WaterOfs.Upd();
	svRoR_Trees.Upd();  svRoR_Grass.Upd();

	svRoR_TileMul.Upd();  svRoR_ObjOfsY.Upd();

	svRoR_RoadLay.Upd();  slRoR_RoadLay(0);
	ckRoR_RoadCols.Upd();

	svRoR_RoadStep.Upd();  svRoR_RoadAngle.Upd();
	svRoR_RoadHadd.Upd();  svRoR_WallX.Upd();  svRoR_WallY.Upd();

	svRoR_RoadVegetDist.Upd();

	bGI = true;
}

//  ⛰️ Terrain
//-----------------------------------------------
void CGui::SetGuiTerFromXml()
{
	// String fname = scn->getHmap(scn->terCur, false);//-
	// td().getFileSize(fname);
	updTabHmap();
	svTerTriSize.UpdF(&td().fTriangleSize);  //`
	UpdTxtTerSize();
	// svTerNormScale.Upd();  svTerSpecPow.Upd();
	
	svTerDiffR.UpdF(&td().clrDiff.x);
	svTerDiffG.UpdF(&td().clrDiff.y);
	svTerDiffB.UpdF(&td().clrDiff.z);

	ckTerrainEmissive.Upd(&td().emissive);
	svTerReflect.UpdI(&td().reflect);
	// svTerSpecPowEm.UpdF(&td().specularPowEm);

	tabTerLayer(tabsTerLayers, idTerLay);
	updTersTxt();
}

//  🛣️ Road
//-----------------------------------------------
void CGui::SetGuiRoadFromXml()
{
	SplineRoad* rd = scn->road;
	for (int i=0; i < 4/*MTRs*/; ++i)
	{	btnRoad[i]->setCaption(rd->sMtrRoad[i]);
		btnPipe[i]->setCaption(rd->sMtrPipe[i]);
	}
	btnRoadW->setCaption(rd->sMtrWall);
	btnPipeW->setCaption(rd->sMtrWallPipe);
	btnRoadCol->setCaption(rd->sMtrCol);

	_Ed(RdHeightOfs, rd->g_Height);
	_Ed(RdSkirtLen, rd->g_SkirtLen);  _Ed(RdSkirtH, rd->g_SkirtH);
	SldUpd_Road();
	ckRoad1Mtr.Upd();
	updRoadsTxt();
}


void CGui::btnNewGame(WP)
{
	if (trkName)  trkName->setCaption(gcom->sListTrack);
	pSet->gui.track = gcom->sListTrack;
	pSet->gui.track_user = gcom->bListTrackU;
	app->LoadTrack();
}

void CGui::btnEdTut(WP)
{
	PATHS::OpenUrl("https://github.com/stuntrally/stuntrally3/blob/main/docs/Editor.md");
}


//  🔁🪟 Update  input, info
//-------------------------------------------------------------------------------
//  tool wnds show/hide
void App::UpdEditWnds()
{
	bool on = pSet->hud_on;
	if (mWndBrush)
	{	if (edMode == ED_Deform)
		{	mWndBrush->setCaption(TR("D - #{TerDeform}"));
			mWndBrush->setColour(Colour(0.5f, 0.9f, 0.3f));
			mWndBrush->setVisible(on);
		}
		else if (edMode == ED_Filter)
		{	mWndBrush->setCaption(TR("F - #{TerFilter}"));
			mWndBrush->setColour(Colour(0.5f, 0.75f, 1.0f));
			mWndBrush->setVisible(on);  
		}
		else if (edMode == ED_Smooth)
		{	mWndBrush->setCaption(TR("S - #{TerSmooth}"));
			mWndBrush->setColour(Colour(0.3f, 0.8f, 0.8f));
			mWndBrush->setVisible(on);
		}
		else if (edMode == ED_Height)
		{	mWndBrush->setCaption(TR("E - #{TerHeight}"));
			mWndBrush->setColour(Colour(0.7f, 1.0f, 0.7f));
			mWndBrush->setVisible(on);
		}else
			mWndBrush->setVisible(false);
	}
	if (mWndRoadCur)  mWndRoadCur->setVisible(edMode == ED_Road && on);
	if (mWndCam)      mWndCam->setVisible(edMode == ED_PrvCam && on);
	
	if (mWndStart)    mWndStart->setVisible(edMode == ED_Start && on);

	if (mWndFluids)   mWndFluids->setVisible(edMode == ED_Fluids && on);
	UpdFluidBox();

	if (mWndObjects)   mWndObjects->setVisible(edMode == ED_Objects && on);
	if (mWndCollects)  mWndCollects->setVisible(edMode == ED_Collects && on);
	if (mWndFields)    mWndFields->setVisible(edMode == ED_Fields && on);

	if (mWndParticles) mWndParticles->setVisible(edMode == ED_Particles && on);
	UpdEmtBox();

	UpdStartPos(edMode != ED_PrvCam);  // StBox visible
	UpdVisGui();  //br prv..
}


//  change editor mode
//-----------------------------------------------
void App::SetEdMode(ED_MODE newMode)
{
	static bool first = true;
	if (newMode == ED_Objects && first)
	{
		SetObjNewType(iObjTNew);
		first = false;
	}
	bool edit = bEdit();
	if (boxObj.nd)  boxObj.nd->setVisible(newMode == ED_Objects && edit);
	if (boxEmit.nd)  boxEmit.nd->setVisible(newMode == ED_Particles && edit);

	//  collects hide
	bool vis = newMode == ED_Collects;
	for (auto& c : scn->sc->collects)
	if (c.nd)
		c.nd->setVisible(vis);
	if (boxCol.nd)  boxCol.nd->setVisible(vis && edit);

	edMode = newMode;
	if (edMode >= ED_Deform && edMode <= ED_Filter)
		iCurBr = edMode;
}


//  🪟 wnd vis
//-----------------------------------------------
void App::UpdVisGui()
{
	static bool f1 = true;
	if (bGuiFocus && f1)
	{	f1 = false;
		// todo: preload tex?..
		/** auto* rndSys = mRoot->getRenderSystem();
		auto* texMgr = rndSys->getTextureGpuManager();
		std::vector<string> vs{"gui_icons.png", "stuntrally-logo.jpg" };
		for (auto s : vs)
		{	auto* t = texMgr->createOrRetrieveTexture(s, GpuPageOutStrategy::SaveToSystemRam, TextureFlags::AllowAutomipmaps, TextureTypes::Type2D);
			t->_setNextResidencyStatus(GpuResidency::OnSystemRam);
			t->waitForData();
		}*/
	}
	//  wnd
	bool g = bGuiFocus, on = pSet->hud_on;
	bool notMain = g && !pSet->bMain;
	mWMainMenu->setVisible(g && pSet->bMain);
	
	mWndTrack->setVisible(notMain && pSet->inMenu == WND_Track);
	mWndEdit->setVisible(notMain && pSet->inMenu == WND_Edit);
	mWndEdObj->setVisible(notMain && pSet->inMenu == WND_EdObj);
	
	mWndHelp->setVisible(notMain && pSet->inMenu == WND_Help);
	mWndOpts->setVisible(notMain && pSet->inMenu == WND_Options);
	
	//  mat editor
	bool mat = notMain && pSet->inMenu == WND_Materials;
	static bool mat1st = 1;
	if (mat && mat1st)
		gcom->FillTweakMtr();  // on 1st show
	mWndMaterials->setVisible(mat);


	if (!g)  mWndPick->setVisible(false);
	if (!g)  mWndTrkFilt->setVisible(false);
	if (/*!g &&*/ gui->wndColor)  gui->wndColor->setVisible(false);
	if (gcom->bnQuit)  gcom->bnQuit->setVisible(g);

	//  mode
	if (gui->imgCam)
	{	gui->imgCam->setVisible(!g && bMoveCam && on);
		gui->imgEdit->setVisible(!g && !bMoveCam && on);
		gui->imgGui->setVisible(g && on);
	}

	bool vis = g || !bMoveCam;
	// mInputWrapper->setMouseVisible(vis);
	// mInputWrapper->setMouseRelative(!vis);
	// mInputWrapper->setAllowGrab(pSet->mouse_capture);
	// mInputWrapper->setGrabPointer(!vis);

	if (scn->road)
	{	scn->road->SetTerHitVis(bEdit());
		/*if (!bEdit())*/  scn->road->HideMarks();  }
	if (!g && gcom->mToolTip)  gcom->mToolTip->setVisible(false);

	if (ndBrush)
		ndBrush->setVisible(edMode < ED_Road && !bMoveCam && on);

	for (int i=0; i < ciMainBtns; ++i)
		mMainPanels[i]->setVisible(pSet->inMenu == i);
		
	if (gui->txWarn)  gui->txWarn->setVisible(false);

	//  1st center mouse
	static bool first = true;
	if (g && first)
	{	first = false;
		gcom->GuiCenterMouse();
	}
}

void CGui::toggleGui(bool toggle)
{
	if (app->edMode == ED_PrvCam)  return;
	if (toggle)
		app->bGuiFocus = !app->bGuiFocus;
	app->UpdVisGui();
}


//  bottom status bar
void CGui::Status(String s, float r,float g,float b)
{
	if (!txtStatus)  return;
	txtStatus->setCaption(TR(s));
	txtStatus->setTextColour(Colour(r,g,b));
	panStatus->setColour(Colour(r,g,b));
	panStatus->setAlpha(1.f);  panStatus->setVisible(true);
	app->fStFade = 1.5f;
}


///  🖼️ Preview Camera mode  - - - - - - - - - - - - - - - - - - - - - - - -
void App::togPrvCam()
{
	static bool oldV = false, oldI = false;
	if (edMode == ED_PrvCam)  // leave
	{
		SetEdMode(edModeOld);
		// mViewport->setVisibilityMask(RV_MaskAll);  //?

		// scn->UpdateWaterRTT(mCamera);
		//----  restore: Fog, Veget, weather, emitters
		scn->UpdFog();
		if (oldV)  {  bVegetGrsUpd = true;  oldV = false;  }
		pSet->bWeather = oldI;
		if (!pSet->bEmitters)
			scn->DestroyEmitters(false);

		scn->sc->camPos = mCamera->getPosition();
		scn->sc->camDir = mCamera->getDirection();
		mCamera->setPosition( mCamPosOld);
		mCamera->setDirection(mCamDirOld);
	}else  // enter
	{
		edModeOld = edMode;
		SetEdMode(ED_PrvCam);
		bMoveCam = true;  // will hide cursors
		UpdVisGui();
		// mViewport->setVisibilityMask(RV_MaskPrvCam);  //?-
		// rt[RT_View].ws->setEnabled(1);  //? render only now

		// scn->UpdateWaterRTT(rt[RT_View].cam);
		// scn->mTerrainGlobals->setMaxPixelError(0.5f);  //hq ter ..

		//----  force on: Fog, Veget, Weather, Emitters
		scn->UpdFog(true);
		if (!pSet->bTrees)  {  bVegetGrsUpd = true;  oldV = true;  }
		oldI = pSet->bWeather;  pSet->bWeather = true;
		bRecreateEmitters = true;

		mCamPosOld = mCamera->getPosition();
		mCamDirOld = mCamera->getDirection();
		mCamera->setPosition( scn->sc->camPos);
		mCamera->setDirection(scn->sc->camDir);
	}
	UpdEditWnds();
	
	UpdMiniVis();
	UpdMiniSize();
}


//  🎛️⌨️ Gui Shortcut  alt-letters
//.......................................................................................
void CGui::GuiShortcut(WND_Types wnd, int tab, int subtab)
{
	if (subtab == -1 && (!app->bGuiFocus || pSet->inMenu != wnd))
		subtab = -2;  // cancel subtab cycling

	if (!app->bGuiFocus)
	if (app->edMode != ED_PrvCam)  {
		app->bGuiFocus = !app->bGuiFocus;  app->UpdVisGui();  }

	//isFocGui = true;
	pSet->bMain = false;  pSet->inMenu = wnd;
	
	TabPtr tabs = 0;
	std::vector<TabControl*>* subt = 0;
	
	switch (wnd)
	{	case WND_Track:		tabs = app->mTabsTrack; subt = &vSubTabsTrack; break;
		case WND_Edit:		tabs = app->mTabsEdit;  subt = &vSubTabsEdit;  break;
		case WND_EdObj:		tabs = app->mTabsEdObj; subt = &vSubTabsEdObj;  break;
		
		case WND_Help:		tabs = app->mTabsHelp;  subt = &vSubTabsHelp;  break;
		case WND_Options:	tabs = app->mTabsOpts;  subt = &vSubTabsOpts;  break;
		case WND_Materials:	tabs = app->mTabsMat;   subt = &vSubTabsMat;  break;
		default:            tabs = app->mTabsTrack; subt = &vSubTabsTrack; break;
	}
	if (wnd != WND_Edit)
		app->mWndPick->setVisible(false);
	toggleGui(false);

	if (tab < 0)  return;
	size_t t = tabs->getIndexSelected();
	tabs->setIndexSelected(tab);

	if (!subt)  return;
	TabControl* tc = (*subt)[tab];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{	if (app->shift)
			tc->setIndexSelected( (tc->getIndexSelected()-1+cnt) % cnt );
		else
			tc->setIndexSelected( (tc->getIndexSelected()+1) % cnt );
	}
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
}


//  next num tab  alt-1,2
//.......................................................................................
void CGui::NumTabNext(int rel)
{
	if (!app->bGuiFocus || pSet->bMain /*|| pSet->inMenu != WND_Edit*/)  return;

	TabPtr tab = 0;

	#define tabNum(event)  {  \
		int cnt = tab->getItemCount();  \
		tab->setIndexSelected( (tab->getIndexSelected()+rel+cnt) % cnt );  \
		event(tab, tab->getIndexSelected());  }

	int id = app->mTabsEdit->getIndexSelected();
	switch (id)
	{
		case TAB_Layers:  tab = tabsTerLayers;  tabNum(tabTerLayer);  break;
		case TAB_Grass:  switch (vSubTabsEdit[id]->getIndexSelected())
		{	case 1:  tab = tabsGrLayers;  tabNum(tabGrLayers);  break;
			case 2:  tab = tabsGrChan;  tabNum(tabGrChan);  break;  }  break;
		case TAB_Veget:  tab = tabsPgLayers;  tabNum(tabPgLayers);  break;
		case TAB_Surface:  {  int t = (surfList->getIndexSelected() +5 +rel) % 5;
			surfList->setIndexSelected(t);  listSurf(surfList, t);  }  break;
	}
}


///  💫 Update (frame start)  .,.,.,.,..,.,.,.,..,.,.,.,..,.,.,.,.
void CGui::GuiUpdate()
{
	gcom->UnfocusLists();
	
	if (iLoadNext)  // load next/prev track  (warnings)
	{	size_t cnt = gcom->trkList->getItemCount();
		if (cnt > 0)  
		{	int i = std::max(0, std::min((int)cnt-1, (int)gcom->trkList->getIndexSelected() + iLoadNext ));
			iLoadNext = 0;
			gcom->trkList->setIndexSelected(i);
			gcom->trkList->beginToItemAt(std::max(0, i-11));  // center
			gcom->listTrackChng(gcom->trkList,i);
			btnNewGame(0);
	}	}
	
	if (gcom->bGuiReinit)  // after language change from combo  fixme
	{	gcom->bGuiReinit = false;

		mGui->destroyWidgets(app->vwGui);
		gcom->bnQuit=0; app->mWndOpts=0; gcom->trkList=0; //todo: rest too..

		bGI = false;
		InitGui();
		
		SetGuiFromXmls();
		app->bWindowResized = true;
	}

	//  📃 sort trk list
	gcom->SortTrkList();


	if (app->bWindowResized && gcom)
	{	app->bWindowResized = false;
		LogO("[]-- WindowResized");

		gcom->ResizeOptWnd();
		//bSizeHUD = true;
		gcom->SizeGUI();
		gcom->updTrkListDim();
		// viewCanvas->setCoord(GetViewSize());

		//slSizeMinimap(0);
		//LoadTrack();  // shouldnt be needed ...
	}
}


///  🎨 Color tool window
//...............................................................................
void CGui::btnClrSet(WP w)
{
	SColor* v;  // rgb
	if (w == clrAmb)   v = &sc->lAmb;   else
	if (w == clrDiff)  v = &sc->lDiff;  else
	if (w == clrSpec)  v = &sc->lSpec;  else
	if (w == clrGrass) v = &sc->grLayersAll[idGrLay].color;
	
	bool other = wpClrSet != w;  wpClrSet = w;

	svAlp.setVisible(false);
	svHue.UpdF(&v->h);  svSat.UpdF(&v->s);  svVal.UpdF(&v->v);
	svNeg.UpdF(&v->n);

	IntPoint p = w->getAbsolutePosition();  p.left += 100;  p.top -= 50;
	wndColor->setPosition(p);
	if (!(wndColor->getVisible() && other))  // dont hide if changed
		wndColor->setVisible(!wndColor->getVisible());
}
void CGui::btnClrSetA(WP w)
{
	SColor* v;  // rgba
	if (w == clrFog)   v = &sc->fogClr;   else
	if (w == clrFog2)  v = &sc->fogClr2;  else
	if (w == clrFogH)  v = &sc->fogClrH;  else
	if (w == clrTrail) {
		TerLayer* l = GetTerRdLay();
		v = &l->tclr;  }
	bool oth = wpClrSet != w;  wpClrSet = w;

	svAlp.setVisible(true);
	svHue.UpdF(&v->h);  svSat.UpdF(&v->s);  svVal.UpdF(&v->v);
	svAlp.UpdF(&v->a);  svNeg.UpdF(&v->n);

	IntPoint p = w->getAbsolutePosition();  p.left += 100;  p.top -= 50;
	wndColor->setPosition(p);
	if (!(wndColor->getVisible() && oth))
		wndColor->setVisible(!wndColor->getVisible());
}

void CGui::slUpdClr(SV* sv)
{
	SColor c(svHue.getF(), svSat.getF(), svVal.getF(), svAlp.getF(), svNeg.getF());
	Vector3 cc = c.GetRGB1();
	wpClrSet->setColour(Colour(cc.x,cc.y,cc.z));  // img

	WP w = wpClrSet;
	if (w == clrAmb) {  sc->lAmb = c;    scn->UpdSun();  }else
	if (w == clrDiff){  sc->lDiff = c;   scn->UpdSun();  }else
	if (w == clrSpec){  sc->lSpec = c;   scn->UpdSun();  }else

	if (w == clrFog) {  sc->fogClr = c;   scn->UpdFog();  }else
	if (w == clrFog2){  sc->fogClr2 = c;  scn->UpdFog();  }else
	if (w == clrFogH){  sc->fogClrH = c;  scn->UpdFog();  }else

	if (w == clrTrail) {
		TerLayer* l = GetTerRdLay();
		l->tclr = c;  }
}
