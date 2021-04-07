/* Copyright (C) 2020 SovietPony
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License ONLY.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CP866_H_INCLUDED
#define CP866_H_INCLUDED

int cp866_isalpha (int ch);
int cp866_isupper (int ch);
int cp866_islower (int ch);
int cp866_toupper (int ch);
int cp866_tolower (int ch);
int cp866_strcasecmp (const char *a, const char *b);
int cp866_strncasecmp (const char *a, const char *b, unsigned int n);

int cp866_ctou (int ch);
int cp866_ctoug (int ch);
int cp866_utoc (int ch);

#endif /* CP866_H_INCLUDED */
