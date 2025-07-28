#include "par.h"
#include "pch.h"
#include "Def_Str.h"
#include "SceneXml.h"
#include "CScene.h"
#include "RenderConst.h"
// #include "GuiCom.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "game.h"
#include "Road.h"
// #include "SplitScreen.h"

#include <OgreWindow.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
// #include <OgreOverlayManager.h>
// #include <OgreOverlayElement.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
using namespace Ogre;
using namespace MyGUI;


//  util
float CHud::getHudScale()
{
	const int plr = pSet->game.local_players;
	const static float scPlr[6] =
	{	1.f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f  };
	return scPlr[plr-1];  // scale down for SplitScreen
}

///  🗜️ HUD resize
//---------------------------------------------------------------------------------------------------------------
void CHud::Size()
{
	if (!pSet->hud_on)  return;
	int cnt = pSet->game.local_players;
	#ifdef DEBUG
	assert(cnt <= hud.size());
	#endif
	//  for each car
	for (int c=0; c < cnt; ++c)
	{
		const auto& dim = app->mDims[c];
		Hud& h = hud[c];
		float wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();
		asp = wx/wy;

		//  ⏲️ gauges
		Real xcRpm=0.f,ycRpm=0.f, xcRpmL=0.f,
			 xcVel=0.f,ycVel=0.f, ygMax=0.f, xBFuel=0.f;  // -1..1
		{
			Real sc = pSet->size_gauges * dim.avgsize1;
			Real spx = sc * 1.1f, spy = spx*asp;
			xcRpm = dim.right1 - spx*0.5f;  ycRpm = -dim.bottom1 + spy*2.f;
			xcRpmL= dim.right1 - spx;
			xcVel = dim.right1 - spx;       ycVel = -dim.bottom1 + spy*0.9f;
			ygMax = ycVel - sc;  xBFuel = xcVel - sc;

			// LogO(toStr(c)+" =>  x "+fToStr(dim.left1)+"  "+fToStr(dim.right1)+"  y "+fToStr(dim.top1)+"  "+fToStr(dim.bottom1));
			// LogO(toStr(c)+" =>  vel x "+fToStr(xcVel)+" y "+fToStr(ycVel)+"  rpm x "+fToStr(xcRpm)+" y "+fToStr(ycRpm));
			h.vcRpm = Vector2(xcRpm,ycRpm);  // store for hr updates
			h.vcVel = Vector2(xcVel,ycVel);
			h.fScale = sc;
		}
		
		//  🌍 minimap
		Real sc = pSet->size_minimap * dim.avgsize1;
		const Real marg = 1.3f; //1.05f;  // from border
		Real fMiniX = dim.left1 + sc * marg;  //(dim.right - sc * marg);
		Real fMiniY =-dim.bottom1 + sc*asp * marg;  //-dim.top - sc*asp * marg;
		Real miniTopY = fMiniY + sc*asp;

		if (h.ndMap)
		{	h.ndMap->setScale(sc, sc*asp,1);
			h.ndMap->setPosition(Vector3(fMiniX,fMiniY,0.f));
		}
	
		//  current viewport  min,max x,y in pixels  0..1 * wx,wy
		int xMin = dim.left0* wx, xMax = dim.right0 * wx,
			yMin = dim.top0 * wy, yMax = dim.bottom0* wy;
		int my = (1.f-ygMax)*0.5f*wy;  // gauge bottom y

		if (!h.txVel)  continue;

		//  positioning, min yMax - dont go below viewport bottom
		int vv = pSet->gauges_type > 0 ? -45 : 40;
		int gx = (xcRpm+1.f)*0.5f*wx - 10, gy = (-ycRpm+1.f)*0.5f*wy +22;
		int gxL=(xcRpmL+1.f)*0.5f*wx - 10;
		int vx = (xcVel+1.f)*0.5f*wx + vv, vy = std::min(yMax -21, my - 15);
		int bx =(xBFuel+1.f)*0.5f*wx - 10, by = std::min(yMax -36, my + 5);
			vx = std::min(vx, xMax -100);
			bx = std::min(bx, xMax -180);  // not too near to vel

		//  ⏲️ gear, vel
		const float f = 1.f; //pSet->font_hud * getHudScale();  //?-
		if (h.txGear)  h.txGear->setPosition(      gx*f, (gy +10)*f);
		if (h.bckGear) h.bckGear->setPosition((gx-12)*f, (gy +10)*f);

		if (h.bckVel)  h.bckVel->setPosition((vx-32)*f, (vy-6)*f);
		h.txVel->setPosition(vx*f,vy*f);  //h.bckVel

		#if 0
		h.txRewind ->setPosition(bx,   by);
		h.icoRewind->setPosition(bx+50,by-5);
		#endif

		if (h.txDamage)  // 🔨
		{	h.txDamage ->setPosition(gxL-83-72, gy+10-5);
			h.icoDamage->setPosition(gxL-83+57, gy+10-5);
			h.imgDamage->setPosition(gxL-83-26, gy+10-7);
		}
		if (h.txBFuel)  // 💨
		{	h.txBFuel ->setPosition(gxL-83-74, gy-60);
			h.icoBFuel->setPosition(gxL-83+57, gy-60);
			//h.icoBInf->setPosition(gxL-83+14,gy-60-5+2);
		}

		//  ⏱️ Times
		// bool hasLaps = pSet->game.hasLaps();
		const float t = pSet->font_times * getHudScale();
		int w = 160 * t, tx = xMin + 40 * t, ty = yMin + 55 * t;  //40
		// tx *= t;  ty *= t;
		
		h.bckTimes->setPosition(xMin + 20 * t,ty);
		//tx = 24;  ty = (hasLaps ? 16 : 4);
		h.txTimTxt->setPosition(tx,ty);
		h.txTimes->setPosition(tx+w,ty);  //?-
		h.txCollect->setPosition(tx+w, ty +90*t);  //..

		//  🏁 lap result
		int lx = xMax - 360 * t, ly = ty;
		h.bckLap->setPosition(lx-14,ly-8);
		h.txLapTxt->setPosition(lx,ly);
		h.txLap->setPosition(lx+w,ly);

		//  ❌ warn  🥇 win
		int ox = (xMax-xMin)/2 + xMin - 200;  int oy = yMin + 15;
		h.bckWarn->setPosition(ox,oy);
		h.bckPlace->setPosition(ox,oy + 40);
		
		h.txCountdown->setPosition((xMax-xMin)/2 -100, (yMax-yMin)/2 -60);

		//  🎥 camera info
		if (h.txCam)
			h.txCam->setPosition(xMax-300,yMax-30);
		//  abs,tcs
		h.txAbs->setPosition(xMin+160,yMax-30);
		h.txTcs->setPosition(xMin+220,yMax-30);
	}
	if (txCamInfo)
	{	txCamInfo->setPosition(270, app->mWindow->getHeight() -100);
		bckMsg->setPosition(256,0);
	}
	//  debug txt  === ===
	//..
}


