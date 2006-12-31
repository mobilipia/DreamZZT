/* http.h - HTTP file downloader
 * Copyright (c) 2000-2006 Sam Steele
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

bool http_get_file(std::string filename, std::string URL);
std::string http_post_file(std::string filename, std::string contentType, std::string URL);
std::string http_get_string(std::string URL);
char *strtolower(char *str);

#define DZZTNET_HOST std::string("http://forums.c99.org")
#define DZZTNET_HOME std::string("/extensions/DreamZZT/dzztnet.php")