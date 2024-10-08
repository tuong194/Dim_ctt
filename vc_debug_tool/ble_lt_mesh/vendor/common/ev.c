/********************************************************************************************************
 * @file	ev.c
 *
 * @brief	for TLSR chips
 *
 * @author	telink
 * @date	Sep. 30, 2010
 *
 * @par     Copyright (c) 2017, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "ev.h"
#if MI_API_ENABLE
static ev_loop_ctrl_t ev_loop, *loop = &ev_loop;

// will be called in every main loop
void ev_on_poll(ev_poll_e e, ev_poll_callback_t cb){
	loop->poll[e].valid = 1;
	loop->poll[e].cb = cb;
}
/*	
// no need to use unregister by now, use ev_disable_poll instead
void ev_unon_poll(ev_poll_e e){
	loop->poll[e].valid = 0;
}
*/
void ev_enable_poll(ev_poll_e e){
	loop->poll[e].valid = 1;
}

void ev_disable_poll(ev_poll_e e){
	loop->poll[e].valid = 0;
}

/* Process poll */
void ev_poll() {
	int i;
	for(i = 0; i < EV_POLL_MAX; ++i){
		if(loop->poll[i].valid){
#if(__LOG_RT_ENABLE__)
			if(i < TR_T_POLL_E - TR_T_POLL_0){
				LOG_TICK((TR_T_POLL_0 + i), loop->poll[i].cb());
			}else
#endif
			loop->poll[i].cb();
		}
	}
}
void ev_on_event(ev_event_e e, ev_event_t *ev){
	assert(ev->next == 0);	// !! not allow to add different e with the same ev
	LL_PREPEND((loop->events[e]), ev);
}

void ev_unon_event(ev_event_e e, ev_event_t *ev){
	ev_event_t *head = loop->events[e];
	LL_DELETE(head, ev);
	loop->events[e] = head;
	ev->next = 0;			// !! must,  allow to re-add
}

void ev_emit_event(ev_event_e e, void *data){
	STATIC_ASSERT_POW2(EV_FIRED_EVENT_MAX);
	STATIC_ASSERT(EV_FIRED_EVENT_MAX == EV_FIRED_EVENT_MAX_MASK + 1);
	assert(!irq_is_in_handler());

	u32 r = irq_disable();
	
	int c = (loop->fired_index + loop->fired_count) & EV_FIRED_EVENT_MAX_MASK;
	loop->fired_queue[c].e = (int)e;
	loop->fired_queue[c].data = data;
	++loop->fired_count;

	irq_restore(r);
}

static inline void ev_call_callbacks(int e, void *data){
	ev_event_t *ev;
	ev_event_t *head = loop->events[e];
    for(ev = head; ev; ev = ev->next){
		ev->cb(data);
	}
}
void ev_emit_event_syn(ev_event_e e, void *data){
	//u32 r = irq_disable();
#if(__LOG_RT_ENABLE__)
	if(e < TR_T_EVENT_E - TR_T_EVENT_0){
		LOG_TICK(TR_T_EVENT_0 + e, ev_call_callbacks((int)e, data));
	}else
#endif
	ev_call_callbacks((int)e, data);
	//irq_restore(r);
}

void ev_clear_event(void){
	loop->fired_count = 0;
}

void ev_process_event(){
	int i;
	int fired_count = loop->fired_count;	//  cache count, because p->cb will emit new events, will change loop->fired_count
	int end_index = loop->fired_index + fired_count;
	for(i = loop->fired_index; i < end_index; ++i){

		int j = i & EV_FIRED_EVENT_MAX_MASK;

		ev_fired_event_t *fe = &loop->fired_queue[j];
#if(__LOG_RT_ENABLE__)
		if(fe->e < TR_T_EVENT_E - TR_T_EVENT_0){
			LOG_TICK((TR_T_EVENT_0 + fe->e), ev_call_callbacks(fe->e, fe->data));					// may emit new event
		}else
#endif
		ev_call_callbacks(fe->e, fe->data);
	}
	u32 r = irq_disable();
	loop->fired_count -= fired_count;		// loop->fired_count may bring up race condition, if ev_emit_event is called in irq_handler 
	irq_restore(r);
	loop->fired_index = (loop->fired_index + fired_count) & EV_FIRED_EVENT_MAX_MASK;
}

// calculate how much time elapse till the timer fired
static u32 inline ev_cal_timer_distant(u32 t, u32 now){
	if((u32)(now - t) < EV_TIMER_SAFE_MARGIN)
		return 0;
	else
		return (u32)(t - now);
}

// to tell which fired first
// return < 0,  e1 fired first
// return == ,  e1 and e2 fired the same time
// return > 0,  e2 fired first
static int ev_cmp_timer(ev_time_event_t *e1, ev_time_event_t *e2, u32 now){
	return (int)(ev_cal_timer_distant(e1->t, now) - ev_cal_timer_distant(e2->t, now));
}

void ev_reset_timer(ev_time_event_t *e){
	e->t = clock_time() + e->interval;	// becare of overflow
}

