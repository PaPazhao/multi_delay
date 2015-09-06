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

/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/    
/*============================ TYPES =========================================*/
//! \name d_list_ret_t
//!> @{
typedef enum {
    D_LIST_RET_OK,
    D_LIST_RET_FAIL,
    D_LIST_RET_INVALID_PARAMS,
    D_LIST_RET_STOP
}d_list_ret_t;
//!> @}

typedef d_list_ret_t  destroy_node_data_t(void* pvCtx,void* pvData);
typedef d_list_ret_t  d_list_visit_t(void* pvCtx, void* PvData);

//! \name d_list_node_t
//!> @{
typedef struct __d_list_node_t{
    struct __d_list_node_t*  ptPrev;
    struct __d_list_node_t*  ptNext;
    void*                    pvData;
}d_list_node_t;
//!> @}

//! \name d_list_t
//!> @{
typedef struct {
    d_list_node_t*          ptHead;
    destroy_node_data_t*    fnDataDestroy;
    void*                   pvDataDestroyCtx;
}d_list_t;
//!> @}

//! \name fail_ret_node_t
//!> @{
typedef enum{
    FAIL_RET_NULL,
    FAIL_RET_LAST
}fail_ret_node_t;
//!> @}

/*============================ PROTOTYPES ====================================*/
 void* d_list_malloc_mem(uint32_t wSize);
 void d_list_free_mem(void* pvPTR);


static void d_list_destroy_data(d_list_t* ptDlist,void* pvData);
static d_list_node_t* d_list_create_node(d_list_t* ptDlist, void* pvData);
static d_list_node_t* d_list_get_node(d_list_t* ptDlist, size_t hwIndex,fail_ret_node_t tFailRet);

static void d_list_destory_node(d_list_t* ptDlist,d_list_node_t* ptNode);
            
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/
bool d_list_create(d_list_t* ptDlist,destroy_node_data_t* fnDataDestroy,void* pvDataDestroyCtx)
{
    if(NULL == ptDlist) {
        return false;
    }

    ptDlist->ptHead = NULL;
    ptDlist->fnDataDestroy = fnDataDestroy;
    ptDlist->pvDataDestroyCtx = pvDataDestroyCtx;
    
    return true;
}

void d_list_destory(d_list_t* ptDlist)
{
    d_list_node_t* ptNode = NULL;
    d_list_node_t* ptNext = NULL;
    
    if(NULL == ptDlist) {
        return ;
    }
    
    ptNode = ptDlist->ptHead;
    while(NULL != ptNode) {
        ptNext = ptNode->ptNext;
        d_list_destory_node(ptDlist,ptNode);
        ptNode = ptNext;
    }
    
    ptDlist->ptHead = NULL;
    ptDlist->fnDataDestroy = NULL;
    ptDlist->pvDataDestroyCtx = NULL;
}

size_t d_list_length(d_list_t* ptDlist)
{
    d_list_node_t* ptNode = NULL;
    size_t hwLen = 0;
    
    if(NULL == ptDlist) {
        return 0;
    }
    
    ptNode = ptDlist->ptHead;
    
    while(NULL != ptNode) {
        hwLen++;
        ptNode = ptNode->ptNext;
    }
    
    return hwLen;
}

d_list_ret_t d_list_insert(d_list_t* ptDlist, size_t hwIndex, void* pvData)
{
    d_list_node_t* ptNode = NULL;
    d_list_node_t* ptCursor = NULL;
    
    if(NULL == ptDlist) {
        return D_LIST_RET_INVALID_PARAMS;
    }
    
    ptNode = d_list_create_node(ptDlist,pvData);
    if(NULL == ptNode) {
        return D_LIST_RET_FAIL;
    }
    
    if(NULL == ptDlist->ptHead) {
        ptDlist->ptHead = ptNode;
        return D_LIST_RET_OK;
    }
    
    ptCursor = d_list_get_node(ptDlist,hwIndex,FAIL_RET_LAST);
    
    if(hwIndex < d_list_length(ptDlist)) {
        if(ptCursor == ptDlist->ptHead){
            ptDlist->ptHead = ptNode;
        } else {
            ptCursor->ptPrev->ptNext = ptNode;
            ptNode->ptPrev = ptCursor->ptPrev;
        }
        ptNode->ptNext = ptCursor;
        ptCursor->ptPrev = ptNode;
    } else {
        ptCursor->ptNext = ptNode;
        ptNode->ptPrev = ptCursor;
    }
    
    return D_LIST_RET_OK;
}

