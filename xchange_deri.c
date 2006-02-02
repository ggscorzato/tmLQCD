/* $Id$ */

/**********************************************************
 * 
 * exchange routines for gauge fields
 *
 * Author: Carsten Urbach 
 *
 **********************************************************/


#ifdef HAVE_CONFIG_H
# include<config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifdef MPI
# include <mpi.h>
#endif
#include "global.h"
#include "mpi_init.h"
#include "su3.h"
#include "su3adj.h"
#include "xchange_deri.h"

void xchange_deri()
{
#ifdef MPI
  int ix,mu, t, y, z, x;
  MPI_Status status;
  /* send the data to the neighbour on the left in time direction */
  /* recieve the data from the neighbour on the right in time direction */
#  ifndef _NEW_GEOMETRY
  MPI_Sendrecv(&df0[(T+1)*LX*LY*LZ][0].d1,    1, deri_time_slice_cont, g_nb_t_dn, 43,
	       &ddummy[(T-1)*LX*LY*LZ][0].d1, 1, deri_time_slice_cont, g_nb_t_up, 43,
	       g_cart_grid, &status);

  /* add ddummy to df0 */
  for(x = 0; x < LX; x++) {
    for(y = 0; y < LY; y++) {
      for(z = 0; z < LZ; z++) {
	ix = g_ipt[T-1][x][y][z];
	for(mu=0;mu<4;mu++){ 
	  df0[ix][mu].d1 += ddummy[ix][mu].d1;
	  df0[ix][mu].d2 += ddummy[ix][mu].d2;
	  df0[ix][mu].d3 += ddummy[ix][mu].d3;
	  df0[ix][mu].d4 += ddummy[ix][mu].d4;
	  df0[ix][mu].d5 += ddummy[ix][mu].d5;
	  df0[ix][mu].d6 += ddummy[ix][mu].d6;
	  df0[ix][mu].d7 += ddummy[ix][mu].d7;
	  df0[ix][mu].d8 += ddummy[ix][mu].d8;
	}
      }
    }
  }

#  else
  MPI_Sendrecv(&df0[(T+1)*LX*LY*LZ][0].d1,          1, deri_time_slice_cont , g_nb_t_dn, 43, 
	       &ddummy[(T-1)*LX*LY*LZ/2][0].d1,     1, deri_time_slice_split, g_nb_t_up, 43,
	       g_cart_grid, &status);
  /* add ddummy to df0 */
  for(ix=(T-1)*LX*LY*LZ/2;ix < T*LX*LY*LZ/2; ix++){
    for(mu=0;mu<4;mu++){
      df0[ix][mu].d1 += ddummy[ix][mu].d1;
      df0[ix][mu].d2 += ddummy[ix][mu].d2;
      df0[ix][mu].d3 += ddummy[ix][mu].d3;
      df0[ix][mu].d4 += ddummy[ix][mu].d4;
      df0[ix][mu].d5 += ddummy[ix][mu].d5;
      df0[ix][mu].d6 += ddummy[ix][mu].d6;
      df0[ix][mu].d7 += ddummy[ix][mu].d7;
      df0[ix][mu].d8 += ddummy[ix][mu].d8;
    }
  }
  for(ix=VOLUME-LX*LY*LZ/2;ix < VOLUME; ix++){
    for(mu=0;mu<4;mu++){
      df0[ix][mu].d1 += ddummy[ix][mu].d1;
      df0[ix][mu].d2 += ddummy[ix][mu].d2;
      df0[ix][mu].d3 += ddummy[ix][mu].d3;
      df0[ix][mu].d4 += ddummy[ix][mu].d4;
      df0[ix][mu].d5 += ddummy[ix][mu].d5;
      df0[ix][mu].d6 += ddummy[ix][mu].d6;
      df0[ix][mu].d7 += ddummy[ix][mu].d7;
      df0[ix][mu].d8 += ddummy[ix][mu].d8;
    }
  }
#  endif
  /* send the data to the neighbour on the right is not needed*/

#  if (defined PARALLELXT || defined PARALLELXYT)

  /* send the data to the neighbour on the left in x direction */
  /* recieve the data from the neighbour on the right in x direction */
#    ifndef _NEW_GEOMETRY
  MPI_Sendrecv(&df0[(T+2)*LX*LY*LZ + T*LY*LZ][0],    1, deri_x_slice_cont, g_nb_x_dn, 44,
	       &ddummy[(LX-1)*LY*LZ][0],             1, deri_x_slice_gath, g_nb_x_up, 44,
	       g_cart_grid, &status);
#    else
  MPI_Sendrecv(&df0[(T+2)*LX*LY*LZ + T*LY*LZ][0],    1, deri_x_slice_cont      , g_nb_x_dn, 44,
	       &ddummy[(LX-1)*LY*LZ/2][0],           1, deri_x_slice_gath_split, g_nb_x_up, 44,
	       g_cart_grid, &status);
#    endif
  /* add ddummy to df0 */
  for(t = 0; t < T; t++) {
    for(y = 0; y < LY; y++) {
      for(z = 0; z < LZ; z++) {
	ix = g_ipt[t][LX-1][y][z];
	for(mu=0;mu<4;mu++){
	  df0[ix][mu].d1 += ddummy[ix][mu].d1;
	  df0[ix][mu].d2 += ddummy[ix][mu].d2;
	  df0[ix][mu].d3 += ddummy[ix][mu].d3;
	  df0[ix][mu].d4 += ddummy[ix][mu].d4;
	  df0[ix][mu].d5 += ddummy[ix][mu].d5;
	  df0[ix][mu].d6 += ddummy[ix][mu].d6;
	  df0[ix][mu].d7 += ddummy[ix][mu].d7;
	  df0[ix][mu].d8 += ddummy[ix][mu].d8;
	}
      }
    }
  }
  /* send the data to the neighbour on the right is not needed*/  

  /* end of ifdef PARALLELXT || PARALLELXYT */
#  endif

#  ifdef PARALLELXYT
  /* send the data to the neighbour on the left in y direction */
  /* recieve the data from the neighbour on the right in y direction */
#    ifndef _NEW_GEOMETRY
  MPI_Sendrecv(&df0[VOLUME + 2*LZ*(LX*LY + T*LY) + T*LX*LZ][0], 1, deri_y_slice_cont, g_nb_y_dn, 45,
	       &ddummy[(LY-1)*LZ][0],                           1, deri_y_slice_gath, g_nb_y_up, 45,
	       g_cart_grid, &status);
#    else
  MPI_Sendrecv(&df0[VOLUME + 2*LZ*(LX*LY + T*LY) + T*LX*LZ][0], 1, deri_y_slice_cont      , g_nb_y_dn, 45,
	       &ddummy[(LY-1)*LZ/2][0],                         1, deri_y_slice_gath_split, g_nb_y_up, 45,
	       g_cart_grid, &status);
#    endif
  /* add ddummy to df0 */
  for(t = 0; t < T; t++) {
    for(x = 0; x < LX; x++) {
      for(z = 0; z < LZ; z++) {
	ix = g_ipt[t][x][LY-1][z];
	for(mu=0;mu<4;mu++){
	  df0[ix][mu].d1 += ddummy[ix][mu].d1;
	  df0[ix][mu].d2 += ddummy[ix][mu].d2;
	  df0[ix][mu].d3 += ddummy[ix][mu].d3;
	  df0[ix][mu].d4 += ddummy[ix][mu].d4;
	  df0[ix][mu].d5 += ddummy[ix][mu].d5;
	  df0[ix][mu].d6 += ddummy[ix][mu].d6;
	  df0[ix][mu].d7 += ddummy[ix][mu].d7;
	  df0[ix][mu].d8 += ddummy[ix][mu].d8;
	}
      }
    }
  }
  /* send the data to the neighbour on the right is not needed*/  

  /* end of ifdef PARALLELXYT */
#  endif
  return;
#endif
}

static char const rcsid[] = "$Id$";