//  🔁 HUD show/hide
//---------------------------------------------------------------------------------------------------------------
void CHud::Show(bool hideAll)
{
	if (!pSet->hud_on)  return;
	if (!txDbgSurf)  // not inited
		return;
	if (hideAll /*|| app->iLoad1stFrames > -1*/)  // still loading
	{
		app->bckFps->setVisible(false);
		app->txFps->setVisible(false);
		if (bckMsg)
		{
			txCamInfo->setVisible(false);
			bckMsg->setVisible(false);

			for (auto h:hud)
				if (h.parent)
					h.parent->setVisible(false);
		}
		app->hideMouse();
		if (app->mWndRpl)  app->mWndRpl->setVisible(false);
		if (app->mWndRplTxt)  app->mWndRplTxt->setVisible(false);
		return;
	}

	for (int i=0; i < 4; ++i)
		txDbgCar[i]->setVisible(pSet->car_dbgtxt);
	txDbgSurf->setVisible(pSet->car_dbgsurf);
	
	txDbgProfTim->setVisible(pSet->profilerTxt);
	txDbgProfBlt->setVisible(pSet->bltProfilerTxt);

	app->bckFps->setVisible(pSet->fps_bar);
	app->txFps->setVisible(pSet->fps_bar);
	if (bckMsg)
	{
		bool cam = pSet->show_cam && !app->isFocGui, times = pSet->show_times;
		//bool opp = pSet->show_opponents && (app->scn->road && app->scn->road->getNumPoints() > 0);
		bool bfuel = pSet->game.boost_type >= 1; // && pSet->game.boost_type <= 3;
		bool btxt = pSet->game.boost_type == 1 || pSet->game.boost_type == 2;
		//bool binf = pSet->game.boost_type == 3;
		bool bdmg = pSet->game.damage_type > 0;
		txCamInfo->setVisible(cam);

		bool prv = gPar.carPrv == 0;
		bool show = pSet->show_gauges && prv;
		bool dig = pSet->show_digits && prv;

		for (int c=0; c < hud.size(); ++c)
		{	Hud& h = hud[c];

			if (h.parent)
			{	h.parent->setVisible(true);
			
				if (h.bckGear) h.bckGear->setVisible(dig);
				if (h.txGear)  h.txGear->setVisible(dig);

				if (h.bckVel)  h.bckVel->setVisible(dig);
				h.txVel->setVisible(dig);
				
				if (h.txBFuel){  h.txBFuel->setVisible(show && btxt);
					h.icoBFuel->setVisible(show && bfuel);  /*h.icoBInf->setVisible(show && binf);*/  }
				
				if (h.txDamage){  h.txDamage->setVisible(show && bdmg);
					h.icoDamage->setVisible(show && bdmg);
					h.imgDamage->setVisible(show && bdmg);  }
				//txRewind; icoRewind;

				h.ndGauges->setVisible(show);
				h.ndMap->setVisible(pSet->trackmap && prv);
				
				h.txTimes->setVisible(times);  h.txTimTxt->setVisible(times);  h.bckTimes->setVisible(times);
				h.txCollect->setVisible(times);
				h.txLap->setVisible(times);  h.txLapTxt->setVisible(times);  h.bckLap->setVisible(times);
				if (!times)  h.bckWarn->setVisible(0);

				if (h.txCam)  h.txCam->setVisible(cam);
		}	}
	}
	if (ndPos)  ndPos->setVisible(pSet->trackmap);
	
	app->updMouse();
	
	//  📽️ replay controls
	if (app->mWndRpl && !app->bLoading)
		app->mWndRpl->setVisible(app->bRplPlay && app->bRplWnd);
	//  lesson replay  >> >
	if (app->mWndRplTxt && !app->bLoading && app->gui->bLesson)
		app->mWndRplTxt->setVisible(app->bRplPlay);
}
