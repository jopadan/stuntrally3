#include "pch.h"
#include "par.h"
#include "Def_Str.h"
#include "RenderConst.h"
#include "CData.h"
#include "SceneXml.h"
#include "TracksXml.h"
#include "CollectXml.h"
#include "CScene.h"

#include "game.h"
#include "settings.h"
#include "quickprof.h"
#include "Road.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
// #include "SplitScreen.h"
#include "FollowCamera.h"
#include "CarModel.h"
#include "MultiList2.h"
// #include "GraphView.h"

// #include <OgreWindow.h>
#include <OgreSceneNode.h>
// #include <OgreMaterialManager.h>
// #include <OgrePass.h>
// #include <OgreTechnique.h>
#include <OgreSceneManager.h>

#include <MyGUI.h>
#include <MyGUI_Ogre2Platform.h>
#include <list>
using namespace Ogre;
using namespace MyGUI;
using namespace std;


//  HUD utils
//---------------------------------------------------------------------------------------------------------------

bool SortPerc(const CarModel* cm2, const CarModel* cm1)
{
	int l1 = cm1->pGame->timer.GetCurrentLap(cm1->iIndex);
	int l2 = cm2->pGame->timer.GetCurrentLap(cm2->iIndex);
	float p1 = cm1->trackPercent;
	float p2 = cm2->trackPercent;
	return (l1 < l2) || (l1 == l2 && p1 < p2);
}

bool SortWin(const CarModel* cm2, const CarModel* cm1)
{
	int l1 = cm1->pGame->timer.GetCurrentLap(cm1->iIndex);
	int l2 = cm2->pGame->timer.GetCurrentLap(cm2->iIndex);
	float t1 = cm1->pGame->timer.GetPlayerTimeTot(cm1->iIndex);
	float t2 = cm2->pGame->timer.GetPlayerTimeTot(cm2->iIndex);
	return (l1 < l2) || (l1 == l2 && t1 > t2);
}


void CHud::GetCarVals(int id, float* vel, float* rpm, float* clutch, int* gear)
{
	#ifdef DEBUG
	assert(id >= 0);
	assert(id < app->carModels.size());
	assert(id < app->frm.size());
	#endif
	const CarModel* pCarM = app->carModels[id];
	const CAR* pCar = pCarM ? pCarM->pCar : 0;

	if (pCar && !app->bRplPlay && !pCarM->isGhost())
	{	*vel = pCar->GetSpeedometer() * (pSet->show_mph ? 2.23693629f : 3.6f);
		*rpm = pCar->GetEngineRPM();  *gear = pCar->GetGear();
		//*clutch = pCar->GetClutch();  // todo: problems in multi thr1
	}
	if (app->bRplPlay)
	{
		*vel = app->frm[id].vel * (pSet->show_mph ? 2.23693629f : 3.6f);
		*rpm = app->frm[id].rpm;  *gear = app->frm[id].gear;
	}
}


