/* debug.cpp - Debug console
 * Copyright (C) 2000 - 2007 Sam Steele
 *
 * This file is part of DreamZZT.
 *
 * DreamZZT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * DreamZZT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdarg.h>
#include <string.h>
#include <Tiki/tiki.h>
#include <Tiki/hid.h>
#include <Tiki/eventcollector.h>
#include <Tiki/thread.h>
#include <Tiki/tikitime.h>

using namespace Tiki;
using namespace Tiki::GL;
using namespace Tiki::Hid;
using namespace Tiki::Thread;

#include "debug.h"
#include "word.h"
#include "board.h"
#include "status.h"
#include "sound.h"
#include "window.h"
#include "version.h"

extern struct board_info_node *currentbrd;
extern struct board_info_node *board_list;
extern struct world_header world;
extern int switchbrd;
extern ZZTMusicStream *zm;
#if defined(USE_SDL)
extern SDL_Surface *zzt_font;
#elif defined(USE_OPENGL)
extern Texture *zzt_font;
#endif

#define SCREEN_X 640
#if defined(USE_SDL)
#define SCREEN_Y 400
#elif TIKI_PLAT == TIKI_DC
#define SCREEN_Y 424
#else
#define SCREEN_Y 480
#endif

Console *dt=NULL;
extern Console *ct;
extern Console *st;

int debug_hidCookie=-1;
extern Player *player;
extern EventCollector *playerEventCollector;
extern bool gameFrozen;
int debug_visible = 0;
std::string debug_cmdline;
enum debugSelectMode_t { NONE, WATCH, INSPECT} debugSelectMode = NONE;
int debugselect_x=0, debugselect_y=0;
bool debug_show_objects=false;
TUITextInput *debug_input;
Tiki::Thread::Thread *debug_thread;
bool debug_quitting;

void debug_hidCallback(const Event & evt, void * data);

void *process_debug(void *) {
#if TIKI_PLAT != TIKI_NDS
	while(!debug_quitting) {
		debug_cmdline = "";
		debug("");

		debug_hidCookie = Hid::callbackReg(debug_hidCallback, NULL);

		while(!debug_quitting && (debug_cmdline.length() == 0 || (debug_cmdline[debug_cmdline.length() - 1] != '\r'))) {
			Time::sleep(1000);
			if(debugSelectMode != NONE) {
				ct->locate(debugselect_x, debugselect_y);
				ct->color(WHITE | HIGH_INTENSITY, BLACK);
				ct->printf("X");
			}
			if(debug_cmdline == "`")
				debug_cmdline = "";
			if(debug_visible) {
				*dt << "\x1b[s"; // Save cursor position
				dt->locate(0,24);
				debug_input->draw(dt);
				dt->setANSI(true);
				*dt << "\x1b[u"; // Restore cursor position
				debug_input->update();
			}
		} //Wait for enter

		Hid::callbackUnreg(debug_hidCookie);

		if(debug_quitting)
			return 0;

		debug_cmdline.resize(debug_cmdline.length() - 1);

		if(debugSelectMode != NONE) {
			if(debugSelectMode == WATCH) {
				if(currentbrd->board[debugselect_x][debugselect_y].obj->prog() != "") {
					((ZZTOOP *)(currentbrd->board[debugselect_x][debugselect_y].obj))->watch();
					debug("Now watching %s\n",((ZZTOOP *)(currentbrd->board[debugselect_x][debugselect_y].obj))->get_zztobj_name().c_str());
				} else {
					debug("Not an object.");
				}
			} else if(debugSelectMode == INSPECT) {
				if(currentbrd->board[debugselect_x][debugselect_y].obj->prog() != "") {
					dt->setANSI(false);
					TUIWindow t(((ZZTOOP *)(currentbrd->board[debugselect_x][debugselect_y].obj))->get_zztobj_name());
					t.buildFromString(currentbrd->board[debugselect_x][debugselect_y].obj->prog());
					t.doMenu();
					dt->setANSI(true);
					dt->color(GREY, BLACK);
					dt->clear();
					debug("");
				} else {
					debug("Not an object. (%s)", currentbrd->board[debugselect_x][debugselect_y].obj->name().c_str());
				}
			}

			debugSelectMode = NONE;
			gameFrozen = false;

			continue;
		}

		*dt << ">>> " << debug_cmdline.c_str() << endl;
		if(debug_cmdline == "+dark") {
			currentbrd->dark = 1;
			debug("Board is now dark.\n");
		} else if(debug_cmdline == "-dark") {
			currentbrd->dark = 0;
			debug("Board is no longer dark.\n");
		} else if(debug_cmdline == "+ammo") {
			give_ammo(10);
			debug("Increased ammo.\n");
		} else if(debug_cmdline == "-ammo") {
			take_ammo(10);
			debug("Decreased ammo.\n");
		} else if(debug_cmdline == "+gems") {
			give_gems(10);
			debug("Increased gems.\n");
		} else if(debug_cmdline == "-gems") {
			take_gems(10);
			debug("Decreased gems.\n");
		} else if(debug_cmdline == "+score") {
			give_score(10);
			debug("Increased score.\n");
		} else if(debug_cmdline == "-score") {
			take_score(10);
			debug("Decreased score.\n");
		} else if(debug_cmdline == "+health") {
			give_health(10);
			debug("Increased health.\n");
		} else if(debug_cmdline == "-health") {
			take_health(10);
			debug("Decreased health.\n");
		} else if(debug_cmdline == "+torch") {
			give_torch(10);
			debug("Increased torches.\n");
		} else if(debug_cmdline == "-torch") {
			take_torch(10);
			debug("Decreased torches\n");
		} else if(debug_cmdline == "zap") {
			if(player==NULL)
				continue;
			if(player->position().x - 1 >= 0)
				remove_from_board(currentbrd,currentbrd->board[(int)player->position().x-1][(int)player->position().y].obj);
			if(player->position().y - 1 >= 0)
				remove_from_board(currentbrd,currentbrd->board[(int)player->position().x][(int)player->position().y-1].obj);
			if(player->position().x + 1 < BOARD_X)
				remove_from_board(currentbrd,currentbrd->board[(int)player->position().x+1][(int)player->position().y].obj);
			if(player->position().y + 1 < BOARD_Y)
				remove_from_board(currentbrd,currentbrd->board[(int)player->position().x][(int)player->position().y+1].obj);
			debug("Cleared area around player.\n");
		} else if(debug_cmdline.find("keys") == 0) {
			for(int i=0; i<7; i++) {
				world.keys[i] = 1;
			}
			draw_keys();
			debug("Player now has all keys.\n");
		} else if(debug_cmdline.find("warp ") == 0) {
			if(player==NULL)
				continue;
			switchbrd = atoi(debug_cmdline.c_str() + 5);
			player->setFlag(F_SLEEPING);
		} else if(debug_cmdline == "freeze") {
			gameFrozen = true;
			debug("Game has been frozen\n");
		} else if(debug_cmdline == "unfreeze") {
			gameFrozen = false;
			debug("Game has been thawed\n");
		} else if(debug_cmdline == "watch") {
			gameFrozen = true;
			debug("Select an object to watch.\n");
			debugSelectMode = WATCH;
			debugselect_x = 31;
			debugselect_y = 13;
		} else if(debug_cmdline == "inspect") {
			gameFrozen = true;
			debug("Select an object to inspect.\n");
			debugSelectMode = INSPECT;
			debugselect_x = 31;
			debugselect_y = 13;
		} else if(debug_cmdline == "edit") {
			switchbrd=-5;
		} else if(debug_cmdline == "flags") {
			for(int i=0; i<((world.magic == MAGIC_SZT)?16:10); i++) {
				debug("%i: %s\n",i,world.flags[i].c_str());
			}
		} else if(debug_cmdline == "warp") {
			gameFrozen = true;
			int x=0;
			char tmp[100];
			struct board_info_node *current=board_list;
			std::string boardmenu = "";
			while(current!=NULL) {
				sprintf(tmp,"!%i;%s\r",x,current->title);
				boardmenu += tmp;
				x++;
				current=current->next;
			}
			dt->setANSI(false);
			TUIWindow t("Select a board");
			t.buildFromString(boardmenu);
			t.doMenu();
			dt->setANSI(true);
			dt->color(GREY, BLACK);
			dt->clear();
			if(atoi(t.getLabel().c_str()) > 0) {
				debug("Warping to board %i\n",atoi(t.getLabel().c_str()));
				switchbrd = atoi(t.getLabel().c_str());
				player->setFlag(F_SLEEPING);
			} else {
				debug("");
			}
			gameFrozen = false;
		} else if(debug_cmdline == "quit") {
			switchbrd = -2;
		} else if(debug_cmdline.find("play ") == 0) {
			if(zm!=NULL)
				zm->setTune(debug_cmdline.c_str() + 5);
			if(zm!=NULL)
				zm->start();
		} else if(debug_cmdline == "+reveal") {
			debug_show_objects=true;
			debug("Revealing objects.\n");
		} else if(debug_cmdline == "-reveal") {
			debug_show_objects=false;
			debug("Returning objects to normal.\n");
		} else if(debug_cmdline.find("viewport ") == 0) {
			std::vector<std::string> args = wordify(debug_cmdline,' ');
			delete ct;
			ct = new Console(atoi(args[1].c_str()), atoi(args[2].c_str()), zzt_font);
			ct->setSize(30 * 16, SCREEN_Y / 2);
			ct->translate(Vector(30 * 8, SCREEN_Y / 2 * 3,0));
			draw_board();
		} else {
			if(player==NULL)
				continue;
			player->exec(debug_cmdline);
		}

		if(zm!=NULL && !zm->isPlaying()) {
			if(zm!=NULL)
				zm->setTune("i-g");
			if(zm!=NULL)
				zm->start();
		}
	}
#endif
	return 0;
}

void debug_hidCallback(const Event & evt, void * data) {
#if TIKI_PLAT != TIKI_NDS
	if (evt.type == Hid::Event::EvtKeyDown) {
		if(debugSelectMode != NONE) {
			if(evt.key == Event::KeyUp) {
				debugselect_y--;
				if(debugselect_y < 0)
					debugselect_y=0;
			} else if(evt.key == Event::KeyDown) {
				debugselect_y++;
				if(debugselect_y >= BOARD_Y)
					debugselect_y=BOARD_Y-1;
			} else if(evt.key == Event::KeyLeft) {
				debugselect_x--;
				if(debugselect_x < 0)
					debugselect_x=0;
			} else if(evt.key == Event::KeyRight) {
				debugselect_x++;
				if(debugselect_x >= BOARD_X)
					debugselect_x=BOARD_X-1;
			} else if(evt.key == 13) {
				debug_cmdline += '\r';
			}
		} else {
			if(evt.key == '`' && world.online == 0) {
				if(debug_visible==0) {
					debug_visible = 1;
					ct->setSize(ct->getSize().x, ct->getSize().y/2);
					dt->setSize(ct->getSize().x, ct->getSize().y);
					dt->setTranslate(Vector(ct->getSize().x/2,ct->getSize().y / 2,0));
					ct->setTranslate(Vector(ct->getSize().x/2,ct->getSize().y / 2 * 3,0));
					debug_input->focus(true);
					debug_cmdline = "";
				} else {
					debug_visible = 0;
					ct->setSize(ct->getSize().x, ct->getSize().y * 2);
					ct->setTranslate(Vector(ct->getSize().x/2,ct->getSize().y / 2,0));
					dt->setTranslate(Vector(1024,360,0));
					if(playerEventCollector != NULL && !playerEventCollector->listening())
						playerEventCollector->start();
					st->color(WHITE|HIGH_INTENSITY, BLUE);
					st->locate(5,23);
					st->setANSI(true);
					*st << "\x1b[k"; // clear EOL
					st->setANSI(false);
					debug_input->focus(false);
				}
			} else if (evt.key >= 32 && evt.key <= 128 && debug_visible && world.online == 0) {
				if(playerEventCollector != NULL && playerEventCollector->listening())
					playerEventCollector->stop();
			} else if (evt.key == 13 && debug_visible && world.online == 0) {
				if(playerEventCollector != NULL && !playerEventCollector->listening())
					playerEventCollector->start();
				debug_cmdline += '\r';
			}
		}
	}

	if(debug_visible && player != NULL) {
		st->color(WHITE|HIGH_INTENSITY, BLUE);
		st->locate(5,23);
		*st << "X: " << (int)player->position().x << " Y: " << (int)player->position().y << "    ";
	}
#endif
}

void debug_init() {
#if TIKI_PLAT != TIKI_NDS
	dt = new Console(60,25,zzt_font);
	dt->setSize(60*8,240);
	dt->setTranslate(Vector(1024,360,0));
	dt->setANSI(true);
	dt->color(GREY, BLACK);
	dt->clear();
	debug("\n\nDreamZZT %s\n(C) 2000 - 2007 Sam Steele\nAll Rights Reserved.\n\nREADY.\n", VERSION);
	debug_input = new TUITextInput("> ", &debug_cmdline);
	debug_input->setBg(BLACK);
	debug_quitting=false;
	debug_thread = new Tiki::Thread::Thread(process_debug,NULL);
#endif
}

void debug_shutdown() {
	Hid::callbackUnreg(debug_hidCookie);
	debug_quitting=true;
	debug_thread->join();
}

void debug(const char *fmt, ...) {
	if(dt==NULL)
		return;
	char txt[1024];
	va_list args;

	va_start(args,fmt);
	vsprintf(txt,fmt,args);
	va_end(args);

	dt->color(YELLOW | HIGH_INTENSITY, BLACK);
	*dt << txt;
	*dt << "\x1b[s"; // Save cursor position
	dt->color(WHITE | HIGH_INTENSITY, BLUE);
	dt->locate(0,0);
	*dt << "                    DreamZZT Debug Console                  ";
	*dt << "\x1b[u"; // Restore cursor position
}
