#pragma once
#include "util/config.h"
#include "util/debug.h"
struct shellstate_t{
    int key_presses; // Number of key presses
	char c[25][80]; // Character at a location
	bool is_text[25][80]; // Is the current location has text
	int curr_row, curr_col; // The location to write the next character
	bool compute; // If the shell has to compute a command
	int start_row; // The row at which the last command started
	int long_status; // 0: Dead, 1: Ready, 2: Running
	int fibre_status;
	int schedule_status[5];
	int schedule_pointer;
	int schedule_type[5];
	int schedule_argument[5];
	unsigned int schedule_return[5];
};

struct renderstate_t{
    int key_presses;
	char c[25][80];
	bool is_text[25][80];
	int curr_row, curr_col;
};

void shell_init(shellstate_t& state);
void shell_update(uint8_t scankey, shellstate_t& stateinout);
void shell_step(shellstate_t& stateinout);
void shell_render(const shellstate_t& shell, renderstate_t& render);

bool render_eq(const renderstate_t& a, const renderstate_t& b);
void render(const renderstate_t& state, int w, int h, addr_t display_base);
