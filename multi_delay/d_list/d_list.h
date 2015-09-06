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

#ifndef __CONSOLE_H__
#define __CONSOLE_H__
      
/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
//! \name d_list_ret_t
//!< @{
typedef enum {
    D_LIST_RET_OK,
    D_LIST_RET_FAIL,
    D_LIST_RET_INVALID_PARAMS
}d_list_ret_t;
//!< @}

typedef d_list_ret_t  destroy_node_data_t(void* pvCtx,void* pvData);
typedef d_list_ret_t  d_list_visit_t(void* pvCtx, void* PvData);

//! \name d_list_node_t
//!> @{
typedef struct __d_list_node_t{
    struct __d_list_node_t*  ptPrev;
    struct __d_list_node_t*  ptNext;
    void* pvData;
}d_list_node_t;
//!> @}

//! \name d_list_t
//!< @{
EXTERN_CLASS(d_list_t)    
    d_list_node_t*          ptHead;
    destroy_node_data_t*    fnDataDestroy;
    void*                   pvDataDestroyCtx;
END_EXTERN_CLASS(d_list_t) 
//!< @}

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
bool d_list_create(d_list_t* ptDlist,destroy_node_data_t* fnDataDestroy,void* pvDataDestroyCtx);
void d_list_destory(d_list_t* ptDlist);
d_list_ret_t d_list_insert(d_list_t* ptDlist, size_t hwIndex, void* pvData);
d_list_ret_t d_list_delete(d_list_t* ptDlist, size_t hwIndex);
d_list_ret_t d_list_get_by_index(d_list_t* ptDlist, size_t hwIndex, void** ppvData);
d_list_ret_t d_list_set_by_index(d_list_t* ptDlist, size_t hwIndex, void* pvData);
d_list_ret_t d_list_foreach(d_list_t* ptDlist,d_list_visit_t* fnVisit,void* pvCtx);

d_list_ret_t d_list_prepend(d_list_t* ptDlist, void* pvData);
d_list_ret_t d_list_append(d_list_t* ptDlist, void* pvData);

size_t d_list_length(d_list_t* ptDlist);

#endif
/* EOF */
