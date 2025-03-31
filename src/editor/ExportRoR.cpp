#include "pch.h"
#include "ExportRoR.h"
#include "enums.h"
#include "Def_Str.h"
#include "BaseApp.h"
#include "settings.h"
#include "paths.h"

#include "CApp.h"
#include "CGui.h"
#include "CScene.h"
#include "CData.h"
#include "TracksXml.h"
#include "Axes.h"
#include "GuiCom.h"

#include <OgreMath.h>
#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreException.h>

#include <exception>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <time.h>
#include <filesystem>
namespace fs = std::filesystem;
using namespace Ogre;
using namespace std;


//  gui events
//------------------------------------------------------------
void CGui::btnExport(WP)
{
	app->ror->ExportTrack();
}

void CGui::btnConvertTerrain(WP)
{
	app->ror->ConvertTerrainTex();
}
void CGui::btnConvertMat(WP)
{
	app->ror->ConvertMat();
}
void CGui::btnConvertSurface(WP)
{
	app->ror->ConvertSurf();
}
void CGui::btnCreateOdef(WP)
{
	app->ror->CreateOdef();
}

void CGui::btnSaveSceneXml(WP)
{
	String dir = app->gcom->TrkDir();
	sc->SaveXml(dir + "scene.xml");
	Status("#{Saved} .xml", 0.5,0.7,1);
}

//  settings
void CGui::editRoRPath(Ed ed)
{
	pSet->pathExportRoR = ed->getCaption();
}
void CGui::editOldSRPath(Ed ed)
{
	pSet->pathExportOldSR =  ed->getCaption();
}

//  road lay tex name
void CGui::slRoR_RoadLay(SV* sv)
{
	int i = sc->rorCfg.roadTerTexLayer;
	auto& td = sc->tds[0];
	txRoR_RoadLay->setCaption(
		i < td.layers.size() ? td.layersAll[td.layers[i]].texFile  // used ter
		: td.layersAll[i].texFile);  // custom last
}


//  🌟 ctor
ExportRoR::ExportRoR(App* app1)
{
	app = app1;
	gui = app->gui;
	pSet = app->pSet;

	scn = app->scn;  // auto set
	sc = scn->sc;
	cfg = &sc->rorCfg;

	data = scn->data;
	pre = data->pre;

	version = 1;  // increase..

#if 1  //  data in packs
	copyTerTex =0;  copyVeget =0;  copyGrass =0;  copyObjs =0;
#else  //  data inside track
	copyTerTex =1;  copyVeget =1;  copyGrass =1;  copyObjs =1;
#endif
}

//  ⛓️ utils
//------------------------------------------------------------

//  util convert SR pos to RoR pos
Ogre::String ExportRoR::strPos(const Ogre::Vector3& pos)
{
	stringstream ss;
	// todo?  add -? td.posZ
	// ss << half - pos.z - 1.f 
	ss << half - pos.z << ", " << pos.y - hmin << ", " << pos.x + half << ", ";
	return ss.str();
}

//  util copy file
bool ExportRoR::CopyFile(const std::string& from, const std::string& to)
{
	try
	{
	#if 0  // leave
		if (!fs::exists(to.c_str()))
			fs::copy_file(from.c_str(), to.c_str());
	#else  // replace
		if (fs::exists(to.c_str()))
			fs::remove(to.c_str());
		fs::copy_file(from.c_str(), to.c_str());
	#endif
		String s = "Copied: " + from + "\n        to: " + to /*+ "\n  "*/;
		gui->Exp(CGui::DBG, s);
	}
	catch (exception ex)
	{
		String s = "Error copying file: " + from + "\n  to: " + to + "\n  " + ex.what();
		gui->Exp(CGui::WARN, s);
		return false;
	}
	return true;
}

//  rename colliding names with RoR
void ExportRoR::RenameMesh(std::string& mesh)
{
	string name = mesh;
		name = StringUtil::replaceAll(name, "palm2", "palm22");
	if (name == mesh)
		name = StringUtil::replaceAll(name, "palm", "palm11");
	if (name != mesh)
		gui->Exp(CGui::TXT, "renamed to: "+name);
	mesh = name;
}

