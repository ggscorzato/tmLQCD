/* $Id$ */

/*******************************************************************************
 * Generalized minimal residual (GMRES) with a maximal number of restarts.    
 * Solves Q=AP for complex regular matrices A.
 * For details see: Andreas Meister, Numerik linearer Gleichungssysteme        
 *   or the original citation:                                                 
 * Y. Saad, M.H.Schultz in GMRES: A generalized minimal residual algorithm    
 *                         for solving nonsymmetric linear systems.            
 * 			SIAM J. Sci. Stat. Comput., 7: 856-869, 1986           
 *           
 * int gmres(spinor * const P,spinor * const Q, 
 *	   const int m, const int max_restarts,
 *	   const double eps_sq, matrix_mult f)
 *                                                                 
 * Returns the number of iterations needed or -1 if maximal number of restarts  
 * has been reached.                                                           
 *
 * Inout:                                                                      
 *  spinor * P       : guess for the solving spinor                                             
 * Input:                                                                      
 *  spinor * Q       : source spinor
 *  int m            : Maximal dimension of Krylov subspace                                     
 *  int max_restarts : maximal number of restarts                                   
 *  double eps       : stopping criterium                                                     
 *  matrix_mult f    : pointer to a function containing the matrix mult
 *                     for type matrix_mult see matrix_mult_typedef.h
 *
 * Autor: Carsten Urbach <urbach@ifh.de>
 ********************************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include"global.h"
#include"su3.h"
#include"linalg_eo.h"
#include"gmres.h"

#ifdef _SOLVER_OUTPUT
#define _SO(x) x
#else
#define _SO(x)
#endif

static void init_gmres(const int _M, const int _V);

static complex ** H;
static complex * alpha;
static complex * c;
static double * s;
static spinor ** V;
static spinor * _v;
static complex * _h;
static complex * alpha;
static complex * c;
static double * s;

int gmres(spinor * const P,spinor * const Q, 
	   const int m, const int max_restarts,
	   const double eps_sq, matrix_mult f){

  int restart, i, j, k;
  double beta, eps;
  complex tmp1, tmp2;
  int N=VOLUME/2;

/*   init_solver_field(3); */
  eps=sqrt(eps_sq);
  init_gmres(m, VOLUMEPLUSRAND);

  assign(spinor_field[DUM_SOLVER+2], P, N);
  for(restart = 0; restart < max_restarts; restart++){
    /* r_0=Q-AP  (b=Q, x+0=P) */
    f(spinor_field[DUM_SOLVER], spinor_field[DUM_SOLVER+2]);
    diff(spinor_field[DUM_SOLVER], Q, spinor_field[DUM_SOLVER], N);

    /* v_0=r_0/||r_0|| */
    alpha[0].re=sqrt(square_norm(spinor_field[DUM_SOLVER], N));

    if(g_proc_id == g_stdio_proc){
      printf("%d\t%g true residue\n", restart*m, alpha[0].re*alpha[0].re); 
      fflush(stdout);
    }

    if(alpha[0].re==0.){
      assign(P, spinor_field[DUM_SOLVER+2], N);
      return(restart*m);
    }

    mul_r(V[0], 1./alpha[0].re, spinor_field[DUM_SOLVER], N);

    for(j = 0; j < m; j++){
      /* spinor_field[DUM_SOLVER]=A*v_j */

      f(spinor_field[DUM_SOLVER], V[j]);

      /* Set h_ij and omega_j */
      /* spinor_field[DUM_SOLVER+1] <- omega_j */
      assign(spinor_field[DUM_SOLVER+1], spinor_field[DUM_SOLVER], N);
      for(i = 0; i <= j; i++){
	H[i][j] = scalar_prod(V[i], spinor_field[DUM_SOLVER+1], N);
	assign_diff_mul(spinor_field[DUM_SOLVER+1], V[i], H[i][j], N);
      }

      _complex_set(H[j+1][j], sqrt(square_norm(spinor_field[DUM_SOLVER+1], N)), 0.);
      for(i = 0; i < j; i++){
	tmp1 = H[i][j];
	tmp2 = H[i+1][j];
	_mult_real(H[i][j], tmp2, s[i]);
	_add_assign_complex_conj(H[i][j], c[i], tmp1);
	_mult_real(H[i+1][j], tmp1, s[i]);
	_diff_assign_complex(H[i+1][j], c[i], tmp2);
      }

      /* Set beta, s, c, alpha[j],[j+1] */
      beta = sqrt(_complex_square_norm(H[j][j]) + _complex_square_norm(H[j+1][j]));
      s[j] = H[j+1][j].re / beta;
      _mult_real(c[j], H[j][j], 1./beta);
      _complex_set(H[j][j], beta, 0.);
      _mult_real(alpha[j+1], alpha[j], s[j]);
      tmp1 = alpha[j];
      _mult_assign_complex_conj(alpha[j], c[j], tmp1);

      /* precision reached? */
      if(g_proc_id == g_stdio_proc){
	printf("%d\t%g residue\n", restart*m+j, alpha[j+1].re*alpha[j+1].re); 
	fflush(stdout);
      }
      if(alpha[j+1].re <= eps){
	_mult_real(alpha[j], alpha[j], 1./H[j][j].re);
	assign_add_mul(spinor_field[DUM_SOLVER+2], V[j], alpha[j], N);
	for(i = j-1; i >= 0; i--){
	  for(k = i+1; k <= j; k++){
 	    _mult_assign_complex(tmp1, H[i][k], alpha[k]); 
	    _diff_complex(alpha[i], tmp1);
	  }
	  _mult_real(alpha[i], alpha[i], 1./H[i][i].re);
	  assign_add_mul(spinor_field[DUM_SOLVER+2], V[i], alpha[i], N);
	}
	for(i = 0; i < m; i++){
	  alpha[i].im = 0.;
	}
	assign(P, spinor_field[DUM_SOLVER+2], N);
	return(restart*m+j);
      }
      /* if not */
      else{
	if(j != m-1){
	  mul_r(V[(j+1)], 1./H[j+1][j].re, spinor_field[DUM_SOLVER+1], N);
	}
      }

    }
    j=m-1;
    /* prepare for restart */
    _mult_real(alpha[j], alpha[j], 1./H[j][j].re);
    assign_add_mul(spinor_field[DUM_SOLVER+2], V[j], alpha[j], N);
    for(i = j-1; i >= 0; i--){
      for(k = i+1; k <= j; k++){
	_mult_assign_complex(tmp1, H[i][k], alpha[k]);
	_diff_complex(alpha[i], tmp1);
      }
      _mult_real(alpha[i], alpha[i], 1./H[i][i].re);
      assign_add_mul(spinor_field[DUM_SOLVER+2], V[i], alpha[i], N);
    }
    for(i = 0; i < m; i++){
      alpha[i].im = 0.;
    }
  }

  /* If maximal number of restarts is reached */
  assign(P, spinor_field[DUM_SOLVER+2], N);

  return(-1);
}

