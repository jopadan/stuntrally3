#pragma once
#include "configfile.h"


//  🎨 paint  for vehicle
enum CPtype
{
	CP_OneClr, CP_DiffSpec, CP_3Clrs
};
struct CarPaint
{
	int type = CP_OneClr;

	struct Clr
	{	float hue = 0.35f, sat = 0.98f, val = 0.8f;
	} clr[2];  //  diff, spec
	
	float gloss = 0.9f,  // mix:  0 all diff .. 1 all spec
		rough = 0.25f,  // roughness
		clear_coat = 1.f,  // reflection
		clear_rough = 0.01f,
		fresnel = 1.f;  // metalness

	//  3 fresnel paints (color changing) CP_3Clrs
	Clr paints[3];
	float
		paintMulAll = 0.2f,
		paintMul2nd = 4.f,
		paintPow3rd = 3.f;

	std::string group;  // gui txt title
	bool new_line = false;  // gui
	int rate = 0;  // user rating / bookmark

	Clr glow;  // underglow
	void GlowDef();

	int y =0;  // gui only auto
};
const int MaxPaints = 990;  // par max ini


//  🎨 paints.ini  palette for Gui
//--------------------------------------------------------------------
class PaintsIni
{
public:
	std::vector<CarPaint> v, vf;  // orig from ini and filtered
	int perRow = 12, imgSize = 18;  // gui params
	
//------------------------------------------
	// PaintsIni();

	template <typename T>
	static bool Param(CONFIGFILE & conf, bool write, std::string pname, T & value)
	{
		if (write)
		{	conf.SetParam(pname, value);
			return true;
		}else
			return conf.GetParam(pname, value);
	}
	static void SerPaint(bool write, CONFIGFILE & config, const std::string s, CarPaint& p);

	void Serialize(bool write, CONFIGFILE & config);
	void Load(std::string sfile);
	void Save(std::string sfile);
};