//  🔁📡 Multiplayer
//---------------------------------------------------------------------------------------------------------------
void CHud::UpdMultiplayer(CGui* gui, int cnt, float time)
{
	static float tm = 0.f;  tm += time;
	if (tm < 0.2f)  // not every frame, each 0.2s
		return;

	//  sort winners
	std::list<CarModel*> cms;
	for (int c=0; c < cnt; ++c)
		cms.push_back(app->carModels[c]);

	cms.sort(SortWin);
	//stable_sort(cms.begin(), cms.end(), SortWin);
	
	String msg = "";  int place = 1;  // assing places
	for (auto it = cms.begin(); it != cms.end(); ++it)
	{
		CarModel* cm = *it;
		bool end = app->pGame->timer.GetCurrentLap(cm->iIndex) >= pSet->game.num_laps;
		cm->iWonPlace = end ? place++ : 0;  // when ended race

		//  detect change (won),  can happen more than once, if time diff < ping delay
		if (cm->iWonPlace != cm->iWonPlaceOld)
		{	cm->iWonPlaceOld = cm->iWonPlace;
			cm->iWonMsgTime = gPar.timeWonMsg;
			if (cm->iIndex == 0)  // for local player, show end wnd
				app->mWndNetEnd->setVisible(true);
		}
		if (cm->iWonMsgTime > 0.f)
		{	cm->iWonMsgTime -= tm;
			if (cm->iWonPlace != 0)
				msg += cm->sDispName + " " + TR("#{FinishedCommaPlace}") + ": " + toStr(cm->iWonPlace) + "\n";
		}
	}
	if (app->mClient && /*ap->pGame->timer.pretime <= 0.f &&*/ app->pGame->timer.waiting)
		msg += TR("#{NetWaitingForOthers}")+"...\n";
		
	//  chat 2 last lines
	if (gui->sChatLast1 != "")	msg += gui->sChatLast1 + "\n";
	if (gui->sChatLast2 != "")	msg += gui->sChatLast2;
		
	++gui->iChatMove;
	if (gui->iChatMove >= 10)  //par 2sec
	{	gui->iChatMove = 0;
		gui->sChatLast1 = gui->sChatLast2;
		gui->sChatLast2 = "";
	}
	
	//  upd hud msgs
	if (txMsg)
	{	txMsg->setCaption(msg);
		bckMsg->setVisible(!msg.empty());
	}

	//  upd end list
	if (app->mWndNetEnd->getVisible())
	{
		MyGUI::MultiList2* li = gui->liNetEnd;
		li->removeAllItems();
		for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it)
		{
			CarModel* cm = *it;
			String clr = StrClr(cm->color);

			li->addItem(""/*clr+ toStr(c+1)*/, 0);  int l = li->getItemCount()-1;
			li->setSubItemNameAt(1,l, clr+ (cm->iWonPlace == 0 ? "--" : toStr(cm->iWonPlace)));
			li->setSubItemNameAt(2,l, clr+ cm->sDispName);
			li->setSubItemNameAt(3,l, clr+ StrTime( cm->iWonPlace == 0 ? 0.f : app->pGame->timer.GetPlayerTimeTot(cm->iIndex) ));
			//li->setSubItemNameAt(4,l, clr+ fToStr(cm->iWonMsgTime,1,3));
			li->setSubItemNameAt(4,l, clr+ StrTime( app->pGame->timer.GetBestLapRace(cm->iIndex) ));
			li->setSubItemNameAt(5,l, clr+ toStr( app->pGame->timer.GetCurrentLap(cm->iIndex) ));
	}	}
	tm = 0.f;
}


#if 0
///  Opponents list
// -----------------------------------------------------------------------------------
void CHud::UpdOpponents(Hud& h, int cnt, CarModel* pCarM)
{
	int c;
	bool visOpp = h.txOpp[0] && pSet->show_opponents;
	if (visOpp && pCarM && pCarM->pMainNode)
	{
		std::list<CarModel*> cms;  // sorted list
		for (c=0; c < cnt; ++c)
		{	//  cars only
			CarModel* cm = app->carModels[c];
			if (!cm->isGhost())
			{	if (app->bRplPlay)
					cm->trackPercent = app->carPoses[app->iCurPoses[c]][c].percent;
				cms.push_back(cm);
		}	}
		if (pSet->opplist_sort)
			cms.sort(SortPerc);
		
		for (c=0; c < cnt; ++c)
		{	//  add last, if visible, ghost1 (dont add 2nd) and track's ghost
			CarModel* cm = app->carModels[c];
			if (cm->eType == (app->isGhost2nd ? CarModel::CT_GHOST2 : CarModel::CT_GHOST) && pSet->rpl_ghost ||
				cm->isGhostTrk() && pSet->rpl_trackghost)
			{
				cm->trackPercent = app->carPoses[app->iCurPoses[c]][c].percent;  // ghost,rpl
				cms.push_back(cm);
		}	}

		bool bGhostEnd = app->pGame->timer.GetPlayerTime(0) > app->ghplay.GetTimeLength();
		String s0,s1,s2;  // Track% Dist Nick
		ColourValue clr;  c = 0;
		for (std::list<CarModel*>::iterator it = cms.begin(); it != cms.end(); ++it)
		{
			CarModel* cm = *it;
			if (cm->pMainNode)
			{
				bool bGhost = cm->isGhost() && !cm->isGhostTrk();
				bool bGhostVis = (app->ghplay.GetNumFrames() > 0) && pSet->rpl_ghost;
				bool bGhEmpty = bGhost && !bGhostVis;

				//  dist  -----------
				if (cm == pCarM || bGhEmpty)  // no dist to self or to empty ghost
					s1 += "\n";
				else
				{	Vector3 v = cm->pMainNode->getPosition() - pCarM->pMainNode->getPosition();
					float dist = v.length();  // meters
					Real h = std::min(60.f, dist) / 60.f;
					clr.setHSB(0.5f - h * 0.4f, 1, 1);
					s1 += StrClr(clr)+ fToStr(dist,0,3)+"m\n";
				}
					
				//  percent %  -----------
				if (bGhEmpty || cm->isGhostTrk())
					s0 += "\n";
				else
				{	float perc = bGhost && bGhostEnd ? 100.f : cm->trackPercent;
					clr.setHSB(perc*0.01f * 0.4f, 0.7f, 1);
					s0 += StrClr(clr)+ fToStr(perc,0,3)+"%\n";
				}
				
				//  nick name  -----------
				if (cm->eType != CarModel::CT_REPLAY)
				{
					s2 += StrClr(cm->color)+ cm->sDispName;
					bool end = app->pGame->timer.GetCurrentLap(cm->iIndex) >= pSet->game.num_laps
							&& (app->mClient || pSet->game.local_players > 1);  // multiplay or split
					if (end)  //  place (1)
						s2 += "  (" + toStr(cm->iWonPlace) + ")";
				}
				s2 += "\n";  ++c;
		}	}
	}
}
#endif