static void init_gmres(const int _M, const int _V){
  static int Vo = -1;
  static int M = -1;
  static int init = 0;
  int i;
  if((M != _M)||(init == 0)||(Vo != _V)){
    if(init == 1){
      free(H);
      free(V);
      free(_h);
      free(_v);
      free(alpha);
      free(c);
      free(s);
    }
    Vo = _V;
    M = _M;
    H = calloc(M+1, sizeof(complex *));
    V = calloc(M, sizeof(spinor *));
#if (defined SSE || defined SSE2)
    _h = calloc((M+2)*M, sizeof(complex));
    H[0] = (complex *)(((unsigned int)(_h)+ALIGN_BASE)&~ALIGN_BASE); 
    _v = calloc(M*Vo+1, sizeof(spinor));
    V[0] = (spinor *)(((unsigned int)(_v)+ALIGN_BASE)&~ALIGN_BASE);
#else
    _h = calloc((M+1)*M, sizeof(complex));
    H[0] = _h;
    _v = calloc(M*Vo, sizeof(spinor));
    V[0] = _v;
#endif
    s = calloc(M, sizeof(double));
    c = calloc(M, sizeof(complex));
    alpha = calloc(M+1, sizeof(complex));
    for(i = 1; i < M; i++){
      V[i] = V[i-1] + Vo;
      H[i] = H[i-1] + M;
    }
    H[M] = H[M-1] + M;
    init = 1;
  }
}