d_list_ret_t d_list_delete(d_list_t* ptDlist, size_t hwIndex)
{
    d_list_node_t* ptCursor = NULL;
    d_list_ret_t tRet = D_LIST_RET_INVALID_PARAMS;
    
    if(NULL == ptDlist) {
        return tRet;
    }
    
    ptCursor = d_list_get_node(ptDlist,hwIndex,FAIL_RET_NULL);
    
    if(NULL != ptCursor){
        if(ptCursor == ptDlist->ptHead){
            ptDlist->ptHead = ptCursor->ptNext;
        }
    
        if(NULL !=  ptCursor->ptNext){
            ptCursor->ptNext->ptPrev = ptCursor->ptPrev;
        }
    
        if(NULL != ptCursor->ptPrev ){
            ptCursor->ptPrev->ptNext = ptCursor->ptNext;
        }
        
        d_list_destory_node(ptDlist, ptCursor);
    }
    
    return D_LIST_RET_OK;    
}

d_list_ret_t d_list_get_by_index(d_list_t* ptDlist, size_t hwIndex, void** ppvData)
{
    d_list_node_t* ptCursor  = NULL ;
    
    if(NULL == ptDlist){
        return D_LIST_RET_INVALID_PARAMS;
    }
    
    ptCursor = d_list_get_node(ptDlist, hwIndex, FAIL_RET_NULL);
    
    if(NULL != ptCursor ){
        *ppvData = ptCursor->pvData;
    }
    
    return ptCursor != NULL ? D_LIST_RET_OK : D_LIST_RET_FAIL;
}

d_list_ret_t d_list_set_by_index(d_list_t* ptDlist, uint16_t hwIndex, void* pvData)
{
    d_list_node_t* ptCursor  = NULL ;
    
    if(NULL == ptDlist ){
        return D_LIST_RET_INVALID_PARAMS;
    }
    
    ptCursor = d_list_get_node(ptDlist, hwIndex, FAIL_RET_NULL);
    
    if(ptCursor != NULL){
        ptCursor->pvData = pvData;
    }
    
    return ptCursor != NULL ? D_LIST_RET_OK : D_LIST_RET_FAIL;
}

d_list_ret_t d_list_prepend(d_list_t* ptDlist, void* pvData)
{
    return d_list_insert(ptDlist, 0, pvData);
}

d_list_ret_t d_list_append(d_list_t* ptDlist, void* pvData)
{
    return d_list_insert(ptDlist, -1, pvData);
}

d_list_ret_t d_list_foreach(d_list_t* ptDlist,d_list_visit_t* fnVisit,void* pvCtx)
{
    d_list_ret_t tRet = D_LIST_RET_OK;
    
    if((NULL == ptDlist) || (NULL == fnVisit)) {
        return D_LIST_RET_INVALID_PARAMS;
    }
    
    d_list_node_t* ptNode = ptDlist->ptHead;

    while((NULL != ptNode) && (D_LIST_RET_STOP != tRet) ){
        tRet = fnVisit(pvCtx,ptNode->pvData);
        ptNode = ptNode->ptNext;
    }
    
    return tRet;
}

static void d_list_destroy_data(d_list_t* ptDlist,void* pvData)
{
    if(NULL == ptDlist) {
        return ;
    }
    
    if(NULL != ptDlist->fnDataDestroy){
        ptDlist->fnDataDestroy(ptDlist->pvDataDestroyCtx,pvData);
    }
}


/*！\brief creat one double list node
 *! 
 */
static d_list_node_t* d_list_create_node(d_list_t* ptDlist, void* pvData)
{
    d_list_node_t* ptNode = NULL;
    
    if(NULL == ptDlist) {
        return NULL;
    }
    
    ptNode = (d_list_node_t*)d_list_malloc_mem(sizeof(d_list_node_t));
    
    if(NULL != ptNode) {
        ptNode->ptPrev = NULL;
        ptNode->ptNext = NULL;
        ptNode->pvData = pvData;
    }
    
    return ptNode;
}

static void d_list_destory_node(d_list_t* ptDlist,d_list_node_t* ptNode)
{
    if((NULL == ptDlist) || (NULL == ptNode)) {
        return ;
    }
    
    ptNode->ptNext = NULL;
    ptNode->ptPrev = NULL;
    d_list_destroy_data(ptDlist,ptNode->pvData);
    d_list_free_mem(ptNode);
}

static d_list_node_t* d_list_get_node(d_list_t* ptDlist, size_t hwIndex,fail_ret_node_t tFailRet)
{
    d_list_node_t* ptNode = NULL;
    
    if(NULL == ptDlist) {
        return NULL;
    }
    
    ptNode = ptDlist->ptHead;
    
    while((NULL != ptNode) && (NULL != ptNode->ptNext) && (hwIndex > 0)) {
        ptNode = ptNode->ptNext;
        hwIndex--;
    }
    
    if(FAIL_RET_NULL == tFailRet) {
        ptNode = hwIndex > 0 ? NULL : ptNode;
    }
    
    return ptNode;
}



#include <stdlib.h>

static void* d_list_malloc_mem(uint32_t wSize)
{
    return malloc(wSize);
}

static void d_list_free_mem(void* pvPTR)
{
    free(pvPTR);
}

/* EOF */
