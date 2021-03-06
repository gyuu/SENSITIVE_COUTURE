/*
DelFEM (Finite Element Analysis)
Copyright (C) 2009  Nobuyuki Umetani    n.umetani@gmail.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


////////////////////////////////////////////////////////////////
// eqn_hyper.cpp : 超弾性体方程式の要素剛性作成部の実装
////////////////////////////////////////////////////////////////


#if defined(__VISUALC__)
    #pragma warning ( disable : 4786 )
#endif
#define for if(0);else for

#include "delfem/matvec/matdia_blkcrs.h"
#include "delfem/matvec/vector_blk.h"
#include "delfem/field_world.h"
#include "delfem/field.h"
#include "delfem/linearsystem_interface_eqnsys.h"
#include "delfem/femeqn/ker_emat_tri.h"
#include "delfem/femeqn/ker_emat_tet.h"
#include "delfem/femeqn/ker_emat_quad.h"
#include "delfem/femeqn/ker_emat_hex.h"
#include "delfem/femeqn/eqn_hyper.h"

using namespace Fem::Eqn;
using namespace Fem::Field;
using namespace MatVec;

////////////////////////////////////////////////////////////////
// ３Ｄの方程式

////////////////
// 定常

bool AddLinSys_Hyper3D_HEX100100_000010(
        Fem::Eqn::ILinearSystem_Eqn& ls, 
		double c1, double c2,
		double rho, double g_x, double g_y, double g_z,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world, 
		const unsigned int id_ea);

bool AddLinSys_Hyper2D_P1bP1(
        Fem::Eqn::ILinearSystem_Eqn& ls, 
		double c1, double c2,
		double rho, double g_x, double g_y,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world, 
		const unsigned int id_ea);

bool AddLinSys_Hyper3D_NonStatic_NewmarkBeta_HEX100100_000010(
		double dt, double gamma, double beta, 
        Fem::Eqn::ILinearSystem_Eqn& ls, 
		double lambda, double myu,
		double rho, double g_x, double g_y, double g_z,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world, 
		bool is_initial,
		const unsigned int id_ea );


////////////////////////////////

// 定常
bool Fem::Eqn::AddLinSys_Hyper2D_Static(
        Fem::Eqn::ILinearSystem_Eqn& ls,
		double c1, double c2,
		double  rho, double g_x, double g_y,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const Fem::Field::CFieldWorld& world, 
        unsigned int id_ea)
{
	const CField& field_disp = world.GetField(id_field_disp);
	if( field_disp.GetFieldType() != VECTOR2 ){ assert(0); return false; }

	if( id_ea != 0 ){	
		if( field_disp.GetInterpolationType(id_ea,world) == TRI1011 ){
			AddLinSys_Hyper2D_P1bP1(ls,
				c1,c2,  rho, g_x, g_y,
				id_field_disp,id_field_lambda,world,
                id_ea);
        }
    }
	else{
		const std::vector<unsigned int>& aIdEA = field_disp.GetAryIdEA();
		for(unsigned int iiea=0;iiea<aIdEA.size();iiea++){
			const unsigned int id_ea = aIdEA[iiea];
			bool res = AddLinSys_Hyper2D_Static(ls,
                c1,c2,  rho,g_x,g_y,
                id_field_disp, id_field_lambda,
                world, id_ea);
			if( !res ) return false;
        }
    }
	return true;
}

bool Fem::Eqn::AddLinSys_Hyper3D_Static(
        Fem::Eqn::ILinearSystem_Eqn& ls,
		double c1, double c2,
		double  rho, double g_x, double g_y, double g_z,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world, 
        unsigned int id_ea )
{
	const CField& field_disp = world.GetField(id_field_disp);
	if( field_disp.GetFieldType() != VECTOR3 ){ assert(0); return false; }

    
	if( id_ea != 0 ){	
        if( field_disp.GetInterpolationType(id_ea,world) == HEX11 ){
			AddLinSys_Hyper3D_HEX100100_000010(ls,
				c1,c2,  rho, g_x, g_y, g_z,
				id_field_disp,id_field_lambda, world,
                id_ea);
        }
    }
	else{
		const std::vector<unsigned int>& aIdEA = field_disp.GetAryIdEA();
		for(unsigned int iiea=0;iiea<aIdEA.size();iiea++){
			const unsigned int id_ea = aIdEA[iiea];
			bool res = AddLinSys_Hyper3D_Static(ls,
                c1,c2,  rho,g_x,g_y,g_z,
                id_field_disp, id_field_lambda,
                world, id_ea);
			if( !res ) return false;
        }
    }

	return true;
}


bool Fem::Eqn::AddLinSys_Hyper3D_NonStatic_NewmarkBeta(
        double dt, double gamma, double beta, 
        Fem::Eqn::ILinearSystem_Eqn& ls,
		double c1, double c2,
		double  rho, double g_x, double g_y, double g_z,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world,  
		bool is_initial, 
        unsigned int id_ea )
{
	const CField& field_disp = world.GetField(id_field_disp);
	if( field_disp.GetFieldType() != VECTOR3 ){ assert(0); return false; }

    
	if( id_ea != 0 ){	
        if( field_disp.GetInterpolationType(id_ea,world) == HEX11 ){
			AddLinSys_Hyper3D_NonStatic_NewmarkBeta_HEX100100_000010(
				dt,gamma,beta,ls, 
                c1,c2,  rho, g_x, g_y, g_z,
				id_field_disp,id_field_lambda, world,
				is_initial, 
                id_ea );
		}
	}
	else{
		const std::vector<unsigned int>& aIdEA = field_disp.GetAryIdEA();
		for(unsigned int iiea=0;iiea<aIdEA.size();iiea++){
			const unsigned int id_ea = aIdEA[iiea];
			bool res = AddLinSys_Hyper3D_NonStatic_NewmarkBeta(
                dt,gamma,beta, ls,
                c1,c2,  rho,g_x,g_y,g_z,
                id_field_disp, id_field_lambda,  world, 
                is_initial,
                id_ea);
			if( !res ) return false;
        }
    }

	return true;
}

// 応力と構成則テンソルを作る関数
void MakeStressConstitute_Hyper2D(
			const double c1, const double c2,
			const double dudx[][2], const double press,
			double stress_2ndpk[][2], double constit[][2][2][2],
			double& rcg_pinv1, double& rcg_pinv2, double& rcg_pinv3,
			double rcg_inv[][2])
{
	const unsigned int ndim = 2;
	double strain_gl[ndim][ndim];
	for(unsigned int idim=0;idim<ndim;idim++){
	for(unsigned int jdim=0;jdim<ndim;jdim++){
		strain_gl[idim][jdim] = 0.5*( dudx[idim][jdim] + dudx[jdim][idim] );
		for(unsigned int kdim=0;kdim<ndim;kdim++){
			strain_gl[idim][jdim] += 0.5*dudx[kdim][idim]*dudx[kdim][jdim];
		}
	}
	}

	double rcg[ndim][ndim];
	for(unsigned int idim=0;idim<ndim;idim++){
		for(unsigned int jdim=0;jdim<ndim;jdim++){
			rcg[idim][jdim] = dudx[idim][jdim] + dudx[jdim][idim];
			for(unsigned int kdim=0;kdim<ndim;kdim++){
				rcg[idim][jdim] += dudx[kdim][idim]*dudx[kdim][jdim];
			}
		}
		rcg[idim][idim] += 1.0;
	}

	rcg_pinv1 = rcg[0][0]+rcg[1][1]+1.0;
	rcg_pinv2 = rcg[0][0]*rcg[1][1]+rcg[0][0]+rcg[1][1]-rcg[0][1]*rcg[1][0];
	rcg_pinv3 = rcg[0][0]*rcg[1][1]-rcg[0][1]*rcg[1][0];
	{
		const double inv_pinv3 = 1.0 / rcg_pinv3;
		rcg_inv[0][0] =  inv_pinv3*rcg[1][1];
		rcg_inv[0][1] = -inv_pinv3*rcg[0][1];
		rcg_inv[1][0] = -inv_pinv3*rcg[1][0];
		rcg_inv[1][1] =  inv_pinv3*rcg[0][0];
	}

	const double inv13_rcg_pinv3 = 1.0/pow(rcg_pinv3,1.0/3.0);
	const double inv23_rcg_pinv3 = 1.0/pow(rcg_pinv3,2.0/3.0);

//	std::cout << rcg_pinv1 << " " << rcg_pinv2 << " " << rcg_pinv3 << std::endl;

	////////////////////////////////

	{	// 応力を求める

		// ゼロクリア
		for(unsigned int i=0;i<ndim*ndim;i++){ *(&stress_2ndpk[0][0]+i) = 0.0; }
/*
		// 変位の応力を加える(Moonin-Rivelen体)
		for(unsigned int idim=0;idim<ndim;idim++){
			for(unsigned int jdim=0;jdim<ndim;jdim++){
				stress_2ndpk[idim][jdim] -= 2*c2*rcg[idim][jdim];
			}
			stress_2ndpk[idim][idim] += 2*(c1+c2*rcg_pinv1);
		}
		for(unsigned int idim=0;idim<ndim;idim++){
		for(unsigned int jdim=0;jdim<ndim;jdim++){
			stress_2ndpk[idim][jdim] -= 2*(c1+2*c2)*rcg_pinv3*rcg_inv[idim][jdim];
		}
		}*/

		// 変位の応力を加える(低減不変量Moonin-Rivelen体)
		for(unsigned int idim=0;idim<ndim;idim++){
		for(unsigned int jdim=0;jdim<ndim;jdim++){
			stress_2ndpk[idim][jdim] -=
				  2.0*c2*inv23_rcg_pinv3*rcg[idim][jdim]
			    + 2.0*(c1*rcg_pinv1*inv13_rcg_pinv3+c2*2.0*rcg_pinv2*inv23_rcg_pinv3)/3.0
				  *rcg_inv[idim][jdim];
		}
		}
		{
			double dtmp1 = 2.0*c1*inv13_rcg_pinv3+2.0*c2*inv23_rcg_pinv3*rcg_pinv1;
			for(unsigned int idim=0;idim<ndim;idim++){
				stress_2ndpk[idim][idim] += dtmp1;
			}
		}

		{	// 圧力の応力を求める
			double dtmp1 = 2.0*press*rcg_pinv3;
			for(unsigned int idim=0;idim<ndim;idim++){
			for(unsigned int jdim=0;jdim<ndim;jdim++){
				stress_2ndpk[idim][jdim] += dtmp1*rcg_inv[idim][jdim];
			}
			}
		}
	}

	{	// 構成則テンソルを求める
		//ゼロクリア
		for(unsigned int i=0;i<ndim*ndim*ndim*ndim;i++){ *(&constit[0][0][0][0]+i) = 0.0; }
		// 変位の構成則テンソルを足し合わせる
		for(unsigned int idim=0;idim<ndim;idim++){
		for(unsigned int jdim=0;jdim<ndim;jdim++){
			for(unsigned int kdim=0;kdim<ndim;kdim++){
			for(unsigned int ldim=0;ldim<ndim;ldim++){
				constit[idim][jdim][kdim][ldim] 
					+= 4.0*c1*inv13_rcg_pinv3/3.0*(
					   rcg_inv[idim][jdim]*rcg_inv[kdim][ldim]*rcg_pinv1/3.0
					  +rcg_inv[idim][kdim]*rcg_inv[ldim][jdim]*rcg_pinv1*0.5
					  +rcg_inv[idim][ldim]*rcg_inv[kdim][jdim]*rcg_pinv1*0.5);
				constit[idim][jdim][kdim][ldim]
					+= 4.0*c2*inv23_rcg_pinv3*2.0/3.0*(
					   rcg_inv[idim][jdim]*rcg_inv[kdim][ldim]*rcg_pinv2*(2.0/3.0)
					  +rcg_inv[idim][jdim]*rcg[    kdim][ldim]
					  +rcg[    idim][jdim]*rcg_inv[kdim][ldim]
					  +rcg_inv[idim][kdim]*rcg_inv[jdim][ldim]*rcg_pinv2*0.5
					  +rcg_inv[idim][ldim]*rcg_inv[jdim][kdim]*rcg_pinv2*0.5);
			}
			}
			double dtmp1 = 4.0*c1*inv13_rcg_pinv3/3.0*rcg_inv[idim][jdim];
			for(unsigned int kdim=0;kdim<ndim;kdim++){
				constit[idim][jdim][kdim][kdim] -= dtmp1;
				constit[kdim][kdim][idim][jdim] -= dtmp1;
			}
			double dtmp2 = 4.0*c2*inv23_rcg_pinv3*rcg_pinv1*(2.0/3.0)*rcg_inv[idim][jdim];
			for(unsigned int kdim=0;kdim<ndim;kdim++){
				constit[idim][jdim][kdim][kdim] -= dtmp2;
				constit[kdim][kdim][idim][jdim] -= dtmp2;
			}
			constit[idim][idim][jdim][jdim] += 4.0*c2*inv23_rcg_pinv3;
			constit[idim][jdim][jdim][idim] -= 2.0*c2*inv23_rcg_pinv3;
			constit[idim][jdim][idim][jdim] -= 2.0*c2*inv23_rcg_pinv3;
		}
		}
		// 圧力の構成則テンソルを足し合わせる
		for(unsigned int idim=0;idim<ndim;idim++){
		for(unsigned int jdim=0;jdim<ndim;jdim++){
		for(unsigned int kdim=0;kdim<ndim;kdim++){
		for(unsigned int ldim=0;ldim<ndim;ldim++){
			constit[idim][jdim][kdim][ldim] +=
				 4.0*press*rcg_pinv3*
				     rcg_inv[idim][jdim]*rcg_inv[kdim][ldim]
				-2.0*press*rcg_pinv3*(
				 	 rcg_inv[idim][kdim]*rcg_inv[ldim][jdim]
					+rcg_inv[idim][ldim]*rcg_inv[kdim][jdim] );
		}
		}
		}
		}
	}
}




