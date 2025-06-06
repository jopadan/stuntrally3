#ifndef SR_EDITOR
#include "pch.h"
#include "par.h"
#include "cardefs.h"
#include "cardynamics.h"
#include "collision_world.h"
#include "settings.h"
#include "game.h"
#include "tobullet.h"

#include "Def_Str.h"
#include "CData.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "ShapeData.h"
#include "SceneClasses.h"
#include "CGame.h"
#include "Buoyancy.h"


//  executed as last function(after integration) in bullet singlestepsimulation
void CARDYNAMICS::updateAction(btCollisionWorld * collisionWorld, btScalar dt)
{
	SynchronizeBody();  // get velocity, position orientation after dt

	UpdateWheelContacts();  // update wheel contacts given new velocity, position

	Tick(dt);  // run internal simulation

	SynchronizeChassis();  // update velocity
}

void CARDYNAMICS::Update()
{
	if (!chassis)  return;//
	btTransform tr;
	chassis->getMotionState()->getWorldTransform(tr);
	chassisRotation = ToMathQuaternion<Dbl>(tr.getRotation());
	chassisCenterOfMass = ToMathVector<Dbl>(tr.getOrigin());
	MATHVECTOR<Dbl,3> com = center_of_mass;
	chassisRotation.RotateVector(com);
	chassisPosition = chassisCenterOfMass - com;
	
	//V*  these ignore fluids by antigravity
	if (!(vtype == V_Hovercar || vtype == V_Drone))
		UpdateBuoyancy();  // 🌊

	if (!inFields.empty())
		UpdateFields();

	//**  veget bush damp
	if (fVegetDamp > 0.f)
	{
		btVector3 v = chassis->getLinearVelocity();
		chassis->applyCentralForce(v * -500 * fVegetDamp);  //par`
		fVegetDamp = 0.f;
	}
}


//  🎆 Fields  ..........................................................
void CARDYNAMICS::UpdateFields()
{
	auto m = chassis->getMass();
	for (auto* fld : inFields)
	switch (fld->type)
	{
		case TF_Gravity:
		{	auto f = m * sc->gravity * fld->factor;
			chassis->applyCentralForce( btVector3(0,0,f) );
		}	break;
		
		case TF_Accel:
		{	auto f = m * fld->factor * 10.f;
		
			chassis->applyCentralForce( f * fld->dir );
		}	break;
		
		case TF_Teleport:
		{
			// pCar->SetPosition(fld->pos2, fld->dir2)
			SetPosition(fld->pos2);  // destination
			// SetOrientation(fld->dir2);  // todo..
		}	break;
		
		case TF_Damp:
		{	btVector3 v = chassis->getLinearVelocity();
			chassis->applyCentralForce(v * -1000.f * fld->factor);
		}	break;
		
		case TF_All:
			break;
	}
	inFields.clear();
}