void ev_set_timer(ev_time_event_t *e, int t_us){
	assert(t_us < (CLOCK_MAX_US - EV_TIMER_SAFE_MARGIN_US));	// about 143000 in 30M clock, about 2 minutes
	e->interval = t_us * CLOCK_SYS_CLOCK_1US;
	ev_reset_timer(e);
}

static int inline ev_is_timer_expired(ev_time_event_t *e, u32 now){
	return ((u32)(now - e->t) < EV_TIMER_SAFE_MARGIN);
}

int ev_timer_expired(ev_time_event_t *e){
	u32 now = clock_time();
	return ev_is_timer_expired(e, now);
}

static ev_time_event_t *ev_search_nearest_timer()
{
    ev_time_event_t *p_time_evt = loop->timer_head;
    ev_time_event_t *nearest = 0;
	u32 now = clock_time();

    while(p_time_evt) {
		if(!nearest || ev_cmp_timer(p_time_evt, nearest, now) < 0){
			nearest = p_time_evt;
		}
        p_time_evt = p_time_evt->next;
    }
    return nearest;
}

u8 ev_timer_exist(const ev_time_event_t * e){
	ev_time_event_t *p_time_evt = loop->timer_head;
	while(p_time_evt){
	    if (p_time_evt == e)
	        return 1;
	    p_time_evt = p_time_evt->next;
	}
	return 0;
}

#if(__LOG_RT_ENABLE__)
int	ev_ttl_timer_id = 0;			//  for realtime VCD log only
#endif
// !!!  	if the callback return < 0,  delete the timer
// 		if the callback return 0,  keep the original interval
// 		if the callback return > 0,  set new interval
void ev_start_timer(ev_time_event_t * e){
	// Reserve  4 second margin in case some event run too long
	// that is even a task run nearly 4 second, 
	// the timers will be fired correctly after then.
	u32 r = irq_disable();
	
	u32 now = clock_time();

	u32 t = now + e->interval;	// becare of overflow
    
	// add to timer list
	ev_time_event_t * out;
	LL_EXIST(loop->timer_head, e, out);
	if(out){
	    out->t = t;
	}else{
		e->t = t;
		LL_PREPEND(loop->timer_head, e);
	}
	
	// check if e will be the nearest timer
	// "Use sorted list or skiplist to improve performance"
	if(!loop->timer_nearest || ev_cmp_timer(e, loop->timer_nearest, now) < 0){
		loop->timer_nearest = e;
	}
#if(__LOG_RT_ENABLE__)
	// start from 1,  0 is reserved to indicate that has not been added
	if(0 == e->id){
		e->id = (++ev_ttl_timer_id);
		log_data(TR_24_TIMERS_ADDR + e->id, (u32)e);	// to show relation of id and the address
	}
#endif	
	irq_restore(r);
}

void ev_on_timer(ev_time_event_t * e, u32 t_us){
	assert(t_us < (CLOCK_MAX_US - EV_TIMER_SAFE_MARGIN_US));	// about 143000 in 30M clock, about 2 minutes
    e->interval = t_us * CLOCK_SYS_CLOCK_1US;
	ev_start_timer(e);
}

void ev_unon_timer(ev_time_event_t * e){
    LL_DELETE(loop->timer_head, e);
}

/* Process time events */
static void ev_process_timer(){
	u32 now = clock_time();
	#if 0 // clear this code ,and make the timer can refresh more faster 
	if(!loop->timer_nearest || !ev_is_timer_expired(loop->timer_nearest, now))
		return;
	#endif
	ev_time_event_t *p_time_evt = loop->timer_head;
	ev_time_event_t *prev_head = p_time_evt;
	while(p_time_evt){
		if(ev_is_timer_expired(p_time_evt, now)){
			int t =0;

#if(__LOG_RT_ENABLE__)
			if(p_time_evt->id < TR_T_TIMER_E){
				LOG_TICK(p_time_evt->id, t = p_time_evt->cb(p_time_evt->data));
			}else
#endif
            
			if(p_time_evt->mode == MIBLE_TIMER_SINGLE_SHOT){
				ev_unon_timer(p_time_evt);		// delete timer
				p_time_evt->cb(p_time_evt->data);
			}else if (p_time_evt->mode == MIBLE_TIMER_REPEATED){
				p_time_evt->cb(p_time_evt->data);
				p_time_evt->t = now + p_time_evt->interval;	// becare of overflow
			}else{
				p_time_evt->cb(p_time_evt->data);
				p_time_evt->interval = t * CLOCK_SYS_CLOCK_1US;
				p_time_evt->t = now + p_time_evt->interval;	// becare of overflow
			}
			if(prev_head != loop->timer_head){
				// restart the whole from timer_head.  because the head could be changed within the loop
				p_time_evt = loop->timer_head;
				prev_head = p_time_evt;
			}else{
				p_time_evt = p_time_evt->next;
			}
		}else{
			p_time_evt = p_time_evt->next;
		}
	}
	// recalculate the nearest timer
	loop->timer_nearest = ev_search_nearest_timer();
	
}

void ev_main(void){
	ev_process_timer();
	//ev_poll();
	//ev_process_event();
}
#endif 
