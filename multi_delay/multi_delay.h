/***************************************************************************
 *   Copyright(C)2009-2013 by zhao_li <lizhao15431230@qq.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __USE_MULTI_DELAY_H__
#define __USE_MULTI_DELAY_H__


/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
typedef bool timeout_handle_t(void);
typedef uint32_t DelayID_t;

/*============================ TYPES =========================================*/
//! \name delay_status_t 
//! @{
typedef enum {
    DELAY_DOWN_COUNTING,
    DELAY_TIME_OUT,
    DELAY_ERROR
}delay_status_t;
//! @}

//! \name delay service interface
//! @{
DEF_INTERFACE (i_delay_t)
    void            (*Init)         (void); 
    DelayID_t       (*Add_item)     (uint32_t wTime,timeout_handle_t *fnPtr);
    void            (*Cancel_item)  (DelayID_t wID);
    delay_status_t  (*Is_time_out)  (DelayID_t wID);    
END_DEF_INTERFACE (i_delay_t)
//! @}

/*============================ GLOBAL VARIABLES ==============================*/
extern const i_delay_t  DELAY;

/*============================ PROTOTYPES ====================================*/
extern void mult_delay_insert_ms_timer_isr_code(void);

#endif