///  🌊 Buoyancy ................................................................................................
void CARDYNAMICS::UpdateBuoyancy()
{
	if (!sc || (sc->fluids.size() == 0) || !pFluids)  return;

	//float bc = /*sinf(chassisPosition[0]*20.3f)*cosf(chassisPosition[1]*30.4f) +*/
	//	sinf(chassisPosition[0]*0.3f)*cosf(chassisPosition[1]*0.32f);
	//LogO("pos " + toStr((float)chassisPosition[0]) + " " + toStr((float)chassisPosition[1]) + "  b " + toStr(bc));

	for (auto* fb : inFluids)
	if (fb->id >= 0)
	{
		const FluidParams& fp = pFluids->fls[fb->id];

		WaterVolume water;
		//float bump = 1.f + 0.7f * sinf(chassisPosition[0]*fp.bumpFqX)*cosf(chassisPosition[1]*fp.bumpFqY);
		water.density = fp.density /* (1.f + 0.7f * bc)*/;  water.angularDrag = fp.angularDrag;
		water.linearDrag = fp.linearDrag;  water.linearDrag2 = 0.f;  //1.4f;//fp.linearDrag2;
		water.velocity.SetZero();
		water.plane.offset = fb->pos.y;  water.plane.normal = Vec3(0,0,1);
		//todo: fluid boxes rotation yaw, pitch ?-

		RigidBody body;  body.mass = body_mass;
		body.inertia = Vec3(body_inertia.getX(),body_inertia.getY(),body_inertia.getZ());

		///  body initial conditions
		//  pos & rot
		body.x.x = chassisPosition[0];  body.x.y = chassisPosition[1];  body.x.z = chassisPosition[2];
		if (vtype == V_Sphere)
		{	body.q.x = 0.f;  body.q.y = 0.f;  body.q.z = 0.f;  body.q.w = 1.f;  // no rot
		}else
		{	body.q.x = chassisRotation[0];  body.q.y = chassisRotation[1];  body.q.z = chassisRotation[2];  body.q.w = chassisRotation[3];
			body.q.Normalize();//
		}
		//LogO(fToStr(body.q.x,2,4)+" "+fToStr(body.q.y,2,4)+" "+fToStr(body.q.z,2,4)+" "+fToStr(body.q.w,2,4));

		//  vel, ang vel
		btVector3 v = chassis->getLinearVelocity();
		btVector3 a = chassis->getAngularVelocity();
		body.v.x = v.getX();  body.v.y = v.getY();  body.v.z = v.getZ();
		if (vtype == V_Sphere)
		{	body.omega.SetZero();  // no ang vel
		}else
		{	body.omega.x = a.getX();  body.omega.y = a.getY();  body.omega.z = a.getZ();
		}
		body.F.SetZero();  body.T.SetZero();
		
		//  damp for height vel
		body.F.z += fp.heightVelRes * -1000.f * body.v.z;
		
		///  add buoyancy force
		if (ComputeBuoyancy(body, poly, water, 9.8f))
		{
			if (vtype != V_Car)
			{	body.F.x *= 0.15f;  body.F.y *= 0.15f;  }
			chassis->applyCentralForce( btVector3(body.F.x,body.F.y,body.F.z) );
			chassis->applyTorque(       btVector3(body.T.x,body.T.y,body.T.z) );
		}	
	}

	///  ⚫💧 wheel spin force (for mud)
	//_______________________________________________________
	for (int w=0; w < numWheels; ++w)
	{
		if (inFluidsWh[w].size() > 0)  // 0 or 1 is there
		{
			MATHVECTOR<Dbl,3> up(0,0,1);
			Orientation().RotateVector(up);
			float upZ = std::max(0.f, (float)up[2]);
			
			const FluidBox* fb = *inFluidsWh[w].begin();
			if (fb->id >= 0)
			{
				const FluidParams& fp = pFluids->fls[fb->id];

				WHEEL_POSITION wp = WHEEL_POSITION(w);
				float whR = GetWheel(wp).GetRadius() * 1.2f;  //bigger par
				MATHVECTOR<float,3> wheelpos = GetWheelPosition(wp, 0);
				wheelpos[2] -= whR;
				whP[w] = fp.idParticles;
				whDmg[w] = fp.fDamage;
				
				//  height in fluid:  0 just touching surface, 1 fully in fluid
				//  wheel plane distance  water.plane.normal.z = 1  water.plane.offset = fl.pos.y;
				whH[w] = (wheelpos[2] - fb->pos.y) * -0.5f / whR;
				whH[w] = std::max(0.f, std::min(1.f, whH[w]));
				if (vtype == V_Hovercraft)
					whH[w] *= 0.1f;  //V*  less water particles and sound

				if (fp.bWhForce)
				{
					//bool inAir = GetWheelContact(wp).col == NULL;

					//  bump, adds some noise
					MATHVECTOR<Dbl,3> whPos = GetWheelPosition(wp) - chassisPosition;
					float bump = sinf(whPos[0]*fp.bumpFqX)*cosf(whPos[1]*fp.bumpFqY);
					
					float f = std::min(fp.whMaxAngVel, std::max(-fp.whMaxAngVel, (float)wheel[w].GetAngularVelocity() ));
					QUATERNION<Dbl> steer;
					float ba = numWheels==2 && w==0 ? 2.f : 1.f;  //bike
					float angle = -wheel[wp].GetSteerAngle() * fp.whSteerMul * ba  + bump * fp.bumpAng;
					steer.Rotate(angle * PI_d/180.f, 0, 0, 1);

					//  forwards, side, up
					MATHVECTOR<Dbl,3> force(whH[w] * fp.whForceLong * f, 0, /*^ 0*/100.f * whH[w] * fp.whForceUp * upZ);
					(Orientation()*steer).RotateVector(force);
					
					//  wheel spin resistance
					wheel[w].fluidRes = whH[w] * fp.whSpinDamp  * (1.f + bump * fp.bumpAmp);
					
					if (whH[w] > 0.01f /*&& inAir*/)
						chassis->applyForce( ToBulletVector(force), ToBulletVector(whPos) );
				}
			}
		}
		else
		{	whH[w] = 0.f;  wheel[w].fluidRes = 0.f;  whP[w] = -1;	}
	}

}

