/* console_sdl.cpp - SDL version of Tiki's console object
 * Copyright (C) 2000 - 2007 Sam Steele
 * Portions (C) Atani Software
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

#include <Tiki/tiki.h>
#include "console.h"
#include <stdarg.h>

using namespace Tiki::GL;
using namespace Tiki::GL::Plxcompat;

ConsoleText::ConsoleText(int cols, int rows, SDL_Surface * font) {
	m_font = font;
	m_rows = rows;
	m_cols = cols;

	m_palette[0] = Color(0,0,0);
	m_palette[1] = Color(0,0,0.5);
	m_palette[2] = Color(0,0.5,0);
	m_palette[3] = Color(0,0.5,0.5);
	m_palette[4] = Color(0.5,0,0);
	m_palette[5] = Color(0.5,0,0.5);
	m_palette[6] = Color(0.5,0.5,0);
	m_palette[7] = Color(0.5,0.5,0.5);
		
	m_charData.resize(rows*cols, 0);
	m_colorData.resize(rows*cols, 0);
	
	m_cursor_x = 0;
	m_cursor_y = 0;
	
	m_autoScroll = true;
	m_autoWrap = true;
	m_ansi = false;
	m_autoRefresh = false;
	
	color(GREY,BLACK);
	clear();
}

ConsoleText::~ConsoleText() {
}

void ConsoleText::setSize(float w, float h) {
	m_w = w;
	m_h = h;
}
Tiki::Math::Vector ConsoleText::getSize() const {
	return Tiki::Math::Vector(m_w, m_h, 0.0f);
}

Color ConsoleText::getConsoleColor(const int colorNumber) const {
	Color color = m_palette[colorNumber%8];
	if(colorNumber > 7) {
		color += Color(0.25f, 0.25f, 0.25f);
	}
	return color;
}

void ConsoleText::clear() {
	int x;
	
	for(x=0; x<m_rows*m_cols; x++) {
		m_charData[x] = 0;
		m_colorData[x] = m_attr;
	}
	
	m_cursor_x = 0;
	m_cursor_y = 0;
}

void ConsoleText::scroll(int rows, int top, int left, int bottom, int right) {
	assert(top >= 0 && left >= 0 && bottom < m_rows && right < m_cols);
	
	for(int y=top; y<=bottom; y++) {
		for(int x=left; x<=right; x++) {
			if(y + rows < m_rows && y + rows >= 0) {
				m_charData[(y * m_cols) + x] = m_charData[((y + rows) * m_cols) + x];
				m_colorData[(y * m_cols) + x] = m_colorData[((y + rows) * m_cols) + x];
			} else {
				m_charData[(y * m_cols) + x] = 0;
				m_colorData[(y * m_cols) + x] = m_attr;
			}
		}
	}
}

void ConsoleText::scroll(int rows) {
	scroll(rows,0,0,m_rows-1,m_cols-1);
}

void ConsoleText::locate(int x, int y) {
	m_cursor_x = x;
	m_cursor_y = y;
}

void ConsoleText::color(int fg, int bg) {
	m_attr = 0x0000;
	if(fg > 7 && fg <= 16) { //Old style high intensity
		m_attr |= HIGH_INTENSITY;
		fg -= 8;
	}
	m_attr |= fg;
	m_attr |= (bg << 8);
}

void ConsoleText::printf(const char *fmt, ...) {
	char buf[1024];
	va_list args;
	
	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	va_end(args);

	for(size_t i=0; i<strlen(buf); i++) {
		if(m_cursor_x >= m_cols && m_autoWrap) {
			m_cursor_y++;
			m_cursor_x = 0;
		}
		
		if(m_cursor_y >= m_rows && m_autoScroll) {
			scroll(1);
			m_cursor_y = m_rows - 1;
		}

		if(ansiptr) {
			if(ansiptr > 50) {
				ansiptr = 0;
			}
			ansistr[ansiptr++] = buf[i];
			ansistr[ansiptr] = 0;
			if(!isdigit(buf[i]) && ((buf[i] != '[') && (buf[i] != ';'))) {
				processAnsiString(); 
			}
		} else if(buf[i] == '\x1b' && m_ansi) {
			ansistr[0] = buf[i];
			ansiptr = 1;
			ansistr[ansiptr] = 0;
		} else {
			if(buf[i] == '\n') {
				m_cursor_y++;
				m_cursor_x=0;
				continue;
			} else if(buf[i] == '\r') {
				m_cursor_x = 0;
				continue;
			} else if(buf[i] == '\b') {
				m_cursor_x--;
				if(m_cursor_x < 0) m_cursor_x = 0;
				continue;
			} /*else if(buf[i] == 12) { // funky old style clearscreen
				clear();
				continue;
			}*/
			
			assert(m_cursor_x >= 0 && m_cursor_y >= 0 && m_cursor_x < m_cols && m_cursor_y < m_rows);
			
			m_charData[(m_cursor_y * m_cols) + m_cursor_x] = buf[i];
			m_colorData[(m_cursor_y * m_cols) + m_cursor_x] = m_attr;
			
			m_cursor_x++;
		}			
	}
}

