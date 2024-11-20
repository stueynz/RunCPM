/* z80_debugger_variables.c: routines to expose Z80 registers to the debugger
   Copyright (c) 2016 Philip Kendall

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   E-mail: philip-fuse@shadowmagic.org.uk

*/

/*
#include "config.h"

#include "libspectrum.h"

#include "debugger/debugger.h"
#include "z80.h"
#include "z80_internals.h"
#include "z80_macros.h"
*/

#include "../globals.h"
#include "internals.h"

extern int32 PCX; /* external view of PC                          */
extern int32 AF;  /* AF register                                  */
extern int32 BC;  /* BC register                                  */
extern int32 DE;  /* DE register                                  */
extern int32 HL;  /* HL register                                  */
extern int32 IX;  /* IX register                                  */
extern int32 IY;  /* IY register                                  */
extern int32 PC;  /* program counter                              */
extern int32 SP;  /* SP register                                  */
extern int32 AF1; /* alternate AF register                        */
extern int32 BC1; /* alternate BC register                        */
extern int32 DE1; /* alternate DE register                        */
extern int32 HL1; /* alternate HL register                        */
extern int32 IFF; /* Interrupt Flip Flop                          */
extern int32 IR;  /* Interrupt (upper) / Refresh (lower) register */

static const char * const debugger_type_string = "z80";

static const char * const a_detail_string = "a";
static const char * const b_detail_string = "b";
static const char * const c_detail_string = "c";
static const char * const d_detail_string = "d";
static const char * const e_detail_string = "e";
static const char * const f_detail_string = "f";
static const char * const h_detail_string = "h";
static const char * const l_detail_string = "l";

static const char * const a__detail_string = "a'";
static const char * const b__detail_string = "b'";
static const char * const c__detail_string = "c'";
static const char * const d__detail_string = "d'";
static const char * const e__detail_string = "e'";
static const char * const f__detail_string = "f'";
static const char * const h__detail_string = "h'";
static const char * const l__detail_string = "l'";

static const char * const af_detail_string = "af";
static const char * const bc_detail_string = "bc";
static const char * const de_detail_string = "de";
static const char * const hl_detail_string = "hl";

static const char * const af__detail_string = "af'";
static const char * const bc__detail_string = "bc'";
static const char * const de__detail_string = "de'";
static const char * const hl__detail_string = "hl'";

static const char * const sp_detail_string = "sp";
static const char * const pc_detail_string = "pc";
static const char * const ix_detail_string = "ix";
static const char * const iy_detail_string = "iy";

static const char * const i_detail_string = "i";
static const char * const r_detail_string = "r";

static const char * const iff_detail_string = "iff";

/*
static const char * const memptr_detail_string = "memptr";
static const char * const wz_detail_string = "wz"; // Synonym for MEMPTR

static const char * const im_detail_string = "im";
static const char * const iff2_detail_string = "iff2";

static const char * const q_detail_string = "q";
*/

#define DEBUGGER_CALLBACKS(reg) \
  static uint32 get_##reg( void ) { return reg;  } \
  static void set_##reg( uint32 value ) { reg = value; }


#define DEBUGGER_CALLBACKS_HI(pair, reg) \
  static uint32 get_##reg( void ) { return HIGH_REGISTER(pair);  } \
  static void set_##reg( uint32 value ) { SET_HIGH_REGISTER(pair, value); }


#define DEBUGGER_CALLBACKS_LO(pair, reg) \
  static uint32 get_##reg( void ) { return LOW_REGISTER(pair); } \
  static void set_##reg( uint32 value ) { SET_LOW_REGISTER(pair, value); }


DEBUGGER_CALLBACKS_HI(AF, A)
DEBUGGER_CALLBACKS_LO(AF, F)
DEBUGGER_CALLBACKS_HI(BC, B)
DEBUGGER_CALLBACKS_LO(BC, C)
DEBUGGER_CALLBACKS_HI(DE, D)
DEBUGGER_CALLBACKS_LO(DE, E)
DEBUGGER_CALLBACKS_HI(HL, H)
DEBUGGER_CALLBACKS_LO(HL, L)

DEBUGGER_CALLBACKS_HI(AF1, A_)
DEBUGGER_CALLBACKS_LO(AF1, F_)
DEBUGGER_CALLBACKS_HI(BC1, B_)
DEBUGGER_CALLBACKS_LO(BC1, C_)
DEBUGGER_CALLBACKS_HI(DE1, D_)
DEBUGGER_CALLBACKS_LO(DE1, E_)
DEBUGGER_CALLBACKS_HI(HL1, H_)
DEBUGGER_CALLBACKS_LO(HL1, L_)


DEBUGGER_CALLBACKS(AF)
DEBUGGER_CALLBACKS(BC)
DEBUGGER_CALLBACKS(DE)
DEBUGGER_CALLBACKS(HL)