///  🗒️ print debug info to the given ostream.
///  set p1, p2, etc if debug info part 1, and/or part 2, etc is desired
///..........................................................................................................
void CARDYNAMICS::DebugPrint( std::ostream & out, bool p1, bool p2, bool p3, bool p4 )
{
	using namespace std;
	out.precision(2);  out.width(6);  out << fixed;
	int cnt = pSet->car_dbgtxtcnt, w;

	if (p1)
	{
		#if 0  //  bullet hit data-
			out << "hit P : " << fParIntens << endl;
			//out << "hit t : " << fHitTime << endl;
			out << "v Vel : " << GetSpeed() << endl;
		#endif
		#if 0
			out << "Damage : " << fToStr(fDamage,0,3) << "  "
				//<< vHitCarN.x << ", " << vHitCarN.y << ", " << vHitCarN.z
				//<< "  x " << fToStr(vHitDmgN.x ,2,4)
				//<< "  y " << fToStr(vHitDmgN.y ,2,4)
				//<< "  z " << fToStr(vHitDmgN.z ,2,4)
				<< "  a " << fToStr(fHitDmgA ,2,4)
				<< endl;
			return;
		#endif

		//  body
		{
			//out << "___ Body ___" << endl;  // L| front+back-  W_ left-right+  H/ up+down-
			out << "com: W right+ " << -center_of_mass[1] << " L front+ " << center_of_mass[0] << " H up+ " << center_of_mass[2] << endl;
			out.precision(0);
			out << "mass: " << body.GetMass();

			/*if (hover)			
			{	MATHVECTOR<Dbl,3> sv = -GetVelocity();
				(-Orientation()).RotateVector(sv);
				out << " vel: " << sv << endl << "  au: " << al << endl;
			}/**/
			//  wheel pos, com ratio
			Dbl whf = wheel[0].GetExtendedPosition()[0], whr = wheel[numWheels==2?1:2].GetExtendedPosition()[0];
			out.precision(2);
			out << "  wh fr " << whf << "  rr " << whr;
			out.precision(1);
			out << "  fr% " << (center_of_mass[0]+whf)/(whf-whr)*100 << endl;
			out.precision(0);
			MATRIX3 <Dbl> inertia = body.GetInertiaConst();
			
			out << "inertia: roll " << inertia[0] << " pitch " << inertia[4] << " yaw " << inertia[8] << endl;

			//out << "inertia: " << inertia[0] <<" "<< inertia[4] <<" "<< inertia[8] <<" < "<< inertia[1] <<" "<< inertia[2] <<" "<< inertia[3] <<" "<< inertia[5] <<" "<< inertia[6] <<" "<< inertia[7] << endl;
			//MATHVECTOR<Dbl,3> av = GetAngularVelocity();  Orientation().RotateVector(av);
			//out << "ang vel: " << fToStr(av[0],2,5) <<" "<< fToStr(av[1],2,5) <<" "<< fToStr(av[2],2,5) << endl;
			//out << "pos: " << chassisPosition << endl;
			out.precision(2);
			//MATHVECTOR<Dbl,3> up(0,0,1);  Orientation().RotateVector(up);
			//out << "up: " << up << endl;
			out << endl;
			//out << sHov.c_str() << endl;
		}

		//  fluids
		if (cnt > 1)
		{	out << "in fluids: " << inFluids.size() << " wh:";
			for (w=0; w < numWheels; ++w)  out << " " << inFluidsWh[w].size();
			out << endl;
			out << "wh fl H:";
			for (w=0; w < numWheels; ++w)  out << " " << fToStr(whH[w],1,3);
			out << " \n\n";
		}

		if (cnt > 3)
		{
			engine.DebugPrint(out);  out << endl;
			//fuel_tank.DebugPrint(out);  out << endl;  //mass 8- for ES,
			clutch.DebugPrint(out);  out << endl;
			transmission.DebugPrint(out);	out << endl;
		}

		if (cnt > 5)
		{
			out << "___ Differential ___\n";
			if (drive == RWD)  {
				out << " rear\n";		diff_rear.DebugPrint(out);	}
			else if (drive == FWD)  {
				out << " front\n";		diff_front.DebugPrint(out);	}
			else if (drive == AWD) {
				out << " center\n";		diff_center.DebugPrint(out);
				out << " front\n";		diff_front.DebugPrint(out);
				out << " rear\n";		diff_rear.DebugPrint(out);	}
			else if (drive == WD6) {
				out << " 1 front\n";	diff_front.DebugPrint(out);
				out << " 2 rear\n";		diff_rear.DebugPrint(out);
				out << " 3 rear2\n";	diff_rear2.DebugPrint(out);
				out << " 12 center\n";	diff_center.DebugPrint(out);
				out << " center2\n";	diff_center2.DebugPrint(out);  }
			else if (drive == WD8) {
				out << " 1 front\n";	diff_front.DebugPrint(out);
				out << " 2 rear\n";		diff_rear.DebugPrint(out);
				out << " 3 rear2\n";	diff_rear2.DebugPrint(out);
				out << " 4 rear3\n";	diff_rear3.DebugPrint(out);
				out << " 12 center\n";	diff_center.DebugPrint(out);
				out << " 34 center2\n";	diff_center2.DebugPrint(out);
				out << " center3\n";	diff_center3.DebugPrint(out);  }
			out << endl;
		}
	}

	const static char sWh[MAX_WHEELS][8] = {" FL [^", " FR ^]", " RL [_", " RR _]", " RL2[_", " RR2_]"};
	if (p2)
	{
		out << "\n\n\n\n";
		if (cnt > 4)
		{
			out << "___ Brake ___n";
			for (w=0; w < numWheels; ++w)
			{	out << sWh[w] << endl;	brake[w].DebugPrint(out);  }
		}
		if (cnt > 7)
		{
			out << "\n___ Suspension ___\n";
			for (w=0; w < numWheels; ++w)
			{	out << sWh[w] << endl;	suspension[w].DebugPrint(out);  }
		}
	}

	if (p3)
		if (cnt > 6)
		{
			out << "\n\n___Wheel___\n";
			for (w=0; w < numWheels; ++w)
			{	out << sWh[w] << endl;	wheel[w].DebugPrint(out);  }
		}

	if (p4)
		if (cnt > 0)
		{
			out << "___Aerodynamic___\n";
			Dbl down = GetAerodynamicDownforceCoefficient();
			Dbl drag = GetAeordynamicDragCoefficient();
			out << "down: " << fToStr(down,2,5) << "  drag: " << fToStr(drag,2,4) << endl;

			if (cnt > 2)
			{
			MATHVECTOR<Dbl,3> aero = GetTotalAero();
			out << "total: " << endl;
			out << fToStr(aero[0],0,5) << " " << fToStr(aero[1],0,4) << " " << fToStr(aero[2],0,6) << endl;

			for (const auto& a : aerodynamics)
				a.DebugPrint(out);
			}

			//if (cnt > 1)
			{
			// get force and torque at 160kmh  from ApplyAerodynamicsToBody
			out << "--at 160 kmh--" << endl;
			MATHVECTOR<Dbl,3> wind_force(0), wind_torque(0), air_velocity(0);
			air_velocity[0] = -160/3.6;

			for (const auto& a : aerodynamics)
			{
				MATHVECTOR<Dbl,3> force = a.GetForce(air_velocity, false);
				wind_force = wind_force + force;
				wind_torque = wind_torque + (a.GetPosition() - center_of_mass).cross(force);
			}
			out << "F: " << wind_force << endl << "Tq: " << wind_torque << endl;
			}

			//---
			/*out << "__Tires__" << endl;
			for (int i=0; i < numWheels; ++i)
			{
				CARWHEEL::SlideSlip& sl = wheel[i].slips;
				out << "Fx " << fToStr(sl.Fx,0,6) << "  FxM " << fToStr(sl.Fxm,0,6) << "   Fy " << fToStr(sl.Fy,0,6) << "  FyM " << fToStr(sl.Fym,0,6) << endl;
			}*/
		}
}


