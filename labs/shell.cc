#include "labs/shell.h"
#include "labs/vgatext.h"

/*
README
Commands supported:
1. parityx - Printes even or odd depending on parity of x. Blocks shell untill completion.
2. long - Prints debug output large number of times. Blocks shell untill completion.
3. longc - Coroutine implementation of long. 
4. longf - Fibre implementation of long.
5. factx - Prints factorial x % 1000000007. x may overflow. Scheduler schedules this task. Contains explicit calls to stack_saverestore.
4. fibx - Prints fibonacci x % 1000000007. x may overflow. Scheduler schedules this task. Does not contain explicit calls to stack_saverestore.
*/

//
// initialize shellstate
//
void shell_init(shellstate_t& state) {
	state.key_presses = 0;
	state.curr_row = 2;
	state.curr_col = 10;
	state.compute = false;
	state.start_row = 2;
	for(int i = 0; i < 25; i++) {
		for(int j = 0; j < 80; j++) {
			state.is_text[i][j] = false;
			state.c[i][j] = ' ';
		}
	}
	char temp[] = "Key presses: 0";
	char prompt[] = "hoh-user$ ";
	for(int i = 0; i < 14; i++) {
		state.c[0][i] = temp[i];
		state.is_text[0][i] = true;
	}
	for(int i = 0; i < 10; i++) {
		state.c[2][i] = prompt[i];
		state.is_text[2][i] = true;
	}
	state.long_status = 0; // Dead initially
	state.fibre_status = 0;
	state.schedule_status[0] = 0;
	state.schedule_status[1] = 0;
	state.schedule_status[2] = 0;
	state.schedule_status[3] = 0;
	state.schedule_status[4] = 0;
	state.schedule_pointer = 0;
	state.schedule_type[0] = -1;
	state.schedule_type[1] = -1;
	state.schedule_type[2] = -1;
	state.schedule_type[3] = -1;
	state.schedule_type[4] = -1;
}