//  ⚙️ Setup path, name, create dir
//------------------------------------------------------------
void ExportRoR::SetupPath()
{
	string& dirRoR = pSet->pathExportRoR;
	if (dirRoR.empty())
	{	gui->Exp(CGui::ERR, "Export path empty. Need to set export RoR path first.");
		return;
	}
	//  end with /
	if (!StringUtil::endsWith(dirRoR, "\\") &&
		!StringUtil::endsWith(dirRoR, "/"))
		dirRoR += "/";

	//  dir  track name
	name = pSet->gui.track;
	dirName = "_" + name;  // _ for on top

	const string dirTrk = dirRoR + dirName;
	if (!PATHS::CreateDir(dirTrk))
	{	gui->Exp(CGui::ERR, "Can't create track dir: "+dirTrk);
		return;
	}
	path = dirTrk + "/";
}


//  Export current track for Rigs Of Rods
//------------------------------------------------------------------------------------------------------------------------
void ExportRoR::ExportTrack()  // whole, full
{
	gui->edExportLog->setCaption("");  // clear log
	
	if (first)
		ListPacks();
	first =0;
	packs.clear();
	
	//  Gui status
	Ogre::Timer ti;
	gui->Status("RoR Export..", 1,0.5,1);
	gui->Exp(CGui::INFO, "Export to RoR started..");


	SetupPath();

	ExportTerrain();

	ExportWaterSky();

	ExportObjects();

	ExportVeget();

	ExportRoad();

	
	//  no, copy common .material to track dir  (done by convert in materials/)
	if (0)
	{
	string pathMtr = pSet->pathExportRoR + "materials/";
	std::vector<string> files{
		"objects_static.material",
		"rocks.material",
		"trees_ch.material",
		"trees.material",
		"trees_old.material"};
	for (auto& mtr : files)
		CopyFile(pathMtr + mtr, path + mtr);
	}
	// todo  and their textures? for used mesh materials ?


	//  🖼️ copy Preview  mini
	//------------------------------------------------------------
	// String pathPrv = PATHS::Tracks() + "/" + name + "/preview/";  // new SR
	string pathPrv = pSet->pathExportOldSR + "tracks/" + name + "/preview/";  // old SR
	string from = pathPrv + "view.jpg", to = path + name + "-mini.jpg";
	CopyFile(from, to);


	//  get Authors etc from tracks.ini
	//------------------------------------------------------------
	string authors = "CryHam", scenery;
	int difficulty = -1, rating = -1;
	int trkId = 0;  // N from ini  // todo Test* same-

	int id = scn->data->tracks->trkmap[pSet->gui.track];
	if (id > 0)
	{	const TrackInfo& ti = scn->data->tracks->trks[id-1];

		difficulty = ti.diff;  rating = ti.rating;
		scenery = ti.scenery;
		authors = ti.author=="CH" ? "CryHam" : ti.author;
		trkId = ti.n;
	}else
		gui->Exp(CGui::ERR, "Track not in tracks.ini, no guid id or authors set.");
	

	//------------------------------------------------------------------------------------------------------------------------
	//  🏞️ Track/map setup  save  .terrn2
	//------------------------------------------------------------------------------------------------------------------------
	string terrn2File = path + name + ".terrn2";
	ofstream trn;
	trn.open(terrn2File.c_str(), std::ios_base::out);

	trn << "[General]\n";
	trn << "Name = " + dirName + "\n";
	trn << "GeometryConfig = " + name + ".otc\n";
	trn << "\n";
	trn << "Water=" << water << "\n";
	if (water)
		trn << "WaterLine=" << Ywater << "\n";
	trn << "\n";
	trn << "AmbientColor = 1.0, 1.0, 1.0\n";  // unused-

	Vector3 st = Axes::toOgre(sc->startPos[0]);  // start
	trn << "StartPosition = " << strPos(st) + "\n";
	const float* rot = &sc->startRot[0][0];
	Quaternion q(rot[0],rot[1],rot[2],rot[3]);
	Real a = q.getPitch().valueDegrees();
	trn << "StartRotation = " << a << "\n";  // todo yaw:  0 right  90 down  180 left  270 up
	trn << "\n";

	trn << "CaelumConfigFile = " + name + ".os\n";
	trn << "CaelumFogStart = " << sc->fogStart * cfg->fogMul << "\n";  // fog
	trn << "CaelumFogEnd = " << sc->fogEnd * cfg->fogMul << "\n";

	trn << "SandStormCubeMap = tracks/skyboxcol\n";  // sky meh-
	trn << "Gravity = " << -sc->gravity << "\n";
	trn << "\n";

	trn << "CategoryID = 129\n";
	trn << "Version = " << version << "\n";
	
	//  Guid
	//------------------------------------------------------------
	//  hash from tacrk name
	size_t hsh = std::hash<std::string>()(name);
	hsh &= 0xFFFFFFFFFFFFu;  // max 12 chars
	char hex[32];  sprintf(hex, "%012zX", hsh);
	string shex = hex;  //if (shex.length() > 12)  shex = shex.substr(0,12);
	gui->Exp(CGui::TXT, "Track id: " + toStr(trkId) + "  Name hash: " + shex);
	
	trn << "GUID = 11223344-5566-7788-" << fToStr(trkId,0,4,'0') <<"-"<< shex <<"\n";
	trn << "\n";

	//  surfaces   if has groundmodel, define landuse file
	trn << "TractionMap = " << name + "-landuse.cfg\n";
	trn << "\n";


	//  📊 Info text
	//------------------------------------------------------------
	trn << "[Authors]\n";
	auto e = "   .\n";
	trn << "Authors = " + authors << e;
	trn << "Conversion = Exported from Stunt Rally 3 Track Editor, version: " << SET_VER << e;
	trn << "License = Track: GPLv3. For data see inside asset packs _*.txt files" << e;

	//  datetime now
	time_t now = time(0);
	tm tn;  tn = *localtime(&now);
	char dtm[80];  strftime(dtm, sizeof(dtm), "%Y-%m-%d.%X", &tn);
	trn << "Date = " << dtm << e;

	//  extra info from SR3 track
	if (!scenery.empty())
		trn << "stat1 = " << "Scenery:  " << scenery << e;
	if (difficulty >= 0)
		trn << "stat2 = " << "Difficulty:  " << TR("#{Diff"+toStr(difficulty)+"}") << e;  // no TR? _en
	if (rating >= 0)
		trn << "stat3 = " << "Rating:  " << rating << " / 6" << e;

	const bool roadtxt = !scn->roads.empty();
	if (roadtxt)
	{	auto& rd = scn->roads[0];
		auto len = rd->st.Length;  // road stats

		trn << "stat4 = " << "Length:  " <<  fToStr(len * 0.001f,2,4) << " km  /  " << fToStr(len * 0.000621371f,2,4) << " mi" << e;
		trn << "stat5 = " << "Width average:  " << fToStr(rd->st.WidthAvg,1,3) << " m" << e;
		trn << "stat6 = " << "Height range:  " << fToStr(rd->st.HeightDiff,0,2) << " m" << e;
		trn << "stat7 = " << "Bridges:  " << fToStr(rd->st.OnTer,0,2) << " %" << e;
		// trn << "stat7 = " << "bank angle avg: " << fToStr(rd->st.bankAvg,0,2) << "\n";
		trn << "stat8 = " << "Max banking angle:  " << fToStr(rd->st.bankMax,0,1) << "°" << e;

		trn << "Description = "+rd->sTxtDescr << e;  // text
		trn << "Drive_Advice = "+rd->sTxtAdvice << e;
	}
	trn << " \n";


	//  📦 packs
	//------------------------------------------------------------
	trn << "[AssetPacks]\n";
	packs.emplace("sr-checkpoint-v1");  // common, always
	packs.emplace("sr-materials-v1");
	packs.emplace("sr-grass-v1");
	
	// packs.emplace("sr-terrain-v1");  // auto if needed etc
	// packs.emplace("sr-rocks-v1");

	gui->Exp(CGui::NOTE, "\nNeeded packs:");
	for (auto& p : packs)
	{
		gui->Exp(CGui::TXT, p);
		trn << p << ".assetpack=\n";
	}
	trn << "\n";


	//  road veget objs .tobj
	//------------------------------------------------------------
	trn << "[Objects]\n";
	if (hasRoad)
		trn << name+"-road.tobj=\n";
	if (hasVeget)
		trn << name+"-veget.tobj=\n";
	if (hasObjects)
		trn << name+"-obj.tobj=\n";
	trn << "\n";

	if (hasRoadChks)
	{	trn << "[Scripts]\n";
		trn << name + ".as=\n";	 //".terrn.as=\n";
	}

	trn.close();


	gui->Exp(CGui::INFO, "\nExport to RoR end.");
	gui->Exp(CGui::INFO, "Time Total: " + fToStr(ti.getMilliseconds()/1000.f,1,3) + " s");
}