//  💫 Update  🚗 car
///..........................................................................................................
void CARDYNAMICS::UpdateBody(Dbl dt, Dbl drive_torque[])
{
	body.Integrate1(dt);
	cam_body.Integrate1(dt);
	//chassis->clearForces();


	//  camera bounce sim - spring and damper
	MATHVECTOR<Dbl,3> p = cam_body.GetPosition(), v = cam_body.GetVelocity();
	MATHVECTOR<Dbl,3> f = p * gPar.camBncSpring + v * gPar.camBncDamp;
	cam_body.ApplyForce(f);
	cam_body.ApplyForce(cam_force);
	cam_force[0]=0.0;  cam_force[1]=0.0;  cam_force[2]=0.0;


	bool car = vtype == V_Car;
	bool hover = isHover();
	if (car)
	{
		UpdateWheelVelocity();

		ApplyEngineTorqueToBody();  //-

		ApplyAerodynamicsToBody(dt);
	}
	

	//  extra damage from scene <><>
	if (sc && pSet->game.damage_type > 0)
	{
		float fRed = pSet->game.damage_type==1 ? 0.5f : 1.f;

		/// <><> terrain layer damage _
		int w;
		for (w=0; w < numWheels; ++w)
		if (!iWhOnRoad[w])
		{
			float d = 0.5f * wheel_contact[w].GetDepth() / wheel[w].GetRadius();
			int mtr = whTerMtr[w]-1;
			auto& td = sc->tds[0];  // 1st ter only-
			if (d < 1.f && mtr >= 0 && mtr < td.layers.size())
			{
				const TerLayer& lay = td.layersAll[td.layers[mtr]];
				if (lay.fDamage > 0.f)
					fDamage += lay.fDamage * fRed * dt;
		}	}

		/// <><> height fog damage _
		if (sc->fHDamage > 0.f && chassisPosition[2] < sc->fogHeight)
		{
			float h = (sc->fogHeight - chassisPosition[2]) / sc->fogHDensity;
			if (h > 0.2f)  //par
				fDamage += sc->fHDamage * h * fRed * dt;
		}

		/// <><> fluid damage _
		for (w=0; w < numWheels; ++w)
		if (whH[w] > 0.01f)
			fDamage += whDmg[w] * whH[w] * fRed * dt;
	}
	

	///***  🌪️ wind push  ~->
	if (sc && sc->windForce > 0.01f)
	{
		//  simple modulation
		float n = 1.f + 0.3f * sin(time*4.3f)*cosf(time*7.74f);
		time += dt;
		float f = body.GetMass() * sc->windForce * n;
		//LogO(fToStr(n,4,6));
		float yaw = sc->windYaw * PI_d/180.f;
		MATHVECTOR<Dbl,3> v(f*cosf(yaw), f*sinf(yaw), 0);
		ApplyForce(v);
	}

	///***  ^ manual car flip over  ----------------------------
	if ((doFlip > 0.01f || doFlip < -0.01f) &&
		pSet->game.flip_type > 0 && fDamage < 100.f)
	{
		MATHVECTOR<Dbl,3> av = GetAngularVelocity();  Orientation().RotateVector(av);
		Dbl angvel = fabs(av[0]);
		if (angvel < 2.0)  // max rot vel allowed
		{
		float t = 20000.f * doFlip * flip_mul;  // strength

		if (pSet->game.flip_type == 1)  // fuel dec
		{
			boostFuel -= doFlip > 0.f ? doFlip * dt : -doFlip * dt;
			if (boostFuel < 0.f)  boostFuel = 0.f;
			if (boostFuel <= 0.f)  t = 0.0;
		}
		MATHVECTOR<Dbl,3> v(t,0,0);
		Orientation().RotateVector(v);
		ApplyTorque(v);
		}
	}

	///***  💨 boost  -------------------------------------------
	if (vtype != V_Sphere &&
		doBoost > 0.01f && pSet->game.boost_type > 0)
	{
		/// <><> damage reduce
		float dmg = fDamage >= 80.f ? 0.f : (130.f - fDamage)*0.01f;
		boostVal = doBoost * dmg;
		if (pSet->game.boost_type == 1 || pSet->game.boost_type == 2)  // fuel dec
		{
			boostFuel -= doBoost * dt;
			if (boostFuel < 0.f)  boostFuel = 0.f;
			if (boostFuel <= 0.f)  boostVal = 0.f;
		}
		if (boostVal > 0.01f)
		{
			float f = body.GetMass() * boostVal * 12.f * pSet->game.boost_power;  // power
			MATHVECTOR<Dbl,3> v(f,0,0);
			Orientation().RotateVector(v);
			ApplyForce(v);
		}
	}else
		boostVal = 0.f;
	
	fBoostFov += (boostVal - fBoostFov) * pSet->fov_smooth * 0.0001f;
	
	//  💨 add fuel over time
	if (pSet->game.boost_type == 2 && pGame->timer.pretime < 0.001f && fDamage < 100.f)
	{
		boostFuel += dt * pSet->game.boost_add_sec;
		if (boostFuel > pSet->game.boost_max)  boostFuel = pSet->game.boost_max;
	}
	///***  --------------------------------------------------
	
	
	///  🚤 Hovercraft
	if (isHover())  //V*
		SimulateHover(dt);
	else
	///  🚀 Spaceship
	if (vtype == V_Spaceship)
		SimulateSpaceship(dt);
	else
	///  🔘 Sphere
	if (vtype == V_Sphere)
		SimulateSphere(dt);
	

	int i;
		Dbl normal_force[MAX_WHEELS];
	if (car || hover)
	{
		for (i = 0; i < numWheels; ++i)
		{
			MATHVECTOR<Dbl,3> suspension_force = UpdateSuspension(i, dt);
			normal_force[i] = suspension_force.dot(wheel_contact[i].GetNormal());
			if (normal_force[i] < 0) normal_force[i] = 0;

			MATHVECTOR<Dbl,3> tire_friction = ApplyTireForce(i, normal_force[i], wheel_orientation[i]);
			if (car)
				ApplyWheelTorque(dt, drive_torque[i], i, tire_friction, wheel_orientation[i]);
		}
		if (car)  DampAtEnd();
	}

	body.Integrate2(dt);
	cam_body.Integrate2(dt);
	//chassis->integrateVelocities(dt);

	// update wheel state
	if (car || hover)
	{
		for (i = 0; i < numWheels; ++i)
		{
			wheel_position[i] = GetWheelPositionAtDisplacement(WHEEL_POSITION(i), suspension[i].GetDisplacementPercent());
			wheel_orientation[i] = Orientation() * GetWheelSteeringAndSuspensionOrientation(WHEEL_POSITION(i));
		}
		InterpolateWheelContacts(dt);

		for (i = 0; i < numWheels; ++i)
		{
			if (abs)  DoABS(i, normal_force[i]);
			if (tcs)  DoTCS(i, normal_force[i]);
		}
	}
}


