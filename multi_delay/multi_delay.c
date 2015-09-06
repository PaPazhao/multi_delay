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
#include ".\d_list\d_list.h"

/*============================ MACROS ========================================*/
#define DELAY_MAX_ITEM_COUNT 3
#ifndef DELAY_MAX_ITEM_COUNT
#   error macro DELAY_MAX_ITEM_COUNT is not define for multi_delay.c
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
typedef uint32_t DelayID_t;
typedef bool timeout_handle_t(void);

//! \name delay_item_t 
//! @{
typedef struct {
    DelayID_t           tID;
    uint32_t            wTime;
    timeout_handle_t *  fnPtr;
}delay_item_t;
//! @}

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
    void            (*Cancel_item)  (DelayID_t tID);
    delay_status_t  (*Is_time_out)  (DelayID_t tID);
END_DEF_INTERFACE (i_delay_t)
//! @}

typedef struct {
    volatile bool       bEnable;
    volatile uint32_t   s_wDelayTimerCounter;
    volatile uint32_t   wTime;
}delay_block_t;

/*============================ LOCAL VARIABLES ===============================*/
static delay_block_t s_tControlBlock = {    .wTime = 0,
                                            .bEnable = false,
                                            .s_wDelayTimerCounter = 0};
DEF_POOL(PoolDelay, delay_item_t,uint8_t,uint8_t);
static POOL_ITEM(PoolDelay) s_tDelayBuff[DELAY_MAX_ITEM_COUNT];
static POOL(PoolDelay)  s_tPoolDelay;
static d_list_t s_tTimeOutList;
static d_list_t s_tDelayList;

/*============================ PROTOTYPES ====================================*/
static void mutidelay_init(void);
static DelayID_t add_delay_item(uint32_t wTime,timeout_handle_t *fnPtr);
static void cancel_delay_item(DelayID_t tID);
static delay_status_t is_delay_time_out(DelayID_t tID);
static d_list_ret_t free_node_data(void* pvCtx,void* pvData);
static d_list_ret_t search_ID(void* pvCtx, void* PvData);
static d_list_ret_t visit_min(void* pvCtx, void* PvData);
static void update_DCB(void);

/*============================ GLOBAL VARIABLES ==============================*/
//! \name delay service interface 
//! @{
const i_delay_t DELAY = {
    .Init           =   &mutidelay_init,
    .Add_item       =   &add_delay_item,
    .Cancel_item    =   &cancel_delay_item,
    .Is_time_out    =   &is_delay_time_out,
};
//! @}

/*============================ IMPLEMENTATION ================================*/
void mult_delay_insert_ms_timer_isr_code(void)
{
    if(s_tControlBlock.bEnable) {
        if(s_tControlBlock.s_wDelayTimerCounter++ >= s_tControlBlock.wTime) {
            do {
                delay_item_t tItem;
                if(D_LIST_RET_OK != d_list_get_by_index(&s_tDelayList,0,(void**)&tItem)) {
                    break;
                }
                if(s_tControlBlock.s_wDelayTimerCounter++ >= s_tControlBlock.wTime) {
                    if(tItem.timeout_handle_t) {
                        tItem.timeout_handle_t();
                    } else {
                        d_list_prepend(&s_tTimeOutList,(void*)&tItem);
                    }
                    d_list_delete(&s_tDelayList,0);
                } else {
                    break;
                }
            } while(1);
            
            update_DCB();
        }
    }
}

#if 0
static d_list_ret_t isr_delay(void* pvCtx, void* PvData)
{
    if(NULL == pvCtx) || (NULL == PvData) {
        return xx;
    }
    
    (*((uint32_t*)pvCtx))++;
    if(PvData.tID >= *((uint32_t*)(pvCtx+1)) ){
        if(PvData.timeout_handle_t) {
            PvData.timeout_handle_t();
        } else {
            d_list_prepend(&s_tTimeOutList, PvData);
        }
        d_list_delete(&s_tDelayList,0);
    } else {
        return D_LIST_STOP;
    }
    
    return DLIST_RET_OK;
}
#endif


static void mutidelay_init(void)
{
    s_tControlBlock.s_wDelayTimerCounter = 0
    s_tControlBlock.bEnable = false;
    s_tControlBlock.wTime = 0;
            
    POOL_INIT(PoolDelay,&s_tPoolDelay,);
    POOL_ADD_HEAP(&s_tPoolDelay,s_tDelayBuff,DELAY_MAX_ITEM_COUNT);

    d_list_create(&s_tDelayList,free_node_data,NULL);
    d_list_create(&s_tTimeOutList,free_node_data,NULL);
}