//
// handle keyboard event.
// key is in scancode format.
// For ex:
// scancode for following keys are:
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     | esc |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 |  0 |  - |  = |back|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 01  | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0a | 0b | 0c | 0d | 0e |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     | tab |  q |  w |  e |  r |  t |  y |  u |  i |  o |  p |  [ |  ] |entr|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 0f  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1a | 1b | 1c |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | keys     |ctrl |  a |  s |  d |  f |  g |  h |  j |  k |  l |  ; |  ' |    |shft|
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//      | scancode | 1d  | 1e | 1f | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 2a |
//      +----------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+
//
// so and so..
//
// - restrict yourself to: 0-9, a-z, esc, enter, arrows
// - ignore other keys like shift, control keys
// - only handle the keys which you're interested in
// - for example, you may want to handle up(0x48),down(0x50) arrow keys for menu.
//
void shell_update(uint8_t scankey, shellstate_t& stateinout) {
		hoh_debug("Got: "<< unsigned(scankey));
		stateinout.key_presses++;
		char temp[50];
		int temp2 = stateinout.key_presses;
		int counter = 0;
		while(temp2 != 0) {
			temp[counter++] = (temp2%10)+'0';
			temp2 /= 10;
		}
		for(int i = counter-1; i >= 0; i--) {
			stateinout.c[0][13+counter-i-1] = temp[i];
			stateinout.is_text[0][13+counter-i-1] = true;
		}

		// if(stateinout.long_status > 0) return; // Executing a coroutine, dont take further input for now
		// if(stateinout.fibre_status > 0) return ;
		

		char first[] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
		char second[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'};
		char third[] = {'z', 'x', 'c', 'v', 'b', 'n', 'm'};
		char entered = '/';
		if(scankey == 0x1) { // esc
			if(stateinout.start_row != stateinout.curr_row or stateinout.curr_col != 10){
				// stateinout.is_text[stateinout.curr_row][stateinout.curr_col] = true;
				if(stateinout.curr_col == 0){
					stateinout.curr_row--;
					stateinout.curr_col = 79;
					stateinout.is_text[stateinout.curr_row][stateinout.curr_col] = false;
				}
				else{
					stateinout.curr_col--;
					stateinout.is_text[stateinout.curr_row][stateinout.curr_col] = false;
				}
			}
		}
		else if(scankey == 0x1c) { //enter
			stateinout.compute = true;
			// stateinout.is_text[stateinout.curr_row][stateinout.curr_col] = true;
		}
		else if(scankey >= 2 && scankey <=10) {
			entered = scankey-1+'0';
		}
		else if(scankey == 11) {
			entered = '0';
		}
		else if(scankey >= 0x10 && scankey <= 0x19) {
			entered = first[scankey-0x10];
		}
		else if(scankey >= 0x1e && scankey <= 0x26) {
			entered = second[scankey-0x1e];
		}
		else if(scankey >= 0x2c && scankey <= 0x32) {
			entered = third[scankey-0x2c];
		}
		if(entered != '/') {
			stateinout.c[stateinout.curr_row][stateinout.curr_col] = entered;
			stateinout.is_text[stateinout.curr_row][stateinout.curr_col] = true;
			if(stateinout.curr_col < 79) {
				stateinout.curr_col++;
			}
			else if(stateinout.curr_row < 24) {
				stateinout.curr_col = 0;
				stateinout.curr_row++;
			} else {
				stateinout.curr_col = 0;
				for(int i = 2; i < 24; i++) {
					for(int j = 0; j < 80; j++) {
						stateinout.c[i][j] = stateinout.c[i+1][j];
						stateinout.is_text[i][j] = stateinout.is_text[i+1][j];
					}
				}
				for(int j = 0; j < 80; j++) {
					stateinout.c[24][j] = ' ';
					stateinout.is_text[24][j] = false;
				}
			}
		}
		
}

void shell_move_down(shellstate_t& stateinout) {
	if(stateinout.curr_row < 24)
		stateinout.start_row = ++stateinout.curr_row;
	else {
		stateinout.curr_col = 0;
		stateinout.start_row = stateinout.curr_row;
		for(int i = 2; i < 24; i++) {
			for(int j = 0; j < 80; j++) {
				stateinout.c[i][j] = stateinout.c[i+1][j];
				stateinout.is_text[i][j] = stateinout.is_text[i+1][j];
			}
		}
		for(int j = 0; j < 80; j++) {
			stateinout.c[24][j] = ' ';
			stateinout.is_text[24][j] = false;
		}
	}
}

void display_prompt(shellstate_t& stateinout, char* prompt, int n) {
	for(int i = 0; i < n; i++) {
		stateinout.c[stateinout.curr_row][i] = prompt[i];
		stateinout.is_text[stateinout.curr_row][i] = true;
	}
}

void shell_step(shellstate_t& stateinout){
	//
	//one way:
	// if a function is enabled in stateinout
	//   call that function( with arguments stored in stateinout) ;
	//stateinout.args[0] = 5;
	//stateinout.args[1] = 5;
	//

	if(stateinout.compute == true) { // Some command was entered
		// Get current command
		char command[2001];
		int counter = 0;
		for(int i = stateinout.start_row; i <= stateinout.curr_row; i++) {
			bool flag = true;
			for(int j = ((i == stateinout.start_row) ? 10 : 0); j < 80; j++) {
				if(stateinout.is_text[i][j] == false) {
					flag = false;
					break;
				}
				command[counter++] = stateinout.c[i][j];
			}
			if(!flag) break;
		}

		if(counter >= 6 && command[0] == 'p' && command[1] == 'a' && command[2] == 'r' && command[3] == 'i' && command[4] == 't' && command[5] == 'y') {
			int num = 0;
			for(int i = 4; i < counter; i++) {
				num = num*10 + command[i]-'0';
			}
			shell_move_down(stateinout);
			hoh_debug(num);
			if(num&1) {
				display_prompt(stateinout, "Odd", 3);
			} else {
				display_prompt(stateinout, "Even", 4);
			}
		}
		else if(counter == 4 && command[0] == 'l' && command[1] == 'o' && command[2] == 'n' && command[3] == 'g') {
			int var = 0;
			for(int i = 0; i < 100000; i++) {
				hoh_debug("Performing Long Computation: Iteration 0x" << i);
			}
		} else if(counter == 5 && command[0] == 'l' && command[1] == 'o' && command[2] == 'n' && command[3] == 'g' && command[4] == 'c') {
			if(stateinout.long_status == 0) {
				stateinout.long_status = 1;
			} else {
				shell_move_down(stateinout);
				char prompt[] = "longc task already running !";
				display_prompt(stateinout, prompt, 28);
			}
		} else if(counter == 5 && command[0] == 'l' && command[1] == 'o' && command[2] == 'n' && command[3] == 'g' && command[4] == 'f') {
			if(stateinout.fibre_status == 0) {
				stateinout.fibre_status = 1;
			} else {
				shell_move_down(stateinout);
				char prompt[] = "longf task already running !";
				display_prompt(stateinout, prompt, 28);
			}
		} else if(counter >= 4 && command[0] == 'f' && command[1] == 'a' && command[2] == 'c' && command[3] == 't') {
			bool valid = true;
			int num = 0;
			for(int i = 4; i < counter; i++) {
				if(command[i] < '0' || command[i] > '9') valid = false;
				num = num*10 + command[i]-'0';
			}
			int count3 = 0;
			int count4 = 0;
			for(int i = 0; i < 5; i++) {
				if(stateinout.schedule_status[i] > 0 and stateinout.schedule_type[i] == 4) {
					count4++;
				}
				if(stateinout.schedule_status[i] > 0 and stateinout.schedule_type[i] == 3) {
					count3++;
				}
			}
			if(count3 < 3 && count3 + count4 < 5 && valid) {
				for(int i = 0; i < 5; i++) {
					if(stateinout.schedule_status[i] == 0) {
						stateinout.schedule_type[i] = 3;
						stateinout.schedule_argument[i] = num;
						stateinout.schedule_status[i] = 1;
						break;
					}
				}
			} else if(!valid) {
				shell_move_down(stateinout);
				char prompt[] = "Invalid Argument !";
				display_prompt(stateinout, prompt, 18);
			} else {
				shell_move_down(stateinout);
				char prompt[] = "Too much buffered !";
				display_prompt(stateinout, prompt, 19);
			}
		} else if(counter >= 3 && command[0] == 'f' && command[1] == 'i' && command[2] == 'b') {
			bool valid = true;
			int num = 0;
			for(int i = 3; i < counter; i++) {
				if(command[i] < '0' || command[i] > '9') valid = false;
				num = num*10 + command[i]-'0';
			}
			int count3 = 0;
			int count4 = 0;
			for(int i=0;i<5;i++){
				if(stateinout.schedule_status[i] > 0 and stateinout.schedule_type[i] == 4){
					count4++;
				}
				if(stateinout.schedule_status[i] > 0 and stateinout.schedule_type[i] == 3){
					count3++;
				}
			}
			if(count4 < 3 && count3 + count4 < 5 && valid) {
				for(int i = 0; i < 5; i++) {
					if(stateinout.schedule_status[i] == 0) {
						stateinout.schedule_type[i] = 4;
						stateinout.schedule_argument[i] = num;
						stateinout.schedule_status[i] = 1;
						break;
					}
				}
			} else if(!valid) {
				shell_move_down(stateinout);
				char prompt[] = "Invalid Argument !";
				display_prompt(stateinout, prompt, 18);
			} else {
				shell_move_down(stateinout);
				char prompt[] = "Too much buffered !";
				display_prompt(stateinout, prompt, 19);
			}
		}

		shell_move_down(stateinout);
		char prompt[] = "hoh-user$ ";
		display_prompt(stateinout, prompt, 10);
		stateinout.curr_col = 10;
		stateinout.compute = false;
	}
}


//
// shellstate --> renderstate
//
void shell_render(const shellstate_t& shell, renderstate_t& render){
	render.key_presses = shell.key_presses;
	render.curr_col = shell.curr_col;
	render.curr_row = shell.curr_row;
	for(int i = 0; i < 25; i++) {
		for(int j = 0; j < 80; j++) {
			render.is_text[i][j] = shell.is_text[i][j];
			render.c[i][j] = shell.c[i][j];
		}
	}
	
	//
	// renderstate. number of keys pressed = shellstate. number of keys pressed
	//
	// renderstate. menu highlighted = shellstate. menu highlighted
	//
	// renderstate. function result = shellstate. output argument
	//
	// etc.
	//
}


//
// compare a and b
//
bool render_eq(const renderstate_t& a, const renderstate_t& b){
	if(a.key_presses != b.key_presses) return false;
	if(a.curr_col != b.curr_col || a.curr_row != b.curr_row) return false;
	for(int i = 0; i < 25; i++) {
		for(int j = 0; j < 80; j++) {
			if(a.c[i][j] != b.c[i][j] || a.is_text[i][j] != b.is_text[i][j]) return false;
		}
	}
	return true;
}


static void fillrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawtext(int x,int y, const char* str, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);
static void drawnumberinhex(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base);


//
//
// helper functions
//
//

static void writecharxy(int x, int y, uint8_t c, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
	vgatext::writechar(y*w+x,c,bg,fg,vgatext_base);
}

static void fillrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
	for(int y=y0;y<y1;y++){
		for(int x=x0;x<x1;x++){
			writecharxy(x,y,0,bg,fg,w,h,vgatext_base);
		}
	}
}