///  💫 Tick  (one Simulation step)
//---------------------------------------------------------------------------------
void CARDYNAMICS::Tick(Dbl dt)
{
	// has to happen before UpdateDriveline, overrides clutch, throttle
	UpdateTransmission(dt);

	const int num_repeats = pSet->dyn_iter;  /// 60  30~  org 10
	const float internal_dt = dt / num_repeats;
	// LogO(fToStr(dt,6,9)+" "+fToStr(internal_dt,8,9));  // 0.006250 0.00010417
	for (int i = 0; i < num_repeats; ++i)
	{
		Dbl drive_torque[MAX_WHEELS];

		UpdateDriveline(internal_dt, drive_torque);

		UpdateBody(internal_dt, drive_torque);

		feedback += 0.5 * (wheel[FRONT_LEFT].GetFeedback() + wheel[FRONT_RIGHT].GetFeedback());
	}

	feedback /= (num_repeats + 1);

	fuel_tank.Consume(engine.FuelRate() * dt);
	//engine.SetOutOfGas(fuel_tank.Empty());
	
	if (fHitTime > 0.f)
		fHitTime -= dt * 2.f;

	const float tacho_factor = 0.1;
	tacho_rpm = engine.GetRPM() * tacho_factor + tacho_rpm * (1.0 - tacho_factor);
}
//---------------------------------------------------------------------------------


void CARDYNAMICS::SynchronizeBody()
{
	MATHVECTOR<Dbl,3> v = ToMathVector<Dbl>(chassis->getLinearVelocity());
	MATHVECTOR<Dbl,3> w = ToMathVector<Dbl>(chassis->getAngularVelocity());
	MATHVECTOR<Dbl,3> p = ToMathVector<Dbl>(chassis->getCenterOfMassPosition());
	QUATERNION<Dbl> q = ToMathQuaternion<Dbl>(chassis->getOrientation());
	body.SetPosition(p);
	body.SetOrientation(q);
	body.SetVelocity(v);
	body.SetAngularVelocity(w);
}

void CARDYNAMICS::SynchronizeChassis()
{
	chassis->setLinearVelocity(ToBulletVector(body.GetVelocity()));
	chassis->setAngularVelocity(ToBulletVector(body.GetAngularVelocity()));
}

void CARDYNAMICS::UpdateWheelContacts()
{
#if 1
	MATHVECTOR<float,3> raydir = GetDownVector();
	for (int i = 0; i < numWheels; ++i)
	{
		COLLISION_CONTACT & wheelContact = wheel_contact[WHEEL_POSITION(i)];
		MATHVECTOR<float,3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
		raystart = raystart - raydir * wheel[i].GetRadius();// *0.5;  //*?!
		float raylen = wheel[i].GetRayLength();
		// 🎯 wheel cast ray
		world->CastRay( raystart, raydir, raylen,
			chassis, wheelContact, this,i,
			!pSet->game.collis_cars,
			vtype == V_Hovercraft );  //V* does not go underwater  todo: if fluid deep .. how?
	}
#else
	MATHVECTOR<Dbl,3> x(1, 0, 0);
	Orientation().RotateVector(x);
	MATHVECTOR<Dbl,3> y(0, 1, 0);
	Orientation().RotateVector(x);

	const int aa = 3, bb = 2;  // todo .. more wheel rays
	// const int aa = 0, bb = 0;
	for (int i = 0; i < numWheels; ++i)
	{
		std::vector<COLLISION_CONTACT> vwc;
		for (int b = -bb; b <= bb; ++b)
		for (int a = -aa; a <= aa; ++a)
		{
			Dbl ang = a * PI_d / 6.f, bng = b * PI_d / 6.f;
			MATHVECTOR<Dbl,3> v(sinf(ang), sinf(bng), -cosf(ang));
			v.Normalize();
			Orientation().RotateVector(v);
			MATHVECTOR<float,3> raydir = v;

			// COLLISION_CONTACT & wheelContact = wheel_contact[WHEEL_POSITION(i)];
			COLLISION_CONTACT wc;
			MATHVECTOR<float,3> raystart = LocalToWorld(wheel[i].GetExtendedPosition());
			raystart = raystart - raydir * wheel[i].GetRadius();// *0.5;  //*?!
			float raylen = wheel[i].GetRayLength();
			// float raylen = wheel[i].GetRadius();  //-
			
			// 🎯 wheel cast ray
			bool hit = world->CastRay( raystart, raydir, raylen,
				chassis, wc, this,i,
				!pSet->game.collis_cars,
				vtype == V_Hovercraft );  //V* does not go underwater  todo: if fluid deep .. how?
			// if (hit 
			vwc.push_back(wc);
		}
		
		float depth = 1000.f;
		COLLISION_CONTACT & cls = vwc[0];
		for (auto& wc : vwc)
			if (wc.GetDepth() < depth)
			{
				depth = wc.GetDepth();
				cls = wc;
			}
		wheel_contact[WHEEL_POSITION(i)] = cls;
	}
#endif
}