bool AddLinSys_Hyper2D_P1bP1(
        Fem::Eqn::ILinearSystem_Eqn& ls, 
		double c1, double c2,
		double  rho, double g_x, double g_y,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world, 
		unsigned int id_ea )
{
	
	std::cout << "MooninRevelen2D Tri P1b/P1" << std::endl;

	if( !world.IsIdField(id_field_disp) ) return false;
	const CField& field_disp = world.GetField(id_field_disp);

	if( !world.IsIdField(id_field_lambda) ) return false;
	const CField& field_lambda = world.GetField(id_field_lambda);

	assert( world.IsIdEA(id_ea) );
	const CElemAry& ea = world.GetEA(id_ea);
	assert( ea.ElemType() == TRI );

	unsigned int num_integral = 2;
	const unsigned int nInt = NIntTriGauss[num_integral];
	const double (*Gauss)[3] = TriGauss[num_integral];
	double detwei;

	const CElemAry::CElemSeg& es_c_co = field_disp.GetElemSeg(  id_ea,CORNER,false,world);
	const CElemAry::CElemSeg& es_cu   = field_disp.GetElemSeg(  id_ea,CORNER,true, world);
	const CElemAry::CElemSeg& es_bu   = field_disp.GetElemSeg(  id_ea,BUBBLE,true, world);
//	const CElemAry::CElemSeg& es_p    = field_lambda.GetElemSeg(id_ea,CORNER,true, world);

	const unsigned int nno_c = 3;
	const unsigned int nno_b = 1;
	const unsigned int ndim = 2;

	unsigned int noes_c[nno_c];	// 要素節点の全体節点番号
	unsigned int noes_b;	// 要素節点の全体節点番号

	double disp_c[nno_c][ndim];	// 要素節点の値
	double disp_b[ndim];	// 要素節点の値
	double press_c[nno_c];
	double coords[nno_c][ndim];	// 要素節点の座標
				
	double dldx[nno_c][ndim];	// 形状関数のxy微分
	double const_term[nno_c];	// 形状関数の定数項
	double dncdx[nno_c][ndim];
	double dnbdx[ndim];
	double am[nno_c];

	double emat_cucu[nno_c][nno_c][ndim][ndim];
	double emat_cubu[nno_c][ndim][ndim];
	double emat_cup[nno_c][nno_c][ndim];

	double emat_bubu[ndim][ndim];
	double emat_bucu[nno_c][ndim][ndim];
	double emat_bup[nno_c][ndim];

	double emat_pcu[nno_c][nno_c][ndim];
	double emat_pbu[nno_c][ndim];
	double emat_pp[nno_c][nno_c];

	double eres_cu[nno_c][ndim], eres_bu[ndim], eres_p[nno_c];
	double eqf_in_cu[nno_c][ndim], eqf_in_bu[ndim], eqf_in_p[nno_c];

	CMatDia_BlkCrs& mat_cucu = ls.GetMatrix(id_field_disp,  CORNER,world);
	CMatDia_BlkCrs& mat_bubu = ls.GetMatrix(id_field_disp,  BUBBLE,world);
	CMatDia_BlkCrs& mat_pp   = ls.GetMatrix(id_field_lambda,CORNER,world);

	CMat_BlkCrs& mat_cubu = ls.GetMatrix(id_field_disp,  CORNER, id_field_disp,  BUBBLE, world);
	CMat_BlkCrs& mat_bucu = ls.GetMatrix(id_field_disp,  BUBBLE, id_field_disp,  CORNER, world);
	CMat_BlkCrs& mat_cup  = ls.GetMatrix(id_field_disp,  CORNER, id_field_lambda,CORNER, world);
	CMat_BlkCrs& mat_bup  = ls.GetMatrix(id_field_disp,  BUBBLE, id_field_lambda,CORNER, world);
	CMat_BlkCrs& mat_pcu  = ls.GetMatrix(id_field_lambda,CORNER, id_field_disp,  CORNER, world);
	CMat_BlkCrs& mat_pbu  = ls.GetMatrix(id_field_lambda,CORNER, id_field_disp,  BUBBLE, world);

	CVector_Blk& res_cu = ls.GetResidual(id_field_disp,  CORNER,world);
	CVector_Blk& res_bu = ls.GetResidual(id_field_disp,  BUBBLE,world);
	CVector_Blk& res_p  = ls.GetResidual(id_field_lambda,CORNER,world);

	const CNodeAry::CNodeSeg& ns_c_co= field_disp.GetNodeSeg(  CORNER,false,world,VALUE);//na_co.GetSeg(id_ns_co);
	const CNodeAry::CNodeSeg& ns_cu  = field_disp.GetNodeSeg(  CORNER,true, world,VALUE);//na_velo.GetSeg(id_ns_velo);
	const CNodeAry::CNodeSeg& ns_bu  = field_disp.GetNodeSeg(  BUBBLE,true, world,VALUE);//na_velo.GetSeg(id_ns_velo);
	const CNodeAry::CNodeSeg& ns_p   = field_lambda.GetNodeSeg(CORNER,true, world,VALUE);//na_press.GetSeg(id_ns_press);

    for(unsigned int ielem=0;ielem<ea.Size();ielem++){
		// CORNER節点の座標の取得
		es_c_co.GetNodes(ielem,noes_c);
		for(unsigned int inoes=0;inoes<nno_c;inoes++){
			ns_c_co.GetValue(noes_c[inoes],coords[inoes]);
		}
		// CORNER節点の変位の取得
		es_cu.GetNodes(ielem,noes_c);
		for(unsigned int inoes=0;inoes<nno_c;inoes++){
			ns_cu.GetValue(  noes_c[inoes],disp_c[inoes]);
			ns_p.GetValue(   noes_c[inoes],&press_c[inoes]);
		}
		// BUBBLE節点の変位の取得
		es_bu.GetNodes(ielem,&noes_b);
		ns_bu.GetValue(noes_b,disp_b);

		////////////////////////////////

		// 要素剛性行列、残差を０で初期化
		for(unsigned int i=0;i<nno_c*nno_c*ndim*ndim; i++){ *(&emat_cucu[0][0][0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*      ndim*ndim; i++){ *(&emat_cubu[0][0][0]+i)    = 0.0; }
		for(unsigned int i=0;i<nno_c*      ndim*ndim; i++){ *(&emat_bucu[0][0][0]+i)    = 0.0; }
		for(unsigned int i=0;i<            ndim*ndim; i++){ *(&emat_bubu[0][0]+i)       = 0.0; }
		for(unsigned int i=0;i<nno_c*nno_c*ndim; i++){ *(&emat_cup[0][0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*      ndim; i++){ *(&emat_bup[0][0]+i)    = 0.0; }
		for(unsigned int i=0;i<nno_c*nno_c*ndim; i++){ *(&emat_pcu[0][0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*      ndim; i++){ *(&emat_pbu[0][0]+i)    = 0.0; }
		for(unsigned int i=0;i<nno_c*nno_c; i++){ *(&emat_pp[0][0]+i) = 0.0; }
		
		for(unsigned int i=0;i<nno_c*ndim; i++){ *(&eqf_in_cu[0][0]+i) = 0.0; }
		for(unsigned int i=0;i<      ndim; i++){ *(&eqf_in_bu[0]+i)    = 0.0; }
		for(unsigned int i=0;i<nno_c;      i++){ *(&eqf_in_p[0]+i)     = 0.0; }

		// 面積を求める
		const double area = TriArea(coords[0],coords[1],coords[2]);

		// 形状関数のｘｙ微分を求める
		TriDlDx(dldx, const_term,   coords[0],coords[1],coords[2]);

		for(unsigned int ir=0;ir<nInt;ir++){
			const double r1 = Gauss[ir][0];
			const double r2 = Gauss[ir][1];
			const double r3 = 1.0-r1-r2;
			detwei = area * Gauss[ir][2];
			{
				const double tmp2[2] = {
					dldx[0][0]*r2*r3 + r1*dldx[1][0]*r3 + r1*r2*dldx[2][0],
					dldx[0][1]*r2*r3 + r1*dldx[1][1]*r3 + r1*r2*dldx[2][1]
				};
				dncdx[0][0] = dldx[0][0]-9*tmp2[0];  dncdx[0][1] = dldx[0][1]-9*tmp2[1];
				dncdx[1][0] = dldx[1][0]-9*tmp2[0];  dncdx[1][1] = dldx[1][1]-9*tmp2[1];
				dncdx[2][0] = dldx[2][0]-9*tmp2[0];  dncdx[2][1] = dldx[2][1]-9*tmp2[1];
				dnbdx[0]    = 27*tmp2[0];            dnbdx[1]    = 27*tmp2[1];
			}
			{
//				const double tmp1 = r1*r2*r3;
				am[0] = r1;  am[1] = r2;  am[2] = r3;
			}
			
			double dudx[ndim][ndim] = { {0.0,0.0}, {0.0,0.0} };
			{
				for(unsigned int inoes=0;inoes<nno_c;inoes++){
					for(unsigned int idim=0;idim<ndim;idim++){
						dudx[idim][0] += disp_c[inoes][idim]*dncdx[inoes][0];
						dudx[idim][1] += disp_c[inoes][idim]*dncdx[inoes][1];
					}
				}	
				for(unsigned int idim=0;idim<ndim;idim++){
					dudx[idim][0] += disp_b[idim]*dnbdx[0];
					dudx[idim][1] += disp_b[idim]*dnbdx[1];
				}
			}
			double press = am[0]*press_c[0]+am[1]*press_c[1]+am[2]*press_c[2];

			double stress_2ndpk[ndim][ndim];
			double constit[ndim][ndim][ndim][ndim];
			double rcg_pinv1, rcg_pinv2, rcg_pinv3;
			double rcg_inv[ndim][ndim];

			// 応力と構成則テンソルを作る関数
			MakeStressConstitute_Hyper2D(
				c1, c2, dudx, press,
				stress_2ndpk, constit,
				rcg_pinv1, rcg_pinv2, rcg_pinv3,
				rcg_inv);

			////////////////////////////////
			// 変位-歪　関係行列を作る
			double disp_c2strain[nno_c][ndim][ndim][ndim];
			double disp_b2strain[ndim][ndim][ndim];
			{
				double f_mat[ndim][ndim];
				for(unsigned int idim=0;idim<ndim;idim++){
					for(unsigned int jdim=0;jdim<ndim;jdim++){ 
						f_mat[idim][jdim] = dudx[idim][jdim];
					}
					f_mat[idim][idim] += 1.0;
				}
				for(unsigned int ino=0;ino<nno_c;ino++){
				for(unsigned int idim=0;idim<ndim;idim++){
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
						disp_c2strain[ino][idim][gdim][hdim] = dncdx[ino][hdim]*f_mat[idim][gdim];
					}
					}
				}
				}
				for(unsigned int idim=0;idim<ndim;idim++){
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
						disp_b2strain[idim][gdim][hdim] = dnbdx[hdim]*f_mat[idim][gdim];
					}
					}
				}
			}

			////////////////////////////////
			// 接線剛性行列を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int jno=0;jno<nno_c;jno++){
				for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int jdim=0;jdim<ndim;jdim++){
					double dtmp1 = 0.0;
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
					for(unsigned int edim=0;edim<ndim;edim++){
					for(unsigned int fdim=0;fdim<ndim;fdim++){
						dtmp1 += constit[edim][fdim][gdim][hdim]
							*disp_c2strain[ino][idim][gdim][hdim]
							*disp_c2strain[jno][jdim][edim][fdim];
					}
					}
					}
					}
					emat_cucu[ino][jno][idim][jdim] += detwei*dtmp1;
				}
				}
				{
					double dtmp2 = 0.0;
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
						dtmp2 += stress_2ndpk[gdim][hdim]*dncdx[ino][gdim]*dncdx[jno][hdim];
					}
					}
					for(unsigned int idim=0;idim<ndim;idim++){
						emat_cucu[ino][jno][idim][idim] += detwei*dtmp2;
					}
				}
			}
			}
			for(unsigned int ino=0;ino<nno_c;ino++){
				for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int jdim=0;jdim<ndim;jdim++){
					double dtmp1 = 0.0;
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
					for(unsigned int edim=0;edim<ndim;edim++){
					for(unsigned int fdim=0;fdim<ndim;fdim++){
						dtmp1 += constit[edim][fdim][gdim][hdim]
							*disp_b2strain[idim][gdim][hdim]*disp_c2strain[ino][jdim][edim][fdim];
					}
					}
					}
					}
					emat_bucu[ino][idim][jdim] += detwei*dtmp1;
					emat_cubu[ino][idim][jdim] += detwei*dtmp1;
				}
				}
				{
					double dtmp2 = 0.0;
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
						dtmp2 += stress_2ndpk[gdim][hdim]*dncdx[ino][gdim]*dnbdx[hdim];
					}
					}
					for(unsigned int idim=0;idim<ndim;idim++){
						emat_bucu[ino][idim][idim] += detwei*dtmp2;
						emat_cubu[ino][idim][idim] += detwei*dtmp2;
					}
				}
			}
			{
				for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int jdim=0;jdim<ndim;jdim++){
					double dtmp1 = 0.0;
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
					for(unsigned int edim=0;edim<ndim;edim++){
					for(unsigned int fdim=0;fdim<ndim;fdim++){
						dtmp1 += constit[edim][fdim][gdim][hdim]
							*disp_b2strain[idim][gdim][hdim]*disp_b2strain[jdim][edim][fdim];
					}
					}
					}
					}
					emat_bubu[idim][jdim] += detwei*dtmp1;
				}
				}
				{
					double dtmp2 = 0.0;
					for(unsigned int gdim=0;gdim<ndim;gdim++){
					for(unsigned int hdim=0;hdim<ndim;hdim++){
						dtmp2 += stress_2ndpk[gdim][hdim]*dnbdx[gdim]*dnbdx[hdim];
					}
					}
					for(unsigned int idim=0;idim<ndim;idim++){
						emat_bubu[idim][idim] += detwei*dtmp2;
					}
				}
			}
			for(unsigned int inoel=0;inoel<nno_c;inoel++){
			for(unsigned int idim=0;idim<ndim;idim++){
				double dtmp1 = 0.0;
				for(unsigned int gdim=0;gdim<ndim;gdim++){
				for(unsigned int hdim=0;hdim<ndim;hdim++){
					dtmp1 += rcg_inv[gdim][hdim]*disp_c2strain[inoel][idim][gdim][hdim];
				}
				}
                for(unsigned int jnoel=0;jnoel<nno_c;jnoel++){
					emat_cup[inoel][jnoel][idim] += detwei*dtmp1*am[jnoel]*rcg_pinv3*2;
					emat_pcu[jnoel][inoel][idim] += detwei*dtmp1*am[jnoel]*rcg_pinv3*2;
				}
			}
			}
			for(unsigned int idim=0;idim<ndim;idim++){
				double dtmp1 = 0.0;
				for(unsigned int gdim=0;gdim<ndim;gdim++){
				for(unsigned int hdim=0;hdim<ndim;hdim++){
					dtmp1 += rcg_inv[gdim][hdim]*disp_b2strain[idim][gdim][hdim];
				}
				}
				for(unsigned int jnoel=0;jnoel<nno_c;jnoel++){
					emat_bup[jnoel][idim] += detwei*dtmp1*am[jnoel]*rcg_pinv3*2;
					emat_pbu[jnoel][idim] += detwei*dtmp1*am[jnoel]*rcg_pinv3*2;
				}
			}

			////////////////////////////////
			// 内力を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int gdim=0;gdim<ndim;gdim++){
				for(unsigned int hdim=0;hdim<ndim;hdim++){
					eqf_in_cu[ino][idim] += detwei*stress_2ndpk[gdim][hdim]*disp_c2strain[ino][idim][gdim][hdim];
				}
				}
			}
			}
			for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int gdim=0;gdim<ndim;gdim++){
				for(unsigned int hdim=0;hdim<ndim;hdim++){
					eqf_in_bu[idim] += detwei*( stress_2ndpk[gdim][hdim]*disp_b2strain[idim][gdim][hdim] );
				}
				}
			}
			for(unsigned int ino=0;ino<nno_c;ino++){
				eqf_in_p[ino] += detwei*am[ino]*(rcg_pinv3-1);
			}
		}

		////////////////
		// 要素内残差ベクトルを求める
		for(unsigned int ino_c=0;ino_c<nno_c;ino_c++){
			eres_cu[ino_c][0] = g_x*area*11.0/60.0 - eqf_in_cu[ino_c][0];
			eres_cu[ino_c][1] = g_y*area*11.0/60.0 - eqf_in_cu[ino_c][1];
		}
		eres_bu[0] = g_x*area*27.0/60.0 - eqf_in_bu[0];
		eres_bu[1] = g_y*area*27.0/60.0 - eqf_in_bu[1];
		for(unsigned int ino_c=0;ino_c<nno_c;ino_c++){ 
			eres_p[ino_c] = -eqf_in_p[ino_c]; 
		}

		// 要素剛性行列の全体剛性行列へのマージ
		mat_cucu.Mearge(nno_c, noes_c, nno_c, noes_c,	4,&emat_cucu[0][0][0][0]);
		mat_cubu.Mearge(nno_c, noes_c, nno_b,&noes_b, 	4,&emat_cubu[0][0][0]   );
		mat_bucu.Mearge(nno_b,&noes_b, nno_c, noes_c,	4,&emat_bucu[0][0][0]   );
		mat_bubu.Mearge(nno_b,&noes_b, nno_b,&noes_b,	4,&emat_bubu[0][0]      );
		mat_cup .Mearge(nno_c, noes_c, nno_c, noes_c,	2,&emat_cup[0][0][0]    );
		mat_pcu .Mearge(nno_c, noes_c, nno_c, noes_c,	2,&emat_pcu[0][0][0]    );
		mat_bup .Mearge(nno_b,&noes_b, nno_c, noes_c,	2,&emat_bup[0][0]       );
		mat_pbu .Mearge(nno_c, noes_c, nno_b,&noes_b,	2,&emat_pbu[0][0]       );
		mat_pp  .Mearge(nno_c, noes_c, nno_c, noes_c,	1,&emat_pp[0][0]        );

		// 残差ベクトルのマージ
		for(unsigned int ino=0;ino<nno_c;ino++){
			res_cu.AddValue( noes_c[ino],0,eres_cu[ino][0]);
			res_cu.AddValue( noes_c[ino],1,eres_cu[ino][1]);
		}
		res_bu.AddValue( noes_b,0,eres_bu[0]);
		res_bu.AddValue( noes_b,1,eres_bu[1]);
		for(unsigned int ino=0;ino<nno_c;ino++){
			res_p.AddValue( noes_c[ino],0,eres_p[ino]);
		}

	}

	return true;
}