//  Car texts
//-------------------------------------------------------------------------------------------------------------------
void CHud::UpdCarTexts(int carId, Hud& h, float time, CAR* pCar)
{
	float vel=0.f, rpm=0.f, clutch=1.f;  int gear=1;
	GetCarVals(carId,&vel,&rpm,&clutch,&gear);

	if (!h.txVel)
		return;

	///  ⏲️ gear, vel texts  -----------------------------
	if (h.txGear)
	{
		const static int gearCnt = 12;
		const static Colour gearClr[gearCnt] = {
			Colour(0.3f, 1.0f, 1.f),  // R
			Colour(0.3f, 1.0f, 0.3f),  // N
			Colour(1.0f, 0.2f, 0.2f),  // 1
			Colour(1.0f, 0.4f, 0.2f),
			Colour(1.0f, 0.6f, 0.2f),
			Colour(1.0f, 0.8f, 0.2f),  // 4
			Colour(0.9f, 0.9f, 0.2f),
			Colour(0.9f, 0.9f, 0.9f),  // 6
			Colour(0.7f, 0.7f, 1.0f),
			Colour(0.7f, 0.5f, 1.0f),  // 8
			Colour(0.6f, 0.4f, 1.0f),
			Colour(0.5f, 0.5f, 1.0f)};
		float cl = clutch*0.8f + 0.2f;
		if (gear < 0)
			h.txGear->setCaption("R");
		else if (gear == 0)
			h.txGear->setCaption("N");
		else
			h.txGear->setCaption(toStr(gear));
		h.txGear->setTextColour(gearClr[std::max(0, std::min(gearCnt-1, gear+1))]);
	}
	if (h.txVel && pCar)
	{
		h.txVel->setCaption(fToStr(fabs(vel),0,3));
	#if 0  //  old vel clr-
		float k = pCar->GetSpeedometer() * 3.6f * 0.01f;
		#define m01(x)  std::min(1.f, std::max(0.f, (float) (x) ))
		h.txVel->setTextColour(Colour(m01(k*2.f), m01(0.5f+k*1.5f-k*k*2.5f), m01(1+k*0.8f-k*k*3.5f)));
	#else  //  vel clr
		float vel = pCar->GetSpeedometer() * 3.6f;
		h.txVel->setTextColour(GetVelClr(vel));
	#endif
	}

	//  💨 boost fuel (time)  ------
	if (h.txBFuel && pCar && h.txBFuel->getVisible())
	{
		float f = 0.1f * std::min(10.f, pCar->dynamics.boostFuel);
		//ColourValue c;  c.setHSB(0.6f - f*0.1f, 0.7f + f*0.3f, 0.8f + f*0.2f);
		//h.txBFuel->setTextColour(Colour(c.r,c.g,c.b));
		h.txBFuel->setTextColour(Colour(f*0.35f +0.25f, std::min(1.f, f*0.9f +0.5f), std::min(1.f, f*0.6f +0.75f)));
		h.txBFuel->setCaption(fToStr(pCar->dynamics.boostFuel,1,3));
	}

	//  🔨 damage %  ------
	if (h.txDamage && pCar && h.txDamage->getVisible())
	{
		float d = std::min(100.f, Math::Floor(pCar->dynamics.fDamage));
		float& a = h.dmgBlink;
		if (d != h.dmgOld)  // par blink
		{	float b = std::max(0.f, std::min(1.f, 0.3f + (d - h.dmgOld) * 0.1f));
			if (b > a)  a = b;
			h.dmgOld = d;
		}
		h.txDamage->setCaption(fToStr(d,0,3)+" %");  d *= 0.01f;
		// float e = std::min(1.f, 0.8f + d*2.f);
		//h.txDamage->setTextColour(Colour(e-d*d*0.4f, std::max(0.f, e-d), std::max(0.f, e-d*2.f) ));
		h.txDamage->setTextColour(Colour(1.f, 1.f - 0.4f*d, 1.f - 0.6f*d ));
		
		if (h.dmgOld >= 99.f)  a = 1.f;
		d = std::max(0.01f, d-0.5f);
		if (a < d)  a = d;
		h.imgDamage->setColour(Colour(1.f, 0.1f, 0.1f));
		h.imgDamage->setAlpha(a);
		a -= time;
	}
	
	//  abs, tcs on  ------
	if (h.txAbs && h.txTcs && pCar)
	{
		bool vis = pCar->GetABSEnabled();  h.txAbs->setVisible(vis);
		if (vis)  h.txAbs->setAlpha(pCar->GetABSActive() ? 1.f : 0.6f);
		
		vis = pCar->GetTCSEnabled();  h.txTcs->setVisible(vis);
		if (vis)  h.txTcs->setAlpha(pCar->GetTCSActive() ? 1.f : 0.6f);
	}
}