/// calculate the center of mass, calculate the total mass of the body, calculate the inertia tensor
/// then store this information in the rigid body
void CARDYNAMICS::UpdateMass()
{
	typedef std::pair <Dbl, MATHVECTOR<Dbl,3> > MASS_PAIR;

	Dbl total_mass(0);
	center_of_mass.Set(0,0,0);

	// calculate the total mass, and center of mass
	for (const auto& i : mass_only_particles)
	{
		// add the current mass to the total mass
		total_mass += i.first;

		// incorporate the current mass into the center of mass
		center_of_mass = center_of_mass + i.second * i.first;
	}

	// account for fuel
	total_mass += fuel_tank.GetMass();
	center_of_mass = center_of_mass + fuel_tank.GetPosition() * fuel_tank.GetMass();

	body.SetMass(total_mass);
	cam_body.SetMass(1350/*total_mass/**/ * gPar.camBncMass);
	fBncMass = 1350;//1.0; //1350.0 / total_mass;

	center_of_mass = center_of_mass * (1.0 / total_mass);
	
	// calculate the inertia tensor
	MATRIX3 <Dbl> inertia;
	for (int i = 0; i < 9; ++i)
		inertia[i] = 0;

	for (const auto& i : mass_only_particles)
	{
		// transform into the rigid body coordinates
		MATHVECTOR<Dbl,3> pos = i.second - center_of_mass;
		Dbl mass = i.first;

		// add the current mass to the inertia tensor
		inertia[0] += mass * ( pos[1] * pos[1] + pos[2] * pos[2] );
		inertia[1] -= mass * ( pos[0] * pos[1] );
		inertia[2] -= mass * ( pos[0] * pos[2] );
		inertia[3] = inertia[1];
		inertia[4] += mass * ( pos[2] * pos[2] + pos[0] * pos[0] );
		inertia[5] -= mass * ( pos[1] * pos[2] );
		inertia[6] = inertia[2];
		inertia[7] = inertia[5];
		inertia[8] += mass * ( pos[0] * pos[0] + pos[1] * pos[1] );
	}
	// inertia.DebugPrint(std::cout);
	body.SetInertia( inertia );
}

//  damp all vehicles if  ----
//  champ, chall ended or preview
bool CARDYNAMICS::DampAtEnd()
{
	float edamp = gPar.carPrv > 0 ? -10000.f :
		pGame->timer.end_sim ? -500.f : 0.f;
	if (edamp < 0.f)
	{
		MATHVECTOR<Dbl,3> sv = GetVelocity();
		ApplyForce(sv * edamp);

		MATHVECTOR<Dbl,3> av = GetAngularVelocity();
		ApplyTorque(av * edamp * 0.3f);
		return true;
	}
	return false;
}

///  🚤 Hovercraft, Hovercar, Drone
///..........................................................................................................
void CARDYNAMICS::SimulateHover(Dbl dt)
{
	//vtype == V_Hovercar || vtype == V_Hovercraft || vtype == V_Drone

	float dmg = fDamage > 50.f ? 1.f - (fDamage-50.f)*0.02f : 1.f;
	float dmgE = 1.f - 0.2 * dmg;

	//  handbrake faster
	Dbl h = brake[0].GetHandbrakeFactor();

	//  vel
	MATHVECTOR<Dbl,3> sv = -GetVelocity();
	(-Orientation()).RotateVector(sv);
	MATHVECTOR<Dbl,3> av = GetAngularVelocity();

	//  steer  < >
	bool rear = sv[0] > 0.0;
	Dbl rr = 1.0 + h; //std::max(-1.0, std::min(1.0, -sv[0] * 0.4));  //par..
	if (gPar.carPrv > 0)  rr = 0.0;

	MATHVECTOR<Dbl,3> t(0,0, -1000.0 * rr * hov.steerForce * steerValue * dmgE);
	Orientation().RotateVector(t);
	Dbl damp = hov.steerDamp;  //damp *= 1 - fabs(steerValue);
	ApplyTorque(t - av * damp * 1000.0);  // rotation damping

	
	//  engine  ^
	float vel = sv.Magnitude(),  //  decrease power with velocity
		velMul = 1.f - std::min(1.f, hov.engineVelDec * vel),
		velMulR = 1.f - std::min(1.f, hov.engineVelDecR * vel);
		//sHov += " m  "+fToStr(velMul,2,5)+"\n";

	Dbl brk = brake[0].GetBrakeFactor() * (1.0 - h);
	float f = hov.engineForce * velMul * hov_throttle
			- hov.brakeForce * (rear ? velMulR : 1.f) * brk;
	if (gPar.carPrv > 0)  f = 0.0;

	MATHVECTOR<Dbl,3> vf(body.GetMass() * f * dmgE, 0, 0);
	Orientation().RotateVector(vf);
	ApplyForce(vf);

	Dbl low = gPar.carPrv ? 12.0 :
		12.0 + h * 52.0;  // par  down force ..
	MATHVECTOR<Dbl,3> vg(0, 0,
		-body.GetMass() * low * dmgE);
	ApplyForce(vg);

	//  todo  h > 0  align to ter normal ..

	if (DampAtEnd())  return;

	//  side, vel damping  --
	sv[0] *= hov.dampAirRes;
	sv[1] *= hov.dampSide;
	sv[2] *= 0;  //! par  sv[2] > 0.0 ? hov.dampUp : hov.dampDn;
	Orientation().RotateVector(sv);
	Dbl ss = /*pipe ? hov.dampPmul :*/ 1;
	ApplyForce(sv * ss);
}