void putpixel(SDL_Surface *screen, int x, int y, int color)
{
	if(x<640 && y < 400) {
		unsigned int *ptr = (unsigned int*)screen->pixels;
		int lineoffset = y * (screen->pitch / 4);
		ptr[lineoffset + x] = color;
	}
}

void ConsoleText::draw(SDL_Surface *screen) {
	int x=0,y=0;
	float x_step=(m_w / m_cols);
	float y_step=(m_h / m_rows);
	Color fg,bg;
	Vector pos = getPosition() - Vector(m_w/2, m_h/2, 0);
	SDL_Rect r;
	r.w = x_step;
	r.h = y_step;

	for(y=0; y<m_rows; y++) {
		for(x=0; x<m_cols; x++) {
			if(m_colorData[y*(m_cols) + x] & HIGH_INTENSITY) {
				fg = Color(0.25, 0.25, 0.25);
			} else {
				fg = Color(0,0,0);
			}
			fg += m_palette[m_colorData[y*(m_cols) + x] & 0x07];
			bg = m_palette[(m_colorData[y*(m_cols) + x] >> 8) & 0x07];
			int u = (m_charData[y*(m_cols) + x] % 16) * 8;
			int v = (m_charData[y*(m_cols) + x] / 16) * 8;
			int fgc = SDL_MapRGB(screen->format, fg.r * 255, fg.g * 255, fg.b * 255);
			int bgc = SDL_MapRGB(screen->format, bg.r * 255, bg.g * 255, bg.b * 255);
			r.x = x*x_step + pos.x;
			r.y = y*y_step + pos.y;
			SDL_FillRect(screen,&r,bgc);
			
			for(int j=0; j<y_step; j++) {
				for(int i=0; i<x_step; i++) {
					if(((unsigned int*)m_font->pixels)[int((j/(y_step / 8))+v)*(m_font->pitch / 4) + i + u]) {
						putpixel(screen, x*x_step + i + pos.x, y*y_step + j + pos.y, fgc);
					}
				}
			}
			/*if(list == Trans) { //Characters!
				renderCharacter(x*x_step, y*y_step, x_step, y_step, m_charData[y*(m_cols) + x], m_colorData[y*(m_cols) + x]);
			} else { //Background blocks!
				renderBackground(x*x_step, y*y_step, x_step, y_step, (m_colorData[y*(m_cols) + x] >> 8) & 0x07);
			}*/
		}
	}
}