static DelayID_t add_delay_item(uint32_t wTime,timeout_handle_t *fnPtr)
{
    void* ptCtx[2] = {NULL,NULL};
    uint16_t hwIndex = 0;
    d_list_ret_t tRet;
    delay_item_t *ptDelayItem = NULL;
    
    if(ptDelayItem = (delay_item_t *)POOL_NEW(PoolDelay,&s_tPoolDelay)) {
        ptDelayItem.tID = (DelayID_t)ptDelayItem;
        ptDelayItem.wTime = wTime + s_tControlBlock.s_wDelayTimerCounter;
        ptDelayItem.fnPtr = fnPtr;

        ptCtx[0] = &hwIndex;
        ptCtx[1] = &(ptDelayItem.wTime);

        d_list_foreach(&s_tDelayList,visit_min,ptCtx);
        tRet = d_list_insert(&s_tDelayList, hwIndex, ptDelayItem);
        if(D_LIST_RET_OK != tRet) {
            POOL_FREE(PoolDelay,&s_tPoolDelay,(POOL_ITEM(PoolDelay)*)ptDelayItem);
            return 0;
        }
        update_DCB();
        
        return ptDelayItem.tID;
    }

    return 0;
}

static void cancel_delay_item(DelayID_t tID)
{
    uint16_t hwIndex = 0;
    DelayID_t tIDTemp = tID;
    void* ptCtx[2] = {NULL,NULL};

    if(!tID) {
        return ;
    }

    ptCtx[0] = (void*)&hwIndex;
    ptCtx[1] = (void*)&(tIDTemp);

    if (D_LIST_STOP == d_list_foreach(&s_tTimeOutList,search_ID,ptCtx)) {
        d_list_delete(&s_tTimeOutList,hwIndex);
    } else if (D_LIST_STOP == d_list_foreach(&s_tDelayList,search_ID,ptCtx)) {
        d_list_delete(&s_tDelayList,hwIndex);
        update_DCB();
    }
}

static delay_status_t is_delay_time_out(DelayID_t tID)
{
    uint16_t hwIndex = 0;
    DelayID_t tIDTemp = tID;
    void* ptCtx[2] = {NULL,NULL};

    if(!tID) {
        return DELAY_ERROR;
    }

    ptCtx[0] = (void*)&(hwIndex);
    ptCtx[1] = (void*)&(tIDTemp);

    if (D_LIST_STOP == d_list_foreach(&s_tDelayList,search_ID,ptCtx)) {
        return DELAY_DOWN_COUNTING;
    } else if (D_LIST_STOP == d_list_foreach(&s_tTimeOutList,search_ID,ptCtx)) {
        d_list_delete(&s_tTimeOutList,hwIndex);
        return DELAY_TIME_OUT;
    } else {
        return DELAY_ERROR;  
    }
}

static d_list_ret_t visit_min(void* pvCtx, void* PvData)
{

    if(NULL == pvCtx) || (NULL == PvData) {
        return xx;
    }
    
    if(PvData.wTime <= *((uint32_t*)(pvCtx+1)) ){
        return D_LIST_STOP;
    }
    
    (*((uint32_t*)pvCtx))++;

    return DLIST_RET_OK;
}

static d_list_ret_t search_ID(void* pvCtx, void* PvData)
{
    if(NULL == pvCtx) || (NULL == PvData) {
        return xx;
    }
    
    if(PvData.tID == *((uint32_t*)(pvCtx+1)) ){
        return D_LIST_STOP;
    }
    
    (*((uint32_t*)pvCtx))++;

    return DLIST_RET_OK;
}
 
static void update_DCB(void)
{
    delay_item_t tItem;
    
    if(DLIST_RET_OK == d_list_get_by_index(&s_tDelayList,0,(void**)&tItem)) {
        SAFE_ATOM_CODE(
            s_tControlBlock.s_wDelayTimerCounter = 0
            s_tControlBlock.bEnable = false;
            s_tControlBlock.wTime = 0;
        )
    } else {
        SAFE_ATOM_CODE(
            s_tControlBlock.bEnable = true;
            s_tControlBlock.wTime = tItem.wTime;
        )
    }
}

static d_list_ret_t  free_node_data(void* pvCtx,void* pvData)
{
    delay_item_t* ptItem = pvData;
    if(NULL == ptItem) {
        return D_LIST_RET_FAIL;
    }
    
    ptItem->tID = 0;
    ptItem->wTime = 0;
    ptItem->fnPtr = NULL;
    
    POOL_FREE(PoolDelay,&s_tPoolDelay,(POOL_ITEM(PoolDelay)*)pvData);
    
    return D_LIST_RET_OK;
}
