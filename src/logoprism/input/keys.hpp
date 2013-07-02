#ifndef __LOGOPRISM_INPUT_KEYS_HPP__
#define __LOGOPRISM_INPUT_KEYS_HPP__

#include <GLFW/glfw3.h>
#include <bitset>

namespace logoprism {
  namespace input {

    /**
     * Mapping to hide GLFW key codes behind some nicer bitset structure to easily match key combinations.
     * Each possible key is mapped to a single bit in the bitset.
     */
    struct keyset : std::bitset< GLFW_KEY_LAST + 1 > {
      typedef std::bitset< GLFW_KEY_LAST + 1 > bitset_type;

      keyset() {}
      keyset(bitset_type const& o) :
        bitset_type(o) {}
      explicit keyset(size_t const value) :
        bitset_type(value) {}

      /** converts to true if any bit is set */
      operator bool() const { return this->any(); }

      /** check whether the given key combination matches exactly the current combination */
      bool operator%(keyset const& o) const { return (((*this) & o) ^ o).none(); }

      /** logical bitwise operators */
      keyset operator|(keyset const& o) const { return static_cast< bitset_type const& >(*this) | o; }
      keyset operator&(keyset const& o) const { return static_cast< bitset_type const& >(*this) & o; }
      keyset operator^(keyset const& o) const { return static_cast< bitset_type const& >(*this) ^ o; }

      static keyset const space;
      static keyset const apostrophe;
      static keyset const comma;
      static keyset const minus;
      static keyset const period;
      static keyset const slash;
      static keyset const number_0;
      static keyset const number_1;
      static keyset const number_2;
      static keyset const number_3;
      static keyset const number_4;
      static keyset const number_5;
      static keyset const number_6;
      static keyset const number_7;
      static keyset const number_8;
      static keyset const number_9;
      static keyset const semicolon;
      static keyset const equal;
      static keyset const letter_a;
      static keyset const letter_b;
      static keyset const letter_c;
      static keyset const letter_d;
      static keyset const letter_e;
      static keyset const letter_f;
      static keyset const letter_g;
      static keyset const letter_h;
      static keyset const letter_i;
      static keyset const letter_j;
      static keyset const letter_k;
      static keyset const letter_l;
      static keyset const letter_m;
      static keyset const letter_n;
      static keyset const letter_o;
      static keyset const letter_p;
      static keyset const letter_q;
      static keyset const letter_r;
      static keyset const letter_s;
      static keyset const letter_t;
      static keyset const letter_u;
      static keyset const letter_v;
      static keyset const letter_w;
      static keyset const letter_x;
      static keyset const letter_y;
      static keyset const letter_z;
      static keyset const left_bracket;
      static keyset const backslash;
      static keyset const right_bracket;
      static keyset const grave_accent;
      static keyset const world_1;
      static keyset const world_2;
      static keyset const escape;
      static keyset const enter;
      static keyset const tab;
      static keyset const backspace;
      static keyset const insert;
      static keyset const suppr;
      static keyset const right;
      static keyset const left;
      static keyset const down;
      static keyset const up;
      static keyset const page_up;
      static keyset const page_down;
      static keyset const home;
      static keyset const end;
      static keyset const caps_lock;
      static keyset const scroll_lock;
      static keyset const num_lock;
      static keyset const print_screen;
      static keyset const pause;
      static keyset const f1;
      static keyset const f2;
      static keyset const f3;
      static keyset const f4;
      static keyset const f5;
      static keyset const f6;
      static keyset const f7;
      static keyset const f8;
      static keyset const f9;
      static keyset const f10;
      static keyset const f11;
      static keyset const f12;
      static keyset const f13;
      static keyset const f14;
      static keyset const f15;
      static keyset const f16;
      static keyset const f17;
      static keyset const f18;
      static keyset const f19;
      static keyset const f20;
      static keyset const f21;
      static keyset const f22;
      static keyset const f23;
      static keyset const f24;
      static keyset const f25;
      static keyset const numpad_0;
      static keyset const numpad_1;
      static keyset const numpad_2;
      static keyset const numpad_3;
      static keyset const numpad_4;
      static keyset const numpad_5;
      static keyset const numpad_6;
      static keyset const numpad_7;
      static keyset const numpad_8;
      static keyset const numpad_9;
      static keyset const numpad_decimal;
      static keyset const numpad_divide;
      static keyset const numpad_multiply;
      static keyset const numpad_substract;
      static keyset const numpad_add;
      static keyset const numpad_enter;
      static keyset const numpad_equal;
      static keyset const left_shift;
      static keyset const left_control;
      static keyset const left_alt;
      static keyset const left_super;
      static keyset const right_shift;
      static keyset const right_control;
      static keyset const right_alt;
      static keyset const right_super;
      static keyset const menu;
    };

  }
}

#endif // ifndef __LOGOPRISM_INPUT_KEYS_HPP__