void ConsoleText::processAnsiString() {
	if(ansistr[1] == '[')
	{
		int args[11], argptr = 0, ptr = 2, tempptr = 0;
		char cmd, temp[11];
		memset(&args, 0, sizeof(int) * 11);
		cmd = ansistr[ansiptr - 1];
		ansistr[ansiptr - 1] = 0;
		while((ansistr[ptr]) && ((argptr < 10) && (tempptr < 10)))
		{
			if(ansistr[ptr] == ';')
			{
				temp[tempptr] = 0; 
				tempptr = 0; 
				args[argptr++] = atoi(temp); 
			}
			else
			{
				temp[tempptr++] = ansistr[ptr]; 
			}
			++ptr;
		}
		if(tempptr && (argptr < 10))
		{
			temp[tempptr] = 0; 
			args[argptr++] = atoi(temp); 
		}
		if((cmd >= 'A') && (cmd <= 'D') && !args[0])
		{
			args[0] = 1;
		}
		switch(cmd)
		{
			case 'f':
			case 'H': // goto XY
			{
				if(argptr == 0)
				{
					m_cursor_x = 0;
					m_cursor_y = 0;
				}
				else
				{
					m_cursor_x = args[1]-1;
					m_cursor_y = args[0]-1;
				}
				if(m_cursor_x < 0) m_cursor_x = 0;
				if(m_cursor_y < 0) m_cursor_y = 0;
				if(m_cursor_x > m_cols - 1) m_cursor_x = m_cols - 1;
				if(m_cursor_y > m_rows - 1) m_cursor_y = m_rows - 1;
				//printf("**** GOTO XY(%u,%u) ****\n", m_cursor_x, m_cursor_y);
			}
				break;
			case 'A': // UP * lines
				m_cursor_y -= args[0];
				if(m_cursor_x < 0) m_cursor_x = 0;
					if(m_cursor_y < 0) m_cursor_y = 0;
						if(m_cursor_x > m_cols - 1) m_cursor_x = m_cols - 1;
							if(m_cursor_y > m_rows - 1) m_cursor_y = m_rows - 1;
								//printf("**** UP %u ****\n",m_cursor_y);
								break;
			case 'B': // DOWN * lines
				m_cursor_y += args[0];
				if(m_cursor_x < 0) m_cursor_x = 0;
					if(m_cursor_y < 0) m_cursor_y = 0;
						if(m_cursor_x > m_cols - 1) m_cursor_x = m_cols - 1;
							if(m_cursor_y > m_rows - 1) m_cursor_y = m_rows - 1;
								//printf("**** DOWN %u ****\n",m_cursor_y);
								break;
			case 'C': // RIGHT * chars
				m_cursor_x += args[0];
				if(m_cursor_x < 0) m_cursor_x = 0;
					if(m_cursor_y < 0) m_cursor_y = 0;
						if(m_cursor_x > m_cols - 1) m_cursor_x = m_cols - 1;
							if(m_cursor_y > m_rows - 1) m_cursor_y = m_rows - 1;
								//printf("**** RIGHT %u ****\n",m_cursor_y);
								break;
			case 'D': // LEFT * chars
				m_cursor_x -= args[0];
				if(m_cursor_x < 0) m_cursor_x = 0;
					if(m_cursor_y < 0) m_cursor_y = 0;
						if(m_cursor_x > m_cols - 1) m_cursor_x = m_cols - 1;
							if(m_cursor_y > m_rows - 1) m_cursor_y = m_rows - 1;
								//printf("**** LEFT %u ****\n",m_cursor_y);
								break;
			case 'J': // clearscreen
				clear();
				//printf("**** CLEAR SCREEN ****\n");
				break;
			case 's': // save location
				m_save_x = m_cursor_x;
				m_save_y = m_cursor_y;
				//printf("**** SAVE XY (%u, %u) ****\n", save_x, save_y);
				break;
			case 'u': // restore location
				m_cursor_x = m_save_x;
				m_cursor_y = m_save_y;
				m_save_x = m_save_y = 0;
				//printf("**** RESTORE XY (%u, %u) ****\n", m_cursor_x, m_cursor_y);
				break;
			case 'n': // ANSI detect
				//remoteHost->write("\x1b[1;1R", 6);
				//printf("**** ANSI DETECTION ****\n");
				break;
			case 'k':
			case 'K': // clear EOL
			{
				int x = m_cursor_x;
				while(x < m_cols - 1) { 
					m_charData[(m_cursor_y * m_cols) + x] = 0;
					m_colorData[(m_cursor_y * m_cols) + x++] = m_attr;
				} 
				//printf("**** CLEAR EOL %u ****\n", m_cursor_y);
			}
				break;
			case 'm': // set color attribute, currently ignored
			{
				//printf("**** SET ATTRIBUTE [%u]", argptr);
				int i;
				for(i = 0; i < argptr; i++)
				{
					//printf(" %u", args[i]);
					switch(args[i])
					{
						case 0:
							m_attr = 0x0007; // all normal
							break;
						case 1:
							m_attr |= 0x1000; // bold (high intensity)
							break;
						case 4:
							m_attr |= 0x2000; // underscore
							break;
						case 5:
							m_attr |= 0x4000; // blink
							break;
						case 7:
							m_attr |= 0x8000; // reverse video
							break;
						case 8:
							m_attr |= 0xA000; // invisible
							break;
						case 30: // black fg
							m_attr &= 0xFF00;
							m_attr |= BLACK;
							break;
						case 31: // red fg
							m_attr &= 0xFF00;
							m_attr |= RED;
							break;
						case 32: // green fg
							m_attr &= 0xFF00;
							m_attr |= GREEN;
							break;
						case 33: // yellow fg
							m_attr &= 0xFF00;
							m_attr |= YELLOW;
							break;
						case 34: // blue fg
							m_attr &= 0xFF00;
							m_attr |= BLUE;
							break;
						case 35: // magenta fg
							m_attr &= 0xFF00;
							m_attr |= MAGENTA;
							break;
						case 36: // cyan fg
							m_attr &= 0xFF00;
							m_attr |= CYAN;
							break;
						case 37: // white fg
							m_attr &= 0xFF00;
							m_attr |= GREY;
							break;
						case 40: // black bg
							m_attr &= 0xF0FF;
							m_attr |= BLACK << 8;
							break;
						case 41: // red bg
							m_attr &= 0xF0FF;
							m_attr |= RED << 8;
							break;
						case 42: // green bg
							m_attr &= 0xF0FF;
							m_attr |= GREEN << 8;
							break;
						case 43: // yellow bg
							m_attr &= 0xF0FF;
							m_attr |= YELLOW << 8;
							break;
						case 44: // blue bg
							m_attr &= 0xF0FF;
							m_attr |= BLUE << 8;
							break;
						case 45: // magenta fg
							m_attr &= 0xF0FF;
							m_attr |= MAGENTA << 8;
							break;
						case 46: // cyan bg
							m_attr &= 0xF0FF;
							m_attr |= CYAN << 8;
							break;
						case 47: // white bg
							m_attr &= 0xF0FF;
							m_attr |= GREY << 8;
							break;
					}
				}
				//printf(" ****\n");
			}
				break;
		}
	}
	//	else
	//	{
	//		printf("Bogus ANSI: %s\n", ansistr);
	//	}
	ansiptr = 0;
}