// 応力と構成則テンソルを作る関数
void MakeStressConstitute_Hyper3D(
			const double c1, const double c2,
			const double dudx[][3], const double press,
			double stress_2ndpk2[], double constit2[][6],
			double& rcg_pinv1, double& rcg_pinv2, double& rcg_pinv3,
			double rcg_inv2[6])
{
	const unsigned int ndim = 3;

	double strain_gl[ndim][ndim];
	for(unsigned int idim=0;idim<ndim;idim++){
	for(unsigned int jdim=0;jdim<ndim;jdim++){
		strain_gl[idim][jdim] = 0.5*( dudx[idim][jdim] + dudx[jdim][idim] );
		for(unsigned int kdim=0;kdim<ndim;kdim++){
			strain_gl[idim][jdim] += 0.5*dudx[kdim][idim]*dudx[kdim][jdim];
		}
	}
	}

	double rcg[ndim][ndim];
	for(unsigned int idim=0;idim<ndim;idim++){
		for(unsigned int jdim=0;jdim<ndim;jdim++){
			rcg[idim][jdim] = dudx[idim][jdim] + dudx[jdim][idim];
			for(unsigned int kdim=0;kdim<ndim;kdim++){
				rcg[idim][jdim] += dudx[kdim][idim]*dudx[kdim][jdim];
			}
		}
		rcg[idim][idim] += 1.0;
	}

	rcg_pinv1 = rcg[0][0]+rcg[1][1]+rcg[2][2];
	rcg_pinv2 = rcg[0][0]*rcg[1][1]+rcg[0][0]*rcg[2][2]+rcg[1][1]*rcg[2][2]
		       -rcg[0][1]*rcg[1][0]-rcg[0][2]*rcg[2][0]-rcg[1][2]*rcg[2][1];
	rcg_pinv3 = rcg[0][0]*rcg[1][1]*rcg[2][2]+rcg[1][0]*rcg[2][1]*rcg[0][2]+rcg[2][0]*rcg[0][1]*rcg[1][2]
			   -rcg[0][0]*rcg[2][1]*rcg[1][2]-rcg[2][0]*rcg[1][1]*rcg[0][2]-rcg[1][0]*rcg[0][1]*rcg[2][2];

	double rcg_inv[ndim][ndim];
	{
		const double inv_det = 1.0/rcg_pinv3;
		rcg_inv[0][0] = inv_det*(rcg[1][1]*rcg[2][2]-rcg[1][2]*rcg[2][1]);
		rcg_inv[0][1] = inv_det*(rcg[0][2]*rcg[2][1]-rcg[0][1]*rcg[2][2]);
		rcg_inv[0][2] = inv_det*(rcg[0][1]*rcg[1][2]-rcg[0][2]*rcg[1][1]);

		rcg_inv[1][0] = inv_det*(rcg[1][2]*rcg[2][0]-rcg[1][0]*rcg[2][2]);
		rcg_inv[1][1] = inv_det*(rcg[0][0]*rcg[2][2]-rcg[0][2]*rcg[2][0]);
		rcg_inv[1][2] = inv_det*(rcg[0][2]*rcg[1][0]-rcg[0][0]*rcg[1][2]);

		rcg_inv[2][0] = inv_det*(rcg[1][0]*rcg[2][1]-rcg[1][1]*rcg[2][0]);
		rcg_inv[2][1] = inv_det*(rcg[0][1]*rcg[2][0]-rcg[0][0]*rcg[2][1]);
		rcg_inv[2][2] = inv_det*(rcg[0][0]*rcg[1][1]-rcg[0][1]*rcg[1][0]);
	}

	const double inv13_rcg_pinv3 = 1.0/pow(rcg_pinv3,1.0/3.0);
	const double inv23_rcg_pinv3 = 1.0/pow(rcg_pinv3,2.0/3.0);

//	std::cout << rcg_pinv1 << " " << rcg_pinv2 << " " << rcg_pinv3 << std::endl;

	////////////////////////////////
	// 応力を求める

	double stress_2ndpk[ndim][ndim];
	// ゼロクリア
	for(unsigned int i=0;i<ndim*ndim;i++){ *(&stress_2ndpk[0][0]+i) = 0.0; }
	// 変位の応力を加える(低減不変量Moonin-Rivelen体)
	for(unsigned int idim=0;idim<ndim;idim++){
	for(unsigned int jdim=0;jdim<ndim;jdim++){
		stress_2ndpk[idim][jdim] -=
			  2.0*c2*inv23_rcg_pinv3*rcg[idim][jdim]
		    + 2.0*(c1*rcg_pinv1*inv13_rcg_pinv3+c2*2.0*rcg_pinv2*inv23_rcg_pinv3)/3.0
			  *rcg_inv[idim][jdim];
	}
	}
	{
		const double dtmp1 = 2.0*c1*inv13_rcg_pinv3+2.0*c2*inv23_rcg_pinv3*rcg_pinv1;
		stress_2ndpk[0][0] += dtmp1;
		stress_2ndpk[1][1] += dtmp1;
		stress_2ndpk[2][2] += dtmp1;
	}
	{	// 圧力の応力を求める
		const double dtmp1 = 2.0*press*rcg_pinv3;
		for(unsigned int idim=0;idim<ndim;idim++){
		for(unsigned int jdim=0;jdim<ndim;jdim++){
			stress_2ndpk[idim][jdim] += dtmp1*rcg_inv[idim][jdim];
		}
		}
	}

	////////////////////////////////
	// 構成則テンソルを求める

	//ゼロクリア
	double constit[ndim][ndim][ndim][ndim];
	for(unsigned int i=0;i<ndim*ndim*ndim*ndim;i++){ *(&constit[0][0][0][0]+i) = 0.0; }
	// 変位の構成則テンソルを足し合わせる
	for(unsigned int idim=0;idim<ndim;idim++){
	for(unsigned int jdim=0;jdim<ndim;jdim++){
	for(unsigned int kdim=0;kdim<ndim;kdim++){
	for(unsigned int ldim=0;ldim<ndim;ldim++){
        double tmp = 0;
		tmp += 4.0*c1*inv13_rcg_pinv3/3.0*( // 変位からくる項
			   rcg_inv[idim][jdim]*rcg_inv[kdim][ldim]*rcg_pinv1/3.0
			  +rcg_inv[idim][kdim]*rcg_inv[ldim][jdim]*rcg_pinv1*0.5
			  +rcg_inv[idim][ldim]*rcg_inv[kdim][jdim]*rcg_pinv1*0.5);
		tmp	+= 4.0*c2*inv23_rcg_pinv3*2.0/3.0*( // 変位からくる項
			   rcg_inv[idim][jdim]*rcg_inv[kdim][ldim]*rcg_pinv2*(2.0/3.0)
			  +rcg_inv[idim][jdim]*rcg[    kdim][ldim]
			  +rcg[    idim][jdim]*rcg_inv[kdim][ldim]
			  +rcg_inv[idim][kdim]*rcg_inv[jdim][ldim]*rcg_pinv2*0.5
			  +rcg_inv[idim][ldim]*rcg_inv[jdim][kdim]*rcg_pinv2*0.5);
		tmp += 4.0*press*rcg_pinv3* // 圧力から来る項
			     rcg_inv[idim][jdim]*rcg_inv[kdim][ldim]
			  -2.0*press*rcg_pinv3*(
			 	 rcg_inv[idim][kdim]*rcg_inv[ldim][jdim]
				+rcg_inv[idim][ldim]*rcg_inv[kdim][jdim] );
        constit[idim][jdim][kdim][ldim] += tmp;
	}
	}
    }
    }
    // 変位の構成則テンソルを足し合わせる
	for(unsigned int idim=0;idim<ndim;idim++){
	for(unsigned int jdim=0;jdim<ndim;jdim++){
		double dtmp1 = 4.0*c1*inv13_rcg_pinv3/3.0*rcg_inv[idim][jdim];
		for(unsigned int kdim=0;kdim<ndim;kdim++){
			constit[idim][jdim][kdim][kdim] -= dtmp1;
			constit[kdim][kdim][idim][jdim] -= dtmp1;
		}
		const double dtmp2 = 4.0*c2*inv23_rcg_pinv3*rcg_pinv1*(2.0/3.0)*rcg_inv[idim][jdim];
		for(unsigned int kdim=0;kdim<ndim;kdim++){
			constit[idim][jdim][kdim][kdim] -= dtmp2;
			constit[kdim][kdim][idim][jdim] -= dtmp2;
		}
		constit[idim][idim][jdim][jdim] += 4.0*c2*inv23_rcg_pinv3;
		constit[idim][jdim][jdim][idim] -= 2.0*c2*inv23_rcg_pinv3;
		constit[idim][jdim][idim][jdim] -= 2.0*c2*inv23_rcg_pinv3;
	}
	}
	
//	double stress_2ndpk2[nstdim];
	{
		rcg_inv2[0] = rcg_inv[0][0];
		rcg_inv2[1] = rcg_inv[1][1];
		rcg_inv2[2] = rcg_inv[2][2];
		rcg_inv2[3] = rcg_inv[0][1];
		rcg_inv2[4] = rcg_inv[1][2];
		rcg_inv2[5] = rcg_inv[2][0];
	}
	{
		stress_2ndpk2[0] = stress_2ndpk[0][0];
		stress_2ndpk2[1] = stress_2ndpk[1][1];
		stress_2ndpk2[2] = stress_2ndpk[2][2];
		stress_2ndpk2[3] = stress_2ndpk[0][1];
		stress_2ndpk2[4] = stress_2ndpk[1][2];
		stress_2ndpk2[5] = stress_2ndpk[2][0];
	}
//	double constit2[nstdim][nstdim];
	{
		unsigned int istdim2ij[6][2] = { {0,0}, {1,1}, {2,2}, {0,1}, {1,2}, {2,0} };
		for(unsigned int istdim=0;istdim<6;istdim++){
		for(unsigned int jstdim=0;jstdim<6;jstdim++){
			const unsigned int* idim_ij = istdim2ij[istdim];
			const unsigned int* jdim_ij = istdim2ij[jstdim];
			constit2[istdim][jstdim] 
				= constit[ idim_ij[0] ][ idim_ij[1] ][ jdim_ij[0] ][ jdim_ij[1] ];
		}
		}
	}
}