static void drawrect(int x0, int y0, int x1, int y1, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){

	writecharxy(x0,  y0,  0xc9, bg,fg, w,h,vgatext_base);
	writecharxy(x1-1,y0,  0xbb, bg,fg, w,h,vgatext_base);
	writecharxy(x0,  y1-1,0xc8, bg,fg, w,h,vgatext_base);
	writecharxy(x1-1,y1-1,0xbc, bg,fg, w,h,vgatext_base);

	for(int x=x0+1; x+1 < x1; x++){
		writecharxy(x,y0, 0xcd, bg,fg, w,h,vgatext_base);
	}

	for(int x=x0+1; x+1 < x1; x++){
		writecharxy(x,y1-1, 0xcd, bg,fg, w,h,vgatext_base);
	}

	for(int y=y0+1; y+1 < y1; y++){
		writecharxy(x0,y, 0xba, bg,fg, w,h,vgatext_base);
	}

	for(int y=y0+1; y+1 < y1; y++){
		writecharxy(x1-1,y, 0xba, bg,fg, w,h,vgatext_base);
	}
}

static void drawtext(int x,int y, const char* str, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
	for(int i=0;i<maxw;i++){
		writecharxy(x+i,y,str[i],bg,fg,w,h,vgatext_base);
		if(!str[i]){
			break;
		}
	}
}