//  ⏱️ Times etc
//-------------------------------------------------------------------------------------------------------------------
//  list nearest, dist
class LiCol
{
public:
	int id;
	float dist;
	
	bool operator<(const LiCol& other)
	{
		return dist < other.dist;
	}
};
void CHud::UpdTimes(int carId, Hud& h, float time, CAR* pCar, CarModel* pCarM)
{
	///  times, race pos  -----------------------------
	if (pSet->show_times && pCar)
	{
		TIMER& tim = app->pGame->timer;
		bool hasLaps = pSet->game.hasLaps();
		if (hasLaps)
		{	//  place
			if (pCarM->iWonPlace > 0 && h.txPlace)
			{
				String s = TR("---  "+toStr(pCarM->iWonPlace)+" #{TBPlace}  ---");
				h.txPlace->setCaption(s);
				const static Colour clrPlace[5] = {
					Colour(0.4,1,0.2), Colour(1,1,0.3), Colour(1,0.7,0.2), Colour(1,0.5,0.2), Colour(0.7,0.6,0.6) };
				h.txPlace->setTextColour(clrPlace[std::min(4, pCarM->iWonPlace-1)]);
				h.bckPlace->setVisible(true);
		}	}

		//  times  ------------------------------
		bool cur = pCarM->iCurChk >= 0 && !app->vTimeAtChks.empty();
		float ghTimeES = cur ? app->vTimeAtChks[pCarM->iCurChk] : 0.f;
		float part = ghTimeES / app->fLastTime;  // fraction which track ghost has driven

		bool coldStart = tim.GetCurrentLap(carId) == 1;  // was 0
		float carMul = app->GetCarTimeMul(pSet->game.car[carId], pSet->game.sim_mode);
		//| cur
		float ghTimeC = ghTimeES + (coldStart ? 0 : 1);
		float ghTime = ghTimeC * carMul;  // scaled
		float diffT = pCarM->timeAtCurChk - ghTime;  // cur car diff at chk
		float diff = 0.f;  // on hud
		
		//!- if (pCarM->updTimes || pCarM->updLap)
		{	pCarM->updTimes = false;

			//  ⏱️ track time, points
			float last = tim.GetLastLap(carId), best = tim.GetBestLap(carId, pSet->game.track_reversed);
			float timeCur = last < 0.1f ? best : last;
			float timeTrk = app->data->tracks->times[pSet->game.track];
			bool b = timeTrk > 0.f && timeCur > 0.f;

			float time = (/*place*/1 * app->data->cars->magic * timeTrk + timeTrk) / carMul;  // trk time (for 1st place)
			//float t1pl = data->carsXml.magic * timeTrk;

			float points = 0.f, curPoints = 0.f;
			int place = app->GetRacePos(timeCur, timeTrk, carMul, coldStart, &points);
			//| cur
			float timCC = timeTrk + (coldStart ? 0 : 1);
			float timCu = timCC * carMul;
			diff = pCarM->timeAtCurChk + /*(coldStart ? 1:0)*carMul*/ - time * part;  ///new

			float chkPoints = 0.f;  // cur, at chk, assume diff time later than track ghost
			int chkPlace = app->GetRacePos(timCu + diffT, timeTrk, carMul, coldStart, &chkPoints);
			bool any = cur || b;
	
			h.sTimes =
				"\n#80E080" + StrTime(time)+
				"\n#D0D040" + (cur ? toStr( chkPlace )     : "--")+
				"\n#F0A040" + (cur ? fToStr(chkPoints,1,3) : "--");

			float dlap = last - time;
			h.sLap =
				"#D0E8FF"+TR("#{TBLapResults}") +
				"\n#80C8FF" + StrTime(last)+
				(last > 0.f ? String("  ") + (dlap > 0.f ? "#80E0FF+" : "#60FF60-") + fToStr(fabs(dlap), 1,3) : "")+
				"\n#80E0E0" + StrTime(best)+
				"\n#80E080" + StrTime(time)+
				"\n#D0D040" + (b ? toStr(place)      : "--")+
				"\n#F0A040" + (b ? fToStr(points,1,3) : "--");
			if (h.txLap)
				h.txLap->setCaption(h.sLap);
		}
		if (h.txTimes)
		if (pSet->game.collect_num >= 0)
		{
			//  💎 collection
			const auto& cols = app->sc->collects;
			int all = cols.size();
			auto pos = pCarM->ndMain->getPosition();

			std::list<LiCol> li;  // nearest few
			//all = app->sc->collects.size();
			// const Collect& colx = app->data->collect->all[pSet->game.collect_num];
			
			for (int i=0; i < all; ++i)
			if (!cols[i].collected && cols[i].nd)
				// (( (1u << cols[i].group) & colx.groups)))
			{	LiCol c;
				c.id = i;
				c.dist = pos.squaredDistance(cols[i].pos);
				li.push_back(c);

				//  scale by dist-  todo in shader..
				// float d = min(0.5f, max(10.f, c.dist * 0.04f));
				// cols[i].ndBeam->setScale(d * Vector3(0.1f, 10.f, 0.1f));
				// cols[i].ndBeam->_getFullTransformUpdated();
			}
			li.sort();

			//  distance texts
			int id = pCarM->iIndex;
			String ss = "\n";
			int i=0;  auto it = li.begin();
			while (i < MAX_ArrCol && it != li.end())
			{
				//  upd arrows
				if (pSet->check_arrow && // !bRplPlay &&
					pCarM->cType == CarModel::CT_LOCAL)
					arrCol[id][i].posTo = cols[(*it).id].pos;

				// todo: 3d txt on arrows?
				ss += "\n";
				float d = sqrt((*it).dist);
				if (d < 3.f)  ss += "#20FFFF";  else
				if (d < 10.f)  ss += "#20FF20";  else
				if (d < 40.f)  ss += "#A0FF20";  else
				if (d < 100.f)  ss += "#FFFF20";  else
				if (d < 400.f)  ss += "#FFB020";  else
				if (d < 800.f)  ss += "#FF8020";  else
				if (d < 1400.f)  ss += "#F080F0";  else
				if (d < 2000.f)  ss += "#8080E0";  else
								 ss += "#4080C0";
				ss += iToStr((*it).id,2)+": "+fToStr(d, 0)+" m";
				++it;  ++i;
			}
			while (i < MAX_ArrCol)
			{	// hide rest if less
				arrCol[id][i].nodeRot->setVisible(0);
				++i;
			}
			h.txCollect->setCaption(ss);

			all = pSet->game.collect_all; //vis, for group
			const Collection& col = app->data->collect->all[pSet->game.collect_num];
			auto* pro = app->gui->progressC.Get(col.name);
			float best = pro && pro->bestTime < 1e5 ?  pro->bestTime : 0.f;

			String s;  // collection
			int cp = app->iCollectedPrize;
			switch (cp)
			{
			case -2:  break;
			case -1:
				s = TR("#F08020#{DidntPass}");  break;
			case 0:  case 1:  case 2:
				s = app->gui->StrPrize(cp+1);  break;
			}
			s +="\n#D0B0FF" + toStr(app->iCollected)+" / "+toStr(all) +
				"\n#A0E0E0" + StrTime(tim.GetPlayerTime(carId))+
				"\n#80E0E0" + StrTime(best);
			h.txTimes->setCaption(s);
		}else
			h.txTimes->setCaption(  // normal race
				(hasLaps ? "#A0E0D0"+toStr(tim.GetCurrentLap(carId)+1)+" / "+toStr(pSet->game.num_laps) : "") +
				"\n#A0E0E0" + StrTime(tim.GetPlayerTime(carId))+
				(cur ? String("  ") + (diff > 0.f ? "#80E0FF+" : "#60FF60-")+
				 	fToStr(fabs(diff), 1,3) : "")+
				h.sTimes+
				"\n#E0B090" + fToStr(pCarM->trackPercent,0,1)+"%"/**/ );

		if (h.txLap)
		{
			//if (pCarM->updLap)
			//{	pCarM->updLap = false;
				//h.txLap->setCaption(h.sLap);
			//}
			float a = std::min(1.f, pCarM->fLapAlpha * 2.f);
			bool hasRoad = app->scn->road && app->scn->road->getNumPoints() > 2;
			bool vis = pSet->show_times && hasRoad && a > 0.f;
			if (vis)
			{	if (app->iLoad1stFrames == -2)  //bLoading)  //  fade out
				{	pCarM->fLapAlpha -= !hasRoad ? 1.f : time * gPar.fadeLapResults;
					if (pCarM->fLapAlpha < 0.f)  pCarM->fLapAlpha = 0.f;
				}
				h.bckLap->setAlpha(a);
				h.txLapTxt->setAlpha(a);  h.txLap->setAlpha(a);
			}
			h.bckLap->setVisible(vis);
			h.txLapTxt->setVisible(vis);  h.txLap->setVisible(vis);
		}
	}

	//  ❌ checkpoint warning  --------
	if (app->scn->road && h.bckWarn && pCarM)
	{
		/* checks debug *
		if (ov[0].oU)  {
			//"ghost:  "  + StrTime(ghost.GetTimeLength()) + "  "  + toStr(ghost.GetNumFrames()) + "\n" +
			//"ghplay: " + StrTime(ghplay.GetTimeLength()) + "  " + toStr(ghplay.GetNumFrames()) + "\n" +
			ov[0].oU->setCaption(String("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n") +
				"         st " + toStr(pCarM->bInSt ? 1:0) + " in" + toStr(pCarM->iInChk) +
				"  |  cur" + toStr(pCarM->iCurChk) + " > next " + toStr(pCarM->iNextChk) +
				"  |  Num " + toStr(pCarM->iNumChks) + " / All " + toStr(road->mChks.size()));
		}	/**/

		if (pCarM->bWrongChk)
			pCarM->fChkTime = gPar.timeShowChkWarn;
			
		bool show = pCarM->fChkTime > 0.f;
		if (show)  pCarM->fChkTime -= time;
		h.bckWarn->setVisible(show && pSet->show_times);
	}

	//  🏁 race countdown  ------
	if (h.txCountdown)
	{
		bool vis = app->pGame->timer.pretime > 0.f && !app->pGame->timer.waiting;
		if (vis)
			h.txCountdown->setCaption(fToStr(app->pGame->timer.pretime,1,3));
		h.txCountdown->setVisible(vis);
	}

	//  🎥 camera cur  ------
	if (h.txCam)
	{	FollowCamera* cam = pCarM->fCam;
		if (cam && cam->updName)
		{	cam->updName = false;
			h.txCam->setCaption(cam->sName);
	}	}
}