bool AddLinSys_Hyper3D_HEX100100_000010(
        Fem::Eqn::ILinearSystem_Eqn& ls, 
		double c1, double c2,
		double  rho, double g_x, double g_y, double g_z,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world, 
		unsigned int id_ea)
{
	
	std::cout << "Moonin-Rivlin3D  disp:Hex1" << std::endl;

	const CField& field_disp = world.GetField(id_field_disp);
	const CField& field_lambda = world.GetField(id_field_lambda);
	assert( world.IsIdEA(id_ea) );
	const CElemAry& ea = world.GetEA(id_ea);
	assert( ea.ElemType() == HEX );

	const CElemAry::CElemSeg& es_cu = field_disp.GetElemSeg(  id_ea,CORNER,true,world);
	const CElemAry::CElemSeg& es_bp = field_lambda.GetElemSeg(id_ea,BUBBLE,true,world);

	unsigned int num_integral = 1;
	const unsigned int nInt = NIntLineGauss[num_integral];
	const double (*Gauss)[2] = LineGauss[num_integral];
	double detjac, detwei;

	const unsigned int nno_c = 8;	assert( nno_c == es_cu.Length() );
	const unsigned int nno_b = 1;	assert( nno_b == es_bp.Length() );
	const unsigned int ndim = 3;
	const unsigned int nstdim = 6;	// 対称テンソルの次元

	unsigned int noes_cu[nno_c];	// 要素内の節点の節点番号
	unsigned int noes_bp;

	double emat_cucu[nno_c][nno_c][ndim][ndim];	// 要素剛性行列
	double emat_cubp[nno_c][ndim];	// 要素剛性行列
	double emat_bpcu[nno_c][ndim];	// 要素剛性行列
    double emat_bpbp;
	double eres_cu[nno_c][ndim];		// 要素内残差ベクトル
	double eres_bp;

	CMatDia_BlkCrs& mat_cucu = ls.GetMatrix(id_field_disp,  CORNER, world);
	CMatDia_BlkCrs& mat_bpbp = ls.GetMatrix(id_field_lambda,BUBBLE, world);
	CMat_BlkCrs& mat_cubp = ls.GetMatrix(id_field_disp,CORNER,   id_field_lambda,BUBBLE, world);
	CMat_BlkCrs& mat_bpcu = ls.GetMatrix(id_field_lambda,BUBBLE, id_field_disp,  CORNER, world);
    assert( mat_cucu.LenBlkCol()==(int)ndim && mat_cucu.LenBlkRow()==(int)ndim );
    assert( mat_cubp.LenBlkCol()==(int)ndim && mat_cubp.LenBlkRow()==1         );
    assert( mat_bpcu.LenBlkCol()==1         && mat_bpcu.LenBlkRow()==(int)ndim );
	
	CVector_Blk& res_cu = ls.GetResidual(id_field_disp,  CORNER,world);
	CVector_Blk& res_bp = ls.GetResidual(id_field_lambda,BUBBLE,world);
    assert( res_cu.Len() == (int)ndim );
	assert( res_bp.Len() == 1 );

	const CNodeAry::CNodeSeg& ns_cuv  = field_disp.GetNodeSeg(  CORNER,true, world,VALUE);
	const CNodeAry::CNodeSeg& ns_c_co = field_disp.GetNodeSeg(  CORNER,false,world,VALUE);
	const CNodeAry::CNodeSeg& ns_bpv  = field_lambda.GetNodeSeg(BUBBLE,true, world,VALUE);

	for(unsigned int ielem=0;ielem<ea.Size();ielem++){
	    double ecoords[nno_c][ndim];		// 要素節点座標
	    double edisp[  nno_c][ndim];		// 要素節点変位
		es_cu.GetNodes(ielem,noes_cu);
		for(unsigned int ino=0;ino<nno_c;ino++){
			ns_c_co.GetValue(noes_cu[ino],ecoords[ino]);
			ns_cuv.GetValue( noes_cu[ino],edisp[  ino]);
		}
		es_bp.GetNodes(ielem,&noes_bp);
	    double lambda;
		ns_bpv.GetValue(noes_bp,&lambda);

		////////////////////////////////

		// 要素剛性行列、残差を０で初期化
		for(unsigned int i=0;i<nno_c*nno_c*ndim*ndim;i++){ *(&emat_cucu[0][0][0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*ndim;i++){ *(&emat_cubp[0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*ndim;i++){ *(&emat_bpcu[0][0]+i) = 0.0; }
		emat_bpbp = 0;
		for(unsigned int i=0;i<nno_c*ndim;i++){ *(&eres_cu[  0][0]+i) = 0.0; }
		eres_bp = 0.0;

		double vol = 0.0;
		for(unsigned int ir1=0;ir1<nInt;ir1++){
		for(unsigned int ir2=0;ir2<nInt;ir2++){
		for(unsigned int ir3=0;ir3<nInt;ir3++){
			const double r1 = Gauss[ir1][0];
			const double r2 = Gauss[ir2][0];
			const double r3 = Gauss[ir3][0];
            ////////////////
	        double dndx[nno_c][ndim];		// 形状関数の空間微分
        	double an[nno_c];
			ShapeFunc_Hex8(r1,r2,r3,ecoords,detjac,dndx,an);
			detwei = detjac*Gauss[ir1][1]*Gauss[ir2][1]*Gauss[ir3][1];
			vol += detwei;
			// 要素剛性行列，要素内力ベクトルを作る
			double dudx[ndim][ndim];
			for(unsigned int idim=0;idim<ndim;idim++){
			for(unsigned int jdim=0;jdim<ndim;jdim++){
				double dtmp1 = 0.0;
				for(unsigned int ino=0;ino<nno_c;ino++){
					 dtmp1 += edisp[ino][idim]*dndx[ino][jdim];
				}
				dudx[idim][jdim] = dtmp1;
			}
			}	

			double stress_2ndpk2[nstdim];
			double constit2[nstdim][nstdim];
			double rcg_pinv1, rcg_pinv2, rcg_pinv3;
			double rcg_inv2[nstdim];
			MakeStressConstitute_Hyper3D(
				c1, c2, dudx, lambda,
				stress_2ndpk2, constit2,
				rcg_pinv1, rcg_pinv2, rcg_pinv3,  rcg_inv2);

			double disp2strain2[nno_c][ndim][nstdim];
			{
				double z_mat[ndim][ndim];
				for(unsigned int idim=0;idim<ndim;idim++){
					for(unsigned int jdim=0;jdim<ndim;jdim++){
						z_mat[idim][jdim] = dudx[idim][jdim];
					}
					z_mat[idim][idim] += 1.0;
				}
				for(unsigned int ino=0;ino<nno_c;ino++){
				for(unsigned int idim=0;idim<ndim;idim++){
					disp2strain2[ino][idim][0] = dndx[ino][0]*z_mat[idim][0];
					disp2strain2[ino][idim][1] = dndx[ino][1]*z_mat[idim][1];
					disp2strain2[ino][idim][2] = dndx[ino][2]*z_mat[idim][2];
					disp2strain2[ino][idim][3] = dndx[ino][0]*z_mat[idim][1]+dndx[ino][1]*z_mat[idim][0];
					disp2strain2[ino][idim][4] = dndx[ino][1]*z_mat[idim][2]+dndx[ino][2]*z_mat[idim][1];
					disp2strain2[ino][idim][5] = dndx[ino][2]*z_mat[idim][0]+dndx[ino][0]*z_mat[idim][2];
				}
				}
			}

			////////////////////////////////
			// 接線剛性行列を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int jno=0;jno<nno_c;jno++){
				for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int jdim=0;jdim<ndim;jdim++){
					double dtmp1 = 0.0;
					for(unsigned int gstdim=0;gstdim<nstdim;gstdim++){
					for(unsigned int hstdim=0;hstdim<nstdim;hstdim++){
						dtmp1 += constit2[gstdim][hstdim]
							*disp2strain2[ino][idim][gstdim]*disp2strain2[jno][jdim][hstdim];
					}
					}
					emat_cucu[ino][jno][idim][jdim] += detwei*dtmp1;
				}
				}
				{
					double dtmp2 = 0.0;
					dtmp2 += stress_2ndpk2[0]*dndx[ino][0]*dndx[jno][0];
					dtmp2 += stress_2ndpk2[1]*dndx[ino][1]*dndx[jno][1];
					dtmp2 += stress_2ndpk2[2]*dndx[ino][2]*dndx[jno][2];
					dtmp2 += stress_2ndpk2[3]*dndx[ino][0]*dndx[jno][1];
					dtmp2 += stress_2ndpk2[4]*dndx[ino][1]*dndx[jno][2];
					dtmp2 += stress_2ndpk2[5]*dndx[ino][2]*dndx[jno][0];
					for(unsigned int idim=0;idim<ndim;idim++){
						emat_cucu[ino][jno][idim][idim] += detwei*dtmp2;
					}
				}
			}
			}

			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				double dtmp1 = 0.0;
				for(unsigned int istdim=0;istdim<nstdim;istdim++){
					dtmp1 += disp2strain2[ino][idim][istdim]*rcg_inv2[istdim];
				}
				emat_cubp[ino][idim] += detwei*dtmp1*rcg_pinv3*2;
				emat_bpcu[ino][idim] += detwei*dtmp1*rcg_pinv3*2;
			}
			}

			////////////////
			// 内力を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				double dtmp1 = 0.0;
				for(unsigned int istdim=0;istdim<nstdim;istdim++){
					dtmp1 += disp2strain2[ino][idim][istdim]*stress_2ndpk2[istdim];
				}
				eres_cu[ino][idim] -= detwei*dtmp1;
			}
			}
			eres_bp -= detwei*(rcg_pinv3-1.0);
			
			// 要素節点等価外力ベクトルを積n分毎に足し合わせる
			for(unsigned int ino=0;ino<nno_c;ino++){
				eres_cu[ino][0] += detwei*rho*g_x*an[ino];
				eres_cu[ino][1] += detwei*rho*g_y*an[ino];
				eres_cu[ino][2] += detwei*rho*g_z*an[ino];
			}
		}
		}
		}

		mat_cucu.Mearge(nno_c, noes_cu,  nno_c, noes_cu,  ndim*ndim,  &emat_cucu[0][0][0][0]);
		mat_cubp.Mearge(nno_c, noes_cu,  nno_b,&noes_bp,  ndim,       &emat_cubp[0][0]      );
		mat_bpcu.Mearge(nno_b,&noes_bp,  nno_c, noes_cu,  ndim,       &emat_bpcu[0][0]      );
        mat_bpbp.Mearge(nno_b,&noes_bp,  nno_b,&noes_bp,  1,          &emat_bpbp);
		for(unsigned int ino=0;ino<nno_c;ino++){
		for(unsigned int idim=0;idim<ndim;idim++){
			res_cu.AddValue(noes_cu[ino],idim,eres_cu[ino][idim]);
		}
		}
		res_bp.AddValue(noes_bp,0,eres_bp);
	}

	return true;
}