static void drawnumberinhex(int x,int y, uint32_t number, int maxw, uint8_t bg, uint8_t fg, int w, int h, addr_t vgatext_base){
	enum {max=sizeof(uint32_t)*2+1};
	char a[max];
	for(int i=0;i<max-1;i++){
		a[max-1-i-1]=hex2char(number%16);
		number=number/16;
	}
	a[max-1]='\0';

	drawtext(x,y,a,maxw,bg,fg,w,h,vgatext_base);
}
//
// Given a render state, we need to write it into vgatext buffer
//
void render(const renderstate_t& state, int w, int h, addr_t vgatext_base){
	// Render cursor
	fillrect(state.curr_col, state.curr_row, state.curr_col+1, state.curr_row+1, (uint8_t) 14, (uint8_t) 0x2, w, h, vgatext_base);

	uint8_t bgcolor = 0x0;
	uint8_t fgcolor = 0x7;
	for(int i = 0;i < 25; i++) {
		for(int j = 0;j < 80; j++) {
			if(state.is_text[i][j]) {
				if(i == 0) {
					bgcolor = 0x1;
					fgcolor = 0x2;
				} else {
					bgcolor = 0x0;
					fgcolor = 0x7;
				}
				writecharxy(j, i, (uint8_t) state.c[i][j], bgcolor, fgcolor, w, h, vgatext_base);
			} else if(i == 0){
				fillrect(j, i, j+1, i+1, (uint8_t) 0x1, (uint8_t) 0x2, w, h, vgatext_base);
			} else if(i != state.curr_row or j != state.curr_col){
				fillrect(j, i, j+1, i+1, (uint8_t) 0x0, (uint8_t) 0x2, w, h, vgatext_base);
			}
		}
	}
	
	// fillrect(0, 24, 80, 25, (uint8_t) 0x3, (uint8_t) 0x2, w, h, vgatext_base);
	//drawtext(0, 2, (const char*) to_string(state.key_presses), 10, (uint8_t) 0x3, (uint8_t) 0x4, w, h, vgatext_base);
	
	// this is just an example:
	//
	// Please create your own user interface
	//
	// You may also have simple command line user interface
	// or menu based interface or a combination of both.
	//

}