///  🚀 Spaceship hovering
///..........................................................................................................
void CARDYNAMICS::SimulateSpaceship(Dbl dt)
{
	//sHov = "";

	//  destroyed  damping
	if (fDamage >= 100.f)
	{
		btVector3 v = chassis->getLinearVelocity();
		v[2] *= 0.1;
		chassis->applyCentralForce(v * -20);

		btVector3 av = chassis->getAngularVelocity();
		chassis->applyTorque(av * -40);
		hov_roll = 0.f;  //-
		return;
	}

	float dmg = fDamage > 50.f ? 1.f - (fDamage-50.f)*0.02f : 1.f;
	float dmgE = 1.f - 0.2 * dmg;

	//  🎯 cast ray down .
	COLLISION_CONTACT ct,ct2;
	MATHVECTOR<Dbl,3> dn = GetDownVector();
	Dbl ups = dn[2] < 0.0 ? 1.0 : -1.0;
		//sHov += " dn "+fToStr(dn[2],2,5)+"\n";

	/*const */Dbl len = hov.hAbove, rlen = hov.hRayLen;
	MATHVECTOR<Dbl,3> p = GetPosition();  // - dn * 0.1;  // v fluids as solids
	world->CastRay(p, dn, rlen, chassis, ct,  0,0, false, true);
	float d = ct.GetDepth();

	//  2nd in front for pitch
	MATHVECTOR<Dbl,3> dx(3.4, 0, 0);  //par
	Orientation().RotateVector(dx);
	
	world->CastRay(p+dx, dn, rlen, chassis, ct2,  0,0, false, true);
	float d2 = ct2.GetDepth();

	//  pipe..?
	bool pipe = false;
	if (ct.GetColObj())
	{
		int su = (long)ct.GetColObj()->getCollisionShape()->getUserPointer();
		if (su >= SU_Pipe && su < SU_RoadWall)
			pipe = true;
	}
	if (pipe)
	{	len *= 1.2;  rlen *= 0.9;  }  ///!par
	
	//  vel
	MATHVECTOR<Dbl,3> sv = -GetVelocity();
	(-Orientation()).RotateVector(sv);
	MATHVECTOR<Dbl,3> av = GetAngularVelocity();

	//  roll /  vis only
	hov_roll = sv[1] * hov.roll;  // vis degrees
	hov_roll = std::max(-90.f, std::min(90.f, hov_roll));


	//  steer  < >
	bool rear = sv[0] > 0.0;
	Dbl rr = std::max(-1.0, std::min(1.0, -sv[0] * 0.4));  //par
		//sHov += " rr "+fToStr(rr,2,5)+"\n";

	MATHVECTOR<Dbl,3> t(0,0, -1000.0 * rr * ups * hov.steerForce * steerValue * dmgE);
	Orientation().RotateVector(t);
	Dbl damp = pipe ? hov.steerDamp : hov.steerDamp;  //damp *= 1 - fabs(steerValue);
	ApplyTorque(t - av * damp * 1000.0);  // rotation damping


	//  handbrake damping  v
	btVector3 v = chassis->getLinearVelocity();
	Dbl h = brake[2].GetHandbrakeFactor();
	
	//  engine  ^
	float vel = sv.Magnitude(),  //  decrease power with velocity
		velMul = 1.f - std::min(1.f, hov.engineVelDec * vel),
		velMulR = 1.f - std::min(1.f, hov.engineVelDecR * vel);
		//sHov += " m  "+fToStr(velMul,2,5)+"\n";

	Dbl brk = brake[0].GetBrakeFactor() * (1.0 - h);
	float f = hov.engineForce * velMul * hov_throttle
			- hov.brakeForce * (rear ? velMulR : 1.f) * brk;

	MATHVECTOR<Dbl,3> vf(body.GetMass() * f * dmgE * 1.03, 0, 0);  //par fix org
	Orientation().RotateVector(vf);
	ApplyForce(vf);


	//  side, vel damping  --
	sv[0] *= hov.dampAirRes;
	sv[1] *= hov.dampSide;
	sv[2] *= sv[2] > 0.0 ? hov.dampUp : hov.dampDn;
	Orientation().RotateVector(sv);
	Dbl ss = pipe ? hov.dampPmul : 1;
	ApplyForce(sv * ss);

	
	//  align straight torque |
	MATHVECTOR <float,3> n = ct.GetNormal();  // ground
	MATHVECTOR <float,3> n2 = ct2.GetNormal();
	if (!(d > 0.f && d < rlen))  n = MATHVECTOR <float,3>(0,0,1);  // in air
	MATHVECTOR<Dbl,3> al = dn.cross(n), ay = al;
	MATHVECTOR<Dbl,3> ay2 = dn.cross(n2);
	if (pipe) {  al[0] *= hov.alp[0];  al[1] *= hov.alp[1];  al[2] *= hov.alp[2];  }
	else      {  al[0] *= hov.alt[0];  al[1] *= hov.alt[1];  al[2] *= hov.alt[2];  }
	ApplyTorque(al * -1000.0);

		//sHov += " a1 "+fToStr(ay[0],2,5)+" "+fToStr(ay[1],2,5)+" "+fToStr(ay[2],2,5) +"\n";
		//sHov += " a2 "+fToStr(ay2[0],2,5)+" "+fToStr(ay2[1],2,5)+" "+fToStr(ay2[2],2,5) +"\n";
		//sHov += " 12 "+fToStr(ay[0]-ay2[0],2,5)+" "+fToStr(ay[1]-ay2[1],2,5)+" "+fToStr(ay[2]-ay2[2],2,5) +"\n";

	//  pitch torque )
	Dbl pitch = (d < len && d2 < len) ? (d2 - d) * hov.pitchTq * 1000.f : 0.f;
	Dbl roll = sv[1] * hov.rollTq * -1000.f;
	Dbl yawP = !pipe ? 0.0 : 600.0 * (ay[0]-ay2[0]);
	Dbl spiP = !pipe ? 0.0 : 600.0 * (ay[1]-ay2[1]);
		//sHov += " yp "+fToStr(yawP,0,5)+" "+fToStr(spiP,0,5) +"\n";
	MATHVECTOR<Dbl,3> tq(roll, pitch + spiP * ups, ups * yawP);
	Orientation().RotateVector(tq);
	ApplyTorque(tq);


	///  heavy landing damp  __
	const float dlen = len * hov.hov_height;
	Dbl vz = chassis->getLinearVelocity().dot( pipe ? ToBulletVector(-dn) : ToBulletVector(n) );
	float fd =            // free fall : below, anti gravity rising
		-(vz < 0.f ? hov.hov_damp * vz : hov.hov_dampUp * vz);  // -10 -2  damp | vel
	
	suspension[1].velocity =  vz * 0.05;   // grn top  for graphs, vis only`
	suspension[2].velocity = -fd * 0.006;  // ylw top
	//suspension[3].velocity = vz < 0.f ? -0.5 : 1;

	//  anti gravity force  |
	float fg = 
		-h * hov.hov_fall * 3 +  //par hbrk dn
	(d < dlen ?
		(pipe ? hov.hov_riseP : hov.hov_rise) * (dlen - d) :  // up^ hover  151
		-hov.hov_fall * (d - dlen));  // down_ fall  32

	suspension[1].displacement = d / dlen;          // grn btm
	suspension[2].displacement = 1.0 - fg * 0.004;  // ylw btm
	// suspension[0].displacement = d / rlen;

	float fn = fd + fg;
	MATHVECTOR<Dbl,3> vg(0, 0, body.GetMass() * fn * dmgE);
	Orientation().RotateVector(vg);
	ApplyForce(vg);
	// chassis->applyCentralForce(ToBulletVector(vg)); //-dn * fn * dmgE));

	if (DampAtEnd())  return;
}


