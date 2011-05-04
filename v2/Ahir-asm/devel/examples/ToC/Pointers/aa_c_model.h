#include <Aa2C.h>
typedef struct increment_State__
{
  uint_32 incr_a;
  uint_32 incr_b;
  uint_32 incr_temp;
  unsigned int _assign_line_12_entry:1;
  unsigned int _assign_line_12_in_progress:1;
  unsigned int _assign_line_12_exit:1;
    uint_32 (*incr_t1);
  unsigned int _assign_line_13_entry:1;
  unsigned int _assign_line_13_in_progress:1;
  unsigned int _assign_line_13_exit:1;
  unsigned int _assign_line_14_entry:1;
  unsigned int _assign_line_14_in_progress:1;
  unsigned int _assign_line_14_exit:1;
  unsigned int _assign_line_15_entry:1;
  unsigned int _assign_line_15_in_progress:1;
  unsigned int _assign_line_15_exit:1;
  unsigned increment_entry:1;
  unsigned increment_in_progress:1;
  unsigned increment_exit:1;
} increment_State;
increment_State *increment_ (increment_State *);
int increment (uint_32, uint_32 *);
typedef struct passpointer_State__
{
  uint_32 a;
  uint_32 b;
  unsigned int _assign_line_23_entry:1;
  unsigned int _assign_line_23_in_progress:1;
  unsigned int _assign_line_23_exit:1;
  unsigned int _assign_line_24_entry:1;
  unsigned int _assign_line_24_in_progress:1;
  unsigned int _assign_line_24_exit:1;
  unsigned int _assign_line_25_entry:1;
  unsigned int _assign_line_25_in_progress:1;
  unsigned int _assign_line_25_exit:1;
    uint_32 (*pu);
  unsigned int _assign_line_26_entry:1;
  unsigned int _assign_line_26_in_progress:1;
  unsigned int _assign_line_26_exit:1;
    uint_32 (*pv);
  unsigned int _assign_line_27_entry:1;
  unsigned int _assign_line_27_in_progress:1;
  unsigned int _assign_line_27_exit:1;
  uint_32 t;
  unsigned int _assign_line_28_entry:1;
  unsigned int _assign_line_28_in_progress:1;
  unsigned int _assign_line_28_exit:1;
  uint_32 s;
  unsigned int _assign_line_29_entry:1;
  unsigned int _assign_line_29_in_progress:1;
  unsigned int _assign_line_29_exit:1;
  uint_32 t1;
  unsigned int _assign_line_30_entry:1;
  unsigned int _assign_line_30_in_progress:1;
  unsigned int _assign_line_30_exit:1;
  uint_32 q;
  unsigned int _call_line_31_entry:1;
  unsigned int _call_line_31_in_progress:1;
  unsigned int _call_line_31_exit:1;
  increment_State *_call_line_31_called_fn_struct;
  unsigned passpointer_entry:1;
  unsigned passpointer_in_progress:1;
  unsigned passpointer_exit:1;
} passpointer_State;
passpointer_State *passpointer_ (passpointer_State *);
int passpointer (uint_32, uint_32 *);