DEBUGGER_CALLBACKS(AF1)
DEBUGGER_CALLBACKS(BC1)
DEBUGGER_CALLBACKS(DE1)
DEBUGGER_CALLBACKS(HL1)

DEBUGGER_CALLBACKS(SP)
DEBUGGER_CALLBACKS(PC)
DEBUGGER_CALLBACKS(IX)
DEBUGGER_CALLBACKS(IY)
DEBUGGER_CALLBACKS(IFF)

DEBUGGER_CALLBACKS_HI(IR, I)
DEBUGGER_CALLBACKS_LO(IR, R)


void
z80_debugger_variables_init( void )
{
  debugger_system_variable_register( debugger_type_string, a_detail_string,
                                     get_A, set_A );
  debugger_system_variable_register( debugger_type_string, b_detail_string,
                                     get_B, set_B );
  debugger_system_variable_register( debugger_type_string, c_detail_string,
                                     get_C, set_C );
  debugger_system_variable_register( debugger_type_string, d_detail_string, 
                                     get_D, set_D );
  debugger_system_variable_register( debugger_type_string, e_detail_string, 
                                     get_E, set_E );
  debugger_system_variable_register( debugger_type_string, f_detail_string, 
                                     get_F, set_F );
  debugger_system_variable_register( debugger_type_string, h_detail_string, 
                                     get_H, set_H );
  debugger_system_variable_register( debugger_type_string, l_detail_string, 
                                     get_L, set_L );
  
  debugger_system_variable_register( debugger_type_string, a__detail_string, 
                                     get_A_, set_A_ );
  debugger_system_variable_register( debugger_type_string, b__detail_string, 
                                     get_B_, set_B_ );
  debugger_system_variable_register( debugger_type_string, c__detail_string, 
                                     get_C_, set_C_ );
  debugger_system_variable_register( debugger_type_string, d__detail_string, 
                                     get_D_, set_D_ );
  debugger_system_variable_register( debugger_type_string, e__detail_string, 
                                     get_E_, set_E_ );
  debugger_system_variable_register( debugger_type_string, f__detail_string, 
                                     get_F_, set_F_ );
  debugger_system_variable_register( debugger_type_string, h__detail_string, 
                                     get_H_, set_H_ );
  debugger_system_variable_register( debugger_type_string, l__detail_string, 
                                     get_L_, set_L_ );
  
  debugger_system_variable_register( debugger_type_string, af_detail_string, 
                                     get_AF, set_AF );
  debugger_system_variable_register( debugger_type_string, bc_detail_string, 
                                     get_BC, set_BC );
  debugger_system_variable_register( debugger_type_string, de_detail_string, 
                                     get_DE, set_DE );
  debugger_system_variable_register( debugger_type_string, hl_detail_string, 
                                     get_HL, set_HL );

  debugger_system_variable_register( debugger_type_string, af__detail_string, 
                                     get_AF1, set_AF1 );
  debugger_system_variable_register( debugger_type_string, bc__detail_string, 
                                     get_BC1, set_BC1 );
  debugger_system_variable_register( debugger_type_string, de__detail_string, 
                                     get_DE1, set_DE1 );
  debugger_system_variable_register( debugger_type_string, hl__detail_string, 
                                     get_HL1, set_HL1 );

  debugger_system_variable_register( debugger_type_string, sp_detail_string, 
                                     get_SP, set_SP );
  debugger_system_variable_register( debugger_type_string, pc_detail_string, 
                                     get_PC, set_PC );
  debugger_system_variable_register( debugger_type_string, ix_detail_string, 
                                     get_IX, set_IX );
  debugger_system_variable_register( debugger_type_string, iy_detail_string, 
                                     get_IY, set_IY );
  
  debugger_system_variable_register( debugger_type_string, i_detail_string, 
                                     get_I, set_I );
  debugger_system_variable_register( debugger_type_string, r_detail_string, 
                                     get_R, set_R );

  debugger_system_variable_register( debugger_type_string, iff_detail_string, 
                                     get_IFF, set_IFF );

/*
  debugger_system_variable_register( debugger_type_string, memptr_detail_string,
		  		     get_memptr, set_memptr );

  WZ is a synonym for MEMPTR
  debugger_system_variable_register( debugger_type_string, wz_detail_string,
		  		     get_memptr, set_memptr );

  debugger_system_variable_register( debugger_type_string, im_detail_string, 
                                     get_IM, set_IM );
  debugger_system_variable_register( debugger_type_string, iff2_detail_string, 
                                     get_IFF2, set_IFF2 );

  debugger_system_variable_register( debugger_type_string, q_detail_string, 
                                     get_Q, set_Q );
*/
}