///  🔘 Sphere
///..........................................................................................................
void CARDYNAMICS::SimulateSphere(Dbl dt)
{
	//  destroyed  damping
	if (fDamage >= 100.f)
	{
		btVector3 v = chassis->getLinearVelocity();
		v[2] *= 0.1;
		chassis->applyCentralForce(v * -10);

		btVector3 av = chassis->getAngularVelocity();
		chassis->applyTorque(av * -10);
		return;
	}

	float dmg = fDamage > 50.f ? 1.f - (fDamage-50.f)*0.02f : 1.f;
	float dmgE = 1.f - 0.1 * dmg;

	//  🎯 cast ray,  only to check if in pipe
	/*const */Dbl len = hov.hAbove, rlen = hov.hRayLen;
	COLLISION_CONTACT ct;
	MATHVECTOR<Dbl,3> p = GetPosition();  // - dn * 0.1;  // v fluids as solids
	MATHVECTOR<float,3> dn(0,0,-1);
	bool nn = world->CastRay(p, dn, rlen, chassis, ct,  0,0, false, true);
	float d = ct.GetDepth();

	//  pipe
	bool pipe = false;
	if (ct.GetColObj())
	{
		int su = (long)ct.GetColObj()->getCollisionShape()->getUserPointer();
		if (su >= SU_Pipe && su < SU_RoadWall)
			pipe = true;
	}
	const Dbl mul = 29.1;  //par

	if (DampAtEnd())  return;

	//  engine
	//bool rear = false;  //transmission.GetGear() < 0;
	MATHVECTOR<Dbl,3> sv = -GetVelocity();
	//(-Orientation()).RotateVector(sv);
	
	float vel = sv.Magnitude(),  //  decrease power with velocity
		velMul = 1.f - std::min(1.f, hov.engineVelDec * vel);
	float f = hov.engineForce * velMul * hov_throttle * dmgE
			- hov.brakeForce * velMul * brake[0].GetBrakeFactor() * dmgE;
	//if (rear)  f *= -1.f;

	float h = brake[0].GetHandbrakeFactor();

	//  steer <> rotate dir
	float pst = hov.steerForce;  //if (rear)  pst = -pst;
	if (pipe)  pst *= hov.steerDampP;

	float hh = 1.f + 1.0f * sqrt(h);  // factor, faster steer with handbrake
	sphereYaw += steerValue * hh * dt * pst * PI_d/180.f;
	MATHVECTOR<Dbl,3> dir(cosf(sphereYaw), -sinf(sphereYaw), 0);
	// if (nn)  // meh
	// {	auto n = ct.GetNormal();
	// 	dir = dir - dir * n.dot(dir);
	//  // dir = dir.cross(ct.GetNormal());
	// }

	f *= body.GetMass() * -1.0;
	btVector3 fc = ToBulletVector(dir * f);
		if (!pipe)  fc += btVector3(0,0,-hov.hov_fall);

	MATHVECTOR<Dbl,3> ff(0,0,-hov.hov_fall);
	ff = ff + dir * f;
	ApplyForce( ff * mul );

	//  handbrake damping
	btVector3 v = chassis->getLinearVelocity();
	if (h > 0.01f)
	{
		chassis->applyCentralForce(v * h * -10.f);

		btVector3 av = chassis->getAngularVelocity();
		chassis->applyTorque(av * h * -10.f);
	}

	//  side damp --
	btVector3 vv(dir[1], -dir[0], 0.f);
	float pmul = hov.dampSide;
		if (pipe)  pmul *= hov.dampPmul;
	float dot = v.getX()*vv.getX() + v.getY()*vv.getY();

	MATHVECTOR<Dbl,3> vd(dir[1], -dir[0], 0.f);
	ApplyForce(vd * dot * -pmul * mul );
}
#endif