bool AddLinSys_Hyper3D_NonStatic_NewmarkBeta_HEX100100_000010(
		double dt, double gamma, double beta,
        Fem::Eqn::ILinearSystem_Eqn& ls, 
		double c1, double c2,
		double  rho, double g_x, double g_y, double g_z,
		const unsigned int id_field_disp, const unsigned int id_field_lambda,
		const CFieldWorld& world, 
		bool is_inital,
		unsigned int id_ea )
{
//	std::cout << "Moonin-Rivlin3D  disp:HEX1st lam Hex0" << dt << " " << gamma << beta << std::endl;

	const CField& field_disp   = world.GetField(id_field_disp  );
	const CField& field_lambda = world.GetField(id_field_lambda);
	assert( world.IsIdEA(id_ea) );
    const CElemAry& ea = world.GetEA(id_ea);
	assert( ea.ElemType() == HEX );

	const CElemAry::CElemSeg& es_cu = field_disp.GetElemSeg(  id_ea,CORNER,true,world);
	const CElemAry::CElemSeg& es_bp = field_lambda.GetElemSeg(id_ea,BUBBLE,true,world);

	unsigned int num_integral = 1;
	const unsigned int nInt = NIntLineGauss[num_integral];
	const double (*Gauss)[2] = LineGauss[num_integral];
	double detjac, detwei;

	const unsigned int nno_c = 8;	assert( nno_c == es_cu.Length() );
	const unsigned int nno_b = 1;	assert( nno_b == es_bp.Length() );
	const unsigned int ndim = 3;
	const unsigned int nstdim = 6;	// 対称テンソルの次元

	double eKmat_cucu[nno_c][nno_c][ndim][ndim];
	double eKmat_cubp[nno_c][ndim];
	double eKmat_bpcu[nno_c][ndim];
    double eKmat_bpbp;
	double eMmat_cucu[nno_c][nno_c][ndim][ndim];
    ////////////////
	double emat_cucu[nno_c][nno_c][ndim][ndim];	// 要素剛性行列
	double emat_cubp[nno_c][ndim];	// 要素剛性行列
	double emat_bpcu[nno_c][ndim];	// 要素剛性行列
    double emat_bpbp;

	double eres_cu[nno_c][ndim];		// 要素内残差ベクトル
	double eres_bp;

				
	CMatDia_BlkCrs& mat_cucu = ls.GetMatrix(id_field_disp,  CORNER, world);
	CMatDia_BlkCrs& mat_bpbp = ls.GetMatrix(id_field_lambda,BUBBLE, world);
	CMat_BlkCrs& mat_cubp = ls.GetMatrix(id_field_disp,CORNER, id_field_lambda,BUBBLE, world);
	CMat_BlkCrs& mat_bpcu = ls.GetMatrix(id_field_lambda,BUBBLE, id_field_disp,CORNER, world);
    assert( mat_cucu.LenBlkCol()==(int)ndim && mat_cucu.LenBlkRow()==(int)ndim );
    assert( mat_cubp.LenBlkCol()==(int)ndim && mat_cubp.LenBlkRow()==1         );
    assert( mat_bpcu.LenBlkCol()==1         && mat_bpcu.LenBlkRow()==(int)ndim );
	
	CVector_Blk& res_cu = ls.GetResidual(id_field_disp,  CORNER,world);
	CVector_Blk& res_bp = ls.GetResidual(id_field_lambda,BUBBLE,world);
    assert( res_cu.Len() == (int)ndim );
	assert( res_bp.Len() == 1 );

	const CNodeAry::CNodeSeg& ns_c_co = field_disp.GetNodeSeg(  CORNER,false,world,VALUE);
	const CNodeAry::CNodeSeg& ns_cu  = field_disp.GetNodeSeg(  CORNER,true, world,VALUE);
	const CNodeAry::CNodeSeg& ns_cv  = field_disp.GetNodeSeg(  CORNER,true, world,VELOCITY);
	const CNodeAry::CNodeSeg& ns_ca  = field_disp.GetNodeSeg(  CORNER,true, world,ACCELERATION);
	const CNodeAry::CNodeSeg& ns_bp   = field_lambda.GetNodeSeg(BUBBLE,true, world,VALUE);
	const CNodeAry::CNodeSeg& ns_bpv  = field_lambda.GetNodeSeg(BUBBLE,true, world,VELOCITY);
	const CNodeAry::CNodeSeg& ns_bpa  = field_lambda.GetNodeSeg(BUBBLE,true, world,ACCELERATION);

	for(unsigned int ielem=0;ielem<ea.Size();ielem++)
    {    
    	unsigned int noes_cu[nno_c];	// 要素内の節点の節点番号
		es_cu.GetNodes(ielem,noes_cu);
	    double ecoords[nno_c][ndim];	// 要素節点座標
	    double edisp[  nno_c][ndim];	// 要素節点変位
	    double evelo[  nno_c][ndim];	// 要素節点変位
	    double eacc[   nno_c][ndim];	// 要素節点変位
		for(unsigned int ino=0;ino<nno_c;ino++){
			ns_c_co.GetValue(noes_cu[ino],ecoords[ino]);
			ns_cu.GetValue( noes_cu[ino],edisp[ino]);
			ns_cv.GetValue( noes_cu[ino],evelo[ino]);
			ns_ca.GetValue( noes_cu[ino],eacc[ino]);
		}
    	unsigned int noes_bp;
		es_bp.GetNodes(ielem,&noes_bp);
	    double lambda;
	    double lambda_velo;
	    double lambda_acc;
		ns_bp.GetValue(noes_bp,&lambda);
		ns_bpv.GetValue(noes_bp,&lambda_velo);
		ns_bpa.GetValue(noes_bp,&lambda_acc);

		////////////////////////////////

		// 要素剛性行列、残差を０で初期化
		for(unsigned int i=0;i<nno_c*nno_c*ndim*ndim;i++){ *(&eMmat_cucu[0][0][0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*nno_c*ndim*ndim;i++){ *(&eKmat_cucu[0][0][0][0]+i) = 0.0; }
        eKmat_bpbp = 0;
		for(unsigned int i=0;i<nno_c*ndim;i++){ *(&eKmat_cubp[0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*ndim;i++){ *(&eKmat_bpcu[0][0]+i) = 0.0; }
		for(unsigned int i=0;i<nno_c*ndim;i++){ *(&eres_cu[0][0]+i) = 0.0; }
		eres_bp = 0.0;

		double vol = 0.0;
		for(unsigned int ir1=0;ir1<nInt;ir1++){
		for(unsigned int ir2=0;ir2<nInt;ir2++){
		for(unsigned int ir3=0;ir3<nInt;ir3++){
			const double r1 = Gauss[ir1][0];
			const double r2 = Gauss[ir2][0];
			const double r3 = Gauss[ir3][0];
	        double dndx[nno_c][ndim];		// 形状関数の空間微分
	        double an[nno_c];
			ShapeFunc_Hex8(r1,r2,r3,ecoords,detjac,dndx,an);
			detwei = detjac*Gauss[ir1][1]*Gauss[ir2][1]*Gauss[ir3][1];
			vol += detwei;

			////////////////////////////////
			// 変位勾配テンソルを作る
			double dudx[ndim][ndim];
			for(unsigned int idim=0;idim<ndim;idim++){
			for(unsigned int jdim=0;jdim<ndim;jdim++){
				double dtmp1 = 0.0;
				for(unsigned int ino=0;ino<nno_c;ino++){
					 dtmp1 += edisp[ino][idim]*dndx[ino][jdim];
				}
				dudx[idim][jdim] = dtmp1;
			}
			}	

			////////////////////////////////
			// 構成則テンソル，応力を作る
			double stress_2ndpk2[nstdim];
			double constit2[nstdim][nstdim];
			double rcg_pinv1, rcg_pinv2, rcg_pinv3;
			double rcg_inv2[nstdim];
			MakeStressConstitute_Hyper3D(
				c1, c2, dudx, lambda,
				stress_2ndpk2, constit2,
				rcg_pinv1, rcg_pinv2, rcg_pinv3,  rcg_inv2);

			////////////////////////////////
			// 変位-歪み関係行列を作る
			double disp2strain2[nno_c][ndim][nstdim];
			{
				double z_mat[ndim][ndim];
				for(unsigned int idim=0;idim<ndim;idim++){
					for(unsigned int jdim=0;jdim<ndim;jdim++){
						z_mat[idim][jdim] = dudx[idim][jdim];
					}
					z_mat[idim][idim] += 1.0;
				}
				for(unsigned int ino=0;ino<nno_c;ino++){
				for(unsigned int idim=0;idim<ndim;idim++){
					disp2strain2[ino][idim][0] = dndx[ino][0]*z_mat[idim][0];
					disp2strain2[ino][idim][1] = dndx[ino][1]*z_mat[idim][1];
					disp2strain2[ino][idim][2] = dndx[ino][2]*z_mat[idim][2];
					disp2strain2[ino][idim][3] = dndx[ino][0]*z_mat[idim][1]+dndx[ino][1]*z_mat[idim][0];
					disp2strain2[ino][idim][4] = dndx[ino][1]*z_mat[idim][2]+dndx[ino][2]*z_mat[idim][1];
					disp2strain2[ino][idim][5] = dndx[ino][2]*z_mat[idim][0]+dndx[ino][0]*z_mat[idim][2];
				}
				}
			}

			////////////////////////////////
			// 接線剛性行列の初期変位項を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int jno=0;jno<ino; jno++){
			for(unsigned int idim=0;idim<ndim;idim++){
			for(unsigned int jdim=0;jdim<ndim;jdim++){
				double dtmp1 = 0.0;
				for(unsigned int gstdim=0;gstdim<nstdim;gstdim++){
                    const double* p0 = constit2[gstdim];
                    const double* p1 = disp2strain2[jno][jdim];
                    const double dtmp2 = p0[0]*p1[0] + p0[1]*p1[1] + p0[2]*p1[2] 
                                       + p0[3]*p1[3] + p0[4]*p1[4] + p0[5]*p1[5];
                    dtmp1 += dtmp2*disp2strain2[ino][idim][gstdim];
				}
				eKmat_cucu[ino][jno][idim][jdim] += detwei*dtmp1;
				eKmat_cucu[jno][ino][jdim][idim] += detwei*dtmp1;
			}
			}
            }
            }
			for(unsigned int ino=0;ino<nno_c;ino++){
			    for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int jdim=0;jdim<idim;jdim++){
					double dtmp1 = 0.0;
				    for(unsigned int gstdim=0;gstdim<nstdim;gstdim++){
                        const double* p0 = constit2[gstdim];
                        const double* p1 = disp2strain2[ino][jdim];
                        const double dtmp2 = p0[0]*p1[0] + p0[1]*p1[1] + p0[2]*p1[2] 
                                           + p0[3]*p1[3] + p0[4]*p1[4] + p0[5]*p1[5];
                        dtmp1 += dtmp2*disp2strain2[ino][idim][gstdim];
				    }
					eKmat_cucu[ino][ino][idim][jdim] += detwei*dtmp1;
					eKmat_cucu[ino][ino][jdim][idim] += detwei*dtmp1;
				}
				}
            }
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				double dtmp1 = 0.0;
				for(unsigned int gstdim=0;gstdim<nstdim;gstdim++){
				for(unsigned int hstdim=0;hstdim<nstdim;hstdim++){
					dtmp1 += constit2[gstdim][hstdim]
						*disp2strain2[ino][idim][gstdim]*disp2strain2[ino][idim][hstdim];
				}
				}
				eKmat_cucu[ino][ino][idim][idim] += detwei*dtmp1;
            }
            }
            ////////////////////////////////
            // 接線剛性行列の初期変位項を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int jno=0;jno<nno_c;jno++){
				double dtmp2 = 0.0;
				dtmp2 += stress_2ndpk2[0]*dndx[ino][0]*dndx[jno][0];
				dtmp2 += stress_2ndpk2[1]*dndx[ino][1]*dndx[jno][1];
				dtmp2 += stress_2ndpk2[2]*dndx[ino][2]*dndx[jno][2];
				dtmp2 += stress_2ndpk2[3]*dndx[ino][0]*dndx[jno][1];
				dtmp2 += stress_2ndpk2[4]*dndx[ino][1]*dndx[jno][2];
				dtmp2 += stress_2ndpk2[5]*dndx[ino][2]*dndx[jno][0];
				eKmat_cucu[ino][jno][0][0] += detwei*dtmp2;
				eKmat_cucu[ino][jno][1][1] += detwei*dtmp2;
				eKmat_cucu[ino][jno][2][2] += detwei*dtmp2;
			}
			}
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				double dtmp1 = 0.0;
				for(unsigned int istdim=0;istdim<nstdim;istdim++){
					dtmp1 += disp2strain2[ino][idim][istdim]*rcg_inv2[istdim];
				}
				eKmat_cubp[ino][idim] += detwei*dtmp1*rcg_pinv3*2;
				eKmat_bpcu[ino][idim] += detwei*dtmp1*rcg_pinv3*2;
			}
			}

			////////////////////////////////
			// 質量行列を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int jno=0;jno<nno_c;jno++){
				const double dtmp1 = detwei*rho*an[ino]*an[jno];
				eMmat_cucu[ino][jno][0][0] += dtmp1;
				eMmat_cucu[ino][jno][1][1] += dtmp1;
				eMmat_cucu[ino][jno][2][2] += dtmp1;
			}
			}

			////////////////////////////////
			// 内力を求める
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				double dtmp1 = 0.0;
				for(unsigned int istdim=0;istdim<nstdim;istdim++){
					dtmp1 += disp2strain2[ino][idim][istdim]*stress_2ndpk2[istdim];
				}
				eres_cu[ino][idim] -= detwei*dtmp1;
			}
			}
			eres_bp -= detwei*(rcg_pinv3-1.0);
			
			////////////////////////////////
			// 要素節点等価外力ベクトルを積n分毎に足し合わせる
			for(unsigned int ino=0;ino<nno_c;ino++){
				eres_cu[ino][0] += detwei*rho*g_x*an[ino];
				eres_cu[ino][1] += detwei*rho*g_y*an[ino];
				eres_cu[ino][2] += detwei*rho*g_z*an[ino];
			}
		}
		}
		}

        const double vol_stiff = (c1+c2)*1000;
        eKmat_bpbp = -vol/vol_stiff;
        eres_bp += -eKmat_bpbp*lambda;

		////////////////////////////////

		{	// 係数行列を作る
			const double dtmp1 = beta*dt*dt;
			for(unsigned int i=0;i<nno_c*nno_c*ndim*ndim;i++){
				*(&emat_cucu[0][0][0][0]+i) 
					= *(&eKmat_cucu[0][0][0][0]+i)*dtmp1 + *(&eMmat_cucu[0][0][0][0]+i);
			}
			for(unsigned int i=0;i<nno_c*ndim;i++){
				*(&emat_cubp[0][0]+i) = *(&eKmat_cubp[0][0]+i)*dtmp1;
			}
			for(unsigned int i=0;i<nno_c*ndim;i++){
				*(&emat_bpcu[0][0]+i) = *(&eKmat_bpcu[0][0]+i)*dtmp1;
			}
            emat_bpbp = dtmp1*eKmat_bpbp;
		}
		for(unsigned int ino=0;ino<nno_c;ino++){
		for(unsigned int idim=0;idim<ndim;idim++){
			for(unsigned int jno=0;jno<nno_c;jno++){
			for(unsigned int jdim=0;jdim<ndim;jdim++){
				eres_cu[ino][idim] -= eMmat_cucu[ino][jno][idim][jdim]*eacc[jno][jdim];
			}
			}
		}
		}
		if( is_inital ){
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				for(unsigned int jno=0;jno<nno_c;jno++){
				for(unsigned int jdim=0;jdim<ndim;jdim++){
					eres_cu[ino][idim] 
						-= eKmat_cucu[ino][jno][idim][jdim]*(dt*evelo[jno][jdim]+0.5*dt*dt*eacc[jno][jdim]);
				}
				}
				eres_cu[ino][idim] -= eKmat_cubp[ino][idim]*(dt*lambda_velo+0.5*dt*dt*lambda_acc);
			}
			}
			for(unsigned int ino=0;ino<nno_c;ino++){
			for(unsigned int idim=0;idim<ndim;idim++){
				eres_bp -= eKmat_bpcu[ino][idim]*(dt*evelo[ino][idim]+0.5*dt*dt*eacc[ino][idim]);
			}
			}
            eres_bp -= eKmat_bpbp*(dt*lambda_velo+0.5*dt*dt*lambda_acc);
		}

		mat_cucu.Mearge(nno_c, noes_cu,  nno_c, noes_cu,  ndim*ndim,  &emat_cucu[0][0][0][0] );
		mat_cubp.Mearge(nno_c, noes_cu,  nno_b,&noes_bp,  ndim,       &emat_cubp[0][0]       );
		mat_bpcu.Mearge(nno_b,&noes_bp,  nno_c, noes_cu,  ndim,       &emat_bpcu[0][0]       );
        mat_bpbp.Mearge(nno_b,&noes_bp,  nno_b,&noes_bp,  1,          &emat_bpbp             );
		for(unsigned int ino=0;ino<nno_c;ino++){
		for(unsigned int idim=0;idim<ndim;idim++){
			res_cu.AddValue(noes_cu[ino],idim,eres_cu[ino][idim]);
		}
		}
		res_bp.AddValue(noes_bp,0,eres_bp);
	}

	return true;
}

